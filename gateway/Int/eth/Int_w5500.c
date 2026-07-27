#include "Int_w5500.h"

// 远程服务器地址与端口号
uint8_t dest_ip[4] = {112, 125, 89, 8};
uint16_t dest_port = 34095;

// 网络信息
wiz_NetInfo default_net_info = {
    .mac = {0x00, 0x08, 0xdc, 0x12, 0x22, 0x12},
    .ip = {192, 168, 55, 130},
    .gw = {192, 168, 55, 1},
    .sn = {255, 255, 255, 0}};

// 初始化方法
void Int_W5500_Init(void)
{
    // 1.给w5500的结构体成员进行赋值
    wizchip_userRegisterCallBack();

    // 2.主控让w5500复位:复位,至少维持500us
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
    Com_Delay_Ms(10);
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
    Com_Delay_Ms(10);

    // 3.配置网络信息
    setSHAR(default_net_info.mac); // 设置mac地址
    setGAR(default_net_info.gw);   // 设置网关地址(路由器)
    setSUBR(default_net_info.sn);  // 设置子网掩码
    setSIPR(default_net_info.ip);  // 设置IP地址
}

// 查询通道的状态
SOCKET_STATUS_T Int_W5500_GetSocketStatus(void)
{

    switch (getSn_SR(SOCKET_ID))
    {
    case SOCK_CLOSED:
        // 先关闭通道
        close(SOCKET_ID);
        // 打开一个通道
        int8_t result = socket(SOCKET_ID, Sn_MR_TCP, 50000, 0x00);
        if (result == SOCKET_ID)
        {
            COM_DEBUG_LN("w5500通道0打开成功");
        }
        else
        {
            COM_DEBUG_LN("w5500通道0打开失败");
        }
        break;
    case SOCK_INIT:
        // 通道已经打开,链接远程服务器
        int8_t result1 = connect(SOCKET_ID, dest_ip, dest_port);
        if (result1 == SOCK_OK)
        {
            COM_DEBUG_LN("通道0链接远程服务器成功");
        }
        else
        {
            COM_DEBUG_LN("通道0链接远程服务器失败");
        }
        break;
    case SOCK_ESTABLISHED:
        // 通过查询w5500的SN_IR寄存器最低位,判断通道与远程服务器建立连接了
        if (getSn_IR(SOCKET_ID) & Sn_IR_CON)
        {
            return COM_OK;
        }
        else
        {
            return COM_ERROR;
        }
    }

    return COM_ERROR;
}
// 发送数据
void Int_w5500_SendData(uint8_t *buffers, uint16_t sizes)
{
    // 1.轮询等待通道与远程服务器建立链接
    while (Int_W5500_GetSocketStatus() != COM_OK)
        ;

    // 2.通道与远程服务器建立链接成功
    send(SOCKET_ID, buffers, sizes);
}