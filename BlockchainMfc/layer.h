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

#define KEY_LEN 4 //密钥对字节数
#define KEY_E 4 //公钥e字节数
#define KEY_MASK 0 //指数盲化字节数

//#define TRANSACTION_NONE 0 //未加入tip队列，未做地址验证/账本验证
#define TRANSACTION_TIP 0 //已加入tip队列，未做地址验证/账本验证
#define TRANSACTION_DAG 1 //已加入dag队列，已做地址验证/账本验证
//#define TRANSACTION_MILESTONE 3 //

#define STATUS_DONE 0 //验证通过
#define STATUS_DEVICE 1 //地址验证错误
#define STATUS_LEDGER 2 //账本验证错误
#define STATUS_SRC 3 //源地址
#define STATUS_DST 4 //目标地址

#define TIMER_CONNECT 1 //组网更新时间(重节点->服务器)

//typedef
//struct
struct index_t
{
	uint32 number;//待处理索引数目
	uint32 *device_index;//待处理索引列表
	uint8 *key;//待处理索引公钥
	uint32 *token;//账户数额(0-可使用数额,1-冻结数额)
	uint8 *node;//0-重节点,1-轻节点
};
struct deal_t
{
	uint32 device_index[2];//设备索引(类似于唯一物理地址).0-源设备,1-目标设备
	uint32 token;//交易数额
};
struct spv_t
{
	//uint32 transaction;//交易状态.0-none,1-tip,2-dag
	uint32 index;//交易索引
	deal_t deal;//交易原子
	uint8 plain[KEY_LEN];//明文验证
	uint8 cipher[KEY_LEN];//密文验证
};
struct ledger_t
{
	uint32 status;//交易状态(正确/错误)
	uint32 index;//交易索引
	uint32 device_index;//设备索引(类似于唯一物理地址)
	uint32 token;//交易数额
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
struct key_t
{
	uint8 e[KEY_E];//公钥
	uint8 n[KEY_LEN];//模数
};
struct list_t
{
	uint32 dag_index;//区域索引(默认为0,从1开始有效)
	uint32 device_index;//设备索引
	key_t key;//公钥
	uint32 token;//账户数额
	uint8 node;//0-重节点,1-轻节点
};
struct route_t
{
	uint8 flag;//0-未连接,1-连接
	uint32 device_index;//设备索引(类似于唯一物理地址)
	//uint32 hops;//跳跃间隔
	//uint32 *path;//路由路径
	key_t key;//公钥
	uint32 token[2];//账户数额(0-可使用数额,1-冻结数额)
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
	uint32 list_number;//节点数目
	uint32 dag_number;//区域数目
	list_t *list;//节点属性列表
	transaction_t *dag;//账本dag链表(全局账本)
};
struct device_t
{
	uint32 x;
	uint32 y;
	uint8 node;//0-重节点,1-轻节点
	uint32 device_index;//设备索引(类似于唯一物理地址)
	route_t *route;//连接设备路由链表
	queue_t *queue;//消息队列
	rsa_t rsa;//当前设备的公私钥对
	uint32 token[2];//账户数额(0-可使用数额,1-冻结数额)
	//transaction_t *dag;//账本dag链表(重节点-区域账本,轻节点-无账本)
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