#ifndef __INT_DS3553_H__
#define __INT_DS3553_H__
#include "i2c.h"
#include "Com_Delay.h"
#include "gpio.h"
#include "Com_Debug.h"
// 从机地址+读写标识
#define DEVICE_READ 0x4F
#define DEVICE_WRITE 0x4E

// DS3553五个寄存器的地址
#define CHIP_ID 0x01
#define USER_SET 0xC3
#define STEP_CNT_L 0xC4
#define STEP_CNT_M 0xC5
#define STEP_CNT_H 0xC6

//USER_SET对应八位数值掩码
#define PEDO_0 (1<<0)
#define PEDO_1 (1<<1)
#define CLEAR_EN (1<<2)
#define NOISE_DIS (1<<3)
#define PULSE_EN (1<<4)
#define RAISE_EN (1<<5)
#define sen_DIS (1<<6)
#define PWR_MOD (1<<7)

// PB5引脚,控制DS3553使能开关引脚
#define CS_H (HAL_GPIO_WritePin(DS3553_EN_GPIO_Port, DS3553_EN_Pin, GPIO_PIN_SET))
#define CS_L (HAL_GPIO_WritePin(DS3553_EN_GPIO_Port, DS3553_EN_Pin, GPIO_PIN_RESET))
// 1.初始化方法
void Int_DS3553_Init(void);
// 3.读取牛马的步数
uint32_t Int_DS3553_GetStepNumber(void);
#endif /* __INT_DS3553_H__ */
