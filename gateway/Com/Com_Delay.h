#ifndef __COM_DELAY_H__
#define __COM_DELAY_H__
#include "stdint.h"
#include "main.h"
/*1.在嵌入式项目当中,延迟的操作必然无处不在.HAL库提供的延迟(HAL_Delay,底层使用的Systick)
  2.HAL_Delay只能做到ms级别,如果你的项目,出现us级别的延迟。只能自己封装us级别的延迟!

  3.今晚需要做的事情?
     今晚STM32学习的定时器,今晚稍微复习一下! 后期电机motor项目经常使用!
     今晚C语言指针(函数指针、指针函数、数组指针、指针数组)、结构体稍微看一下！
     今晚函数,回调函数!
*/

// ms级别的延迟
void Com_Delay_Ms(uint16_t ms);
// s级别的延迟
void Com_Delay_Sec(uint16_t second);

#endif /* __COM_DELAY_H__ */
