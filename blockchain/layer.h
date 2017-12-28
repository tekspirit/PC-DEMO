#pragma once

//include
#include "include.h"
#include "crypt_rsa.h"
//#include "crypt_hash.h"
//define
#define NODE_HEAVY 0
#define NODE_LIGHT 1

#define STEP_CONNECT 0
#define STEP_TRANSACTION 1
#define STEP_TANGLE 2
#define STEP_MOVE 3

#define KEY_LEN 4 //密钥对字节数
#define KEY_E 4 //公钥e字节数
#define KEY_MASK 0 //指数盲化字节数

#define TRANSACTION_NORMAL 0 //普通交易
#define TRANSACTION_VALUE 1 //有价交易(需账本验证)
/*
#define TAG_NONE 0 //未交易认证transaction
#define TRANSACTION_TIP 1 //已链入tangle成为tip
#define TRANSACTION_TANGLE 2 //已链入tangle并被其他交易指引
#define TRANSACTION_SOLID 3 //已交易认证transaction
#define TRANSACTION_MILESTONE 4 //已账本验证milestone
//#define QUEUE_LENGTH 0x20
*/
#define TIMER_CONNECT 1 //组网更新时间(重节点向服务器)
//typedef
//struct
struct index_t
{
	uint32 number;//待处理索引数目
	uint32 *index;//待处理索引列表
	uint8 *key;//待处理索引公钥
	uint8 *node;//0-重节点,1-轻节点
};
struct key_t
{
	uint8 e[KEY_E];//公钥
	uint8 n[KEY_LEN];//模数
};
struct list_t
{
	uint32 dag_index;//区域索引
	uint32 device_index;//设备索引
	key_t key;//公钥
	uint8 node;//0-重节点,1-轻节点
};
struct route_t
{
	uint8 flag;//0-未连接,1-连接
	uint32 device_index;//终设备索引(类似于唯一物理地址)
	//uint32 hops;//跳跃间隔
	//uint32 *path;//路由路径
	key_t key;//公钥
	uint8 node;//0-重节点,1-轻节点
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
	queue_t *queue;//消息队列
	uint32 dag_number;//区域数目
	uint32 list_number;//节点数目
	list_t *list;//节点属性列表
};
struct device_t
{
	uint32 x;
	uint32 y;
	uint8 node;//0-重节点(区域账本),1-轻节点(无账本)
	uint32 device_index;//设备索引(类似于唯一物理地址)
	route_t *route;//连接设备路由链表
	queue_t *queue;//消息队列
	rsa_t rsa;//当前设备的公私钥对
/*
	//index_t *index;//设备索引链表
	//dag
	volatile uint32 dag_index;//子链索引.0-孤立点,其他-索引号
	uint8 status;//0-作为free,1-作为master,2-作为slave
	route_t *route;//连接设备路由链表
	//queue_t queue[QUEUE_LENGTH];//设备的消息队列(发送/接收)
	uint32 queue_index;//queue索引长度
	//dag
	volatile uint32 dag_index;//子链索引.0-孤立点,其他-索引号
	transaction_t tangle[TANGLE_LENGTH];//主tangle队列
	volatile uint32 tangle_index;//主tangle索引长度
	transaction_t transaction[TRANSACTION_LENGTH];//transaction队列
	volatile uint32 transaction_index;//transaction索引长度
	//application
	uint32 account_id;//账户id
	uint32 account_money;//账户现金
	*/
};
//function
void route_insert(device_t *device,route_t *route);
void route_delete(device_t *device);
void queue_insert(device_t *device,queue_t *queue);
void queue_insert(mainchain_t *mainchain,queue_t *queue);
void queue_delete(device_t *device);
void list_delete(mainchain_t *mainchain);
void key_generate(device_t *device);
