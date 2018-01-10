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
uint8 transaction_seek(transaction_t *trunk,transaction_t *branch,device_t *device)
{
	//search 2 tips(random algorithm):0-dag为空,1-dag非空.原则上需要使用手动构造出较宽的tangle(判断tip),这里我使用自动构造tangle(判断solid).
	uint32 i,j,k;
	transaction_t *transaction;

	if (!device->dag)//genesis
		return 1;
	i=compute_tip(device->dag);
	dag_clear(device->dag);
	if (i==1)//point to genesis
	{
		trunk=NULL;
		branch=NULL;
	}
	else
	{
		//find tip's index
		while(1)
		{
			j=rand()%i;
			k=rand()%i;
			if (j!=k)
				break;
		}
		i=0;
		transaction=device->dag;
		while(transaction)
		{
			if (j==i)
				trunk=transaction;
			if (k==i)
				branch=transaction;
			i++;
			transaction=transaction->next;
		}
	}

	return 0;
}

uint8 transaction_verify(device_t *device,transaction_t *transaction)
{
	//通过rsa公钥验签验证交易。0-正确,1-错误
	uint32 i;
	rsa_t rsa;
	route_t *route;
	uint8 result[KEY_LEN];

	rsa.le=KEY_E;
	rsa.len=KEY_LEN;
	route=device->route;
	while(route)
	{
		if (route->device_index==transaction->deal.device_index[0])
			break;
		route=route->next;
	}
	memcpy(rsa.e,route->key.e,KEY_E);
	memcpy(rsa.n,route->key.n,KEY_LEN);
	i=rsa_enc(result,transaction->cipher,rsa.len,&rsa);
	memset(&result[i],0,rsa.len-i);
	if (memcmp(result,transaction->plain,rsa.len))
		return 1;

	return 0;
}

uint32 transaction_pow(transaction_t *transaction)
{
	//通过sha256计算hash pow
	uint64 i;
	uint32 length;
	crypt_sha256 *sha256;
	uint8 content[KEY_LEN+1],result[HASH_LEN];

	length=KEY_LEN;
	memcpy(content,transaction->plain,length);
	sha256=new crypt_sha256;
	for (i=1;i<0x100000000;i++)
	{
		length=_add(content,content,1,length);
		sha256->sha256_init();
		sha256->sha256_update(content,length);
		sha256->sha256_final(result);
		if (_bitlen(result,HASH_LEN)<=HASH_LEN*8-COMPARE_LEN)
			break;
	}
	if (i==0x100000000)
		i=0;
	delete sha256;

	return (uint32)i;
}
/*
void transaction_modify()
{
		//clip trunk/branch into dag
		insert=new transaction_t;
		insert->index=*(uint32 *)queue->data;
		memcpy(&insert->deal,queue->data+1*sizeof(uint32),sizeof(deal_t));
		memcpy(insert->plain,queue->data+1*sizeof(uint32)+sizeof(deal_t),KEY_LEN);
		memcpy(insert->cipher,queue->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN,KEY_LEN);
		insert->flag=0;
		i=0;
		transaction=device->dag;
		while(transaction)
		{
			if (trunk==i)
				point[0]=


			transaction=transaction->next;
		}


		queue=new queue_t;
	queue->step=STEP_TRANSACTION;
	queue->data=new uint8[sizeof(spv_t)];
	*(uint32 *)queue->data=transaction->index;
	memcpy(queue->data+1*sizeof(uint32),&transaction->deal,sizeof(deal_t));
	memcpy(queue->data+1*sizeof(uint32)+sizeof(deal_t),transaction->plain,KEY_LEN);
	memcpy(queue->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN,transaction->cipher,KEY_LEN);
	*(uint32 *)(queue->data+1*sizeof(uint32)+sizeof(deal_t)+2*KEY_LEN)=transaction->trunk;
	*(uint32 *)(queue->data+2*sizeof(uint32)+sizeof(deal_t)+2*KEY_LEN)=transaction->branch;

uint32 index;//交易索引
	deal_t deal;//交易原子
	//uint8 type;//交易类型.0-普通信息,1-有价信息
	uint8 plain[KEY_LEN];//明文验证
	uint8 cipher[KEY_LEN];//密文验证
	uint32 pow[2];//按计算规则得到的前序trunk/branch的pow值
	//uint8 status;//交易状态.0-none,1-solid,2-tangle,3-milestone
	uint8 flag;

	//uint32 index_trunk;//主交易索引
	//uint32 index_branch;//从交易索引
	transaction_t *trunk;//主交易节点
	transaction_t *branch;//从交易节点
	transaction_t *next;//tip链表使用


		count=0;
		for (i=0;i<device->tangle_index;i++)
			if (device->tangle[i].flag!=TRANSACTION_SOLID)
			{
				if (trunk==count)
					trunk=i;
				if (branch==count)
					branch=i;
				count++;
			}
	}

	return 0;
}*/

