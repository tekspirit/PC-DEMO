#include "blockchain.h"

////global var
//device
uint32 g_devicenum[2];//设备个数.0-重节点,1-轻节点
uint32 g_devicerange;//设备坐标范围
uint32 g_devicestep;//设备步进值
//dag
//common
CRITICAL_SECTION g_cs;
device_t *g_device;//设备数组
mainchain_t g_mainchain;//主链
volatile uint32 g_index;//临时用来统计交易号码的(以后会用hash_t代替,计数从1开始)
volatile uint8 g_task;//传递给线程的过程标记
volatile uint8 *g_init;//每个线程的初始化任务.0-未初始化,1-已初始化
volatile uint8 *g_flag;//每个线程的当前任务标记
CString g_string[9]={"none","device_initial","device_connect","device_merge","device_optimize","device_indexdag","device_walk","dag_initial","dag_tangle"};

//主链线程
uint32 WINAPI thread_mainchain(PVOID pParam)
{
	mainchain_t *mainchain;

	mainchain=(mainchain_t *)pParam;
	while(1)
	{
		EnterCriticalSection(&g_cs);
		process_mainchain(mainchain);
		//printf("initial thread_mainchain pass\r\n");
		LeaveCriticalSection(&g_cs);
	}

	return 0;
}

//设备线程
uint32 WINAPI thread_device(PVOID pParam)
{
	device_t *device;

	device=(device_t *)pParam;
	//SetTimer(NULL,device->device_index,TIMER_CONNECT*1000,NULL);
	while(1)
	{
		EnterCriticalSection(&g_cs);
		process_device(device);
		//printf("%d",device->step);
		//printf("%s thread_device%ld pass\r\n",g_string[g_task],device->device_index);
		//g_flag[device->device_index]=TASK_NONE;
		LeaveCriticalSection(&g_cs);
	}

	return 0;
}

void main(int argc,char* argv[])
{
	uint32 account_num;//账户个数
	uint32 money;//初始化资金
	//
	uint32 i;
	uint8 flag;
	int8 buf[1000];
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
	account_num=atol(&buf[12]);//account_num
	fgets(buf,1000,file);
	buf[strlen(buf)-1]=0;
	money=atol(&buf[6]);//money
	fclose(file);
	//0.initial device/timer/thread_device/thread_mainchain/cs
	InitializeCriticalSection(&g_cs);
	srand((unsigned)time(NULL));/*
	g_index=0;
	g_task=TASK_DEVICE_INITIAL;
	g_init=new uint8[g_devicenum];
	memset((void *)g_init,0,g_devicenum*sizeof(uint8));
	g_flag=new uint8[g_devicenum];
	memset((void *)g_flag,TASK_DEVICE_INITIAL,g_devicenum*sizeof(uint8));*/
	g_device=new device_t[g_devicenum[0]+g_devicenum[1]];
	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
	{
		EnterCriticalSection(&g_cs);
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
		LeaveCriticalSection(&g_cs);


		//g_device[i].line=DEVICE_LINE_ON;
		//g_device[i].status=STATUS_FREE;
		//g_device[i].step=STEP_CONNECT;
		//g_device[i].dag_index=0;
		//memset((void *)g_device[i].buffer,0,BUFFER_LENGTH*sizeof(uint8));
		/*
		memset((void *)g_device[i].queue,0,QUEUE_LENGTH*sizeof(queue_t));//INFO_TX
		g_device[i].queue_index=0;
		g_device[i].tangle_index=0;
		g_device[i].transaction_index=0;
		g_device[i].key_index=0;
		g_device[i].account_id=rand()%account_num;
		g_device[i].account_money=money;*/
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
	g_mainchain.list_number=0;
	g_mainchain.list=NULL;
	thread_handle=CreateThread(NULL,0,thread_mainchain,(PVOID)&g_mainchain,0,&thread_id);
	if (!thread_handle)
	{
		printf("initial thread_mainchain failed\r\n");
		return;
	}
	//msg loop
	while(1)
	{
#if 1
		EnterCriticalSection(&g_cs);
		flag=0;
		if (g_mainchain.queue && g_mainchain.queue->step==STEP_CONNECT)
			flag=1;
		for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
			if (g_device[i].queue && g_device[i].queue->step==STEP_CONNECT)
			{
				flag=1;
				break;
			}
		if (!flag)
			printf("ok");
		/*
		for (i=0;i<g_devicenum;i++)
			printf("%d",g_device[i].step);
		printf("\r\n");
		*/
		LeaveCriticalSection(&g_cs);
#endif
	}
	//release
	/*
	for (i=0;i<g_devicenum;i++)
		device_release(&g_device[i]);
	delete[] (uint8 *)g_init;
	delete[] (uint8 *)g_flag;
	delete[] g_device;*/
	DeleteCriticalSection(&g_cs);
}
