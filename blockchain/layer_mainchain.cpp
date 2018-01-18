#include "layer_mainchain.h"

extern uint32 g_devicenum[2];//设备个数
extern uint32 g_devicerange;//设备坐标范围
extern uint32 g_devicestep;//设备步进值
extern uint32 g_dealnumber;//交易原子列表个数
extern uint32 g_dealindex;//交易原子列表索引
extern deal_t *g_deal;//交易原子列表
extern device_t *g_device;//设备数组
extern mainchain_t g_mainchain;//主链
extern volatile uint32 g_index;//临时用来统计交易号码的(以后会用hash_t代替,计数从1开始)

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
		index.token=(uint32 *)(queue->data+(1+index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN));
		index.node=queue->data+(1+2*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN);
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
				memcpy(list[k].token,mainchain->list[j].token,2*sizeof(uint32));
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
uint8 transaction_verify(mainchain_t *mainchain,transaction_t *transaction)
{
	//交易验证：使用rsa公钥验签验证交易地址，验证交易账本。0-正确,1-交易地址错误,2-交易账本错误
	uint32 i;
	rsa_t rsa;
	uint8 result[KEY_LEN];

	//get device
	rsa.le=KEY_E;
	rsa.len=KEY_LEN;
	for (i=0;i<mainchain->list_number;i++)
	{
		if (mainchain->list[i].device_index==transaction->deal.device_index[0])
			break;
	}
	//地址验证
	rsa.e=new uint8[rsa.le];
	rsa.n=new uint8[rsa.len];
	memcpy(rsa.e,mainchain->list[i].key.e,KEY_E);
	memcpy(rsa.n,mainchain->list[i].key.n,KEY_LEN);
	i=rsa_enc(result,transaction->cipher,rsa.len,&rsa);
	memset(&result[i],0,rsa.len-i);
	if (memcmp(result,transaction->plain,rsa.len))
		return STATUS_DEVICE;
	//账本验证
	if (transaction->deal.token>mainchain->list[i].token[0])
		return STATUS_LEDGER;

	return STATUS_DONE;
}

