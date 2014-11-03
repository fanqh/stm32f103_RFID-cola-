#ifndef __DataTransfer_H
#define __DataTransfer_H

#define USB_CON	0
#define BT_CON	1

#define USB_RECV_MAX 272
#define USB_SEND_MAX 272

//usbָ�
#define CmdQuery			0xA100		//��ʱ���ȡˢ����¼
#define CmdQuery_Ack		0xA101		//

#define CmdGetDeviceID		0xA200		//��ȡ����ID
#define CmdGetDeviceID_Ack	0xA201		//
#define CmdSetDeviceID		0xA210		//���ñ���ID
#define CmdSetDeviceID_Ack	0xA211		//

#define CmdSetDateTime		0xA300		//����ʱ��
#define CmdSetDateTime_Ack	0xA301		//
#define CmdDataFormat		0xA400		//���ȫ��ˢ����¼
#define CmdDataFormat_Ack	0xA401		//

#define CmdSendRFID			0xA501		//

#define ERROR			0xAF02		//ͨ�õĴ��󷵻�ֵ����
#define ERROR_PARA		0xAF03		//������ʽ����
#define ERROR_TIMEOUT	0xAF04		//��ʱ
#define END_DATAS		0xCACA		//������

#define CmdSendCurTagInfo 0xb100	 //������������ָ��
#define CmdSendCurTagInfo_Ack 0xb101 //�����������ݳɹ�

typedef struct 
{
	u16 cmd;
	u16 lenth;
	u8 data[USB_RECV_MAX-4];
}usb_rx_buff;

typedef struct 
{
 	u32 count;		//˳�������
	u32 id;
	u32 time;
}_pag;

typedef struct 
{
 	u8 crc1;
	u8 crc2;
	u16 eof;
}_crc_end;

typedef struct 
{
	u16 cmd;
	u16 lenth;
	u32 id;
 	u8 crc1;
	u8 crc2;
	u16 eof;
}_send;

typedef struct 
{	
   	u32 boot; 	    //������
   	u32 desID;    	//Ŀ���豸ID
	u32 souID;     	//Դ�豸ID
	u16 lenth;		//����
   	u16 cmd;     	//����
}SYS_CMD;

typedef struct 
{
	u32 StartTime;
	u32 EndTime;
}QUERY;
typedef struct 
{
 	u16 cmd;		//
	u16 lenth;
}_cmd_lenth;

bool con_cmd(void);
u16 BT_RecvDatas(u8* buf, u16 lenth, u16 timeout);
void ProCmd(u16 cmd);
bool IsCmdValid(u16 cmd);
//////////////////////////////////////////////////////////////////////////////////	 
#endif
	 
