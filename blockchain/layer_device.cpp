#include "layer_device.h"

extern uint32 g_devicenum[2];//设备个数
extern uint32 g_devicerange;//设备坐标范围
extern uint32 g_devicestep;//设备步进值
extern uint32 g_dealnumber;//交易原子列表个数
extern uint32 g_dealindex;//交易原子列表索引
extern deal_t *g_deal;//交易原子列表
extern CRITICAL_SECTION g_cs;
extern device_t *g_device;//设备数组
extern mainchain_t g_mainchain;//主链
extern volatile uint32 g_index;//临时用来统计交易号码的(以后会用hash_t代替,计数从1开始)

//STEP_CONNECT
void connect_recv(device_t *device)
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
			//queue process
			index.number=*(uint32 *)queue->data;
			index.index=(uint32 *)(queue->data+1*sizeof(uint32));
			index.key=queue->data+(1+index.number)*sizeof(uint32);
			index.token=(uint32 *)(queue->data+(1+index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN));
			index.node=queue->data+(1+2*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN);
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
					memcpy(route->key.e,&index.key[i*(KEY_E+KEY_LEN)],KEY_E);
					memcpy(route->key.n,&index.key[i*(KEY_E+KEY_LEN)+KEY_E],KEY_LEN);
					route->token=index.token[i];
					route->node=index.node[i];
					route->next=NULL;
					route_insert(device,route);
				}
			}
			//queue delete
			if (queue==device->queue)
			{
				device->queue=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
				delete queue;
				queue=device->queue;
			}
			else
			{
				prev->next=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
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

void connect_seek(device_t *device)
{
	//broadcasting to seek/malloc device->route
	uint32 i;
	uint8 flag;
	route_t *route;

	for (i=0;i<g_devicenum[0]+g_devicenum[1];i++)
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
				memcpy(route->key.e,g_device[i].rsa.e,KEY_E);
				memcpy(route->key.n,g_device[i].rsa.n,KEY_LEN);
				route->token=g_device[i].token;
				route->node=g_device[i].node;
				route->next=NULL;
				route_insert(device,route);
			}
		}
	}
}

void connect_send(device_t *device)
{
	//route->queue(device/mainchain)
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
	index.token=new uint32[index.number];
	index.node=new uint8[index.number];
	index.number=0;
	index.index[index.number]=device->device_index;
	memcpy(&index.key[index.number*(KEY_E+KEY_LEN)],device->rsa.e,KEY_E);
	memcpy(&index.key[index.number*(KEY_E+KEY_LEN)+KEY_E],device->rsa.n,KEY_LEN);
	index.token[index.number]=device->token;
	index.node[index.number]=device->node;
	index.number++;
	route=device->route;
	while(route)
	{
		index.index[index.number]=route->device_index;
		memcpy(&index.key[index.number*(KEY_E+KEY_LEN)],route->key.e,KEY_E);
		memcpy(&index.key[index.number*(KEY_E+KEY_LEN)+KEY_E],route->key.n,KEY_LEN);
		index.token[index.number]=route->token;
		index.node[index.number]=route->node;
		index.number++;
		route=route->next;
	}
	//fill route
	route=device->route;
	while(route)
	{
		queue=new queue_t;
		queue->step=STEP_CONNECT;
		queue->data=new uint8[(1+2*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN)+index.number*sizeof(uint8)];
		*(uint32 *)queue->data=index.number;//align problem?
		memcpy(queue->data+1*sizeof(uint32),index.index,index.number*sizeof(uint32));
		memcpy(queue->data+(1+index.number)*sizeof(uint32),index.key,index.number*(KEY_E+KEY_LEN));
		memcpy(queue->data+(1+index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN),index.token,index.number*sizeof(uint32));
		memcpy(queue->data+(1+2*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN),index.node,index.number*sizeof(uint8));
		queue_insert(&g_device[route->device_index],queue);
		route=route->next;
	}
	//fill mainchain
	queue=new queue_t;
	queue->step=STEP_CONNECT;
	queue->data=new uint8[(1+2*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN)+index.number*sizeof(uint8)];
	*(uint32 *)queue->data=index.number;//align problem?
	memcpy(queue->data+1*sizeof(uint32),index.index,index.number*sizeof(uint32));
	memcpy(queue->data+(1+index.number)*sizeof(uint32),index.key,index.number*(KEY_E+KEY_LEN));
	memcpy(queue->data+(1+index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN),index.token,index.number*sizeof(uint32));
	memcpy(queue->data+(1+2*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN),index.node,index.number*sizeof(uint8));
	queue_insert(&g_mainchain,queue);
	//release
	delete[] index.index;
	delete[] index.key;
	//update queue
	queue=new queue_t;
	queue->step=STEP_TRANSACTION;
	queue->data=NULL;
	queue_insert(device,queue);
}

