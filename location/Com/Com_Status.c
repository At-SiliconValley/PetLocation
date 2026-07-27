#include "Com_Status.h"

// 定义一个结构体变量
Upload_Data_T upload;

// 字符串格式化时间: 2021-01-01 00:00:00
/**
 * @brief 将UTC时间字符串转换为北京时间字符串
 *
 * 该函数接收一个UTC时间格式的字符串，将其转换为时间戳，
 * 然后加上8小时（北京时间比UTC时间快8小时），
 * 最后将得到的时间戳转换为北京时间格式的字符串。
 *
 * @param utc 指向UTC时间字符串的指针，格式为 "YYYY-MM-DD HH:MM:SS"
 * @param beijing 用于存储转换后的北京时间字符串的字符数组
 */

//2025-05-06 15:49:19
void Com_Util_utc2beijing(char *utc, char beijing[]) // to =>two =>2  json4c for=>four=>4  i18n
{
    /* 1. 先utc字符串时间变成时间戳 */
    // 初始化一个tm结构体，用于存储解析后的UTC时间
    struct tm utcTm = {0};
    // 从utc字符串中解析出年、月、日、时、分、秒，并存储到utcTm结构体中
    sscanf(utc,
           "%d-%d-%d %d:%d:%d",
           &utcTm.tm_year, //
           &utcTm.tm_mon,
           &utcTm.tm_mday,
           &utcTm.tm_hour,
           &utcTm.tm_min,
           &utcTm.tm_sec);

    // tm_year字段从1900年开始计数，所以需要减去1900
    utcTm.tm_year -= 1900; // 1900 表示 0 => 2004-1900 = 124
    // tm_mon字段从0开始计数，所以需要减去1
    utcTm.tm_mon -= 1;

    // 将utcTm结构体转换为时间戳，单位为秒
    time_t _utc = mktime(&utcTm); // 时间戳 单位: s
    /* 2. 时间 + 8个小时 */
    // 北京时间比UTC时间快8小时，所以在UTC时间戳上加上8小时的秒数
    time_t _beijing = _utc + 8 * 3600;

    /* 3. 变成北京时间字符串 */
    // 将北京时间的时间戳转换为本地时间结构体
    struct tm *beijingTm = localtime(&_beijing);
    // 将beijingTm结构体中的时间信息格式化为字符串，并存储到beijing数组中
    sprintf(beijing,
            "%04d-%02d-%02d %02d:%02d:%02d",
            beijingTm->tm_year + 1900,
            beijingTm->tm_mon + 1,
            beijingTm->tm_mday,
            beijingTm->tm_hour,
            beijingTm->tm_min,
            beijingTm->tm_sec);
}




/**
 * @brief 将UploadDataType结构体数据转换为JSON字符串并存储在结构体的jsonData成员中
 *
 * 该函数使用cJSON库将UploadDataType结构体中的数据转换为JSON格式的字符串，
 * 并将该字符串存储在结构体的jsonData成员中。最后释放cJSON对象和字符串占用的内存。
 *
 * @param uploadData 指向UploadDataType结构体的指针，包含需要转换的数据
 */
void Com_Util_UploadData2JsonString(Upload_Data_T *uploadData)
{
    // 创建一个新的JSON对象
    cJSON *obj = cJSON_CreateObject(); // 
    /* 给json数据添加id */
    // char uid[25] = {0}; // 96位=> 16进制:24
    // sprintf(uid, "%08X%08X%08X", HAL_GetUIDw2(), HAL_GetUIDw1(), HAL_GetUIDw0());
    // cJSON_AddStringToObject(obj, "uid", uid);

    // 向JSON对象中添加步数计数
    cJSON_AddNumberToObject(obj, "stepNum", uploadData->step);

    // // 向JSON对象中添加纬度信息
    cJSON_AddNumberToObject(obj, "lat", uploadData->lat);
    // // 向JSON对象中添加纬度方向信息
    cJSON_AddStringToObject(obj, "latDir", uploadData->latDir);

    // // 向JSON对象中添加经度信息
    cJSON_AddNumberToObject(obj, "lon", uploadData->lon);
    // // 向JSON对象中添加经度方向信息
    cJSON_AddStringToObject(obj, "lonDir", uploadData->lonDir);

    // // 向JSON对象中添加速度信息
    cJSON_AddNumberToObject(obj, "speed", uploadData->speed);

    // // 向JSON对象中添加日期时间信息
    cJSON_AddStringToObject(obj, "dateTime", uploadData->timeDate);

    // 将JSON对象转换为未格式化的字符串
    // char *objStr = cJSON_Print(obj); // json对象变成字符串格式
    char *objStr = cJSON_PrintUnformatted(obj); // json对象变成字符串格式
    // 清空uploadData结构体中的jsonData数组
    memset(uploadData->JSONData, 0, strlen(uploadData->JSONData));
    // 将JSON字符串复制到uploadData结构体的jsonData数组中
    memcpy(uploadData->JSONData, objStr, strlen(objStr));

    // 释放cJSON对象占用的内存
    cJSON_Delete(obj);
    // 释放JSON字符串占用的内存
    cJSON_free(objStr);
}
