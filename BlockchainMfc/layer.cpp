#include "stdafx.h"
#include "layer.h"

extern volatile uint8 g_run;//0-停止,1-运行

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
	while(route)
	{
		device->route=route->next;
		delete route;
		route=device->route;
	}
}

//route find node by device_index
uint8 route_node(device_t *device,uint32 device_index)
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

//route find by device_index
route_t *route_find(device_t *device,uint32 device_index)
{
	route_t *route;

	route=device->route;
	while(route)
	{
		if (route->device_index==device_index)
			break;
		route=route->next;
	}
	
	return route;
}

//queue insert into device->queue
void queue_insert(device_t *device,queue_t *queue)
{
	queue_t *next;

	if (queue->step==STEP_CONNECT)
	{
		queue->next=device->queue;
		device->queue=queue;
	}
	else
	{
		if (!device->queue)
		{
			queue->next=device->queue;
			device->queue=queue;
		}
		else
		{
			next=device->queue;
			while(next->next) next=next->next;
			queue->next=NULL;
			next->next=queue;
		}
	}
}

//queue insert into mainchain->queue
void queue_insert(mainchain_t *mainchain,queue_t *queue)
{
	queue_t *next;

	if (queue->step==STEP_CONNECT)
	{
		queue->next=mainchain->queue;
		mainchain->queue=queue;
	}
	else
	{
		if (!mainchain->queue)
		{
			queue->next=mainchain->queue;
			mainchain->queue=queue;
		}
		else
		{
			next=mainchain->queue;
			while(next->next) next=next->next;
			queue->next=NULL;
			next->next=queue;
		}
	}
}

//queue delete from device/mainchain->queue
void queue_delete(queue_t *queue)
{
	queue_t *point;

	point=queue;
	while(point)
	{
		queue=point->next;
		if (point->data)
		{
			delete[] point->data;
			point->data=NULL;
		}
		delete point;
		point=queue;
	}
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

//dag delete:删除dag中的某项tip
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

//dag delete:删除整个dag
void dag_delete(mainchain_t *mainchain)
{
	transaction_t *transaction,*trunk,*branch;

	//dag
	transaction=mainchain->dag;
	while(transaction)
	{
		trunk=transaction->trunk;
		branch=transaction->branch;

		transaction=transaction->next;
	}
	//tip



}

//dag clear:清除dag中的flag
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

//dag tip number:计算dag中的tip数
uint32 dag_tipnum(transaction_t *dag)
{
	uint32 number;
	transaction_t *transaction;

	number=0;
	transaction=dag;
	while(transaction)
	{
		if (!transaction->flag)//正确的tip
			number++;
		transaction=transaction->next;
	}

	return number;
}

//dag nontip number:计算dag中的nontip数
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

//dag num(tip+nontip):计算dag中所有交易数(tip+nontip)
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

//dag nontip point:计算dag中的nontip数
uint32 dag_dagpoint(transaction_t *transaction)
{
	if (transaction->flag)
		return 0;
	transaction->flag=1;

	if (transaction->trunk && transaction->branch)
		return dag_dagpoint(transaction->trunk)+dag_dagpoint(transaction->branch);
	if (transaction->trunk)
		return dag_dagpoint(transaction->trunk)+1;
	if (transaction->branch)
		return dag_dagpoint(transaction->branch)+1;

	return 1;
}

//dag tip:动态调整tip个数(初始构造、动态运行时调整宽度)
uint32 dag_tip(void)
{
	return 5;
}

//dag height:创世交易至当前交易所有路径中的最长路径(NP-Hard问题)
uint32 dag_height(transaction_t *dag,transaction_t *transaction)
{
	return 0;
}

//dag depth:当前交易至某个tip的最长路径
uint32 dag_depth(transaction_t *dag,transaction_t *transaction)
{
	return 0;
}

//device delete
void device_delete(device_t *device)
{
	route_t *route;

	//route
	route_delete(device);
	//queue
	queue_delete(device->queue);
	//rsa
	delete[] device->rsa.e;
	delete[] device->rsa.n;
	delete[] device->rsa.p;
	delete[] device->rsa.q;
	delete[] device->rsa.d;
	delete[] device->rsa.dp;
	delete[] device->rsa.dq;
	delete[] device->rsa.qp;
}

struct mainchain_t
{
	queue_t *queue;//消息队列
	uint32 list_number;//节点数目
	uint32 dag_number;//区域数目
	list_t *list;//节点属性列表
	transaction_t *dag;//账本dag链表(全局账本)
};



struct transaction_t
{
	uint32 index;//交易索引
	deal_t deal;//交易原子
	uint8 plain[KEY_LEN];//明文验证
	uint8 cipher[KEY_LEN];//密文验证
	uint32 pow[2];//按计算规则得到的前序trunk/branch的pow值
	//
	uint8 transaction;//交易状态.0-tip,1-dag
	uint8 flag;//dag:0-未计算,1-已计算.tip:0-正确,1-错误
	uint16 reserved;
	//uint8 type;//交易类型.0-普通信息,1-有价信息
	//
	transaction_t *trunk;//主交易节点
	transaction_t *branch;//从交易节点
	transaction_t *next;//tip链表使用
};

//mainchain delete
void mainchain_delete(mainchain_t *mainchain)
{
	//queue
	queue_delete(mainchain->queue);
	//list
	delete[] mainchain->list;
	//dag

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

//move device
void move_location(device_t *device,uint32 step,uint32 range)
{
	if (rand()%2)
	{
		device->x+=step;
		device->x=math_min(device->x,range-1);
	}
	else
	{
		device->x-=step;
		device->x=math_max(device->x,(uint32)0);
	}
	if (rand()%2)
	{
		device->y+=step;
		device->y=math_min(device->y,range-1);
	}
	else
	{
		device->y-=step;
		device->y=math_max(device->y,(uint32)0);
	}
}

void delay(void)
{
	while(!g_run);
	Sleep(500);
}