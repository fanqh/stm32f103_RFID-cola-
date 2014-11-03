#ifndef __DataTransfer_H
#define __DataTransfer_H

#define USB_CON	0
#define BT_CON	1

#define USB_RECV_MAX 272
#define USB_SEND_MAX 272

//usb指令集
#define CmdQuery			0xA100		//按时间获取刷卡记录
#define CmdQuery_Ack		0xA101		//

#define CmdGetDeviceID		0xA200		//获取本机ID
#define CmdGetDeviceID_Ack	0xA201		//
#define CmdSetDeviceID		0xA210		//设置本机ID
#define CmdSetDeviceID_Ack	0xA211		//

#define CmdSetDateTime		0xA300		//设置时间
#define CmdSetDateTime_Ack	0xA301		//
#define CmdDataFormat		0xA400		//清除全部刷卡记录
#define CmdDataFormat_Ack	0xA401		//

#define CmdSendRFID			0xA501		//

#define ERROR			0xAF02		//通用的错误返回值定义
#define ERROR_PARA		0xAF03		//参数格式错误
#define ERROR_TIMEOUT	0xAF04		//超时
#define END_DATAS		0xCACA		//结束符

#define CmdSendCurTagInfo 0xb100	 //蓝牙发送数据指令
#define CmdSendCurTagInfo_Ack 0xb101 //蓝牙发送数据成功

typedef struct 
{
	u16 cmd;
	u16 lenth;
	u8 data[USB_RECV_MAX-4];
}usb_rx_buff;

typedef struct 
{
 	u32 count;		//顺序计数器
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
   	u32 boot; 	    //引导字
   	u32 desID;    	//目标设备ID
	u32 souID;     	//源设备ID
	u16 lenth;		//长度
   	u16 cmd;     	//命令
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
	 
