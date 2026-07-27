#include "Int_AT6558R.h"
// 1.主备一个缓冲区,接受AT6558R返回的卫星的数据
#define MAXSIZE 2048
uint8_t gps_buffers[MAXSIZE];
uint16_t gps_length;

// AT6558R提供一个方法,方法作用是当接受变长数据的中断触发的时候,次函数执行一次!
void Int_AT6558T_GetSize(uint16_t sizes)
{

    // 存储每一次接受到数据的长度
    gps_length = sizes;
    HAL_StatusTypeDef status = HAL_BUSY;
    while (status != HAL_OK)
    {
        // 这种写法中断服务程序能保证开启成功一次!
        status = HAL_UARTEx_ReceiveToIdle_IT(&huart2, gps_buffers, MAXSIZE);
    }
}

// 发送命令方法
void Int_AT6558R_SendCMD(uint8_t *cmd)
{
    // 准备一个字符数组:存放最终计算完毕命令
    uint8_t sendData[20] = {0};
    // 1. cmd:PCAS01,1
    uint8_t check_sum = cmd[0];
    // 循环计算
    for (uint8_t i = 1; cmd[i] != '\0'; i++)
    {
        check_sum ^= cmd[i];
    }
    // 2.check_sum计算完毕

    // 3.拼凑出完成要发送命令
    sprintf((char *)sendData, "$%s*%2X", cmd, check_sum);

    // 4.发送命令
    HAL_UART_Transmit(&huart2, sendData, strlen((char *)sendData), 1000);
}
// 1.初始化方法
void Int_AT6558R_Init(void)
{
    // 1.让主控芯片PB3引脚拉高,目的是让LDO可以给AT6558R功3V点,让他正常工作
    HAL_GPIO_WritePin(GPS_EN_GPIO_Port, GPS_EN_Pin, GPIO_PIN_SET);

    // 2.给AT6558R发送一些命令:告诉1s给定为几次、告诉工作波特率修改、告诉他要的是谁家卫星的数据!
    // 设置波特率:9600
    Int_AT6558R_SendCMD("PCAS01,1");
    // 设置更新频率:1S四次定为
    Int_AT6558R_SendCMD("PCAS02,250");
    // 设置获取谁家卫星的数据:北斗
    Int_AT6558R_SendCMD("PCAS04,2");

    // 3.执行到这里的时候,AT6558RGPS芯片其实已经可以获取到卫星的数据,但是需要注意.
    // 主控芯片C8T6需要获取到GPS数据,整理好,上报服务器!

    HAL_StatusTypeDef status = HAL_BUSY;
    while (status != HAL_OK)
    {
        // 这种写法中断服务程序能保证开启成功一次!
        status = HAL_UARTEx_ReceiveToIdle_IT(&huart2, gps_buffers, MAXSIZE);
    }
}

// 此方法用于获取GPS数据
void Int_AT6558R_GetGPS(uint8_t *receive_buffers, uint16_t *length)
{
    // 清空对应外部数组内容
    memset(receive_buffers, 0, strlen((char *)receive_buffers));
    *length = 0;

    // 判断,判断最新的GPS获取到
    if (gps_length > 0)
    {
        memcpy(receive_buffers, gps_buffers, strlen((char *)gps_buffers));
        *length = gps_length;

        // 清除卫星的数据,接受最新的gps数据
        memset(gps_buffers, 0, strlen((char *)gps_buffers));
        gps_length = 0;
    }
}

// 1.进入低功耗方法
void Int_AT6558R_EnterLowPower(void)
{
    HAL_GPIO_WritePin(GPS_EN_GPIO_Port, GPS_EN_Pin, GPIO_PIN_RESET);
}

// 2.唤醒
void Int_AT6558R_LeaveLowPower(void)
{
    HAL_GPIO_WritePin(GPS_EN_GPIO_Port, GPS_EN_Pin, GPIO_PIN_SET);
}
