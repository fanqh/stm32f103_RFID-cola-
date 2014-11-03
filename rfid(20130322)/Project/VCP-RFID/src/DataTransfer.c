/*
数据传送模块

1. VCOM接口
2. COM2（蓝牙）接口


*/



#include "stm32f10x_flash.h"

#include "file.h"	   
#include "bluetooth.h"
#include "MyUsart.h"
#include "usb_driver.h"
#include "delay.h"
#include "beep.h"
#include "oled.h"
#include "PCF8563.h"
#include "DataTransfer.h" 
#include "power.h"
#include "config.h"

extern char TxetTab[][20];
usb_rx_buff USB_RX_BUFF;
u8 SendBuff[USB_SEND_MAX];
u16 SendLenth;
u8 *pSendBuff;

extern u8 BT_State;			//蓝牙状态
u8 Nowaday_Con;		//当前使用的通讯接口
//------------------------------------------------------------------


void Send_SourceRecod(u8 con, u32 StartTime, u32 EndTime)
{
	FILE_RECORD_STRUCT record;	//记录项
	u16 CountMax;		//记录条数
	u8 crc1=0, crc2=0;
	u16 cmd_send = CmdSend;
	u16 cmd=CmdQuery_Ack;
	u16 lenth=0;
	_pag pag;
	_crc_end crc_end;

	//提取行号和计数器
	CountMax = GetStoreCount();	//提取参数

	//数据流--发送返回指令
	if(con==USB_CON)
	{
		usb_write((u8*)&cmd, 2);
		usb_write((u8*)&lenth, 2);
	}
	//数据流--发送返回指令
	if(con==BT_CON)
	{
		com_clear(COM2);
		com_write(COM2, (u8*)&cmd_send, 2);
		com_write(COM2, (u8*)&cmd, 2);
		com_write(COM2, (u8*)&lenth, 2);
	}

	//没有记录
	if(CountMax == 0)
	{
	//	OLED_ShowString(0,48,1,1,TxetTab[8]);	//没有记录	
	//	OLED_Refresh_Gram();
		goto loop_end;
	}
	//全部记录
	if(StartTime==0)
		EndTime=0xFFFFFFFF;

	//数据流--发送记录体
	SendLenth=0;
	pSendBuff=SendBuff;
	for(pag.count=0; pag.count<CountMax; pag.count++)
	{
		//从文件中提取一条记录
		u32 addr=FILE_DATA_ADDR + pag.count * sizeof(FILE_RECORD_STRUCT);		//计算地址
		SPI_Flash_Read((u8*)&record, addr, sizeof(FILE_RECORD_STRUCT));//读取一条记录
		//比较时间段
		if(record.time>=StartTime && record.time<EndTime)	//在时间段结束之前
		{
			pag.id=record.rfid;
			pag.time=record.time;
			if(con==USB_CON)
				usb_write((u8*)&pag, sizeof(pag));
			else
			if(con==BT_CON)
				com_write(COM2, (u8*)&pag, sizeof(pag));			//发送数据
		}
		else
			break;
	}

loop_end:
	//数据流--发送校验码、结束符
	crc_end.crc1=crc1;
	crc_end.crc2=crc2;
	crc_end.eof=0xCACA;
	if(con==USB_CON)
		usb_write((u8*)&crc_end, sizeof(crc_end));
	else
	if(con==BT_CON)
		com_write(COM2, (u8*)&crc_end, sizeof(crc_end));			//发送数据
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void send_recod(u8 con, u32 StartTime, u32 EndTime)
{
	FILE_RECORD_STRUCT record;	//记录项
	u16 CountMax;		//记录条数
	u8 crc1=0, crc2=0;
    _pag pag; 
	_crc_end crc_end;
    _cmd_lenth cmd_lenth;
	u32 count;
	u32 addr;
	
	cmd_lenth.cmd = CmdQuery_Ack;
	cmd_lenth.lenth = 0;
	pag.count = 0;


	//提取行号和计数器
	CountMax = GetStoreCount();	//提取参数

///	CountMax = 9999;

	//数据流--发送返回指令								    
	if(con==USB_CON)
		usb_write((u8*)&cmd_lenth, 4);

	//数据流--发送返回指令
	if(con==BT_CON)
	{
		if( !BT_SendDatas( (u8*)&cmd_lenth, 4 ) )
			return;
	}

	//没有记录
	if(CountMax == 0)
	{
		goto loop_end;
	}
	//全部记录
	if(StartTime==0)
		EndTime=0xFFFFFFFF;

	//数据流--发送记录体
	SendLenth=0;
	pSendBuff=SendBuff;
	for(count=0; count<CountMax; count++)
	{
		//从文件中提取一条记录

		addr = FILE_DATA_ADDR + (count) * sizeof(FILE_RECORD_STRUCT);		//计算地址

		SPI_Flash_Read((u8*)&record, addr, sizeof(FILE_RECORD_STRUCT));//读取一条记录
		//比较时间段
		if(record.time>=StartTime && record.time<=EndTime)	//在时间段结束之前
		{				
	
		   		pag.count++;
				pag.id = record.rfid;
				pag.time = record.time;	 
				if(con==USB_CON)
				{
					if(usb_write((u8*)&pag, sizeof(pag))==FALSE)
						return ;
				}
				else
				if(con==BT_CON)
				{
					if( !BT_SendDatas((u8*)&pag, sizeof(pag)) )
						return;
				}
				else
				{
					 return;
				}	  
	   }
	   else
			break;
	}

loop_end:
	//数据流--发送校验码、结束符
	crc_end.crc1=crc1;
	crc_end.crc2=crc2;
	crc_end.eof=0xCACA;
	if(con==USB_CON)
		usb_write((u8*)&crc_end, sizeof(crc_end));
	else
	if(con==BT_CON)
		BT_SendDatas( (u8*)&crc_end, sizeof(crc_end));			//发送数据
}

//----按时间段发送记录-----------------------------------------------
void Query()
{
	QUERY *q;
	u32 StartTime, EndTime;

	q=(QUERY *)USB_RX_BUFF.data;
	StartTime = q->StartTime;
	EndTime = q->EndTime;
 #ifdef K_COSTUMER_NAIVEE
 	Send_SourceRecod(Nowaday_Con, StartTime, EndTime);
 #else 
	send_recod(Nowaday_Con, StartTime, EndTime);
 #endif
}

//----获取设备ID------------------------------------------------------
void GetDeviceID()
{
	_send send;

	send.cmd=CmdGetDeviceID_Ack;
	send.lenth=0;
	send.id = HWREG(FLASE_ID_ADDR);
	send.crc1=0, 
	send.crc2=0;
	send.eof=0xCACA;;

	if(Nowaday_Con==USB_CON)
		usb_write((u8*)&send, sizeof(send));
	else
	if(Nowaday_Con==BT_CON)
		BT_SendDatas((u8*)&send, sizeof(send));
}
//----设置时间--------------------------------------------------------
void SetDateTime()
{
	u32 timecount;
	_send send;

	send.cmd=CmdSetDateTime_Ack;
	send.lenth=0;
	send.crc1=0, 
	send.crc2=0;
	send.eof=0xCACA;;

	if(Nowaday_Con==USB_CON)
		usb_write((u8*)&send, sizeof(send));
	else
	if(Nowaday_Con==BT_CON)
		BT_SendDatas((u8*)&send, sizeof(send));
	timecount=*((u32 *)USB_RX_BUFF.data);
	SetRTC(timecount);	//保存RTC时间
}
//----清除全部刷卡记录----------------------------------------------
void DataFormat()
{
	_send send;

	send.cmd=CmdDataFormat_Ack;
	send.lenth=0;
	send.crc1=0, 
	send.crc2=0;
	send.eof=0xCACA;;

	if(Nowaday_Con==USB_CON)
		usb_write((u8*)&send, sizeof(send));
	else
	if(Nowaday_Con==BT_CON)
		BT_SendDatas((u8*)&send, sizeof(send));
	clear_CardID();		//清除全部刷卡记录

}


//----设置本机可修改ID----------------------------------------------
void SetDeviceID()
{
	u32 id;
	_send send;

	send.cmd=CmdSetDeviceID_Ack;
	send.lenth=0;
	send.crc1=0, 
	send.crc2=0;
	send.eof=0xCACA;;

	id = *((u32 *)USB_RX_BUFF.data);
  	FLASH_Unlock();
	//Clear All pending flags
  	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(FLASE_ID_ADDR);
	FLASH_ProgramWord(FLASE_ID_ADDR, id);
  	FLASH_Lock();

	if(Nowaday_Con==USB_CON)
		usb_write((u8*)&send, sizeof(send));
	else
	if(Nowaday_Con==BT_CON)
		BT_SendDatas((u8*)&send, sizeof(send));
}


bool IsCmdValid(u16 cmd)
{
	if((cmd==CmdQuery)||(cmd==CmdGetDeviceID)||(cmd==CmdSetDateTime)||(cmd==CmdDataFormat)
	   ||(cmd==CmdSetDeviceID)||(cmd==HostConnected)||(cmd==CmdHostDisconnect))
	    
		return TRUE;
		
	else 
		return FALSE;
}

void ProCmd(u16 cmd)
{

	switch(cmd)
	{
		case CmdQuery:	Query();
						break;
		case CmdGetDeviceID:	GetDeviceID();
						break;
		case CmdSetDateTime:	SetDateTime();
						break;
		case CmdDataFormat:		DataFormat();	
						break;
		case CmdSetDeviceID:	SetDeviceID();	
						break;
		
		case HostConnected:	
						if(BT_State != BT_STA_Null)
							BT_State = BT_STA_Connected;
						break;

		case CmdHostDisconnect:	
						if(BT_State == BT_STA_Connected)
							BT_State = BT_STA_Initial;
						else
						if(BT_State == BT_STA_Null)
							;
//							PowerBT_OFF();
						break;
		default:
			break;
	}
	
}
//----BT或者USB指令--------------------------------------------------------
bool con_cmd()
{
	vu16 ndata;
	SYS_CMD *psys;

	// 先检测USB通讯
	ndata = usb_ndata();
	//错误
	if(ndata>=USB_RECV_MAX)
	{
		usb_clear();		//清除USB接收区
		return FALSE;
	}
	if(ndata != 0)
	{	
		usb_read((u8*)&USB_RX_BUFF, ndata);	 //USB读取数据
		Nowaday_Con = USB_CON;			
		goto loop1;
	}
	// 再检测BT通讯
	ndata=com_package(COM2);
	//错误
	if(ndata>=USB_RECV_MAX)
	{
		com_clear(COM2);		//清除接收区
		return FALSE;
	}
	if(ndata != 0)
	{
		//BT读取数据
		com_read(COM2, (u8*)&USB_RX_BUFF, ndata);
		Nowaday_Con = BT_CON;			
		goto loop1;
	} 	
	return FALSE;
loop1:
				
	// 是系统复位指令
	psys = (SYS_CMD*)&USB_RX_BUFF;
	if( psys->boot == 0xAAAA5555)
	{
    	if(psys->cmd == 0xFF02)
		{
      		SCB->AIRCR =0X05FA0000|(u32)0x04;	  //软复位
			while(1);
		}
	}
	ProCmd(USB_RX_BUFF.cmd);
	return TRUE;

}

//----BT接收数据-------------------------------------------------
u16 BT_RecvDatas(u8* buf, u16 lenth, u16 timeout)
{
	long t = timeout;
	u16 ndata;

	while(1)
	{
		t--;
		// 超时
		if(t <= 0)	  
		{
			com_clear(COM2);		//清除接收区
			return 0;
		}
		delay_ms(1);
		// 再检测BT通讯
		ndata=com_package(COM2);
		if(ndata != 0)
			break;
	}

	//BT读取数据
	com_read(COM2, buf, lenth);	  //  	   ndata
	return ndata;								  
}	

