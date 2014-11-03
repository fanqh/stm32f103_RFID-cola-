#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "stm32f10x.h"

//蓝牙状态
#define BT_STA_Null			0	//不使用状态
#define BT_STA_Initial		1	//已经初始化状态
#define BT_STA_Connected	2	//已经连接状态

#define BT_A1_ERR				0xFFFF	//蓝牙模块内部崩溃
#define TIME_OUT				0xFFFE	//超时
#define DATA_NULL				0xFFFD	//没有可发送数据
#define OPT_CANCEL				0xFFFC	//取消操作

#define NoAck					0x0000	//失败：初始化失败，设备退出蓝牙模式
#define WithNoHostAdd 			0x0100	//待机-未曾配对过
#define WithHostAdd 			0x0101	//待机-曾经配对过

#define CmdInitDeviceID		   0x1100	 //初始化ID
#define CmdInitDeviceID_Ack	   0x1101	 //初始化成功回复

#define CmdPair					0x0200	//进入Pairable(配对)状态
#define CmdPair_Ack				0x0201	//Pairable(配对)应答
#define CmdPair_TOut			0x0202	//Pairable(配对)超时，设备退出蓝牙模式
#define CmdPair_OK				0x0203	//Pairable(配对)成功

#define CmdGoConnected			0x0300	//进入Connectable(可连接)状态
#define Connectable				0x0301	//Connectable(可连接)状态成功
//#define CmdGoConnected_NoAck	0x0302	//失败：无主机地址，设备退出蓝牙模式
#define HostConnected			0x0401	//蓝牙主机连接成功，蓝牙模块反转至Connected（已连接）状态，主模块可以进行数据收发
#define CmdDisconnect			0x0500	//断开连接
#define CmdDisconnect_Ack		0x0501	//断开连接成功
#define CmdQuitBT				0x0600	//退出蓝牙模式
//#define CmdQuitBT_Ack			0x0601	//退出蓝牙模式成功
#define CmdSend					0x0700	//发送数据
#define CmdSend_Ack				0x0701	//发送数据成功
#define CmdSend_NoAck			0x0702	//发送数据失败，需重试3次后，重试时间间隔100ms，然后设备退出蓝牙模式
#define CmdSend_CRC_Err			0x0703	//发送数据校验失败，需重试3次后，重试时间间隔100ms，然后设备退出蓝牙模式

#define CmdHostDisconnect		0x0801	//手机断开连接

#define CmdConnect				0x0900	//主动连接
#define CmdConnect_Ack			0x0901	//主动连接成功

#define CmdSendFromBT			0x0A00 	//发送数据，Coca-cola项目不需使用
#define CmdSendFromBT_Ack		0x0A01 	//发送数据成功，Coca-cola项目不需使用
//#define CmdGetDeviceID			0x0B00	//获取蓝牙ID
//#define CmdGetDeviceID_Ack		0x0B01	//返回蓝牙ID
													 
#define K_BT_State_Unknown 			-1
#define K_BT_State_WithNoHostAdd	0
#define K_BT_State_WithHostAdd		1

#define PowerBT_ON() (PCout(7)=1)
#define PowerBT_OFF() (PCout(7)=0)
#define BT_A1_PIN PCin(0) 

#define PACKAGE_MAX	256		//数据包空间
#define RECORD_MAX PACKAGE_MAX/RECORD_SIZE	//一个数据包的刷卡记录数

//发送区结构
typedef struct 
{
	u16 cmd;			//发送数据指令
	u16 lenth;			//总长度
	u16 pagemax;		//总包数
	u16 page;			//数据包号
	u8 data[PACKAGE_MAX+4];
}send_struct;

//接收区结构
typedef struct 
{
	u16 cmd;
	u8 data[8];
}bt_rx_buff;

//时间段检索数据结构 dd-dd-dd tt:tt:tt dd-dd-dd tt:tt:tt
typedef struct 
{
	u32 start_time;		//开始时间
    u32 end_time;     	//结束时间
}time_segment;

void BT_Get_DeviceID(void);		//获取蓝牙设备ID
u32 BT_Browse_DeviceID(void);	//检索蓝牙设备ID

u16 BT_bootup(void);			//蓝牙启动		返回蓝牙模块初始化的返回直，表示有无host
bool BT_Start_Prog(void);		//蓝牙启动后对host的处理，对应直接开关蓝牙操作	
void BT_Stop_Prog(void);		//蓝牙关闭

u16 BT_GetCmd(void);			//获取蓝牙指令
void BT_ClrState(void);			//清除蓝牙状态
u8 BT_GetState(void);			//获取蓝牙状态
//void BT_SendCardID(u8* buf);		//发送一条刷卡记录
bool BT_SendDatas(u8* buf, u16 lenth);	//BT发送数据

bool BT_Pairable_Prog(void);	//蓝牙配对过程
bool BT_Tick(void);				//蓝牙指令时序

bool BT_Disconnect_Prog(void);

void bt_sw(void);			//----蓝牙使用切换----	  		  
bool bt_pair(void);			//----蓝牙配对---	
	  		  
void power_off(void);		//手动关机
void send_recod(u8 con, u32 StartTime, u32 EndTime);
void BT_Init(void);

bool BT_Connect_Prog(void);

#endif
