#include "layer_mainchain.h"

//STEP_CONNECT
void connect_recv(mainchain_t *mainchain)
{
	//queue->list
	uint32 i,j,k;
	uint32 dag_index;
	uint32 *dag;
	uint8 flag;
	queue_t *queue,*remove,*prev;
	index_t index;
	list_t *list;

	flag=0;
	while(1)
	{
		queue=mainchain->queue;
		while(queue)
		{
			if (queue->step==STEP_CONNECT)
				break;
			queue=queue->next;
		}
		if (!queue)
			break;
		flag=1;
		//find latest/first queue
		index.number=*(uint32 *)queue->data;
		index.index=(uint32 *)(queue->data+1*sizeof(uint32));
		index.key=queue->data+(1+index.number)*sizeof(uint32);
		index.node=queue->data+(1+index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN);
		//compute list_number
		dag_index=mainchain->dag_number;
		for (j=0;j<index.number;j++)
		{
			for (i=0;i<mainchain->list_number;i++)
				if (mainchain->list[i].device_index==index.index[j])
					break;
			if (i!=mainchain->list_number)
			{
				dag_index=mainchain->list[i].dag_index;
				break;
			}
		}
		k=index.number;
		for (j=0;j<mainchain->list_number;j++)
		{
			for (i=0;i<index.number;i++)
				if (index.index[i]==mainchain->list[j].device_index)
					break;
			if (i==index.number && dag_index!=mainchain->list[j].dag_index)//other device_index & different dag_index
				k++;
		}
		//fill list
		list=new list_t[k];
		for (i=0;i<index.number;i++)
		{
			list[i].dag_index=dag_index;
			list[i].device_index=index.index[i];
			memcpy(list[i].key.e,&index.key[i*(KEY_E+KEY_LEN)],KEY_E);
			memcpy(list[i].key.n,&index.key[i*(KEY_E+KEY_LEN)+KEY_E],KEY_LEN);
			list[i].node=index.node[i];
		}
		k=index.number;
		for (j=0;j<mainchain->list_number;j++)
		{
			for (i=0;i<index.number;i++)
				if (index.index[i]==mainchain->list[j].device_index)
					break;
			if (i==index.number && dag_index!=mainchain->list[j].dag_index)//other device_index & different dag_index
			{
				list[k].dag_index=mainchain->list[j].dag_index;
				list[k].device_index=mainchain->list[j].device_index;
				memcpy(list[k].key.e,&mainchain->list[j].key.e,KEY_E);
				memcpy(list[k].key.n,&mainchain->list[j].key.n,KEY_LEN);
				list[k].node=mainchain->list[j].node;
				k++;
			}
		}
		list_delete(mainchain);
		mainchain->list=new list_t[k];
		memcpy(mainchain->list,list,k*sizeof(list_t));
		delete[] list;
		mainchain->list_number=k;
		//compute dag_number
		mainchain->dag_number=0;
		for (i=0;i<mainchain->list_number;i++)
		{
			for (j=0;j<mainchain->dag_number;j++)
				if (mainchain->list[i].dag_index==dag[j])
					break;
			if (j==mainchain->dag_number)
			{
				dag=j ? (uint32 *)realloc(dag,(j+1)*sizeof(uint32)) : (uint32 *)malloc((j+1)*sizeof(uint32));
				dag[j]=j;
				mainchain->dag_number++;
			}
		}
		if (mainchain->dag_number)
			free(dag);
		//delete all queues with same device_index
		remove=queue;
		while(remove)
		{
			if (remove->step==STEP_CONNECT && *(uint32 *)(remove->data+1*sizeof(uint32))==index.index[0])
			{
				if (remove==mainchain->queue)
				{
					mainchain->queue=remove->next;
					if (remove->data)
					{
						delete[] remove->data;
						remove->data=NULL;
					}
					delete remove;
					remove=mainchain->queue;
				}
				else
				{
					prev->next=remove->next;
					if (remove->data)
					{
						delete[] remove->data;
						remove->data=NULL;
					}
					delete remove;
					remove=prev->next;
				}
			}
			else
			{
				prev=remove;
				remove=remove->next;
			}
		}
	}
	//update queue
	if (flag)
	{
		queue=new queue_t;
		queue->step=STEP_TRANSACTION;
		queue->data=NULL;
		queue_insert(mainchain,queue);
	}
}

//STEP_TRANSACTION
void transaction_recv(mainchain_t *mainchain)
{
}

void process_mainchain(mainchain_t *mainchain)
{
	if (!mainchain->queue)
		return;
	switch(mainchain->queue->step)
	{
	case STEP_CONNECT:
		//recv
		connect_recv(mainchain);//recv queue & process mainchain's list
		break;
	case STEP_TRANSACTION:
		//recv
		transaction_recv(mainchain);//recv queue & process mainchain's
		break;
	}
}

#if 0
extern uint32 g_devicenum;//设备个数
extern uint32 g_devicerange;//设备坐标范围
extern uint32 g_devicestep;//设备步进值
extern CRITICAL_SECTION g_cs;
extern device_t *g_device;//设备数组
extern volatile uint8 g_task;//传递给线程的过程标记
extern volatile uint8 *g_init;//每个线程的初始化任务.0-未初始化,1-已初始化

