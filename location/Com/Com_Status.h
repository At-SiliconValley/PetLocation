#ifndef __COM_STATUS_H__
#define __COM_STATUS_H__
#include "stdint.h"
#include "stdio.h"
#include "time.h"
#include "cJSON.h"
#include "string.h"
typedef enum
{
    COM_OK,      // 0
    COM_ERROR,   // 1
    COM_TIMEOUT, // 2
    COM_OTNHER,  // 3
} Com_Status_T;

// 定义一个结构体,结构体变量将来存储上报的数据
typedef struct
{
    // YYYY-MM-DD HH:MM:SS 2025-05-06 15:49:19
    char timeDate[20]; // 上报北京时间
    float lat;         // 上报纬度
    char latDir[2];    // 维度方向
    float lon;         // 上报经度
    char lonDir[2];    // 经度的方向
    float speed;       // 上报速度
    uint32_t step;     // 上报步数
    // 成员:用于存储最后的字符串形式的JSON数据
    char JSONData[512];
} Upload_Data_T;

// 对外暴露当前结构体变量
extern Upload_Data_T upload;

// 此方法处理UTC时间转换为北京时间
void Com_Util_utc2beijing(char *utc, char beijing[]);

// 此方法用于将结构体成员转换为JSON数据(字符串形式)
void Com_Util_UploadData2JsonString(Upload_Data_T *uploadData);

// 头文件:
// 预处理指令、函数原型、起别名、extern!!!!!!

#endif /* __COM_STATUS_H__ */
