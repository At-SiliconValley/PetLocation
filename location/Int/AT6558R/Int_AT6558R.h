#ifndef __INT_AT6558R_H__
#define __INT_AT6558R_H__
#include "gpio.h"
#include "stdio.h"
#include "Com_Debug.h"
#include "string.h"
#include "usart.h"
//1.初始化方法
void Int_AT6558R_Init(void);
//2.封装一个函数,用于通过串口给AT6559R发送命令
void Int_AT6558R_SendCMD(uint8_t * cmd);
//3.次函数用于获取到GPS相关数据
void Int_AT6558R_GetGPS(uint8_t * receive_buffers,uint16_t * length);


//功耗相关的函数
//1.进入低功耗方法
void Int_AT6558R_EnterLowPower(void);

//2.唤醒
void Int_AT6558R_LeaveLowPower(void);

#endif /* __INT_AT6558R_H__ */
