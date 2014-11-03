
#include <string.h>
#include "stm32f10x.h"
#include "MyHdriver.h"
#include "usb_driver.h"
#include "usb_istr.h"
#include "delay.h"
#include "beep.h"
#include "key.h"
#include "oled.h"
#include "flash.h" 
#include "power.h" 
#include "PCF8563.h"

#include "RFID.h"
#include "file.h"	   
#include "menu.h"
#include "bluetooth.h"
#include "DataTransfer.h"
#include "MyUsart.h"
#include "config.h"
#include "platform.h"

#include "InternalFlash.h"
#include "MyShow.h"
#include "main.h"
#include "log.h"



struct read_rfid_time
{
	u32 id;						//前一次卡号
	u16 time;					//前一次读卡时间计数器
}RRT;


u32 timecount; 
u8 Usb_Status_Reg=0;
extern char TxetTab[][20];
extern u8 Nowaday_Con;		//当前使用的通讯接口
char DateString[12], TimeString[12];

RECORDINF* pstoreaddrlist;



//----检索RFID记录------------------------------------------------------------------
void browse(void)
{
	FILE_RECORD_STRUCT record;
	u16 CountMax;		//记录条数
	int count;			//检索计数器
	u32 addr;			//当前行地址
	char buff[20];
	u8 key;
	u8 step=1;			//单步检索方式
	char Date[12], Time[12];

	
	CountMax = GetStoreCount();	//提取参数
	OLED_Clear();
	//没有记录
	if(CountMax == 0)
	{
		OledClearandShow(FIRSMALLFONTH, TxetTab[8], 0);	 
		delay_s(2);
		goto loop_end;	
	}

	count=0;
	while(1)
	{
		delay_ms(250);

		//计算浏览的行号
//		if(count==0)
//			addr = FILE_DATA_ADDR;
//		else
			addr = FILE_DATA_ADDR + (count ) * sizeof(FILE_RECORD_STRUCT);		//计算地址
		SPI_Flash_Read((u8*)&record, addr, sizeof(FILE_RECORD_STRUCT));//读取一条记录

		sprintf(buff, "Record %04d/%04d", count+1, CountMax);	
		OLED_ShowString(0, 0, 1,1,buff);

		TimerCount_To_DateTimerStr(Date, Time, record.time);	//秒钟数转换为字符串时间
		OLED_ShowString(0,16,1,1,Date);	//显示日期	
		OLED_ShowString(0,32,1,1,Time);	//显示时间
		sprintf(buff, "%08X", record.rfid);
		OLED_ShowString(0,48,1,1,(char*)buff);	//显示ID
		OLED_Refresh_Gram();
	   	
		//扫描键盘，如果没有键盘操作，30秒后关闭电源（节电）
		key=scan_key_time_power();

		//上移
		if(key==KEY_CMD_up)
		{
			//单步检索方式
			if(step==1)
				count--;
			//20步检索方式
			else
				count-=50;

			if(count<0)
				count = CountMax-1;
		}
		//下移
		else
		if(key==KEY_CMD_down)
		{
			//单步检索方式
			if(step==1)
				count++;
			//20步检索方式
			else
				count+=50;

			if(count>=CountMax)
				count=0;
		}
		//改变检索步距	
		else
		if(key==KEY_CMD_ok)		
		{
			step=~step;
			OLED_Clear();
		}
		//退出
		else
		if(key==KEY_CMD_back)		
			goto loop_end;
		else if(key==KEY_CMD_pow)	  //添加fan
			return;
	}

loop_end:
	OLED_Clear();			//刷新屏幕
	OLED_Refresh_Gram();	//刷新显示	 
	while(keys_scan());  	//等待按键脱离
}

bool SaveId(FILE_RECORD_STRUCT id)
{

	u32 addr;
			
	addr = FILE_DATA_ADDR + (pstoreaddrlist->RecordCount ) * sizeof(FILE_RECORD_STRUCT);

	if(SPI_Flash_WriteV((u8*)&id, addr, sizeof(FILE_RECORD_STRUCT))==FALSE)	 //保存id信息 	
	{

		return FALSE;
	}
	else
	{	 
		SaveDate(pstoreaddrlist);
	//	WriteFlashV32(PLACELASTID, addr); 
		return TRUE;		
	}
}


