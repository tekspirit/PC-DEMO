// BlockchainMfcDoc.cpp : implementation of the CBlockchainMfcDoc class
//

#include "stdafx.h"
#include "BlockchainMfc.h"

#include "BlockchainMfcDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcDoc

IMPLEMENT_DYNCREATE(CBlockchainMfcDoc, CDocument)

BEGIN_MESSAGE_MAP(CBlockchainMfcDoc, CDocument)
	//{{AFX_MSG_MAP(CBlockchainMfcDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//global var
uint32 g_devicenum[2];//设备个数.0-重节点,1-轻节点
uint32 g_devicerange;//设备坐标范围
uint32 g_devicestep;//设备步进值
uint32 g_number;//交易原子列表个数
deal_t *g_deal;//交易原子列表
CRITICAL_SECTION g_cs;
device_t *g_device;//设备数组
mainchain_t g_mainchain;//主链
volatile uint32 g_index;//临时用来统计交易号码的(以后会用hash_t代替,计数从1开始)
volatile uint8 g_flag;//使用timer计时重发

//主链线程
uint32 WINAPI thread_mainchain(PVOID pParam)
{
	mainchain_t *mainchain;

	mainchain=(mainchain_t *)pParam;
	while(1)
	{
		EnterCriticalSection(&g_cs);
		process_mainchain(mainchain);
		LeaveCriticalSection(&g_cs);
	}

	return 0;
}

//设备线程
uint32 WINAPI thread_device(PVOID pParam)
{
	device_t *device;

	device=(device_t *)pParam;
	while(1)
	{
		EnterCriticalSection(&g_cs);
		process_device(device);
		LeaveCriticalSection(&g_cs);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcDoc construction/destruction
CBlockchainMfcDoc::CBlockchainMfcDoc()
{
	uint32 account;//账户个数
	uint32 token;//初始化资金
	//
	uint32 i;
	//MSG msg;
	int8 buf[1000],*point[2];
	FILE *file;
	HANDLE thread_handle;
	uint32 thread_id;
	queue_t *queue;

	//read initial.ini
	file=fopen("initial.ini","r");
	fgets(buf,1000,file);//[network]
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	g_devicenum[0]=atol(&buf[13]);//g_devicenum[0]
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	g_devicenum[1]=atol(&buf[13]);//g_devicenum[1]
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	g_devicerange=atol(&buf[13]);//g_devicerange
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	g_devicestep=atol(&buf[12]);//g_devicestep
	fgets(buf,1000,file);//[blockchain]
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	account=atol(&buf[8]);//account
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	token=atol(&buf[6]);//token
	fgets(buf,1000,file);//[transaction]
	i=ftell(file);
	g_number=0;
	while(1)
	{
		if (feof(file))
			break;
		memset(buf,0,1000*sizeof(int8));
		fgets(buf,1000,file);
		if (!strcmp(buf,""))
			break;
		g_number++;
	}
	g_deal=new deal_t[g_number];
	fseek(file,i,SEEK_SET);
	g_number=0;
	while(1)
	{
		if (feof(file))
			break;
		memset(buf,0,1000*sizeof(int8));
		fgets(buf,1000,file);
		if (!strcmp(buf,""))
			break;
		buf[strlen(buf)-1]=0;
		point[0]=buf;
		point[1]=strchr(point[0],',');
		*point[1]='\0';
		g_deal[g_number].device_index[0]=atol(point[0]);
		point[0]=point[1]+1;
		point[1]=strchr(point[0],',');
		*point[1]='\0';
		g_deal[g_number].device_index[1]=atol(point[0]);
		point[0]=point[1]+1;
		g_deal[g_number].token=atol(point[0]);
		g_number++;
	}
	fclose(file);
	//0.initial device/timer/thread_device/thread_mainchain/cs
	InitializeCriticalSection(&g_cs);
	srand((unsigned)time(NULL));
	g_index=0;
	g_flag=0;
	g_device=new device_t[g_devicenum[0]+g_devicenum[1]];
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		g_device[i].x=rand()%g_devicerange;
		g_device[i].y=rand()%g_devicerange;
		g_device[i].node=i<g_devicenum[0] ? NODE_HEAVY : NODE_LIGHT;//rand()%2 ? NODE_LIGHT : NODE_HEAVY;
		g_device[i].device_index=i;
		g_device[i].route=NULL;
		g_device[i].queue=NULL;
		queue=new queue_t;
		queue->step=STEP_CONNECT;
		queue->data=new uint8[1*sizeof(uint32)];
		*(uint32 *)queue->data=0;//align problem?
		queue_insert(&g_device[i],queue);
		key_generate(&g_device[i]);
		g_device[i].token[0]=token;
		g_device[i].token[1]=0;
		g_device[i].dag=NULL;
	}
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		//lpThreadAttributes:指向security attributes
		//dwStackSize:栈大小
		//lpStartAddress:线程函数。定义形式必须uint32 WINAPI xxx(PVOID pParam)
		//lpParameter:参数区
		//dwCreationFlags:线程状态(suspend,running)
		//lpThreadId:返回线程id
		thread_handle=CreateThread(NULL,0,thread_device,(PVOID)&g_device[i],0,&thread_id);
		if (!thread_handle)
		{
			printf("initial thread_device failed\r\n");
			return;
		}
	}
	g_mainchain.queue=NULL;
	g_mainchain.dag_number=0;
	g_mainchain.list_number=g_devicenum[0]+g_devicenum[1];
	g_mainchain.list=new list_t[g_mainchain.list_number];
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		g_mainchain.list[i].device_index=i;
		g_mainchain.list[i].token=token;
		g_mainchain.list[i].node=NODE_NONE;
	}
	g_mainchain.dag=NULL;
	thread_handle=CreateThread(NULL,0,thread_mainchain,(PVOID)&g_mainchain,0,&thread_id);
	if (!thread_handle)
	{
		printf("initial thread_mainchain failed\r\n");
		return;
	}
	SetTimer(NULL,1,TIMER_CONNECT*1000,NULL);//原则上时钟线程应该是每个设备线程配备一个,这里简化为共用一个
	/*
	//msg loop
	while(1)
	{
		GetMessage(&msg,NULL,0,0);
		EnterCriticalSection(&g_cs);
		g_flag=!g_flag;
		for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
			move_location(&g_device[i],g_devicestep,g_devicerange);
		LeaveCriticalSection(&g_cs);
	}
	//release
	//for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	//	device_release(&g_device[i]);
	//delete[] g_device;
	delete[] g_deal;
	DeleteCriticalSection(&g_cs);*/
}

CBlockchainMfcDoc::~CBlockchainMfcDoc()
{
}

BOOL CBlockchainMfcDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcDoc serialization

void CBlockchainMfcDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcDoc diagnostics

#ifdef _DEBUG
void CBlockchainMfcDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBlockchainMfcDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBlockchainMfcDoc commands
