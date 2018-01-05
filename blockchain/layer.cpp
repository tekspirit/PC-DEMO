#include "layer.h"

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

//queue insert into mainchain->queue
void queue_insert(mainchain_t *mainchain,queue_t *queue)
{
	queue->next=mainchain->queue;
	mainchain->queue=queue;
}

//queue delete from device->queue
void queue_delete(device_t *device)
{
	queue_t *queue;

	queue=device->queue;
	device->queue=queue->next;
	if (queue->data)
	{
		delete[] queue->data;
		queue->data=NULL;
	}
	delete queue;
}

void queue_delete(queue_t *prev,queue_t *queue)
{
	//delete queue
	if (prev)
		prev->next=queue->next;
	if (queue->data)
	{
		delete[] queue->data;
		queue->data=NULL;
	}
	delete queue;
}

//list delete from mainchain->list
void list_delete(mainchain_t *mainchain)
{
	if (mainchain->list)
		delete[] mainchain->list;
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