bool BTDateTransfer(FILE_RECORD_STRUCT *ptag)
{
    bool ret = FALSE;
	BTSEND sendbuf;
	u8 TagSendFlag = 1;	
	BtReceiveNoDateCMD BtReCmdbuf;
	u16 ndata;

	sendbuf.cmd = CmdSendCurTagInfo;
	sendbuf.lenth = 0;
	sendbuf.uid = ptag->rfid;
	sendbuf.time = ptag->time;
	sendbuf.crc1=0, 
	sendbuf.crc2=0;
	sendbuf.eof=0xCACA;	

	if((BT_GetState() != BT_STA_Connected))		//&&(BT_Connect_Prog()==FALSE)
	{
		if(BT_Connect_Prog()==FALSE)
			return ret;		 //连接失败返回
	}
	BT_SendDatas((u8*)&sendbuf, sizeof(BTSEND));	//发送一条刷卡记录
	while(TagSendFlag)
	{
		ndata = BT_RecvDatas((u8*)&BtReCmdbuf, sizeof(BtReceiveNoDateCMD), 3000);	  //接收确认
		if((ndata > 0) && (BtReCmdbuf.cmd == CmdSendCurTagInfo_Ack))     // 发送成功
		{
			TagSendFlag = 0;
			ret = TRUE;
		}
		else
		if(IsCmdValid(BtReCmdbuf.cmd))
		{
			Nowaday_Con = BT_CON;
			ProCmd(BtReCmdbuf.cmd);
			ndata = 0;
			memset( &BtReCmdbuf, 0x00, sizeof(BtReCmdbuf));
		} 
		else
			break;
	}
	return ret;			
}

//----扫描卡片-----------------------------------------------------------------------
void rfid_scan(void)
{
	u32 UID=0;    

	LOCATION location1={0,16,2,2};
	LOCATION location2={0,24,1,1};

	char RFID_buff[12]; 
	FILE_RECORD_STRUCT card;
	u8 scancount = 0;
    IDPROCESS process;

    MyShowOled(&location1, &location2, "    正在扫描    ");
	delay_ms(50);
	while(scancount<SCAN_TIME_OUT) //扫描次数
	{
		//读取卡号
		//delay_ms(100);  //不容忽视
		UID=RFID_Read(1);
		if(UID==0)
			UID = RFID_Read(2);
		scancount++;
		RRT.time = RRT_TIME_10S;
		if(UID)	
		{	
			HTRC110_Close();				//关HTRC110电源
			scancount = SCAN_TIME_OUT;	//一旦扫描到数据，次数置0不再扫描第2次
			card.time = timecount;
			card.rfid = UID;

			process = (IDPROCESS) SearchId(card);		//保存一条刷卡记录

			switch(process)
			{
				case savedate:	  ////存储数据。并判断是否需要蓝牙发送
			    
					
					sprintf(RFID_buff, "%08X", UID);
					MyShowOled(&location1, &location1, RFID_buff);
					beep(1, 100, 1);
					if(BT_GetState() != BT_STA_Null)//蓝牙是否打开
					{
						if(BTDateTransfer(&card)==TRUE)
						{
							RRT.time = 0;
						#ifdef K_COSTUMER_COCACOLA	
							break;
						#endif
						}
				    }

					if(SaveId(card)==TRUE) //是否正确存储
					{	 
						RRT.time = 0;	
					}
					else
					{	//存储错误
	              	    MyShowOled(&location1, &location2, "System Fault: 4");
						delay_ms(500);
						break;
					}
					   	
				   break;

				case repeatid:
					MyShowOled(&location1, &location2, "    卡号重复    ");
					delay_ms(500);
					break;

				case memoryfull:
			 	    MyShowOled(&location1, &location2, "    记录已满    ");
				    delay_ms(500);
				    break;

				case timeerror :
					MyShowOled(&location1, &location2, "  系统时间冲突  ");
					delay_ms(500);
					break;
			    
				case systemerror:
					MyShowOled(&location1, &location2, "System Fault: 3");
					delay_ms(500);
					break;

				default: 
					break;
			
		  }	 
		}
	}
	HTRC110_Close();				//关HTRC110电源
}

