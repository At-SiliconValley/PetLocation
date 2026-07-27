#include "Int_Lora.h"

// 初始化方法
void Int_LoRa_Init(void)
{
    // 1.初始化lora
    Ebyte_RF.Init();
    // 设置同步字,确定双方身份
    Ebyte_E220x_SetLoraSyncWord(0x1234);
}

// 发送数据
void Int_Lora_Send(uint8e_t *buffers, uint8e_t sizes)
{
    // 参数:发送数据 + 数据个数 + 超时的时间
    Ebyte_RF.Send(buffers, sizes, 0);
}

// lora芯片接收数据:接受数据的时候,需要让此方法轮询调用
// 当接收到数据的时候,对应接受成功的回调函数会执行!
void Int_Lora_Receive(void)
{
    Ebyte_RF.StartPollTask();
}
