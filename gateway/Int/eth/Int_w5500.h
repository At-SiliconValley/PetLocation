#ifndef __INT_W5500_H__
#define __INT_W5500_H__
#include "wizchip_conf.h"
#include "Com_Delay.h"
#include "socket.h"
#include "Com_Debug.h"

//通道链路号【0-7】
#define SOCKET_ID 0
typedef enum
{
    COM_OK,
    COM_ERROR
} SOCKET_STATUS_T;

// 1.w5500初始化
void Int_W5500_Init(void);

// 2.持续判断当前通道是否与远程的服务器建立链接-----轮询调用
SOCKET_STATUS_T Int_W5500_GetSocketStatus(void);

//3.w5500发送数据方法
void Int_w5500_SendData(uint8_t *buffers,uint16_t sizes);
#endif /* __INT_W5500_H__ */