
#include "bluetooth.h"
#include "stm32f10x.h"									  
#include "gpio_config.h"
#include "MyUsart.h"
//#include "uart2.h"
//#include "uart3.h"
#include "oled.h"
#include "key.h"
#include "delay.h"
#include "file.h" 
#include "flash.h"  
#include "usb_istr.h" 
#include "DataTransfer.h" 

/////////////////////////////////////////////////////////////////////
//在DataTransfer.c中定义
#define BT_CON	1

send_struct	SDB;	     //发送区结构
bt_rx_buff 	BT_RX_BUFF;	 //接收区结构
time_segment TIME_SEG;	 //时间段检索数据结构 dd-dd-dd tt:tt:tt dd-dd-dd tt:tt:tt

u8 BT_State=BT_STA_Null;	//蓝牙状态
u32 BT_DeviceID;			//蓝牙设备ID

extern char TxetTab[][20];

//----蓝牙接口初始化-------------------------------------------------
void BT_Init()
{
	//电源接口
	RCC->APB2ENR|=1<<3;    	//使能PORTC时钟 
 	GPIOC->CRL&=0X0FFFFFFF;	//PC7推挽输出
 	GPIOC->CRL|=0X70000000;	  //开漏
	GPIOC->ODR|=(1<<7);	//输出高电平
	//故障接口
	RCC->APB2ENR|=1<<3;    	//使能PORTC时钟 
 	GPIOC->CRL&=0XFFFFFFF0;	//PC0输入
 	GPIOC->CRL|=0X30000008;
	GPIOC->ODR|=1;			//上拉电平

	com_init(COM2, 115200, 8, 1, 0); 
	delay_ms(10);
	com_clear(COM2);
}
//----发送指令-------------------------------------------------------
void SendCmd(u16 cmd)
{

//	com_clear(COM2);
	com_write(COM2, (u8*)&cmd, 2);
}


//----检测蓝牙模块内部崩溃-------------------------------------------
bool BT_GetErrPin()
{
	//无故障
	if(BT_A1_PIN == 1)
		return FALSE;	
	//有故障
//	PowerBT_OFF();	//关蓝牙电源		
	OLED_Clear();	
	OLED_ShowString(0,0,1,1,"蓝牙故障 !!     ");		
	OLED_Refresh_Gram();	
	return TRUE;
}
//----等待指令，计算超时---------------------------------------------
u16 wait_cmd_time(vu32 t, bool bCancellEnable )
{
	u16 cmd;
	u8 key = 0;			//键盘

	while(t)
	{
		t--;	  
		delay_ms(1);
		if(com_package(COM2)>=2)	//检测接收数据包
		{
			com_read(COM2, (u8*)&cmd, 2);
			return cmd;		//返回蓝牙模块指令
		}

		if( TRUE == bCancellEnable )
		{
			key=keys_scan(); 	 
			//强制退出
			if(key == KEY_CMD_back)
			{
				while(keys_scan()); 			//等待按键脱离
				return OPT_CANCEL;			    //取消操作
			}
			else if((key == KEY_CMD_pow)&&(USB_GetConnFlag()!=TRUE))
			{
				power_off();  	//手动关机
				return KEY_CMD_pow;
			}	
		}

	}
	return TIME_OUT;			//返回超时
}




/////////为了做补丁，蓝牙在链接状态，重新配对过程中突然关机，在关机
//-------------------------------------------------------------------
u16 BT_GetCmd()
{
	u16 cmd;

	//检测蓝牙指令
	if(com_package(COM2)>=2)	//检测接收数据包
	{
		com_read(COM2, (u8*)&cmd, 2);
		return cmd;
	}
	else
		return 0;
}

//----主动连接--------------------------------------------------------
bool BT_Connect_Prog(void)
{
	u16 sta;

	bool ret = FALSE;
	if(BT_State!=BT_STA_Null)
	{
		SendCmd(CmdConnect);
		sta = wait_cmd_time(10000, FALSE);			//等待指令，计算超时  ************120 改 10000


		if(sta == CmdConnect_Ack)
		{ 
		 	BT_State = BT_STA_Connected;
			ret = TRUE;
		}
	}
	return ret;
}