//----初始化程序-------------------------------------------------------------------
void init()
{
	u32 vol;
	u32 addr;
	char DateSt[12], TimeSt[12];
	FILE_RECORD_STRUCT lastsavecard;

	Hdriver_init();					//初始化硬件设备
	delay_init();

  	usb_cable(DISABLE);
	POWER_Init();			//初始化电源管理接口 和采样通道
	PowerCPU_On();			//开CPU电源	
	delay_ms(30);
	keys_init();			//初始化按键
	
	OLED_Init();  //初始化OLED
	BT_Init();
	com_init(COM3, 115200, 8, 1, 0);

	if(initPlatform()==FALSE)  ///增加
	{
		OledClearandShow(FIRSMALLFONTH, "System Fault: 5", 1);
		MyDebugPrint("KEY-13 ERROR!!!!!!!!!!\r\n");
		while(1)
		{  ;
//		    if(keys_wait(0)==KEY_CMD_pow)	//修改 fan
//			{
//			    beep(1, 100, 100);
//				power_off2(); 
//			}   
		}
		
	}	
	vol = BatteryGet();	  //检测电压
	if((vol < BATT_LOW)&&(test_v5()==FALSE) )
	{
		while(keys_scan());  		//等待按键脱离
		power_off2();				//手动关机
	}
		   
	beep(1, 100, 100);

	PCF8563_Init();	        		//初始化RTC,返回0,失败;1,成功;
	OLED_Clear();
	OLED_ShowString(0,16,1,1,TxetTab[0]);	//正在开机	
	OLED_Refresh_Gram();	 
	delay_ms(300);


	//初始化SPI接口，检测SPI-FLASH存储器
	if(!SPI_Flash_Init())
	{
		OledClearandShow(FIRSMALLFONTH, "System Fault: 1", 1);
		MyDebugPrint("KEY-10 ERROR!!!!!!!!!!\r\n");
		while(1)
		{
		    if(keys_wait(0)==KEY_CMD_pow)	//修改 fan
			{
			    beep(1, 100, 100);
				power_off2(); 
			}   
		}
	}
	MyDebugPrint("KEY-10 Passed\r\n");
				 	
	if(!HTRC110_Init(1))	 //初始化HTRC110
	{   
		OledClearandShow(FIRSMALLFONTH, "System Fault: 2", 1); 
		MyDebugPrint("KEY-11 ERROR!!!!!!!!!!\r\n");
		while(1)
		{
		    if(keys_wait(0)==KEY_CMD_pow)  //修改	fan
			{
			    beep(1, 100, 100);
			    power_off2(); 
			}	 
		}
	}
	MyDebugPrint("KEY-11 Passed\r\n");
	HTRC110_Close();	
	usb_init();
	FileInit();
	pstoreaddrlist = GetStoreListInfo();
	addr = FILE_DATA_ADDR + (pstoreaddrlist->RecordCount)*sizeof(FILE_RECORD_STRUCT);

	if(pstoreaddrlist->RecordCount==0)
		LastSaveTime = GetRTC(DateSt, TimeSt);///获取当前时间，秒数 
	else
	{
		addr = FILE_DATA_ADDR + (pstoreaddrlist->RecordCount - 1)*sizeof(FILE_RECORD_STRUCT);
		SPI_Flash_Read((u8*)&lastsavecard, addr, sizeof(FILE_RECORD_STRUCT));
		LastSaveTime = lastsavecard.time;
	}				
}
//----300mS刷新时间显示------------------------------------------------------------

