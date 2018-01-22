#include "layer_mainchain.h"

extern uint32 g_devicenum[2];//�豸����
extern uint32 g_devicerange;//�豸���귶Χ
extern uint32 g_devicestep;//�豸����ֵ
extern uint32 g_number;//����ԭ���б����
extern deal_t *g_deal;//����ԭ���б�
extern device_t *g_device;//�豸����
extern mainchain_t g_mainchain;//����
extern volatile uint32 g_index;//��ʱ����ͳ�ƽ��׺����(�Ժ����hash_t����,������1��ʼ)

//STEP_CONNECT
void connect_recv(mainchain_t *mainchain)
{
	//queue->list
	uint32 i,j;
	uint32 *dag;
	uint8 flag;
	queue_t *queue,*remove,*prev;
	index_t index;

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
		index.node=queue->data+(1+3*index.number)*sizeof(uint32)+index.number*(KEY_E+KEY_LEN);
		//update list
		for (i=0;i<index.number;i++)
		{
			for (j=0;j<mainchain->list_number;j++)
				if (index.index[i]==mainchain->list[j].device_index)
					break;
			if (j!=mainchain->list_number)//find it(δ���ֵ���ʱ��������)
			{
				mainchain->list[j].dag_index=mainchain->dag_number+1;
				memcpy(&mainchain->list[j].key,&index.key[i],KEY_E+KEY_LEN);
				mainchain->list[j].node=index.node[i];
			}
		}
		//compute dag_number
		mainchain->dag_number=0;
		for (i=0;i<mainchain->list_number;i++)
		{
			if (!mainchain->list[i].dag_index)
				continue;
			for (j=0;j<mainchain->dag_number;j++)
				if (mainchain->list[i].dag_index==dag[j])
					break;
			if (j==mainchain->dag_number)
			{
				dag=mainchain->dag_number ? (uint32 *)realloc(dag,(mainchain->dag_number+1)*sizeof(uint32)) : (uint32 *)malloc((mainchain->dag_number+1)*sizeof(uint32));
				dag[mainchain->dag_number++]=mainchain->list[i].dag_index;
			}
		}
		//modify dag_index
		for (i=0;i<mainchain->list_number;i++)
		{
			if (!mainchain->list[i].dag_index)
				continue;
			for (j=0;j<mainchain->dag_number;j++)
				if (mainchain->list[i].dag_index==dag[j])
					break;
			mainchain->list[i].dag_index=j+1;
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
	//������֤��ʹ��rsa��Կ��ǩ��֤���׵�ַ����֤�����˱���0-��ȷ,1-���׵�ַ����,2-�����˱�����
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
	//��ַ��֤
	rsa.e=new uint8[rsa.le];
	rsa.n=new uint8[rsa.len];
	memcpy(rsa.e,mainchain->list[i].key.e,KEY_E);
	memcpy(rsa.n,mainchain->list[i].key.n,KEY_LEN);
	i=rsa_enc(result,transaction->cipher,rsa.len,&rsa);
	memset(&result[i],0,rsa.len-i);
	if (memcmp(result,transaction->plain,rsa.len))
		return STATUS_DEVICE;
	//�˱���֤
	if (transaction->deal.token>mainchain->list[i].token[0])
		return STATUS_LEDGER;

	return STATUS_DONE;
}

uint8 transaction_seek(transaction_t *trunk,transaction_t *branch,mainchain_t *mainchain)
{
	//search 2 tips(random algorithm):0-dagΪ��,1-dag�ǿ�.ԭ������Ҫʹ���ֶ�������Ͽ��tangle(�ж�tip),������ʹ���Զ�����tangle(�ж�solid).
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
		if (!transaction->flag)//��ȷ��tip
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
	//ͨ��sha256����hash pow
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