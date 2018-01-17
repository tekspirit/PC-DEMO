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

//route find node by device_index
uint8 route_find(device_t *device,uint32 device_index)
{
	route_t *route;

	if (device->device_index==device_index)
		return device->node;
	route=device->route;
	while(route)
	{
		if (route->device_index==device_index)
			break;
		route=route->next;
	}

	return route->node;
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

//list delete from mainchain->list
void list_delete(mainchain_t *mainchain)
{
	if (mainchain->list)
		delete[] mainchain->list;
}

//transaction insert into mainchain->dag
void transaction_insert(mainchain_t *mainchain,transaction_t *transaction)
{
	transaction->next=mainchain->dag;
	mainchain->dag=transaction;
}

//dag insert into mainchain->dag
void dag_insert(mainchain_t *mainchain,transaction_t *transaction)
{
	transaction_t *prev,*point;

	point=mainchain->dag;
	while(point)
	{
		if (point==transaction)
		{
			if (point==mainchain->dag)
				mainchain->dag=mainchain->dag->next;
			else
				prev->next=point->next;
			point->transaction=TRANSACTION_DAG;
			break;
		}
		prev=point;
		point=point->next;
	}
}

//dag delete:ɾ��dag�е�ĳ��tip
void dag_delete(mainchain_t *mainchain,transaction_t *transaction)
{
	transaction_t *prev,*point;

	point=mainchain->dag;
	while(point)
	{
		if (point==transaction)
		{
			if (point==mainchain->dag)
				mainchain->dag=mainchain->dag->next;
			else
				prev->next=point->next;
			delete point;
			break;
		}
		prev=point;
		point=point->next;
	}
}

//dag clear:���dag�е�flag
uint32 dag_clear(transaction_t *transaction)
{
	if (!transaction->flag)
		return 0;
	transaction->flag=0;
	if (transaction->trunk && transaction->branch)
		return dag_clear(transaction->trunk)+dag_clear(transaction->branch);
	if (transaction->trunk)
		return dag_clear(transaction->trunk)+1;
	if (transaction->branch)
		return dag_clear(transaction->branch)+1;

	return 1;
}

//dag tip number:����dag�е�tip��
uint32 dag_tipnum(transaction_t *dag)
{
	uint32 number;
	transaction_t *transaction;

	number=0;
	transaction=dag;
	while(transaction)
	{
		if (!transaction->flag)//��ȷ��tip
			number++;
		transaction=transaction->next;
	}

	return number;
}

//dag nontip number:����dag�е�nontip��
uint32 dag_dagnum(transaction_t *transaction)
{
	if (transaction->flag)
		return 0;
	transaction->flag=1;
	if (transaction->trunk && transaction->branch)
		return dag_dagnum(transaction->trunk)+dag_dagnum(transaction->branch);
	if (transaction->trunk)
		return dag_dagnum(transaction->trunk)+1;
	if (transaction->branch)
		return dag_dagnum(transaction->branch)+1;

	return 1;
}

//dag num(tip+nontip):����dag�����н�����(tip+nontip)
uint32 dag_num(transaction_t *dag)
{
	uint32 number;
	transaction_t *transaction;

	number=0;
	transaction=dag;
	while(transaction)
	{
		number+=dag_dagnum(transaction)+1;
		transaction=transaction->next;
	}

	return number;
}

//dag height:������������ǰ��������·���е��·��(NP-Hard����)
uint32 dag_height(transaction_t *dag,transaction_t *transaction)
{
	return 0;
}

//dag depth:��ǰ������ĳ��tip���·��
uint32 dag_depth(transaction_t *dag,transaction_t *transaction)
{
	return 0;
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