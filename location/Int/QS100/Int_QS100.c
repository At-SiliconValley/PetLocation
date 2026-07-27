#include "Int_QS100.h"
// ����QS100��Ӧ������
#define MAXSIZE 128
// ���������ڽ��ܷ����η�������
uint8_t small_buffers[MAXSIZE];
uint16_t small_size;
// ���������ڽ������յķ��ص�����
uint8_t big_buffers[MAXSIZE];
uint16_t big_size;

// ����:Ŀ��ͨ������3��QS100����AT����
static void Int_QS100_SendATCMD(uint8_t *at_cmd)
{

    // 1.��ÿһ�η����µ�����֮ǰ,��������ȫ��������(�洢��ǰ����ص�����)
    memset(small_buffers, 0, strlen((char *)small_buffers));
    small_size = 0;
    memset(big_buffers, 0, strlen((char *)big_buffers));
    big_size = 0;
    // ��������
    HAL_UART_Transmit(&huart3, at_cmd, strlen((char *)at_cmd), 1000);
    // ���д������������������������鵱��-3��4��
    uint8_t count = 4;
    do
    {

        // 1���ȴ����������ݵ���,�ٽ��������������������鵱��
        uint16_t timeout = HAL_GetTick(); // 1000
        while (small_size == 0 && (HAL_GetTick() - timeout) < 3000)
            ;

        // 2.Ҫô����������(��һ�Ρ��ڶ��Ρ�������)������
        // �ȴ���ʱ,��û�����ݷ�����Ϊ����
        if (small_size == 0)
        {
            break;
        }

        // 3.���������ݷ����ж���,����û�г�ʱ
        memcpy(&big_buffers[big_size], small_buffers, strlen((char *)small_buffers));
        big_size += small_size;

        // 4.���С����,׼��������һ������
        memset(small_buffers, 0, strlen((char *)small_buffers));
        small_size = 0;

    } while (strstr((char *)big_buffers, "OK") == NULL && strstr((char *)big_buffers, "ERROR") == NULL && count--);
}

// �˷������ж��ڲ�����,Ŀ���ٴο��������ж�
void Int_QS100_GetSize(uint16_t Size)
{
    // ���η��ص����ݳ���
    small_size = Size;
    HAL_StatusTypeDef status = HAL_BUSY;
    while (status != HAL_OK)
    {
        status = HAL_UARTEx_ReceiveToIdle_IT(&huart3, small_buffers, MAXSIZE);
    }
}

// 1.��ʼ������
void Int_QS100_Init(void)
{

    // 1.����
    Int_QS100_WakeUp();

    // 3.ͨ������3����QS100��Ӧ������
    HAL_StatusTypeDef status = HAL_BUSY;
    while (status != HAL_OK)
    {
        status = HAL_UARTEx_ReceiveToIdle_IT(&huart3, small_buffers, MAXSIZE);
    }

    // 2.����AT����豸��������
    Int_QS100_SendATCMD("AT+RB\r\n");
    Int_QS100_SendATCMD("AT+NITZ=0\r\n");
}

// 2.���ѵķ���
void Int_QS100_WakeUp(void)
{
    // C8T6��PB13���Ż���,���ѷ�ʽ:PB13���ߡ�100us-5s��
    HAL_GPIO_WritePin(QS100_WAKEUP_GPIO_Port, QS100_WAKEUP_Pin, GPIO_PIN_SET);
    Com_Delay_Sec(1);
    HAL_GPIO_WritePin(QS100_WAKEUP_GPIO_Port, QS100_WAKEUP_Pin, GPIO_PIN_RESET);
}

// 3.�ж�QS100�Ƿ��Ѿ�����
Com_Status_T Int_QS100_IsNetWork(void)
{
    // 1. ���ô˷��������ж��Ƿ�����,����оƬC8T6����AT�����!
    Int_QS100_SendATCMD("AT+CGATT?\r\n");
    // 2.�ж�QS100��Ӧ������
    if (strstr((char *)big_buffers, "CGATT:1") != NULL)
    {
        return COM_OK;
    }
    return COM_ERROR;
}