uint8 transaction_seek(transaction_t *trunk,transaction_t *branch,mainchain_t *mainchain)
{
	//search 2 tips(random algorithm):0-dag为空,1-dag非空.原则上需要使用手动构造出较宽的tangle(判断tip),这里我使用自动构造tangle(判断solid).
	uint32 i,j,k;
	transaction_t *transaction;

	if (!mainchain->dag)//no genesis
		return 1;
	i=dag_tipnum(mainchain->dag);
	if (!i)//no correct tip or no genesis
		return 1;
	if (i==1)
	{
		j=0;
		k=0;
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
	}
	i=0;
	transaction=mainchain->dag;
	while(transaction)
	{
		if (!transaction->flag)//正确的tip
		{
			if (j==i)
				trunk=transaction;
			if (k==i)
				branch=transaction;
			i++;
		}
		transaction=transaction->next;
	}

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

void transaction_recv(mainchain_t *mainchain)
{
	//queue->dag
	uint8 flag;
	uint32 pow[2];
	transaction_t *trunk,*branch,*transaction;
	queue_t *queue,*insert,*prev;

	queue=mainchain->queue;
	while(queue)
	{
		if (queue->step==STEP_TRANSACTION)
		{
			if (queue->data)
			{
				//search error tip
				transaction=mainchain->dag;
				while(transaction)
				{
					if (transaction->flag)
					{
						transaction->index=*(uint32 *)queue->data;
						memcpy(&transaction->deal,(uint32 *)(queue->data+1*sizeof(uint32)),sizeof(deal_t));
						memcpy(transaction->plain,(uint32 *)(queue->data+1*sizeof(uint32)+sizeof(deal_t)),KEY_LEN);
						memcpy(transaction->cipher,(uint32 *)(queue->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN),KEY_LEN);
						transaction->transaction=TRANSACTION_TIP;
						transaction->flag=0;
						goto next;
					}
					transaction=transaction->next;
				}
				//find 2 tips & verify them(address/ledger), calculate pow
				do
				{
					trunk=branch=NULL;
					flag=transaction_seek(trunk,branch,mainchain);
					if (!flag)//dag exist
					{
						flag=transaction_verify(mainchain,trunk);
						if (flag)
						{
							trunk->flag=1;
							//notify device
							insert=new queue_t;
							insert->step=STEP_LEDGER;
							insert->data=new uint8[sizeof(ledger_t)];
							*(uint32 *)insert->data=flag;
							*(uint32 *)(insert->data+1*sizeof(uint32))=trunk->index;
							*(uint32 *)(insert->data+2*sizeof(uint32))=trunk->deal.token;
							queue_insert(&g_device[trunk->deal.device_index[0]],insert);
							break;
						}
						flag=transaction_verify(mainchain,branch);
						if (flag)
						{
							branch->flag=1;
							//notify device
							insert=new queue_t;
							insert->step=STEP_LEDGER;
							insert->data=new uint8[sizeof(ledger_t)];
							*(uint32 *)insert->data=flag;
							*(uint32 *)(insert->data+1*sizeof(uint32))=branch->index;
							*(uint32 *)(insert->data+2*sizeof(uint32))=branch->deal.token;
							queue_insert(&g_device[branch->deal.device_index[0]],insert);
							break;
						}
						pow[0]=transaction_pow(trunk);
						pow[1]=transaction_pow(branch);
					}
					else//no genesis
						pow[0]=pow[1]=0;
				}while(0);
				if (!flag)//correct
				{
					//add in tip
					transaction=new transaction_t;			
					transaction->index=*(uint32 *)queue->data;
					memcpy(&transaction->deal,(uint32 *)(queue->data+1*sizeof(uint32)),sizeof(deal_t));
					memcpy(transaction->plain,(uint32 *)(queue->data+1*sizeof(uint32)+sizeof(deal_t)),KEY_LEN);
					memcpy(transaction->cipher,(uint32 *)(queue->data+1*sizeof(uint32)+sizeof(deal_t)+KEY_LEN),KEY_LEN);
					memcpy(transaction->pow,pow,2*sizeof(uint32));
					transaction->transaction=TRANSACTION_TIP;
					transaction->flag=0;
					transaction->trunk=trunk;
					transaction->branch=branch;
					transaction_insert(mainchain,transaction);
					//add trunk/branch in dag
					dag_insert(mainchain,trunk);
					dag_insert(mainchain,branch);
					//notify device
					insert=new queue_t;
					insert->step=STEP_LEDGER;
					insert->data=new uint8[sizeof(ledger_t)];
					*(uint32 *)insert->data=STATUS_DONE;
					*(uint32 *)(insert->data+1*sizeof(uint32))=trunk->index;
					*(uint32 *)(insert->data+2*sizeof(uint32))=trunk->deal.token;
					queue_insert(&g_device[trunk->deal.device_index[0]],insert);
					queue_insert(&g_device[trunk->deal.device_index[1]],insert);
					//notify device
					insert=new queue_t;
					insert->step=STEP_LEDGER;
					insert->data=new uint8[sizeof(ledger_t)];
					*(uint32 *)insert->data=STATUS_DONE;
					*(uint32 *)(insert->data+1*sizeof(uint32))=branch->index;
					*(uint32 *)(insert->data+2*sizeof(uint32))=branch->deal.token;
					queue_insert(&g_device[branch->deal.device_index[0]],insert);
					queue_insert(&g_device[branch->deal.device_index[1]],insert);
				}
			}
next:
			//queue delete
			if (queue==mainchain->queue)
			{
				mainchain->queue=queue->next;
				if (queue->data)
				{
					delete[] queue->data;
					queue->data=NULL;
				}
				delete queue;
				queue=mainchain->queue;
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

void transaction_send(mainchain_t *mainchain)
{
	//transaction->queue
	queue_t *queue;

	queue=new queue_t;
	queue->step=STEP_TRANSACTION;
	queue->data=NULL;
	queue_insert(mainchain,queue);
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
		//send
		transaction_send(mainchain);//pack & send mainchain's route->queue
		break;
	}
}