uint8 process_mainchain(void)
{
	uint32 i,j;
	route_t *route;
	uint32 device_index;

	switch(g_mainchain.queue[0].)
	{
	case TASK_DEVICE_INITIAL:
		break;
	//update device's property:0-device visible(master/slave),1-device invisible(free)
	//1.when is free device:->master then turn to slave;->free' then free' turn to slave;->slave then (connect its master turn to slave,else turn to master)
	//2.when is master device:->free then free turn to slave;->slave not connect;->master connect
	//3.when is slave device:->free/master/slave connect
	case TASK_DEVICE_CONNECT:
		if (!g_init[device->device_index])
		{
			EnterCriticalSection(&g_cs);
			device_index=device_seek(device);
			LeaveCriticalSection(&g_cs);
			g_init[device->device_index]=1;
			if (device_index==device->device_index)//no device visible
				return 1;
		}
		switch(g_device[device->device_index].queue[0].info)
		{
		case INFO_TX:
			route=route_check(device);
			if (!route)
				return 0;
			route_mark(device,route->device_index);
			switch(device->status)
			{
			case DEVICE_STATUS_FREE:
				device->status=g_device[route->device_index].status==DEVICE_STATUS_MASTER ? DEVICE_STATUS_SLAVE : DEVICE_STATUS_MASTER;
				break;
			case DEVICE_STATUS_MASTER:
				break;
			case DEVICE_STATUS_SLAVE:
				break;
			}
			InterlockedExchange((LPLONG)&g_device[device->device_index].queue[0].device_index,device->device_index);
			InterlockedExchange((LPLONG)&g_device[device->device_index].queue[0].info,INFO_TX);
			InterlockedExchange((LPLONG)&g_device[route->device_index].queue[0].device_index,device->device_index);
			InterlockedExchange((LPLONG)&g_device[route->device_index].queue[0].info,INFO_RX);
			break;
		case INFO_RX:
			switch(device->status)
			{
			case DEVICE_STATUS_FREE:
				device->status=DEVICE_STATUS_SLAVE;
				break;
			case DEVICE_STATUS_MASTER:
				break;
			case DEVICE_STATUS_SLAVE:
				break;
			}
			device_index=g_device[device->device_index].queue[0].device_index;
			route_mark(device,device_index);
			InterlockedExchange((LPLONG)&g_device[device->device_index].queue[0].device_index,device->device_index);
			InterlockedExchange((LPLONG)&g_device[device->device_index].queue[0].info,INFO_TX);
			InterlockedExchange((LPLONG)&g_device[device_index].queue[0].device_index,device->device_index);
			InterlockedExchange((LPLONG)&g_device[device_index].queue[0].info,INFO_RX);
			if (!route_check(device))
				return 0;
			break;
		}
		break;
	//1.recursive seek reachable device
	//2.update path with shortest hops
	case TASK_DEVICE_MERGE:
		if (!g_init[device->device_index])
		{
			if (device->status==DEVICE_STATUS_FREE)
				return 1;
			EnterCriticalSection(&g_cs);
			device_recurse(device);
			LeaveCriticalSection(&g_cs);
			g_init[device->device_index]=1;
		}
		switch(g_device[device->device_index].queue[0].info)
		{
		case INFO_TX:
			route=route_check(device);
			if (!route)
				return 0;
			route_mark(device,route->device_index);
			InterlockedExchange((LPLONG)&g_device[device->device_index].queue[0].device_index,device->device_index);
			InterlockedExchange((LPLONG)&g_device[device->device_index].queue[0].info,INFO_TX);
			InterlockedExchange((LPLONG)&g_device[route->device_index].queue[0].device_index,device->device_index);
			InterlockedExchange((LPLONG)&g_device[route->device_index].queue[0].info,INFO_RX);
			break;
		case INFO_RX:
			device_index=g_device[device->device_index].queue[0].device_index;
			route_mark(device,device_index);
			InterlockedExchange((LPLONG)&g_device[device->device_index].queue[0].device_index,device->device_index);
			InterlockedExchange((LPLONG)&g_device[device->device_index].queue[0].info,INFO_TX);
			InterlockedExchange((LPLONG)&g_device[device_index].queue[0].device_index,device->device_index);
			InterlockedExchange((LPLONG)&g_device[device_index].queue[0].info,INFO_RX);
			if (!route_check(device))
				return 0;
			break;
		}
		break;
	//1.update path with shortest hops
	//2.reduce piconets
	//3.delete unnecessary bridges
	case TASK_DEVICE_OPTIMIZE:
		/*
		if (g_device[i].status==DEVICE_STATUS_SLAVE)
		{
			route=g_device[i].route;
			while(route)
			{
				if (g_device[route->device_index].status==DEVICE_STATUS_MASTER && math_distance(device->x,device->y,g_device[route->device_index].x,g_device[route->device_index].y)<=MAX_METRIC)
					return route->device_index;
				route=route->next;
			}
		}*/
		break;
	//1.index dag by same device set
	case TASK_DEVICE_INDEXDAG:
		if (device->status==DEVICE_STATUS_FREE)
			return 1;
		EnterCriticalSection(&g_cs);
		switch(g_device[device->device_index].queue[0].info)
		{
		case INFO_TX:
			device->dag_index=device->queue[0].buffer[0] ? device->queue[0].buffer[0] : 1;
			for (i=0;i<g_devicenum;i++)
			{
				if (i==device->device_index || g_device[g_device[i].device_index].queue[0].info==INFO_RX)
					continue;
				route=route_exist(device,g_device[i].device_index);
				j=device->dag_index;
				if (route)
					g_device[g_device[i].device_index].queue[0].info=INFO_RX;
				else
					j++;
				g_device[i].queue[0].buffer[0]=(uint8)j;
			}
			break;
		case INFO_RX:
			device->dag_index=device->queue[0].buffer[0];
			break;
		}
		LeaveCriticalSection(&g_cs);
		break;
	case TASK_DEVICE_WALK:
		if (!g_init[device->device_index])
		{
			EnterCriticalSection(&g_cs);
			device_location(device);
			device_release(device);
			LeaveCriticalSection(&g_cs);
			g_init[device->device_index]=1;
		}
		break;
	}

	return 0;
}
#endif