#include "Int_DS3553.h"
// 0.读取寄存器的方法
static uint8_t Int_DS3553_ReadRegister(uint16_t reg_addr)
{
    uint8_t r_byte = 0x00;
    // 通知之前需要将CS引脚拉低,至少位置3ms,才能开始通信
    CS_L;
    Com_Delay_Ms(3);
    HAL_I2C_Mem_Read(&hi2c1, DEVICE_READ, reg_addr, I2C_MEMADD_SIZE_8BIT, &r_byte, 1, 1000);

    // 通信完成以后,CS引脚拉高,表示通信结束!
    CS_H;
    Com_Delay_Ms(10);
    return r_byte;
}

// 1.写入寄存器方法
// 五个寄存器,本质上只有USER_SET寄存器能写入的,其余只读的!
static void Int_DS3552_WriteRegister(uint16_t reg_addr, uint8_t mask, uint8_t status)
{
    // 1.先将USER_SET寄存器默认初始化读取出来
    uint8_t reg_value = Int_DS3553_ReadRegister(reg_addr);

    if (status)
    {
        reg_value |= mask;
    }
    else
    {
        reg_value &= ~mask;
    }

    // 通信之前:CS引脚拉低、至少位置3ms
    CS_L;
    Com_Delay_Ms(3);
    HAL_I2C_Mem_Write(&hi2c1, DEVICE_WRITE, reg_addr, I2C_MEMADD_SIZE_8BIT, &reg_value, 1, 1000);
    // 通信节后以后CS引脚拉高,至少位置10ms
    CS_H;
    Com_Delay_Ms(10);
}

// 1.初始化方法
void Int_DS3553_Init(void)
{

    // 1.当DS3553初始化的时候,相应的把USER_SET寄存器的PULSE_EN这一位清零。(不需要向外输出步数脉冲,读取寄存器方式获取步数)
    Int_DS3552_WriteRegister(USER_SET, PULSE_EN, 0);

    // 2.测试代码,查看一下USER_SET寄存器最新的数值
    uint8_t test = Int_DS3553_ReadRegister(USER_SET);
    COM_DEBUG_LN("%#02x", test);
    /**********************************************/
}

// 2.读取牛马步数
uint32_t Int_DS3553_GetStepNumber(void)
{
    // 牛马的步数结果
    uint32_t step = 0;

    step |= Int_DS3553_ReadRegister(STEP_CNT_L);
    step |= (Int_DS3553_ReadRegister(STEP_CNT_M) << 8);
    step |= (Int_DS3553_ReadRegister(STEP_CNT_H) << 16);

    return step;
}
