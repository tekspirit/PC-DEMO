#pragma once

//include
#include "include.h"
#include "crypt_rsa.h"
#include "crypt_sha256.h"
//#include "crypt_hash.h"
//define
#define NODE_NONE 0
#define NODE_HEAVY 1
#define NODE_LIGHT 2

#define STEP_CONNECT 0
#define STEP_TRANSACTION 1
#define STEP_LEDGER 2
#define STEP_MOVE 3

#define KEY_LEN 4 //��Կ���ֽ���
#define KEY_E 4 //��Կe�ֽ���
#define KEY_MASK 0 //ָ��ä���ֽ���

//#define TRANSACTION_NONE 0 //δ����tip���У�δ����ַ��֤/�˱���֤
#define TRANSACTION_TIP 0 //�Ѽ���tip���У�δ����ַ��֤/�˱���֤
#define TRANSACTION_DAG 1 //�Ѽ���dag���У�������ַ��֤/�˱���֤
//#define TRANSACTION_MILESTONE 3 //

#define STATUS_DONE 0 //��֤ͨ��
#define STATUS_DEVICE 1 //��ַ��֤����
#define STATUS_LEDGER 2 //�˱���֤����
#define STATUS_SRC 3 //Դ��ַ
#define STATUS_DST 4 //Ŀ���ַ

#define TIMER_CONNECT 1 //��������ʱ��(�ؽڵ�->������)

//typedef
//struct
struct index_t
{
	uint32 number;//������������Ŀ
	uint32 *device_index;//�����������б�
	uint8 *key;//������������Կ
	uint32 *token;//�˻�����(0-��ʹ������,1-��������)
	uint8 *node;//0-�ؽڵ�,1-��ڵ�
};
struct deal_t
{
	uint32 device_index[2];//�豸����(������Ψһ������ַ).0-Դ�豸,1-Ŀ���豸
	uint32 token;//��������
};
struct spv_t
{
	//uint32 transaction;//����״̬.0-none,1-tip,2-dag
	uint32 index;//��������
	deal_t deal;//����ԭ��
	uint8 plain[KEY_LEN];//������֤
	uint8 cipher[KEY_LEN];//������֤
};
struct ledger_t
{
	uint32 status;//����״̬(��ȷ/����)
	uint32 index;//��������
	uint32 device_index;//�豸����(������Ψһ������ַ)
	uint32 token;//��������
};
struct transaction_t
{
	uint32 index;//��������
	deal_t deal;//����ԭ��
	uint8 plain[KEY_LEN];//������֤
	uint8 cipher[KEY_LEN];//������֤
	uint32 pow[2];//���������õ���ǰ��trunk/branch��powֵ
	//
	uint8 transaction;//����״̬.0-tip,1-dag
	uint8 flag;//dag:0-δ����,1-�Ѽ���.tip:0-��ȷ,1-����
	uint16 reserved;
	//uint8 type;//��������.0-��ͨ��Ϣ,1-�м���Ϣ
	//
	transaction_t *trunk;//�����׽ڵ�
	transaction_t *branch;//�ӽ��׽ڵ�
	transaction_t *next;//tip����ʹ��
};
struct key_t
{
	uint8 e[KEY_E];//��Կ
	uint8 n[KEY_LEN];//ģ��
};
struct list_t
{
	uint32 dag_index;//��������(Ĭ��Ϊ0,��1��ʼ��Ч)
	uint32 device_index;//�豸����
	key_t key;//��Կ
	uint32 token;//�˻�����
	uint8 node;//0-�ؽڵ�,1-��ڵ�
};
struct route_t
{
	uint8 flag;//0-δ����,1-����
	uint32 device_index;//�豸����(������Ψһ������ַ)
	//uint32 hops;//��Ծ���
	//uint32 *path;//·��·��
	key_t key;//��Կ
	uint32 token[2];//�˻�����(0-��ʹ������,1-��������)
	uint8 node;//0-�ؽڵ�,1-��ڵ�
	route_t *next;
};
struct queue_t
{
	volatile uint8 step;
	uint8 *data;
	queue_t *next;
};
struct mainchain_t
{
	queue_t *queue;//��Ϣ����
	uint32 list_number;//�ڵ���Ŀ
	uint32 dag_number;//������Ŀ
	list_t *list;//�ڵ������б�
	transaction_t *dag;//�˱�dag����(ȫ���˱�)
};
struct device_t
{
	uint32 x;
	uint32 y;
	uint8 node;//0-�ؽڵ�,1-��ڵ�
	uint32 device_index;//�豸����(������Ψһ������ַ)
	route_t *route;//�����豸·������
	queue_t *queue;//��Ϣ����
	rsa_t rsa;//��ǰ�豸�Ĺ�˽Կ��
	uint32 token[2];//�˻�����(0-��ʹ������,1-��������)
	//transaction_t *dag;//�˱�dag����(�ؽڵ�-�����˱�,��ڵ�-���˱�)
};
//function
void route_insert(device_t *device,route_t *route);
void route_delete(device_t *device);
uint8 route_node(device_t *device,uint32 device_index);
route_t *route_find(device_t *device,uint32 device_index);
void queue_insert(device_t *device,queue_t *queue);
void queue_insert(mainchain_t *mainchain,queue_t *queue);
void queue_delete(device_t *device);
void list_delete(mainchain_t *mainchain);
void transaction_insert(mainchain_t *mainchain,transaction_t *transaction);
void dag_delete(mainchain_t *mainchain,transaction_t *transaction);
uint32 dag_clear(transaction_t *transaction);
uint32 dag_tipnum(transaction_t *dag);
uint32 dag_dagnum(transaction_t *transaction);
uint32 dag_num(transaction_t *dag);
uint32 dag_tip(void);
void key_generate(device_t *device);
void move_location(device_t *device,uint32 step,uint32 range);
void delay(void);