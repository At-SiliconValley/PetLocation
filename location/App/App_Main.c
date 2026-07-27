#include "App_Main.h"
// 准备接受GPS数据数组
uint8_t buffers[2048];
uint16_t length;
// 服务器的地址
#define SERVER_IP "112.125.89.8"
#define PORT 33057

// 应用层的初始化
void App_Main_Init(void)
{
    // 1.计步模块
    Int_DS3553_Init();
    // 2.GPS模块
    Int_AT6558R_Init();
    // 3.QS100初始化
    Int_QS100_Init();
    // 4.lora初始化
    Int_LoRa_Init();
}

// 应用层上报数据
void App_Main_UploadData2Server(void)
{

    // 1.整理GPS数据、步数(整理好)
    while (1)
    {
        // 1.获取GPS的数据
        Int_AT6558R_GetGPS(buffers, &length);

        // 2.判断是否接收到有效数据,有效数据,循环终止,执行后面上报业务!
        // 3.接受到的无效的数据,继续获取GPS数据,不需要上报!
        // 当前条件语句判断,只能知道GPS数据是接收到了,但是数据是有效的还是无效的无从判断
        if (length > 0)
        {
            // 1.截取子串
            // char *substring = strstr((char *)buffers, "$GNRMC");
            char *substring = "$GNRMC,154919.000,A,3028.09110,N,11423.29505,E,3.44,156.11,060525,,,A,V*00";
            char ch;
            // 2.从子串当中提取字符
            sscanf(substring, "$GNRMC,%*[^AV]%c", &ch);
            if (ch == 'A')
            {
                // 处理数据,将整理完数据进行存储
                uint8_t tmp_array[7] = {0};  // 临时存储小时、分钟、秒
                uint8_t year_array[7] = {0}; // 临时存储年月日
                sscanf(substring, "$GNRMC,%6c%*7c%f,%c,%f,%c,%f,%*f,%6c", tmp_array, &upload.lat, upload.latDir, &upload.lon, upload.lonDir, &upload.speed, year_array);

                // 1.先处理日期,将年月日时分秒 YYYY-MM_DD HH:MM:SS
                sprintf(upload.timeDate,
                        "20%c%c-%c%c-%c%c %c%c:%c%c:%c%c",
                        year_array[4],
                        year_array[5],
                        year_array[2],
                        year_array[3],
                        year_array[0],
                        year_array[1],
                        tmp_array[0],
                        tmp_array[1],
                        tmp_array[2],
                        tmp_array[3],
                        tmp_array[4],
                        tmp_array[5]);
                // 获取到时间UTC时间(老外时间),转换成北京时间
                // 国内的定位芯片获取到世间,百分百不是北京时间(UTC,格林尼治时间(英国))
                Com_Util_utc2beijing(upload.timeDate, upload.timeDate);

                // 处理上报数据之经纬度-度与分相差60
                // 维度
                upload.lat = (int)upload.lat / 100 + (upload.lat - (int)upload.lat / 100 * 100) / 60;
                // 经度
                upload.lon = (int)upload.lon / 100 + (upload.lon - (int)upload.lon / 100 * 100) / 60;

                // 存储上报步数
                upload.step = Int_DS3553_GetStepNumber();

                break;
            }
            else
            {
                COM_DEBUG_LN("获取到GPS-无效的");
            }
        }
    }

    // 2.上报即可！JSON数据格式
    Com_Util_UploadData2JsonString(&upload);

    // 3.上报JSON数据格式
    Com_Status_T status = Int_QS100_UploadData(SERVER_IP, PORT, strlen(upload.JSONData), (uint8_t *)upload.JSONData);

    if (status == COM_OK)
    {
        COM_DEBUG_LN("QS100上报JSON数据成功");
    }
    else
    {
        // 如果QS100芯片发送数据失败(定位器周边可能没有基站、流量卡没流量、天线掉了.....)
        Int_Lora_Send((uint8e_t *)upload.JSONData, strlen(upload.JSONData));
    }
}
// 3.进入低功耗
void App_Main_EnterLowPower(void)
{
    // 上报完数据一次,MCU【STM32、QS100、AT6558R】进入低功耗
    Int_QS100_EnterLowPower();
    Int_AT6558R_EnterLowPower();

    // 主控芯片再进入待机之前设置好闹钟(不能再待机之后设置)
    // 获取当前时间
    RTC_TimeTypeDef current_time;
    HAL_RTC_GetTime(&hrtc, &current_time, RTC_FORMAT_BIN);

    RTC_AlarmTypeDef alarm;
    alarm.AlarmTime.Hours = current_time.Hours;
    alarm.AlarmTime.Minutes = current_time.Minutes;
    alarm.AlarmTime.Seconds = current_time.Seconds + 10;
    // 第三个参数:设置闹钟写入寄存器数据格式
    // bin:22->0001,0110  bin
    // bcd:22->0010,0010
    HAL_RTC_SetAlarm(&hrtc, &alarm, RTC_FORMAT_BIN);

    // 判断是否待机模式,标志位需要清除,不能再进入待机模式
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB))
    {
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    }

    if (__HAL_PWR_GET_FLAG(PWR_FLAG_WU))
    {
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    }

    HAL_PWR_EnterSTANDBYMode();
}

// 退出低功耗
void App_Main_LeaveLowPower(void)
{
    Int_QS100_LeaveLowPower();
    Int_AT6558R_LeaveLowPower();
}