//----可连接状态------------------------------------------------------
bool BT_Connectable_Prog()
{
	u16 sta;
	
	OLED_Clear();	
	OLED_ShowString(0,0,1,1,TxetTab[7]);	//正在连接	
	OLED_Refresh_Gram();			

	SendCmd(CmdGoConnected);			//进入Connectable(可连接)状态
	sta = wait_cmd_time(2000, FALSE);		    //等待指令，计算超时

	if(sta == Connectable)
	{
		BT_Connect_Prog();
		OLED_ShowString(0,0,1,1,"                ");	
		OLED_Refresh_Gram();			
		return TRUE;	
	}

	OLED_ShowString(0,0,1,1,"                ");	
	OLED_Refresh_Gram();
				
	return FALSE;
}  
//----可配对状态-----------------------------------------------------
bool BT_Pairable_Prog()
{
	u16 sta;
	bool flag;
	
	OLED_Clear();	
	OLED_ShowString(0,0,1,1,TxetTab[4]);		//正在配对	
	OLED_Refresh_Gram();	
	
	SendCmd(CmdPair);				//发送进入Pairable(配对)状态指令
	sta = wait_cmd_time(5000, FALSE);		//等待应答指令，计算超时 1000
	if(sta != CmdPair_Ack)
	{
			
		OLED_ShowString(0,0,1,1,"    配对失败    ");
		flag=FALSE;
		goto loop_end;
	}

	sta = wait_cmd_time(120000, TRUE);		//等待指令，计算超时 120000
	if(sta == CmdPair_OK)  			//Pairable(配对)成功
	{
		BT_State = BT_STA_Initial;
//	  	BT_State = BT_STA_Connected;
		OLED_ShowString(0,0,1,1,"    配对成功    ");	
		flag=TRUE;
		goto loop_end;
	}
	if(sta == CmdPair_TOut)
	{
			
		OLED_ShowString(0,0,1,1,"    配对超时    ");	
		flag=FALSE;
		goto loop_end;
	}
	if(sta == OPT_CANCEL)
	{			
		SendCmd(CmdQuitBT);				//退出蓝牙模式
		OLED_ShowString(0,0,1,1,"                ");	
		flag=FALSE;
		goto loop_end;
	}
	if(sta ==KEY_CMD_pow)
	{
		BT_State = BT_STA_Null;	
		return FALSE;	
	}
	   
loop_end:
	if(sta!=CmdPair_OK)	
	{
//		PowerBT_OFF();			//开蓝牙电源
		BT_State = BT_STA_Null;	
	}
	OLED_Refresh_Gram();	
	delay_ms(1000);
	return flag;
}

bool BT_Disconnect_Prog(void)
{
	bool ret = FALSE;
	u16 sta;

	SendCmd(CmdDisconnect);	//发送断开蓝牙指令
	sta = wait_cmd_time(2000, FALSE);		//等待应答指令，计算超时 2000MS
	if(sta==CmdDisconnect_Ack)
		ret = TRUE;	
	return ret; 
}

//----获取蓝牙状态-----------------------------------------------------
u8 BT_GetState()
{
	return BT_State;
}
//----清除蓝牙状态-----------------------------------------------------
void BT_ClrState()
{
	BT_State = BT_STA_Null;
}
//----BT发送数据-------------------------------------------------
bool BT_SendDatas(u8* buf, u16 lenth)
{
	u16 sta;
//	u8 rceivedconfirm = 1;

	SendCmd(CmdSend);						//发送指令
	com_write(COM2, (u8*)&lenth, 2);		//发送数据长度  *************增加**************

	com_write(COM2, buf, lenth);			//发送数据
	while(1)
	{
		sta = wait_cmd_time(1000, FALSE);		//等待指令，超时100mS **************1500 改1000
		if(sta == CmdSend_Ack)
			return TRUE;
		else if(IsCmdValid(sta)==TRUE)
		{
			ProCmd(sta);	
		}
		else
			break;
	}
	return FALSE;
}
	
//----蓝牙关闭----------------------------------------------------------
void BT_Stop_Prog()
{
	u16 sta;

	if(BT_State != BT_STA_Null)
	{	
		OLED_Clear();	
		OLED_ShowString(0,0,1,1,TxetTab[6]);	//关闭蓝牙	
		OLED_Refresh_Gram();	
	 	SendCmd(CmdQuitBT);		//发送退出蓝牙指令
		if(BT_State == BT_STA_Connected)
		{
			sta = wait_cmd_time(2000, FALSE);				//等待指令，超时2S ********只有在连接状态下，等待回应。延时关机
		}	 
		sta = sta;

	//	delay_s(2);
		OLED_ShowString(0,0,1,1,"                ");	
		OLED_Refresh_Gram();
		
	}
//	PowerBT_OFF();			//开蓝牙电源
	BT_State = BT_STA_Null;		
}

///蓝牙复位
static void BTReset(void)
{
	PowerBT_OFF();
	delay_ms(10);
	PowerBT_ON();				
		
}
void AppBTPowerOff(void)
{
	u16	sta;

    BTReset();
	sta = wait_cmd_time(5000, TRUE);	//等待指令，超时100mS
	sta = sta;
	BT_State = BT_STA_Null;
	
}

u16 AppBTPowerOn(void)
{
  	u16 sta;

	BTReset();
	com_clear(COM2);
	sta = wait_cmd_time(5000, FALSE);	//等待指令，超时100mS
	if((sta!=WithNoHostAdd)&&(sta!=WithHostAdd))
	{
		BT_State = BT_STA_Null;
//		PowerBT_OFF();			//关蓝牙电源	
		OLED_ShowString(0,0,1,1,"   蓝牙启动失败   ");	
		OLED_Refresh_Gram();
		delay_s(1);	
		
	}
	else
	{
		BT_State = BT_STA_Initial;	
	}

	return sta;
	
}

