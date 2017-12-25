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

//route delete from device->route
void route_delete(device_t *device)
{
	route_t *route;

	route=device->route;
	device->route=route->next;
	delete route;
}

void route_delete(route_t *route)
{
	//delete route->next
	route_t *next;

	next=route->next;
	route->next=next->next;
	delete next;
}

//queue insert into device->queue
void queue_insert(device_t *device,queue_t *queue)
{
	queue->next=device->queue;
	device->queue=queue;
}

//queue delete from device->queue
void queue_delete(device_t *device)
{
	queue_t *queue;

	queue=device->queue;
	device->queue=queue->next;
	if (queue->data)
		delete[] queue->data;
	delete queue;
}

void queue_delete(queue_t *prev,queue_t *queue)
{
	//delete queue
	prev->next=queue->next;
	if (queue->data)
		delete[] queue->data;
	delete queue;
}

//generate device->rsa
void key_generate(device_t *device)
{
	uint8 flag;

	//malloc
	device->rsa.le=KEY_E;
	device->rsa.len=KEY_LEN;
	device->rsa.lr=KEY_MASK;
	device->rsa.e=new uint8[device->rsa.le];
	device->rsa.n=new uint8[device->rsa.len];
	device->rsa.p=new uint8[device->rsa.len>>1];
	device->rsa.q=new uint8[device->rsa.len>>1];
	device->rsa.d=new uint8[device->rsa.len];
	device->rsa.dp=new uint8[device->rsa.len>>1];
	device->rsa.dq=new uint8[device->rsa.len>>1];
	device->rsa.qp=new uint8[device->rsa.len>>1];
	//generate
	device->rsa.e[0]=0x01;
	device->rsa.e[1]=0x00;
	device->rsa.e[2]=0x01;
	device->rsa.e[3]=0x00;
	while(1)
	{
		_rand(device->rsa.p,device->rsa.len>>1);
		_rand(device->rsa.q,device->rsa.len>>1);
		flag=rsa_genkey(&device->rsa,RSA_CRT);
		if (!flag)
			break;
	}
}

//STEP_CONNECT
void device_recv(device_t *device)
{
	//queue->route
	uint32 i;
	uint8 flag;
	route_t *route;
	queue_t *queue,*prev;
	index_t index;

	queue=device->queue;
	while(queue)
	{
		if (queue->step==STEP_CONNECT)
		{
			index.number=*(uint32 *)queue->data;
			index.index=(uint32 *)(queue->data+1*sizeof(uint32));
			index.key=queue->data+(1+index.number)*sizeof(uint32);
			for (i=0;i<index.number;i++)
			{
				if (index.index[i]==device->device_index)
					continue;
				flag=0;
				route=device->route;
				while(route)
				{
					if (route->device_index==index.index[i])
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
					route->device_index=index.index[i];
					route->path=NULL;
					memcpy(route->key.e,&index.key[i*(KEY_E+KEY_LEN)],KEY_E);
					memcpy(route->key.n,&index.key[i*(KEY_E+KEY_LEN)+KEY_E],KEY_LEN);
					route->next=NULL;
					route_insert(device,route);
				}
			}
			if (queue==device->queue)
			{
				device->queue=queue->next;
				if (queue->data)
					delete[] queue->data;
				delete queue;
				queue=device->queue;
			}
			else
			{
				prev->next=queue->next;
				if (queue->data)
					delete[] queue->data;
				delete queue;
				queue=prev->next;
			}
		}
		else
		{
			prev=queue;
			queue=queue->next;
		}
	}
}

void device_seek(device_t *device)
{
	//broadcasting to seek/malloc device->route
	uint32 i;
	uint8 flag;
	route_t *route;

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
				route->path=NULL;
				memcpy(route->key.e,g_device[i].rsa.e,KEY_E);
				memcpy(route->key.n,g_device[i].rsa.n,KEY_LEN);
				route->next=NULL;
				route_insert(device,route);
			}
		}
	}
}

void device_send(device_t *device)
{
	//route->queue
	uint8 flag;
	route_t *route;
	queue_t *queue;
	index_t index;

	//compute number(should handling)
	flag=0;
	index.number=1;
	route=device->route;
	while(route)
	{
		if (!route->flag)
		{
			route->flag=1;
			flag=1;
		}
		index.number++;
		route=route->next;
	}
	if (!flag)
		return;
	index.index=new uint32[index.number];
	index.key=new uint8[index.number*(KEY_E+KEY_LEN)];
	index.number=0;
	index.index[index.number]=device->device_index;
	memcpy(&index.key[index.number*(KEY_E+KEY_LEN)],device->rsa.e,KEY_E);
	memcpy(&index.key[index.number*(KEY_E+KEY_LEN)+KEY_E],device->rsa.n,KEY_LEN);
	index.number++;
	route=device->route;
	while(route)
	{
		index.index[index.number]=route->device_index;
		memcpy(&index.key[index.number*(KEY_E+KEY_LEN)],route->key.e,KEY_E);
		memcpy(&index.key[index.number*(KEY_E+KEY_LEN)+KEY_E],route->key.n,KEY_LEN);
		index.number++;
		route=route->next;
	}
	//fill route
	route=device->route;
	while(route)
	{
		queue=new queue_t;
		queue->step=STEP_CONNECT;
		queue->data=new uint8[(1+index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN)];
		*(uint32 *)queue->data=index.number;//align problem?
		memcpy(queue->data+1*sizeof(uint32),index.index,index.number*sizeof(uint32));
		memcpy(queue->data+(1+index.number)*sizeof(uint32),index.key,index.number*(KEY_E+KEY_LEN));
		queue_insert(&g_device[route->device_index],queue);
		route=route->next;
	}
	delete[] index.index;
	delete[] index.key;
	//update queue
	queue=new queue_t;
	queue->step=STEP_TANGLE;
	queue->data=NULL;
	queue_insert(device,queue);
}

void process_device(device_t *device)
{
	if (!device->queue)
		return;
	switch(device->queue->step)
	{
	case STEP_CONNECT:
		//recv
		device_recv(device);//recv & process device's route->route
		device_seek(device);//search around nearby->route
		//send
		device_send(device);//pack & send device's route->queue
		break;
	case STEP_TANGLE:
		break;
	}
}