// 4.����ͨ�ŵ�ͨ��
Com_Status_T Int_QS100_CreateSocket(void)
{

    // 1.����AT����AT+NSOCR=����1,����2(TCP��UDP����),����3(�˿ں�,0:���)  ����4:��·��(0-1)
    Int_QS100_SendATCMD("AT+NSOCR=STREAM,6,0,0\r\n");

    // 2.�ж�ͨ��ͨ�������ɹ�����ʧ��
    if (strstr((char *)big_buffers, "OK") != NULL)
    {

        return COM_OK;
    }

    return COM_ERROR;
}

// 5.����Զ�̷�����
Com_Status_T Int_QS100_ConnectServer(uint8_t *serverIP, uint16_t port)
{
    // 1.׼������
    uint8_t result_array[50] = {0};
    sprintf((char *)result_array, "AT+NSOCO=0,%s,%d\r\n", serverIP, port);
    // 2.��������
    Int_QS100_SendATCMD(result_array);

    // 3.�ж����ӷ������Ƿ�ɹ�
    if (strstr((char *)big_buffers, "OK") != NULL)
    {

        return COM_OK;
    }

    return COM_ERROR;
}

// 6.��������:length = 2  data = AB
Com_Status_T Int_QS100_SendData2Server(uint16_t length, uint8_t *data)
{

    // 1.ƴ�ճ���ɵ�����
    // 2.�ȴ�����������,ת��Ϊ16���Ƶ�����{4,1,4,2} = 4142
    uint8_t hex_array[500] = {0};
    for (uint16_t i = 0; i < length; i++)
    {
        sprintf((char *)&hex_array[i * 2], "%02x", data[i]);
    }
    // 3.ƴ����ɵķ�����������
    uint8_t result_array[500] = {0};
    sprintf((char *)result_array, "AT+NSOSD=0,%d,%s,0x200\r\n", length, hex_array);
    // 4.��������
    Int_QS100_SendATCMD(result_array);

    // 5.�������ݳɹ�����ʧ��
    if (strstr((char *)big_buffers, "OK") != NULL)
    {
        return COM_OK;
    }

    return COM_ERROR;
}

// �ϱ�����
Com_Status_T Int_QS100_UploadData(uint8_t *serverIP, uint16_t port, uint16_t length, uint8_t *data)
{

    // 1.�ж�QS100�Ƿ�����
    uint8_t count = 11;
    while (Int_QS100_IsNetWork() != COM_OK && --count)
    {
        Com_Delay_Sec(1);
        COM_DEBUG_LN("QS100������......");
    }
    // 2.������ʱ���������
    if (count == 0)
    {
        COM_DEBUG_LN("QS100������ʱ");
        return COM_TIMEOUT;
    }
    COM_DEBUG_LN("QS100�����ɹ�");

    // 2.�ж�QS100ͨ��ͨ���Ƿ񴴽��ɹ�
    count = 11;
    while (Int_QS100_CreateSocket() != COM_OK && --count)
    {
        Com_Delay_Sec(1);
        COM_DEBUG_LN("QS100����ͨ��ͨ��......");
    }
    // 2.������ʱ���������
    if (count == 0)
    {
        COM_DEBUG_LN("QS100����ͨ��ͨ����ʱ");
        return COM_TIMEOUT;
    }
    COM_DEBUG_LN("QS100����ͨ���ɹ�");

    // 3.����Զ�̷�����
    count = 11;
    while (Int_QS100_ConnectServer(serverIP, port) != COM_OK && --count)
    {
        Com_Delay_Sec(1);
        COM_DEBUG_LN("QS100���ӷ�����....");
    }
    // 2.������ʱ���������
    if (count == 0)
    {
        COM_DEBUG_LN("QS100���ӳ�ʱ");
        return COM_TIMEOUT;
    }
    COM_DEBUG_LN("QS100���ӷ������ɹ�");

    // 4.��Զ�̷�������������
    Com_Status_T status = Int_QS100_SendData2Server(length, data);
    if (status == COM_OK)
    {
        COM_DEBUG_LN("QS100�ϱ����ݳɹ�");
        return COM_OK;
    }
    else
    {
        COM_DEBUG_LN("QS100�ϱ�����ʧ��");
        return COM_ERROR;
    }
}

// ����͹���
void Int_QS100_EnterLowPower(void)
{
    // ����AT����
    // 0:����������п���,��������������Ҫ,��Ҫͨ���ⲿ�жϻ���!
    Int_QS100_SendATCMD("AT+FASTOFF=0\r\n");
}
void Int_QS100_LeaveLowPower(void)
{
    Int_QS100_WakeUp();
}