//----蓝牙启动----------------------------------------------------------
u16 BT_bootup()
{
	u16 sta = TIME_OUT;

	OLED_Clear();			//刷新屏幕
	OLED_ShowString(0,0,1,1,TxetTab[9]);	//启动蓝牙
	OLED_Refresh_Gram();
	delay_s(1);	

	com_init(COM2, 115200, 8, 1, 0);
	sta = AppBTPowerOn();
//	if((sta!=WithNoHostAdd)&&(sta!=WithHostAdd))
//	{
//		BT_State = BT_STA_Null;
//		save_para_BT_enable(0);		//保存参数
////		PowerBT_OFF();			  //关蓝牙电源	
//		OLED_ShowString(0,0,1,1,"   蓝牙启动失败   ");	
//		OLED_Refresh_Gram();
//		delay_s(1);	
//	}
	return sta;	 //

}

//==============================================================
//----发送ID-----------------------------------------------------------------------
bool SendInitDeviceID()
{
	u16 cmd = CmdInitDeviceID;
	u32 id= HWREG(FLASE_ID_ADDR);
	u16 sta;	

	com_clear(COM2);
	com_write(COM2, (u8*)&cmd, 2);
	com_write(COM2, (u8*)&id, 4);

	sta = wait_cmd_time(5000, FALSE);	//等待指令，超时100mS
	if( sta == CmdInitDeviceID_Ack)
		return TRUE;
	else
	{
		BT_State = BT_STA_Null;
//		PowerBT_OFF();			//关蓝牙电源	
		OLED_ShowString(0,0,1,1,"   蓝牙启动失败   ");	
		OLED_Refresh_Gram();
		delay_s(1);	
	}
	return FALSE;
}
//===============================================================


//----蓝牙启动---------------------------------------------------------------------
bool BT_Start_Prog()
{
    u16 sta = BT_bootup();
	bool ret = FALSE;

	//无主机地址，进行配对
	if(sta == WithNoHostAdd)
	{
		if(SendInitDeviceID()==TRUE)
		{	
			if(BT_Pairable_Prog()==TRUE)
				ret = TRUE;
			else
				save_para_BT_enable(0);		//保存参数
		}
	}
   //待机-曾经配对过
	else
	if( sta == WithHostAdd)
	{					
		if(SendInitDeviceID())
		{
			ret = TRUE;
			BT_Connectable_Prog();
		}
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////
//----蓝牙使用切换---------------------------------------------------------------		  		  
void bt_sw()
{
	//未使用蓝牙，则开启
	if(BT_GetState()==BT_STA_Null)
	{
		if(BT_Start_Prog()==TRUE)			//启动蓝牙
		{
		 	if(BT_GetState()!=BT_STA_Null)
				save_para_BT_enable(1);	//保存参数
		}
		else
		{
//			PowerBT_OFF();			//关蓝牙电源
			BT_State = BT_STA_Null;	
			save_para_BT_enable(0);		//保存参数		
		}
	
	}
	//已使用蓝牙，则关闭
	else
	{
		BT_Stop_Prog();				//蓝牙关闭
		save_para_BT_enable(0);		//保存参数
	}
}
//----蓝牙配对---------------------------------------------------------------------		  		  
bool bt_pair()
{
	u16 sta;
	bool ret = FALSE;

	//save_para_BT_enable(0);		//清除参数
	//蓝牙已连接
	if(BT_GetState()==BT_STA_Connected)
	{
		
		if(BT_Disconnect_Prog()==TRUE)
		{
			BT_State = BT_STA_Initial;
			if(BT_Pairable_Prog()==TRUE)//蓝牙配对
			{
				save_para_BT_enable(1);		//保存参数
				return TRUE;
			}
			else
			{  
			 	save_para_BT_enable(0);		//保存参数
				return FALSE;
			}	
		}
		else
		{
			AppBTPowerOff();
			save_para_BT_enable(0);		//保存参数
		}	
	}					
	//未使用蓝牙，则开启
	if(BT_GetState()==BT_STA_Null)
	{
		sta=BT_bootup();
		if((sta!=WithNoHostAdd)&&(sta!=WithHostAdd) )//启动失败
		{
			save_para_BT_enable(0);		//保存参数
			return FALSE;
		}
		else
		{
			if(SendInitDeviceID()==FALSE)
			{
				save_para_BT_enable(0);		//保存参数
				return FALSE;
			}
		}
	}
	if(BT_Pairable_Prog()==TRUE)
	{
		ret = TRUE;
		save_para_BT_enable(1);	//保存参数	
	}
	else
	{
		ret = FALSE;
		save_para_BT_enable(0);		//保存参数
	}

    return ret;		//蓝牙配对 
}

//----检索蓝牙设备ID-------------------------------------------------------
u32 BT_Browse_DeviceID()
{
	return BT_DeviceID;
}
