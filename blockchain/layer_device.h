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
/*
void route_mark(device_t *device,uint32 device_index);
route_t *route_check(device_t *device);
route_t *route_exist(device_t *device,uint32 index);
void device_recurse(device_t *device);
void device_location(device_t *device);
void device_release(device_t *device);
void print_status(void);
void print_route(void);
*/
void route_insert(device_t *device,route_t *route);
void route_delete(device_t *device);
void queue_insert(device_t *device,queue_t *queue);
void queue_insert(mainchain_t *mainchain,queue_t *queue);
void queue_delete(device_t *device);
void key_generate(device_t *device);
void device_recv(device_t *device);
void device_seek(device_t *device);
void device_send(device_t *device);
void process_device(device_t *device);