//----监测电池电压----------300ms---------------------------------------------
void battery_show()
{
	u32 vol;
	u32 chrg;
	static u32 last_vol=BATT_HIGH;

	vol=BatteryGet();	//获取电池电压
	if( test_v5())		//检测V5电压
	{
		vol -= 150;	
		chrg = 3300*(Get_Adc(12))/4095;
		if(chrg>=CHARGEFULL)	
	    {
		    last_vol=BATT_HIGH;
		    OLED_ShowString(BATT_ICON_XY,1,1,"\x88\x87");	//显示电池电压高
			return;
	    }

	    if(last_vol==BATT_LOW)
		{
			OLED_ShowString(BATT_ICON_XY, 1,1,"\x84\x85");	//显示电池电压低
			last_vol = BATT_MID;
		}
		else
		if(last_vol==BATT_HIGH)
		{
			OLED_ShowString(BATT_ICON_XY,1,1,"\x88\x87");	//显示电池电压高
			last_vol=BATT_LOW;
		}
		else
		{
			OLED_ShowString(BATT_ICON_XY,1,1,"\x86\x87");	//显示电池电压中
			last_vol=BATT_HIGH;
		}
	
	}
	else 
    {

	  	if(last_vol==BATT_LOW)
		{
			OLED_ShowString(BATT_ICON_XY, 1,1,"\x84\x85");	//显示电池电压低
			if(vol > BATT_MID_L)		
				last_vol = BATT_MID;
		}
		else
		if(last_vol==BATT_HIGH)
		{
			OLED_ShowString(BATT_ICON_XY,1,1,"\x88\x87");	//显示电池电压高
			if(vol < BATT_MID_H)
				last_vol=BATT_MID;
		}
		else
		{
			OLED_ShowString(BATT_ICON_XY,1,1,"\x86\x87");	//显示电池电压中
			if((vol > BATT_HIGH))
				last_vol=BATT_HIGH;
			else
			if(vol < BATT_LOW)
				last_vol=BATT_LOW;
		}
    }  	 

}

//----刷新主界面--------------------------------------------------------------
void refresh_show_time()
{
	static u16 t1=301;//开机读取时间
	u8 sta;
			
	t1++;
	if(t1>300)
	{
		t1=0;
		//显示日期
		timecount = GetRTC(DateString, TimeString);			//获取RTC日期字符串
		OLED_ShowString(0,0,1,1,"                ");		
		OLED_ShowString(0,0,1,1,DateString);		
		//显示时间
		if(RRT.time>=RRT_TIME_10S)
		{
			OLED_ShowString(0,16,2,2,"        ");	//显示时间
			OLED_ShowString(0,16,2,2,TimeString);	//显示时间
		}
		//获取蓝牙状态
		sta = BT_GetState();
		if(sta == BT_STA_Initial) 
			OLED_ShowString(BT_ICON_XY, 1,1,"\x80\x81");	//蓝牙初始化图标	
		else
		if(sta == BT_STA_Connected) 
			OLED_ShowString(BT_ICON_XY, 1,1,"\x82\x83");	//蓝牙已连接图标
		else
			OLED_ShowString(BT_ICON_XY, 1,1,"  ");		//关图标

		battery_show();		//监测电池电压
		OLED_Refresh_Gram();					//刷新显示
   }
}



