#pragma once

//include
#include "include.h"
//#include "crypt_hash.h"
//define
#define BUFFER_LENGTH 0x100

#define NODE_HEAVY 0
#define NODE_LIGHT 1

#define STATUS_FREE 0
#define STATUS_MASTER 1
#define STATUS_SLAVE 2

#define TIMER_CONNECT 1 //组网更新时间(重节点向服务器)
//enum
typedef enum step
{
	STEP_INITIAL,STEP_CONNECT,STEP_MERGE,STEP_OPTIMIZE,STEP_INDEXDAG,STEP_TANGLE
};
//typedef
//struct
struct msg_t
{
	uint8 type;

};
struct route_t
{
	uint8 flag;//0-未连接,1-连接
	uint32 device_index;//终设备索引(类似于唯一物理地址)
	uint32 hops;//跳跃间隔
	uint32 *path;//路由路径
	route_t *next;
};
struct index_t
{
	uint32 *index;//设备索引列表
	uint32 number;//设备索引数目
	index_t *next;
};
struct device_t
{
	//device
	uint32 x;
	uint32 y;
	uint8 node;//0-主链/重节点(包含全局账本或区域账本),1-轻节点(无账本)
	//uint8 line;//0-在线,1-掉线
	uint32 device_index;//设备索引(类似于唯一物理地址)
	volatile uint8 step;
	uint8 buffer[BUFFER_LENGTH];//数据接收缓冲区




	uint8 status;//0-作为free,1-作为master,2-作为slave
	route_t *route;//连接设备路由链表

	//index_t *index;//设备索引链表
	//dag
	volatile uint32 dag_index;//子链索引.0-孤立点,其他-索引号


	/*

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
	key_t key[DEVICE_LENGTH];//key公钥队列
	volatile uint32 key_index;//key索引长度
	uint8 pair[300*4];//密钥对rsa=1(e)+64(n)+64(d)+32(p)+32(q)+32(dp)+32(dq)+32(qp)=289 word
	//application
	uint32 account_id;//账户id
	uint32 account_money;//账户现金
	*/
};
//function
