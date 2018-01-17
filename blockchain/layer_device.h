#pragma once

//include
#include "layer.h"
//define
#define MAX_MASTER_SLAVE 5 //maybe 7
#define MIN_MASTER_SLAVE 3

#define MAX_METRIC 20 //蓝牙设备间连接距离最大半径10m(不考虑适配器问题)

#define MAX_MESH_DEVICE 10 //一条mesh中的最大设备数(light_device+heavy_device),mesh网络可以连接超过65000个设备
#define MIN_MESH_HEAVYDEVICE 1 //一条mesh中的最小重设备数(heavy_device)
//typedef
//struct
//function
void connect_recv(device_t *device);
void connect_seek(device_t *device);
void connect_send(device_t *device);
uint8 transaction_verify(device_t *device,transaction_t *transaction);
void transaction_recv(device_t *device);
void transaction_signature(transaction_t *transaction,device_t *device);
transaction_t *transaction_generate(device_t *device);
void transaction_send(device_t *device,transaction_t *transaction);
void ledger_recv(device_t *device);
void ledger_send(device_t *device);
void process_device(device_t *device);