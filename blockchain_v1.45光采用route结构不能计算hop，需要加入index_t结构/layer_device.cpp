#include "layer_device.h"

extern uint32 g_devicenum;//设备个数
extern uint32 g_devicerange;//设备坐标范围
extern uint32 g_devicestep;//设备步进值
extern CRITICAL_SECTION g_cs;
extern device_t *g_device;//设备数组
extern volatile uint8 g_task;//传递给线程的过程标记
extern volatile uint8 *g_init;//每个线程的初始化任务.0-未初始化,1-已初始化

#if 0

void route_mark(device_t *device,uint32 device_index)
{
	route_t *route;

	route=device->route;
	while(route)
	{
		if (route->device_index==device_index)
		{
			route->flag=1;
			break;
		}
		route=route->next;
	}
}

route_t *route_check(device_t *device)
{
	route_t *route;

	route=device->route;
	while(route)
	{
		if (!route->flag)
			break;
		route=route->next;
	}

	return route;
}

route_t *route_exist(device_t *device,uint32 index)
{
	route_t *route;

	route=device->route;
	while(route)
	{
		if (route->device_index==index)
			break;
		route=route->next;
	}

	return route;
}



void device_recurse(device_t *device)
{
	//recursive seek/malloc route
	uint8 flag;//0-new,1-update,2-abort
	route_t *route,*point[3];

	point[0]=device->route;
	while(point[0])
	{
		point[1]=g_device[point[0]->device_index].route;
		while(point[1])
		{
			if (point[1]->device_index!=device->device_index)
			{
				flag=0;
				point[2]=device->route;
				while(point[2])
				{
					if (point[1]->device_index==point[2]->device_index)
					{
						flag=point[0]->hops+1<point[2]->hops ? 1 : 2;
						break;
					}
					point[2]=point[2]->next;
				}
				switch(flag)
				{
				case 0:
					route=new route_t;
					route->flag=0;
					route->device_index=point[1]->device_index;
					route->hops=point[0]->hops+1;
					route->path=new uint32[point[0]->hops];
					memcpy(route->path,point[0]->path,(point[0]->hops-1)*sizeof(uint32));
					route->path[point[0]->hops-1]=point[0]->device_index;
					route->next=NULL;
					route_insert(device,route);
					break;
				case 1:
					point[2]->flag=0;
					point[2]->device_index=point[1]->device_index;
					point[2]->hops=point[0]->hops+1;
					delete[] point[2]->path;
					point[2]->path=new uint32[point[0]->hops];
					memcpy(point[2]->path,point[0]->path,(point[0]->hops-1)*sizeof(uint32));
					point[2]->path[point[0]->hops-1]=point[0]->device_index;
					break;
				case 2:
					break;
				}
			}
			point[1]=point[1]->next;
		}
		point[0]=point[0]->next;
	}
}

void device_location(device_t *device)
{
	if (rand()%2)
	{
		device->x+=g_devicestep;
		device->x=math_min(device->x,g_devicerange-1);
	}
	else
	{
		device->x-=g_devicestep;
		device->x=math_max(device->x,(uint32)0);
	}
	if (rand()%2)
	{
		device->y+=g_devicestep;
		device->y=math_min(device->y,g_devicerange-1);
	}
	else
	{
		device->y-=g_devicestep;
		device->y=math_max(device->y,(uint32)0);
	}
}

void device_release(device_t *device)
{
	route_t *route,*point;

	route=device->route;
	while(route)
	{
		point=route->next;
		if (route->path)
			delete[] route->path;
		delete route;
		route=point;
	}
	device->route=NULL;
}

void print_status(void)
{
	uint32 i;

	for (i=0;i<g_devicenum;i++)
		printf("%ld",g_device[i].status);
	printf("\r\n");
}

void print_route(void)
{
	uint32 i;
	route_t *route;

	for (i=0;i<g_devicenum;i++)
	{
		printf("dagindex%d-device%ld:",g_device[i].dag_index,g_device[i].device_index);
		route=g_device[i].route;
		while(route)
		{
			printf("%ld(%ld),",route->device_index,route->hops);
			/*
			printf("%ld(%ld=",route->device_index,route->hops);
			for (j=0;j<route->hops-1;j++)
				printf("%ld-",route->path[j]);
			printf("),");
			*/
			route=route->next;
		}
		printf("\r\n");
	}
}
#endif

//new
//route insert into device->route
void route_insert(device_t *device,route_t *route)
{
	route->next=device->route;
	device->route=route;
}

//0-not find,1-find
uint8 device_seek(device_t *device)
{
	//broadcasting to seek/malloc device
	uint32 i;
	route_t *route;
	uint8 flag,exist;

	exist=0;
	for (i=0;i<g_devicenum;i++)
	{
		if (i==device->device_index)
			continue;
		if (math_distance(device->x,device->y,g_device[i].x,g_device[i].y)<=MAX_METRIC)
		{
			flag=0;
			route=device->route;
			while(route)
			{
				if (route->device_index==g_device[i].device_index)
				{
					flag=1;
					break;
				}
				route=route->next;
			}
			if (!flag)
			{
				route=new route_t;
				route->flag=0;
				route->device_index=g_device[i].device_index;
				route->hops=1;
				route->path=NULL;
				route->next=NULL;
				route_insert(device,route);
				exist=1;
			}
		}
	}

	return exist;
}

