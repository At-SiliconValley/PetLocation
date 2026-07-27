#ifndef __INT_QS100_H__
#define __INT_QS100_H__
#include "usart.h"
#include "Com_Delay.h"
#include "Com_Debug.h"
#include "string.h"
#include "Com_Status.h"
#include "stdlib.h"
#include ""

// 1.��ʼ������
void Int_QS100_Init(void);
// 2.���ѵķ���
void Int_QS100_WakeUp(void);
// 3.ר�ŷ�װһ������,�����ж�QS100�Ƿ��Ѿ���������
Com_Status_T Int_QS100_IsNetWork(void);
// 4.ר�ŷ�װһ������,����QS100����ͨ�ŵ�ͨ��
Com_Status_T Int_QS100_CreateSocket(void);
// 5.ר�ŷ�װһ������,����QS100����Զ�̵ķ�����
Com_Status_T Int_QS100_ConnectServer(uint8_t *serverIP, uint16_t port);
// 6.ר�ŷ�װһ������,������Զ�̵ķ�������������
Com_Status_T Int_QS100_SendData2Server(uint16_t length, uint8_t *data);
// 7.�ϱ�����
Com_Status_T Int_QS100_UploadData(uint8_t *serverIP, uint16_t port, uint16_t length, uint8_t *data);

// �͹��ĺ���
void Int_QS100_EnterLowPower(void);
void Int_QS100_LeaveLowPower(void);

#endif /* __INT_QS100_H__ */