//----检测天线，卡片-----------------------------------------------------------
void test_card()
{
	u8 key;			//键盘
	u32 UID=0;
	char RFID_buff[12];
	bool pow_vcc=FALSE, pow_bt=FALSE;

	OledClearandShow(FIRSMALLFONTH, "   Test  Card   ", 1);
	while(keys_scan());  				//等待按键脱离

	while(1)
	{
		//检测键盘指令
		key=keys_wait(0);

		if(key)
		{
			switch(key)
			{
			case KEY_CMD_pow:
				power_off();		//手动关机
				break;

			case KEY_CMD_up:
				OLED_ShowString(0,0,1,1,"    125 Card   ");	
				UID=RFID_Read(1);
				break;
			case KEY_CMD_down:
				OLED_ShowString(0,0,1,1,"    134 Card   ");	
				UID=RFID_Read(2);
				break;
			case KEY_CMD_menu:	//5V电源
				if(!pow_vcc)
				{
					pow_vcc=TRUE;
					OLED_ShowString(0,0,1,1," Card Power On ");	
					PowerHTRC110=1;			//HTRC110电源开
				}
				else
				{
					pow_vcc=FALSE;
					OLED_ShowString(0,0,1,1," Card Power Off");	
					PowerHTRC110=0;			//HTRC110电源开
				}
				beep(1, 50, 1);
				while(keys_scan());
				break;
			case KEY_CMD_back:	//bt电源
				if(!pow_bt)
				{
					pow_bt=TRUE;
					OLED_ShowString(0,0,1,1," BT Power On   ");	
					PowerBT_ON();			//开蓝牙电源
				}
				else
				{
					pow_bt=FALSE;
					OLED_ShowString(0,0,1,1," BT Power Off  ");	
//					PowerBT_OFF();			//关蓝牙电源
				}
				beep(1, 50, 1);
				while(keys_scan());
				break;
			}

			if(UID!=0)
			{
				sprintf(RFID_buff, "%08X", UID);
				//显示卡号
				OLED_ShowString(0,24,2,2,RFID_buff);	
				beep(1, 50, 1);
				UID=0;
			}
			OLED_Refresh_Gram();
		}
	}
}
//----USB应用-------------------------------------------------------------------
void usb_app()
{
	delay_ms(50);
	if(!USB_GetConnFlag())
		return;

	OledClearandShow(FIRSMALLFONTH, "   USB 已连接   ", 1);

	while(1)
	{
 		delay_ms(1);
		//检测usb连接
		if(!USB_GetConnFlag())
			break;
	
		con_cmd();	//USB指令 bt指令
	}
}

//----主流程-----------------------------------------------------------------------		  		  
int main(void)

{
	u8 key;			//键盘
	u32 t=0;	 //超时计数器
	u16 i;	

	init();		//初始化程序
	for(i=0; i<10; i++)
	{
		if(DetectKeys()==0x14)
			test_card();
	}
	for(i=0; i<10; i++)
	{
		if(DetectKeys()==0x24)
			TestModel();
	}
	  
	while(keys_scan());  				//等待按键脱离
	BT_ClrState();						//清除蓝牙状态
	if	(read_para_BT_enable()==1)
		BT_Start_Prog();				//蓝牙启动

	//清除显示界面
	OLED_Clear();			//刷新屏幕
	OLED_Refresh_Gram();	//刷新显示	
		
	//主循环
	RRT.time=RRT_TIME_10S;

	while(1)
	{	
		delay_ms(1);
		t++;
		if(t>TIME30S)	
		{	
			t=0;
			power_off();		//关机  //CPU电源	 	
		}
		if(RRT.time<RRT_TIME_10S)
			RRT.time++; 
		//300mS刷新时间显示
		refresh_show_time();
		//检测键盘指令
		key = DetectKeys();		    
		if(key)
		{
			t=0;	
			switch(key)
			{
			case KEY_CMD_pow:
				power_off();		//手动关机
				break;

			case KEY_CMD_menu:
				key = Main_Menu();		//进入菜单
				if(key==KEY_CMD_BTsw)
					bt_sw();			//蓝牙连机切换
				else
				if(key==KEY_CMD_bro)
					browse();			//进入检索RFID记录界面
				RRT.time = RRT_TIME_10S;	
				break;

			case KEY_CMD_scan:
				rfid_scan();		//扫描卡片
				break;	

			case KEY_CMD_bro:
				browse();			//进入检索RFID记录界面
				RRT.time=RRT_TIME_10S;	
				break;

			case KEY_CMD_BTsw:
		  	    bt_sw();
				RRT.time = RRT_TIME_10S;			//蓝牙连机切换	  fan 改
				break;

			default	:
				break;
			}
		}

		//检测通讯指令
		if(con_cmd())
			t=0;

		//检测usb连接
		if(USB_GetConnFlag())
			usb_app();
	}	   		  
}   