void process_device(device_t *device)
{
	route_t *route,*insert,*point[2];
	uint8 flag;

	switch(device->step)
	{
	case STEP_CONNECT:
		//send
		device->step=STEP_TANGLE;
		if (!device_seek(device))//no more device addin
			break;
		//recv
		route=device->route;
		while(route)
		{
			g_device[route->device_index].step=STEP_CONNECT;
			flag=0;
			point[1]=g_device[route->device_index].route;//compare with device->device_index
			while(point[1])
			{
				if (point[1]->device_index==device->device_index)
				{
					flag=1;
					break;
				}
				point[1]=point[1]->next;
			}
			if (!flag)
			{
				insert=new route_t;
				insert->flag=0;
				insert->device_index=device->device_index;
				insert->hops=1;
				insert->path=NULL;
				insert->next=NULL;
				route_insert(&g_device[route->device_index],insert);
			}
			point[0]=device->route;
			while(point[0])
			{
				flag=0;
				point[1]=g_device[route->device_index].route;//compare with device->route
				while(point[1])
				{
					if (point[1]->device_index==point[0]->device_index)
					{
						flag=1;
						break;
					}
					point[1]=point[1]->next;
				}
				if (!flag)
				{
					insert=new route_t;
					insert->flag=0;
					insert->device_index=point[0]->device_index;
					insert->hops=1;
					insert->path=NULL;
					insert->next=NULL;
					route_insert(&g_device[route->device_index],insert);
				}
				point[0]=point[0]->next;
			}
			route=route->next;
		}
		break;
	case STEP_TANGLE:
		break;
	}
}

#if 0
void process_device(device_t *device)
{
	//static __declspec(thread) uint8 flag;
	uint32 device_index;
	route_t *route;

	switch(device->flag)
	{
	case RECV:
		switch(device->buffer[0])
		{
		case STEP_INITIAL:
			device->buffer[0]=STEP_CONNECT;
			break;
		case STEP_CONNECT:
			//update device's property:0-device visible(master/slave),1-device invisible(free)
			//1.when is free device:->master then turn to slave;->free' then free' turn to slave;->slave then (connect its master turn to slave,else turn to master)
			//2.when is master device:->free then free turn to slave;->slave not connect;->master connect
			//3.when is slave device:->free/master/slave connect
			device->buffer[0]=STEP_MERGE;
			device_index=device_seek(device);
			if (device_index==device->device_index)//no device visible
				break;
			route=route_check(device);
			if (!route)
				break;
			route_mark(device,route->device_index);
			switch(device->status)
			{
			case STATUS_FREE:
				device->status=g_device[route->device_index].status==STATUS_MASTER ? STATUS_SLAVE : STATUS_MASTER;
				break;
			case STATUS_MASTER:
				break;
			case STATUS_SLAVE:
				break;
			}
			InterlockedExchange((LPLONG)&g_device[device->device_index].queue[0].device_index,device->device_index);
			InterlockedExchange((LPLONG)&g_device[device->device_index].queue[0].info,INFO_TX);
			InterlockedExchange((LPLONG)&g_device[route->device_index].queue[0].device_index,device->device_index);
			InterlockedExchange((LPLONG)&g_device[route->device_index].queue[0].info,INFO_RX);
			break;
		case STEP_MERGE:
			//1.recursive seek reachable device
			//2.update path with shortest hops
			device->buffer[0]=STEP_OPTIMIZE;
			if (device->status==STATUS_FREE)
				break;
			device_recurse(device);
			route=route_check(device);
			if (!route)
				return 0;
			route_mark(device,route->device_index);
			InterlockedExchange((LPLONG)&g_device[device->device_index].queue[0].device_index,device->device_index);
			InterlockedExchange((LPLONG)&g_device[device->device_index].queue[0].info,INFO_TX);
			InterlockedExchange((LPLONG)&g_device[route->device_index].queue[0].device_index,device->device_index);
			InterlockedExchange((LPLONG)&g_device[route->device_index].queue[0].info,INFO_RX);
			break;
		case STEP_OPTIMIZE:
			//1.update path with shortest hops
			//2.reduce piconets
			//3.delete unnecessary bridges
			device->buffer[0]=STEP_INDEXDAG;
			break;
		case STEP_INDEXDAG:
			//1.index dag by same device set
			if (device->status==DEVICE_STATUS_FREE)
				return 1;
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
		case STEP_WALK:
			device_location(device);
			device_release(device);
			break;
		}
		break;
	case SEND:
		switch(device->buffer[0])
		{
		case STEP_INITIAL:
			device->buffer[0]=STEP_CONNECT;
			break;
		case STEP_CONNECT:
			if (!flag)
			{
				flag=1;
				device_index=device_seek(device);
				if (device_index==device->device_index)//no device visible
					break;
			}
			device->buffer[0]=STEP_MERGE;
			switch(device->status)
			{
			case STATUS_FREE:
				device->status=STATUS_SLAVE;
				break;
			case STATUS_MASTER:
				break;
			case STATUS_SLAVE:
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
		case STEP_MERGE:
			if (!flag)
			{
				flag=1;
				if (device->status==STATUS_FREE)
					return 1;
				device_recurse(device);
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
		case STEP_OPTIMIZE:
			break;
		case STEP_INDEXDAG:
			if (device->status==DEVICE_STATUS_FREE)
				return 1;
			device->dag_index=device->queue[0].buffer[0];
			break;
		case STEP_WALK:
			break;
		}
		break;
	}
}
#endif