void transaction_recv(device_t *device)
{
	//queue->delete queue
	transaction_t *trunk,*branch;
	uint32 pow[2];
	queue_t *queue,*prev,*insert;

	queue=device->queue;
	while(queue)
	{
		if (queue->step==STEP_TRANSACTION)
		{
			//queue process
			do
			{
				if (queue->data && device->node==NODE_HEAVY)//若是重节点且有交易内容,则从dag中获取tip交易,pow值计算,交易验证,账本验证
				{
					flag=transaction_seek(trunk,branch,device);
					if (!flag)
					{
						flag=transaction_verify(device,trunk);
						if (flag)
						{
							insert=new queue_t;
							insert->step=STEP_FAIL;
							insert->data=new uint8[sizeof(status_t)];
							*(uint32 *)insert->data=trunk->index;
							*(uint32 *)(insert->data+1*sizeof(uint32))=

							memcpy(insert->data+1*sizeof(uint32),trunk->deal,sizeof(deal_t));
							memcpy(insert->data+1*sizeof(uint32)+sizeof(deal_t),queue->data+1*sizeof(uint32)+sizeof(deal_t),KEY_LEN);
							memcpy(insert->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN,queue->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN,KEY_LEN);
							*(uint32 *)(insert->data+1*sizeof(uint32)+sizeof(deal_t)+2*KEY_LEN)=find_tip(*(uint32 *)(queue->data+1*sizeof(uint32)+sizeof(deal_t)+2*KEY_LEN));
							*(uint32 *)(insert->data+2*sizeof(uint32)+sizeof(deal_t)+2*KEY_LEN)=find_tip(*(uint32 *)(queue->data+2*sizeof(uint32)+sizeof(deal_t)+2*KEY_LEN));
							*(uint32 *)(insert->data+3*sizeof(uint32)+sizeof(deal_t)+2*KEY_LEN)=find_tip(*(uint32 *)(queue->data+1*sizeof(uint32)+sizeof(deal_t)+2*KEY_LEN));
							queue_insert(&g_mainchain,insert);
						}
						flag=transaction_verify(device,&device->tangle[branch]);
						if (flag)
						{
						}
						pow[0]=transaction_pow(device,&device->tangle[trunk]);
						pow[1]=transaction_pow(device,&device->tangle[branch]);
					}
					else
						pow[0]=pow[1]=0;
					flag=transaction_generate(device,pow);
				}
			}while(0);


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
	if (g_deal[g_dealindex].token>device->token)//账本验证
		return NULL;
	transaction=new transaction_t;
	transaction->flag=0;//给寻找和计算使用
	transaction->index=++g_index;
	memcpy(&transaction->deal,&g_deal[g_dealindex],sizeof(transaction_t));
	_rand(transaction->plain,KEY_LEN);
	i=_mod(transaction->plain,transaction->plain,&device->rsa.n,KEY_LEN,KEY_LEN);
	memset(&transaction->plain[i],0,KEY_LEN-i);
	transaction_signature(transaction,device);
	g_dealindex++;

	return transaction;
}

void transaction_send(device_t *device,transaction_t *transaction)
{
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
	*(uint32 *)(queue->data+1*sizeof(uint32)+sizeof(deal_t)+2*KEY_LEN)=transaction->trunk;
	*(uint32 *)(queue->data+2*sizeof(uint32)+sizeof(deal_t)+2*KEY_LEN)=transaction->branch;
	if (device->node==NODE_LIGHT)//若是轻节点,则将交易传给第一重节点
	{
		if (route)
			queue_insert(&g_device[route->device_index],queue);
	}
	else//若是重节点,则更新给自己
		queue_insert(device,queue);
}

//STEP_TANGLE
void tangle_recv(device_t *device)
{
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
		transaction=transaction_generate(device);
		//send
		transaction_send(device,transaction);//pack & send device's route->queue


#if 0
		flag=transaction_seek(trunk,branch,device);
		if (!flag)
		{
			flag=transaction_verify(device,&device->tangle[trunk]);
			if (flag)
			{
				LeaveCriticalSection(&g_cs);
				break;
			}
			flag=transaction_verify(device,&device->tangle[branch]);
			if (flag)
			{
				LeaveCriticalSection(&g_cs);
				break;
			}
			pow[0]=transaction_pow(device,&device->tangle[trunk]);
			pow[1]=transaction_pow(device,&device->tangle[branch]);
		}
		else
			pow[0]=pow[1]=0;
		flag=transaction_generate(device,pow);
#endif
		break;
	case STEP_TANGLE:

		break;
	case STEP_MOVE:

		break;
	}
}













#if 0








uint8 transaction_generate(device_t *device,uint32 *pow)
{
	uint32 i;
	route_t *route;
	transaction_t transaction;

	if (device->transaction_index==TRANSACTION_LENGTH)//transaction full
		return RET_TRANSACTION_FULL;
	if (device->queue_index==QUEUE_LENGTH)//queue full
		return RET_QUEUE_FULL;
	//generate transaction
	transaction.device_index=device->device_index;
	g_index++;
	transaction.index.code=g_index;
	transaction.weight_self=0;
	transaction.weight_accu=0;
	transaction.height=0;
	transaction.depth=0;
	transaction.integral=0;
	transaction.type=TRANSACTION_VALUE;//rand()%2;
	transaction.flag=TRANSACTION_NONE;
	memset(&transaction.trunk,0,sizeof(hash_t));
	memset(&transaction.branch,0,sizeof(hash_t));
	_rand(transaction.plain,KEY_LEN);
	i=_mod(transaction.plain,transaction.plain,&device->pair[4],KEY_LEN,KEY_LEN);
	memset(&transaction.plain[i],0,KEY_LEN-i);
	if (transaction.type==TRANSACTION_TYPE_VALUE)
		transaction_signature(&transaction,device);
	memcpy(transaction.pow,pow,2*sizeof(uint32));
	device->queue[device->queue_index].device_index=device->device_index;
	device->queue[device->queue_index].info=INFO_TRANSACTION;
	memcpy((void *)device->queue[device->queue_index].buffer,&transaction,sizeof(transaction_t));
	//send transaction
	route=device->route;
	while(route)
	{
		if (g_device[route->device_index].queue_index==QUEUE_LENGTH)
		{
			route=route->next;
			continue;
		}
		g_device[route->device_index].queue[g_device[route->device_index].queue_index].device_index=device->device_index;
		g_device[route->device_index].queue[g_device[route->device_index].queue_index].info=INFO_TRANSACTION;
		memcpy((void *)g_device[route->device_index].queue[g_device[route->device_index].queue_index].buffer,(void *)device->queue[device->queue_index].buffer,sizeof(transaction_t));
		g_device[route->device_index].queue_index++;
		route=route->next;
	}
	device->queue_index++;

	return 0;
}

uint8 transaction_recv(device_t *device)
{
	uint32 i;

	if (!device->queue_index)//queue empty
		return RET_QUEUE_EMPTY;
	for (i=0;i<device->queue_index;i++)
		if (device->queue[i].info==INFO_TRANSACTION)
			break;
	if (i==device->queue_index)//no transaction in queue
		return RET_TRANSACTION_NONE;
	if (device->transaction_index==TRANSACTION_LENGTH)//transaction full
		return RET_TRANSACTION_FULL;
	//update transaction
	memcpy(&device->transaction[device->transaction_index],(void *)device->queue[i].buffer,sizeof(transaction_t));
	device->transaction_index++;
	//reset queue
	device->queue_index--;
	for (;i<device->queue_index;i++)
		memcpy(&device->queue[i],&device->queue[i+1],sizeof(queue_t));
	memset(&device->queue[device->queue_index],0,sizeof(queue_t));

	return 0;
}

uint8 tangle_join(device_t *device,uint32 trunk,uint32 branch)
{
	//将transaction队列的一项加入tangle队列
	uint32 i;
	route_t *route;
	
	if (!device->transaction_index)//transaction empty
		return RET_TRANSACTION_EMPTY;
	if (device->tangle_index==TANGLE_LENGTH)//tangle full
		return RET_TANGLE_FULL;
	//join tangle
	memcpy(&device->tangle[device->tangle_index],&device->transaction[0],sizeof(transaction_t));
	device->tangle[device->tangle_index].flag=TRANSACTION_TIP;
	if (device->tangle_index)
	{
		memcpy(&device->tangle[device->tangle_index].trunk,&device->tangle[trunk].index,sizeof(hash_t));
		memcpy(&device->tangle[device->tangle_index].branch,&device->tangle[branch].index,sizeof(hash_t));
		device->tangle[trunk].flag=TRANSACTION_TANGLE;
		device->tangle[branch].flag=TRANSACTION_TANGLE;
	}
	//reset transaction
	device->transaction_index--;
	for (i=0;i<device->transaction_index;i++)
		memcpy(&device->transaction[i],&device->transaction[i+1],sizeof(transaction_t));
	memset(&device->transaction[device->transaction_index],0,sizeof(transaction_t));
	//send tangle
	route=device->route;
	while(route)
	{
		if (g_device[route->device_index].queue_index==QUEUE_LENGTH)
		{
			route=route->next;
			continue;
		}
		g_device[route->device_index].queue[g_device[route->device_index].queue_index].device_index=device->device_index;
		g_device[route->device_index].queue[g_device[route->device_index].queue_index].info=INFO_TANGLE;
		memcpy((void *)g_device[route->device_index].queue[g_device[route->device_index].queue_index].buffer,&device->tangle[device->tangle_index],sizeof(transaction_t));
		g_device[route->device_index].queue_index++;
		route=route->next;
	}
	device->tangle_index++;

	return 0;
}

uint8 tangle_check(void)
{
	//检查各设备的tangle是否一致
	uint32 i,j,k,r;

	for (i=0;i<g_devicenum;i++)
		if (g_device[i].dag_index)
		{
			for (j=i+1;j<g_devicenum;j++)
				if (g_device[i].dag_index==g_device[j].dag_index)
				{
					r=math_min(g_device[i].tangle_index,g_device[j].tangle_index);
					for (k=0;k<r;k++)
						if (g_device[i].tangle[k].device_index!=g_device[j].tangle[k].device_index || memcmp(&g_device[i].tangle[k].index,&g_device[j].tangle[k].index,sizeof(hash_t)) || memcmp(&g_device[i].tangle[k].trunk,&g_device[j].tangle[k].trunk,sizeof(hash_t)) || memcmp(&g_device[i].tangle[k].branch,&g_device[j].tangle[k].branch,sizeof(hash_t)))
							return 1;
				}
		}

	return 0;
}

uint8 tangle_check(device_t *device,transaction_t *transaction)
{
	//检查tangle中是否有存在参照的transaction
	uint32 i;

	if (!device->tangle_index)//tangle empty
		return 0;
	for (i=0;i<device->tangle_index;i++)
		if (!memcmp((void *)&device->tangle[i].index,&transaction->index,sizeof(hash_t)))
			break;
	if (i==device->tangle_index)//no same transaction in tangle
		return 1;

	return 0;
}

uint8 tangle_recv(device_t *device)
{
	uint32 i;
	uint8 flag;

	if (!device->queue_index)//queue empty
		return RET_QUEUE_EMPTY;
	if (device->tangle_index==TANGLE_LENGTH)//tangle full
		return RET_TANGLE_FULL;
	for (i=0;i<device->queue_index;i++)
		if (device->queue[i].info==INFO_TANGLE)
			break;
	if (i==device->queue_index)//no tangle in queue
		return RET_TANGLE_NONE;
	//check tangle's existance
	flag=tangle_check(device,(transaction_t *)device->queue[i].buffer);
	if (flag)//no transaction in tangle
	{
		//update tangle
		memcpy(&device->tangle[device->tangle_index],(void *)device->queue[i].buffer,sizeof(transaction_t));
		device->tangle_index++;
	}
	//reset queue
	device->queue_index--;
	for (;i<device->queue_index;i++)
		memcpy(&device->queue[i],&device->queue[i+1],sizeof(queue_t));
	memset(&device->queue[device->queue_index],0,sizeof(queue_t));

	return 0;
}


//1.generate transaction by random(一种是输入签名,一种是传输信息). broadcast
//2.tangle join(search tip). broadcast
//3.transaction validation:if most of tip reference to old transactions, then solid.validation->broadcast->waiting for device's response,synch device's queue*
//4.ledger validation:check each solid transactions, then milestone. broadcast
while(1)
	{
		switch(task)
		{
		case 0://search tip/verify/generate transaction and broadcast
			EnterCriticalSection(&g_cs);
			flag=transaction_search(trunk,branch,device);
			if (!flag)
			{
				flag=transaction_verify(device,&device->tangle[trunk]);
				if (flag)
				{
					LeaveCriticalSection(&g_cs);
					break;
				}
				flag=transaction_verify(device,&device->tangle[branch]);
				if (flag)
				{
					LeaveCriticalSection(&g_cs);
					break;
				}
				pow[0]=transaction_pow(device,&device->tangle[trunk]);
				pow[1]=transaction_pow(device,&device->tangle[branch]);
			}
			else
				pow[0]=pow[1]=0;
			flag=transaction_generate(device,pow);
			//if (flag)
			//	print_return(device,task,flag);
			LeaveCriticalSection(&g_cs);
			break;
		case 1://recv transaction
			EnterCriticalSection(&g_cs);
			flag=transaction_recv(device);
			//if (flag)
			//	print_return(device,task,flag);
			LeaveCriticalSection(&g_cs);
			break;
		case 2://join tangle and broadcast
			EnterCriticalSection(&g_cs);
			flag=tangle_join(device,trunk,branch);
			//if (flag)
			//	print_return(device,task,flag);
			LeaveCriticalSection(&g_cs);
			break;
		case 3://recv tangle
			EnterCriticalSection(&g_cs);
			flag=tangle_recv(device);
			//if (flag)
			//	print_return(device,task,flag);
			LeaveCriticalSection(&g_cs);
			break;
		case 4:
			EnterCriticalSection(&g_cs);
			tangle_check();
			LeaveCriticalSection(&g_cs);
			break;
		}
		//EnterCriticalSection(&g_cs);
		//printf("thread_device%ld task=%d\r\n",device->device_index,task);
		//LeaveCriticalSection(&g_cs);
		task++;
		if (task==5)
			task=0;
		//Sleep(10);
		//while(1);
	}
#endif