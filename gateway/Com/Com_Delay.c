#include "Com_Delay.h"

// ms级别的延迟
void Com_Delay_Ms(uint16_t ms)
{
    HAL_Delay(ms);
}

// s级别的延迟
void Com_Delay_Sec(uint16_t second)
{

    while (second--)
    {
        Com_Delay_Ms(1000);
    }
}
