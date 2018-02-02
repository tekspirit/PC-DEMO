// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "BlockchainMfc.h"

#include "MainFrm.h"

//include
#include "ConnectView.h"
#include "TransactionView.h"
#include "CommandView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
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
//
volatile uint8 g_run;//0-停止,1-运行
uint8 **g_check;//[g_number][5].0-source,1-mc(src),2-mc(dst),3-node(src),4-node(dst)

//主链线程
uint32 WINAPI thread_mainchain(PVOID pParam)
{
	mainchain_t *mainchain;

	mainchain=(mainchain_t *)pParam;
	while(1)
	{
		delay();
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
		delay();
		EnterCriticalSection(&g_cs);
		process_device(device);
		LeaveCriticalSection(&g_cs);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction
CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	uint32 account;//账户个数
	uint32 token;//初始化资金
	//
	uint32 i;
	int8 buf[1000],*point[2];
	FILE *file;
	HANDLE thread_handle;
	uint32 thread_id;
	queue_t *queue;

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
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
	//add for mfc
	g_run=0;
	g_check=new uint8*[g_number];
	for (i=0;i<g_number;i++)
	{
		g_check[i]=new uint8[5];
		memset(g_check[i],0,5);
	}
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
			//printf("initial thread_device failed\r\n");
			return -1;
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
		//printf("initial thread_mainchain failed\r\n");
		return -1;
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	CSize screen;
	BOOL result;

	screen.cx=GetSystemMetrics(SM_CXFULLSCREEN);//去掉了桌面任务栏
	screen.cy=GetSystemMetrics(SM_CYFULLSCREEN);//去掉了桌面任务栏
	m_scrollsize[0].cx=screen.cx/5;m_scrollsize[0].cy=screen.cy-20;
	m_scrollsize[1].cx=screen.cx/2;m_scrollsize[1].cy=screen.cy-20;
	m_scrollsize[2].cx=screen.cx*3/10;m_scrollsize[2].cy=screen.cy-20;
	result=m_wndSplitter.CreateStatic(this,1,3);
    result|=m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CConnectView),m_scrollsize[0],pContext);
	result|=m_wndSplitter.CreateView(0,1,RUNTIME_CLASS(CTransactionView),m_scrollsize[1],pContext);
	result|=m_wndSplitter.CreateView(0,2,RUNTIME_CLASS(CCommandView),m_scrollsize[2],pContext);
	
	return result;
}