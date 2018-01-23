#pragma once

//include
#include "layer.h"
//define
#define HASH_LEN 32 //哈希字节数
#define COMPARE_LEN 4 //比较位数
//typedef
//struct
//function
void connect_recv(mainchain_t *mainchain);
uint8 transaction_verify(mainchain_t *mainchain,transaction_t *transaction);
uint8 transaction_seek(transaction_t **trunk,transaction_t **branch,mainchain_t *mainchain);
uint32 transaction_pow(transaction_t *transaction);
void transaction_recv(mainchain_t *mainchain);
void transaction_send(mainchain_t *mainchain);
void process_mainchain(mainchain_t *mainchain);
