#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "stm32f10x.h"

//����״̬
#define BT_STA_Null			0	//��ʹ��״̬
#define BT_STA_Initial		1	//�Ѿ���ʼ��״̬
#define BT_STA_Connected	2	//�Ѿ�����״̬

#define BT_A1_ERR				0xFFFF	//����ģ���ڲ�����
#define TIME_OUT				0xFFFE	//��ʱ
#define DATA_NULL				0xFFFD	//û�пɷ�������
#define OPT_CANCEL				0xFFFC	//ȡ������

#define NoAck					0x0000	//ʧ�ܣ���ʼ��ʧ�ܣ��豸�˳�����ģʽ
#define WithNoHostAdd 			0x0100	//����-δ����Թ�
#define WithHostAdd 			0x0101	//����-������Թ�

#define CmdInitDeviceID		   0x1100	 //��ʼ��ID
#define CmdInitDeviceID_Ack	   0x1101	 //��ʼ���ɹ��ظ�

#define CmdPair					0x0200	//����Pairable(���)״̬
#define CmdPair_Ack				0x0201	//Pairable(���)Ӧ��
#define CmdPair_TOut			0x0202	//Pairable(���)��ʱ���豸�˳�����ģʽ
#define CmdPair_OK				0x0203	//Pairable(���)�ɹ�

#define CmdGoConnected			0x0300	//����Connectable(������)״̬
#define Connectable				0x0301	//Connectable(������)״̬�ɹ�
//#define CmdGoConnected_NoAck	0x0302	//ʧ�ܣ���������ַ���豸�˳�����ģʽ
#define HostConnected			0x0401	//�����������ӳɹ�������ģ�鷴ת��Connected�������ӣ�״̬����ģ����Խ��������շ�
#define CmdDisconnect			0x0500	//�Ͽ�����
#define CmdDisconnect_Ack		0x0501	//�Ͽ����ӳɹ�
#define CmdQuitBT				0x0600	//�˳�����ģʽ
//#define CmdQuitBT_Ack			0x0601	//�˳�����ģʽ�ɹ�
#define CmdSend					0x0700	//��������
#define CmdSend_Ack				0x0701	//�������ݳɹ�
#define CmdSend_NoAck			0x0702	//��������ʧ�ܣ�������3�κ�����ʱ����100ms��Ȼ���豸�˳�����ģʽ
#define CmdSend_CRC_Err			0x0703	//��������У��ʧ�ܣ�������3�κ�����ʱ����100ms��Ȼ���豸�˳�����ģʽ

#define CmdHostDisconnect		0x0801	//�ֻ��Ͽ�����

#define CmdConnect				0x0900	//��������
#define CmdConnect_Ack			0x0901	//�������ӳɹ�

#define CmdSendFromBT			0x0A00 	//�������ݣ�Coca-cola��Ŀ����ʹ��
#define CmdSendFromBT_Ack		0x0A01 	//�������ݳɹ���Coca-cola��Ŀ����ʹ��
//#define CmdGetDeviceID			0x0B00	//��ȡ����ID
//#define CmdGetDeviceID_Ack		0x0B01	//��������ID
													 
#define K_BT_State_Unknown 			-1
#define K_BT_State_WithNoHostAdd	0
#define K_BT_State_WithHostAdd		1

#define PowerBT_ON() (PCout(7)=1)
#define PowerBT_OFF() (PCout(7)=0)
#define BT_A1_PIN PCin(0) 

#define PACKAGE_MAX	256		//���ݰ��ռ�
#define RECORD_MAX PACKAGE_MAX/RECORD_SIZE	//һ�����ݰ���ˢ����¼��

//�������ṹ
typedef struct 
{
	u16 cmd;			//��������ָ��
	u16 lenth;			//�ܳ���
	u16 pagemax;		//�ܰ���
	u16 page;			//���ݰ���
	u8 data[PACKAGE_MAX+4];
}send_struct;

//�������ṹ
typedef struct 
{
	u16 cmd;
	u8 data[8];
}bt_rx_buff;

//ʱ��μ������ݽṹ dd-dd-dd tt:tt:tt dd-dd-dd tt:tt:tt
typedef struct 
{
	u32 start_time;		//��ʼʱ��
    u32 end_time;     	//����ʱ��
}time_segment;

void BT_Get_DeviceID(void);		//��ȡ�����豸ID
u32 BT_Browse_DeviceID(void);	//���������豸ID

u16 BT_bootup(void);			//��������		��������ģ���ʼ���ķ���ֱ����ʾ����host
bool BT_Start_Prog(void);		//�����������host�Ĵ�����Ӧֱ�ӿ�����������	
void BT_Stop_Prog(void);		//�����ر�

u16 BT_GetCmd(void);			//��ȡ����ָ��
void BT_ClrState(void);			//�������״̬
u8 BT_GetState(void);			//��ȡ����״̬
//void BT_SendCardID(u8* buf);		//����һ��ˢ����¼
bool BT_SendDatas(u8* buf, u16 lenth);	//BT��������

bool BT_Pairable_Prog(void);	//������Թ���
bool BT_Tick(void);				//����ָ��ʱ��

bool BT_Disconnect_Prog(void);

void bt_sw(void);			//----����ʹ���л�----	  		  
bool bt_pair(void);			//----�������---	
	  		  
void power_off(void);		//�ֶ��ػ�
void send_recod(u8 con, u32 StartTime, u32 EndTime);
void BT_Init(void);

bool BT_Connect_Prog(void);

#endif
