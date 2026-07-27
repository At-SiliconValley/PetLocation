#ifndef __INT_LORA_H__
#define __INT_LORA_H__
#include "ebyte_core.h"
#include "ebyte_e220x.h"


//1.lora初始化方法
void Int_LoRa_Init(void);


//2.lora发送数据
void Int_Lora_Send(uint8e_t * buffers,uint8e_t sizes);

//3.lora接受数据
void Int_Lora_Receive(void);

#endif /* __INT_LORA_H__ */