//STEP_TRANSACTION
uint8 transaction_verify(device_t *device,transaction_t *transaction)
{
	//交易验证：使用rsa公钥验签验证交易地址，验证交易账本。0-正确,1-交易地址错误,2-交易账本错误
	uint32 i;
	rsa_t rsa;
	route_t *route;
	uint8 result[KEY_LEN];

	//get device
	rsa.le=KEY_E;
	rsa.len=KEY_LEN;
	route=device->route;
	while(route)
	{
		if (route->device_index==transaction->deal.device_index[0])
			break;
		route=route->next;
	}
	//地址验证
	memcpy(rsa.e,route->key.e,KEY_E);
	memcpy(rsa.n,route->key.n,KEY_LEN);
	i=rsa_enc(result,transaction->cipher,rsa.len,&rsa);
	memset(&result[i],0,rsa.len-i);
	if (memcmp(result,transaction->plain,rsa.len))
		return STATUS_DEVICE;
	//账本验证
	if (transaction->deal.token>route->token)
		return STATUS_LEDGER;

	return STATUS_DONE;
}

void transaction_recv(device_t *device)
{
	//queue->delete queue
	uint8 node;
	deal_t *deal;
	queue_t *queue,*prev,*insert;

	queue=device->queue;
	while(queue)
	{
		if (queue->step==STEP_TRANSACTION)
		{
			//queue process
			if (queue->data && device->node==NODE_HEAVY)//若是重节点且有交易内容
			{
				//地址验证+账本验证
				deal=(deal_t *)(queue->data+1*sizeof(uint32));
				flag=transaction_verify(device,deal->device_index[0]);
				node=route_findnode(device,deal->device_index[0]);
				if (flag)//error
				{
					if (node==NODE_LIGHT)//源为轻节点,则传回轻节点去更新
					{
						insert=new queue_t;
						insert->step=STEP_LEDGER;
						insert->data=new uint8[sizeof(ledger_t)];
						*(uint32 *)insert->data=flag;
						*(uint32 *)(insert->data+1*sizeof(uint32))=*(uint32 *)queue->data;
						*(uint32 *)(insert->data+2*sizeof(uint32))=deal->token;
						queue_insert(&g_device[deal->device_index[1]],insert);
					}
					else//源为重节点(自己),则更新token
					{
						device->token[0]+=deal->token;
						device->token[1]-=deal->token;
					}
				}
				else//correct则传入服务器
				{
					insert=new queue_t;
					insert->step=STEP_TRANSACTION;
					insert->data=new uint8[sizeof(spv_t)];
					*(uint32 *)insert->data=*(uint32 *)queue->data;
					memcpy(insert->data+1*sizeof(uint32),queue->data+1*sizeof(uint32),sizeof(deal_t));
					memcpy(insert->data+1*sizeof(uint32)+sizeof(deal_t),queue->data+1*sizeof(uint32)+sizeof(deal_t),KEY_LEN);
					memcpy(insert->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN,queue->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN,KEY_LEN);
					queue_insert(&g_mainchain,insert);
					//update token
					if (node==NODE_LIGHT)//源为轻节点,则更新当前重节点token
					{
						device->token[0]-=deal->token;
						device->token[1]+=deal->token;
					}
				}
			}
			//queue delete
			if (queue==device->queue)
			{
				device->queue=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
				delete queue;
				queue=device->queue;
			}
			else
			{
				prev->next=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
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

void transaction_signature(transaction_t *transaction,device_t *device)
{
	//encrypt transaction with rsa private key
	uint32 i;

	i=rsa_dec(transaction->cipher,transaction->plain,device->rsa.len,&device->rsa,RSA_CRT);
	memset(&transaction->cipher[i],0,device->rsa.len-i);
}

transaction_t *transaction_generate(device_t *device)
{
	//generate transaction
	uint32 i;
	transaction_t *transaction;

	if (device->device_index!=g_deal[g_dealindex].device_index[0])//非当前笔交易索引
		return NULL;
	if (g_deal[g_dealindex].token>device->token[0])//账本验证
		return NULL;
	transaction=new transaction_t;
	transaction->index=++g_index;
	memcpy(&transaction->deal,&g_deal[g_dealindex],sizeof(transaction_t));
	_rand(transaction->plain,KEY_LEN);
	i=_mod(transaction->plain,transaction->plain,&device->rsa.n,KEY_LEN,KEY_LEN);
	memset(&transaction->plain[i],0,KEY_LEN-i);
	transaction_signature(transaction,device);
	transaction->transaction=TRANSACTION_NONE;
	transaction->flag=0;//给寻找和计算使用
	g_dealindex++;

	return transaction;
}

void transaction_send(device_t *device,transaction_t *transaction)
{
	//transaction->queue
	route_t *route;
	queue_t *queue;

	//update queue(self)
	if (!transaction || device->node==NODE_LIGHT)
	{
		queue=new queue_t;
		queue->step=STEP_TRANSACTION;
		queue->data=NULL;
		queue_insert(device,queue);
		return;
	}
	if (device->node==NODE_LIGHT)
	{
		route=device->route;
		while(route)
		{
			if (route->node==NODE_HEAVY)
				break;
			route=route->next;
		}
	}
	//update queue(other)
	queue=new queue_t;
	queue->step=STEP_TRANSACTION;
	queue->data=new uint8[sizeof(spv_t)];
	*(uint32 *)queue->data=transaction->index;
	memcpy(queue->data+1*sizeof(uint32),&transaction->deal,sizeof(deal_t));
	memcpy(queue->data+1*sizeof(uint32)+sizeof(deal_t),transaction->plain,KEY_LEN);
	memcpy(queue->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN,transaction->cipher,KEY_LEN);
	if (device->node==NODE_LIGHT)//若是轻节点,则将交易传给第一重节点
	{
		if (route)
			queue_insert(&g_device[route->device_index],queue);
	}
	else//若是重节点,则更新给自己
		queue_insert(device,queue);
	//update token
	device->token[0]-=transaction->deal.token;
	device->token[1]+=transaction->deal.token;
}

//STEP_LEDGER
void ledger_recv(device_t *device)
{
	//queue->delete queue
	uint8 node;
	queue_t *queue,*prev,*insert;

	queue=device->queue;
	while(queue)
	{
		if (queue->step==STEP_LEDGER)
		{
			//queue process
			if (device->node==NODE_HEAVY)//若是重节点
			{
				node=route_findnode(device,*(uint32 *)(queue->data+1*sizeof(uint32)));
				if (node==NODE_LIGHT)//目标节点为轻节点,则传递至轻节点更新
				{
					insert=new queue_t;
					insert->step=STEP_LEDGER;
					insert->data=new uint8[sizeof(ledger_t)];
					memcpy(insert,queue,sizeof(ledger_t));
					queue_insert(&g_device[*(uint32 *)(queue->data+1*sizeof(uint32))],insert);
				}
				else//若为重节点,update token
				{
					if (*(uint32 *)queue->data==STATUS_DONE)
						device->token[1]-=*(uint32 *)(queue->data+2*sizeof(uint32));
					else
					{
						device->token[0]+=*(uint32 *)(queue->data+2*sizeof(uint32));
						device->token[1]-=*(uint32 *)(queue->data+2*sizeof(uint32));
					}
				}
			}
			else//若为轻节点,update token
			{
				if (*(uint32 *)queue->data==STATUS_DONE)
					device->token[1]-=*(uint32 *)(queue->data+2*sizeof(uint32));
				else
				{
					device->token[0]+=*(uint32 *)(queue->data+2*sizeof(uint32));
					device->token[1]-=*(uint32 *)(queue->data+2*sizeof(uint32));
				}
			}
			//queue delete
			if (queue==device->queue)
			{
				device->queue=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
				delete queue;
				queue=device->queue;
			}
			else
			{
				prev->next=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
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

void ledger_send(device_t *device)
{
	//ledger->queue
	queue_t *queue;

	queue=new queue_t;
	queue->step=STEP_MOVE;
	queue->data=NULL;
	queue_insert(device,queue);
}

//STEP_MOVE
void move_location(device_t *device)
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

void process_device(device_t *device)
{
	transaction_t *transaction;

	if (!device->queue)
		return;
	switch(device->queue->step)
	{
	case STEP_CONNECT:
		//recv
		connect_recv(device);//recv & process device's queue->route
		//process
		connect_seek(device);//search around nearby->route
		//send
		connect_send(device);//pack & send device's route->queue
		break;
	case STEP_TRANSACTION:
		//recv
		transaction_recv(device);//recv & process device's queue
		//process
		transaction=transaction_generate(device);//generate transaction
		//send
		transaction_send(device,transaction);//pack & send device's route->queue
		break;
	case STEP_LEDGER:
		//recv
		ledger_recv(device);//recv & process device's queue
		//process
		//send
		ledger_send(device);//pack & send device's route->queue
		break;
	case STEP_MOVE:
		break;
	}
}