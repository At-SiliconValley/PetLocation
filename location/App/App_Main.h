#ifndef __APP_MAIN_H__
#define __APP_MAIN_H__
#include "Int_DS3553.h"
#include "Int_AT6558R.h"
#include "Int_QS100.h"
#include "Com_Status.h"
#include "rtc.h"
#include "Int_Lora.h"
/**
 * @brief
 *  1.应用层的目的:获取到定位器步数、获取GPS数据整理成JSON数据格式
 *  2.通过QS100芯片上报云服务器
 */
// 1.初始化方法
void App_Main_Init(void);

// 2.应用实现上报数据
void App_Main_UploadData2Server(void);


//3.进入低功耗
void App_Main_EnterLowPower(void);

//退出低功耗
void App_Main_LeaveLowPower(void);

#endif /* __APP_MAIN_H__ */
