
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
	u32 id;						//ǰһ�ο���
	u16 time;					//ǰһ�ζ���ʱ�������
}RRT;


u32 timecount; 
u8 Usb_Status_Reg=0;
extern char TxetTab[][20];
extern u8 Nowaday_Con;		//��ǰʹ�õ�ͨѶ�ӿ�
char DateString[12], TimeString[12];

RECORDINF* pstoreaddrlist;



//----����RFID��¼------------------------------------------------------------------
void browse(void)
{
	FILE_RECORD_STRUCT record;
	u16 CountMax;		//��¼����
	int count;			//����������
	u32 addr;			//��ǰ�е�ַ
	char buff[20];
	u8 key;
	u8 step=1;			//����������ʽ
	char Date[12], Time[12];

	
	CountMax = GetStoreCount();	//��ȡ����
	OLED_Clear();
	//û�м�¼
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

		//����������к�
//		if(count==0)
//			addr = FILE_DATA_ADDR;
//		else
			addr = FILE_DATA_ADDR + (count ) * sizeof(FILE_RECORD_STRUCT);		//�����ַ
		SPI_Flash_Read((u8*)&record, addr, sizeof(FILE_RECORD_STRUCT));//��ȡһ����¼

		sprintf(buff, "Record %04d/%04d", count+1, CountMax);	
		OLED_ShowString(0, 0, 1,1,buff);

		TimerCount_To_DateTimerStr(Date, Time, record.time);	//������ת��Ϊ�ַ���ʱ��
		OLED_ShowString(0,16,1,1,Date);	//��ʾ����	
		OLED_ShowString(0,32,1,1,Time);	//��ʾʱ��
		sprintf(buff, "%08X", record.rfid);
		OLED_ShowString(0,48,1,1,(char*)buff);	//��ʾID
		OLED_Refresh_Gram();
	   	
		//ɨ����̣����û�м��̲�����30���رյ�Դ���ڵ磩
		key=scan_key_time_power();

		//����
		if(key==KEY_CMD_up)
		{
			//����������ʽ
			if(step==1)
				count--;
			//20��������ʽ
			else
				count-=50;

			if(count<0)
				count = CountMax-1;
		}
		//����
		else
		if(key==KEY_CMD_down)
		{
			//����������ʽ
			if(step==1)
				count++;
			//20��������ʽ
			else
				count+=50;

			if(count>=CountMax)
				count=0;
		}
		//�ı��������	
		else
		if(key==KEY_CMD_ok)		
		{
			step=~step;
			OLED_Clear();
		}
		//�˳�
		else
		if(key==KEY_CMD_back)		
			goto loop_end;
		else if(key==KEY_CMD_pow)	  //���fan
			return;
	}

loop_end:
	OLED_Clear();			//ˢ����Ļ
	OLED_Refresh_Gram();	//ˢ����ʾ	 
	while(keys_scan());  	//�ȴ���������
}

bool SaveId(FILE_RECORD_STRUCT id)
{

	u32 addr;
			
	addr = FILE_DATA_ADDR + (pstoreaddrlist->RecordCount ) * sizeof(FILE_RECORD_STRUCT);

	if(SPI_Flash_WriteV((u8*)&id, addr, sizeof(FILE_RECORD_STRUCT))==FALSE)	 //����id��Ϣ 	
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
			return ret;		 //����ʧ�ܷ���
	}
	BT_SendDatas((u8*)&sendbuf, sizeof(BTSEND));	//����һ��ˢ����¼
	while(TagSendFlag)
	{
		ndata = BT_RecvDatas((u8*)&BtReCmdbuf, sizeof(BtReceiveNoDateCMD), 3000);	  //����ȷ��
		if((ndata > 0) && (BtReCmdbuf.cmd == CmdSendCurTagInfo_Ack))     // ���ͳɹ�
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

//----ɨ�迨Ƭ-----------------------------------------------------------------------
void rfid_scan(void)
{
	u32 UID=0;    

	LOCATION location1={0,16,2,2};
	LOCATION location2={0,24,1,1};

	char RFID_buff[12]; 
	FILE_RECORD_STRUCT card;
	u8 scancount = 0;
    IDPROCESS process;

    MyShowOled(&location1, &location2, "    ����ɨ��    ");
	delay_ms(50);
	while(scancount<SCAN_TIME_OUT) //ɨ�����
	{
		//��ȡ����
		//delay_ms(100);  //���ݺ���
		UID=RFID_Read(1);
		if(UID==0)
			UID = RFID_Read(2);
		scancount++;
		RRT.time = RRT_TIME_10S;
		if(UID)	
		{	
			HTRC110_Close();				//��HTRC110��Դ
			scancount = SCAN_TIME_OUT;	//һ��ɨ�赽���ݣ�������0����ɨ���2��
			card.time = timecount;
			card.rfid = UID;

			process = (IDPROCESS) SearchId(card);		//����һ��ˢ����¼

			switch(process)
			{
				case savedate:	  ////�洢���ݡ����ж��Ƿ���Ҫ��������
			    
					
					sprintf(RFID_buff, "%08X", UID);
					MyShowOled(&location1, &location1, RFID_buff);
					beep(1, 100, 1);
					if(BT_GetState() != BT_STA_Null)//�����Ƿ��
					{
						if(BTDateTransfer(&card)==TRUE)
						{
							RRT.time = 0;
						#ifdef K_COSTUMER_COCACOLA	
							break;
						#endif
						}
				    }

					if(SaveId(card)==TRUE) //�Ƿ���ȷ�洢
					{	 
						RRT.time = 0;	
					}
					else
					{	//�洢����
	              	    MyShowOled(&location1, &location2, "System Fault: 4");
						delay_ms(500);
						break;
					}
					   	
				   break;

				case repeatid:
					MyShowOled(&location1, &location2, "    �����ظ�    ");
					delay_ms(500);
					break;

				case memoryfull:
			 	    MyShowOled(&location1, &location2, "    ��¼����    ");
				    delay_ms(500);
				    break;

				case timeerror :
					MyShowOled(&location1, &location2, "  ϵͳʱ���ͻ  ");
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
	HTRC110_Close();				//��HTRC110��Դ
}

//----��ʼ������-------------------------------------------------------------------
void init()
{
	u32 vol;
	u32 addr;
	char DateSt[12], TimeSt[12];
	FILE_RECORD_STRUCT lastsavecard;

	Hdriver_init();					//��ʼ��Ӳ���豸
	delay_init();

  	usb_cable(DISABLE);
	POWER_Init();			//��ʼ����Դ����ӿ� �Ͳ���ͨ��
	PowerCPU_On();			//��CPU��Դ	
	delay_ms(30);
	keys_init();			//��ʼ������
	
	OLED_Init();  //��ʼ��OLED
	BT_Init();
	com_init(COM3, 115200, 8, 1, 0);

	if(initPlatform()==FALSE)  ///����
	{
		OledClearandShow(FIRSMALLFONTH, "System Fault: 5", 1);
		MyDebugPrint("KEY-13 ERROR!!!!!!!!!!\r\n");
		while(1)
		{  ;
//		    if(keys_wait(0)==KEY_CMD_pow)	//�޸� fan
//			{
//			    beep(1, 100, 100);
//				power_off2(); 
//			}   
		}
		
	}	
	vol = BatteryGet();	  //����ѹ
	if((vol < BATT_LOW)&&(test_v5()==FALSE) )
	{
		while(keys_scan());  		//�ȴ���������
		power_off2();				//�ֶ��ػ�
	}
		   
	beep(1, 100, 100);

	PCF8563_Init();	        		//��ʼ��RTC,����0,ʧ��;1,�ɹ�;
	OLED_Clear();
	OLED_ShowString(0,16,1,1,TxetTab[0]);	//���ڿ���	
	OLED_Refresh_Gram();	 
	delay_ms(300);


	//��ʼ��SPI�ӿڣ����SPI-FLASH�洢��
	if(!SPI_Flash_Init())
	{
		OledClearandShow(FIRSMALLFONTH, "System Fault: 1", 1);
		MyDebugPrint("KEY-10 ERROR!!!!!!!!!!\r\n");
		while(1)
		{
		    if(keys_wait(0)==KEY_CMD_pow)	//�޸� fan
			{
			    beep(1, 100, 100);
				power_off2(); 
			}   
		}
	}
	MyDebugPrint("KEY-10 Passed\r\n");
				 	
	if(!HTRC110_Init(1))	 //��ʼ��HTRC110
	{   
		OledClearandShow(FIRSMALLFONTH, "System Fault: 2", 1); 
		MyDebugPrint("KEY-11 ERROR!!!!!!!!!!\r\n");
		while(1)
		{
		    if(keys_wait(0)==KEY_CMD_pow)  //�޸�	fan
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
		LastSaveTime = GetRTC(DateSt, TimeSt);///��ȡ��ǰʱ�䣬���� 
	else
	{
		addr = FILE_DATA_ADDR + (pstoreaddrlist->RecordCount - 1)*sizeof(FILE_RECORD_STRUCT);
		SPI_Flash_Read((u8*)&lastsavecard, addr, sizeof(FILE_RECORD_STRUCT));
		LastSaveTime = lastsavecard.time;
	}				
}
//----300mSˢ��ʱ����ʾ------------------------------------------------------------

//----����ص�ѹ----------300ms---------------------------------------------
void battery_show()
{
	u32 vol;
	u32 chrg;
	static u32 last_vol=BATT_HIGH;

	vol=BatteryGet();	//��ȡ��ص�ѹ
	if( test_v5())		//���V5��ѹ
	{
		vol -= 150;	
		chrg = 3300*(Get_Adc(12))/4095;
		if(chrg>=CHARGEFULL)	
	    {
		    last_vol=BATT_HIGH;
		    OLED_ShowString(BATT_ICON_XY,1,1,"\x88\x87");	//��ʾ��ص�ѹ��
			return;
	    }

	    if(last_vol==BATT_LOW)
		{
			OLED_ShowString(BATT_ICON_XY, 1,1,"\x84\x85");	//��ʾ��ص�ѹ��
			last_vol = BATT_MID;
		}
		else
		if(last_vol==BATT_HIGH)
		{
			OLED_ShowString(BATT_ICON_XY,1,1,"\x88\x87");	//��ʾ��ص�ѹ��
			last_vol=BATT_LOW;
		}
		else
		{
			OLED_ShowString(BATT_ICON_XY,1,1,"\x86\x87");	//��ʾ��ص�ѹ��
			last_vol=BATT_HIGH;
		}
	
	}
	else 
    {

	  	if(last_vol==BATT_LOW)
		{
			OLED_ShowString(BATT_ICON_XY, 1,1,"\x84\x85");	//��ʾ��ص�ѹ��
			if(vol > BATT_MID_L)		
				last_vol = BATT_MID;
		}
		else
		if(last_vol==BATT_HIGH)
		{
			OLED_ShowString(BATT_ICON_XY,1,1,"\x88\x87");	//��ʾ��ص�ѹ��
			if(vol < BATT_MID_H)
				last_vol=BATT_MID;
		}
		else
		{
			OLED_ShowString(BATT_ICON_XY,1,1,"\x86\x87");	//��ʾ��ص�ѹ��
			if((vol > BATT_HIGH))
				last_vol=BATT_HIGH;
			else
			if(vol < BATT_LOW)
				last_vol=BATT_LOW;
		}
    }  	 

}

//----ˢ��������--------------------------------------------------------------
void refresh_show_time()
{
	static u16 t1=301;//������ȡʱ��
	u8 sta;
			
	t1++;
	if(t1>300)
	{
		t1=0;
		//��ʾ����
		timecount = GetRTC(DateString, TimeString);			//��ȡRTC�����ַ���
		OLED_ShowString(0,0,1,1,"                ");		
		OLED_ShowString(0,0,1,1,DateString);		
		//��ʾʱ��
		if(RRT.time>=RRT_TIME_10S)
		{
			OLED_ShowString(0,16,2,2,"        ");	//��ʾʱ��
			OLED_ShowString(0,16,2,2,TimeString);	//��ʾʱ��
		}
		//��ȡ����״̬
		sta = BT_GetState();
		if(sta == BT_STA_Initial) 
			OLED_ShowString(BT_ICON_XY, 1,1,"\x80\x81");	//������ʼ��ͼ��	
		else
		if(sta == BT_STA_Connected) 
			OLED_ShowString(BT_ICON_XY, 1,1,"\x82\x83");	//����������ͼ��
		else
			OLED_ShowString(BT_ICON_XY, 1,1,"  ");		//��ͼ��

		battery_show();		//����ص�ѹ
		OLED_Refresh_Gram();					//ˢ����ʾ
   }
}



//----������ߣ���Ƭ-----------------------------------------------------------
void test_card()
{
	u8 key;			//����
	u32 UID=0;
	char RFID_buff[12];
	bool pow_vcc=FALSE, pow_bt=FALSE;

	OledClearandShow(FIRSMALLFONTH, "   Test  Card   ", 1);
	while(keys_scan());  				//�ȴ���������

	while(1)
	{
		//������ָ��
		key=keys_wait(0);

		if(key)
		{
			switch(key)
			{
			case KEY_CMD_pow:
				power_off();		//�ֶ��ػ�
				break;

			case KEY_CMD_up:
				OLED_ShowString(0,0,1,1,"    125 Card   ");	
				UID=RFID_Read(1);
				break;
			case KEY_CMD_down:
				OLED_ShowString(0,0,1,1,"    134 Card   ");	
				UID=RFID_Read(2);
				break;
			case KEY_CMD_menu:	//5V��Դ
				if(!pow_vcc)
				{
					pow_vcc=TRUE;
					OLED_ShowString(0,0,1,1," Card Power On ");	
					PowerHTRC110=1;			//HTRC110��Դ��
				}
				else
				{
					pow_vcc=FALSE;
					OLED_ShowString(0,0,1,1," Card Power Off");	
					PowerHTRC110=0;			//HTRC110��Դ��
				}
				beep(1, 50, 1);
				while(keys_scan());
				break;
			case KEY_CMD_back:	//bt��Դ
				if(!pow_bt)
				{
					pow_bt=TRUE;
					OLED_ShowString(0,0,1,1," BT Power On   ");	
					PowerBT_ON();			//��������Դ
				}
				else
				{
					pow_bt=FALSE;
					OLED_ShowString(0,0,1,1," BT Power Off  ");	
//					PowerBT_OFF();			//��������Դ
				}
				beep(1, 50, 1);
				while(keys_scan());
				break;
			}

			if(UID!=0)
			{
				sprintf(RFID_buff, "%08X", UID);
				//��ʾ����
				OLED_ShowString(0,24,2,2,RFID_buff);	
				beep(1, 50, 1);
				UID=0;
			}
			OLED_Refresh_Gram();
		}
	}
}
//----USBӦ��-------------------------------------------------------------------
void usb_app()
{
	delay_ms(50);
	if(!USB_GetConnFlag())
		return;

	OledClearandShow(FIRSMALLFONTH, "   USB ������   ", 1);

	while(1)
	{
 		delay_ms(1);
		//���usb����
		if(!USB_GetConnFlag())
			break;
	
		con_cmd();	//USBָ�� btָ��
	}
}

//----������-----------------------------------------------------------------------		  		  
int main(void)

{
	u8 key;			//����
	u32 t=0;	 //��ʱ������
	u16 i;	

	init();		//��ʼ������
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
	  
	while(keys_scan());  				//�ȴ���������
	BT_ClrState();						//�������״̬
	if	(read_para_BT_enable()==1)
		BT_Start_Prog();				//��������

	//�����ʾ����
	OLED_Clear();			//ˢ����Ļ
	OLED_Refresh_Gram();	//ˢ����ʾ	
		
	//��ѭ��
	RRT.time=RRT_TIME_10S;

	while(1)
	{	
		delay_ms(1);
		t++;
		if(t>TIME30S)	
		{	
			t=0;
			power_off();		//�ػ�  //CPU��Դ	 	
		}
		if(RRT.time<RRT_TIME_10S)
			RRT.time++; 
		//300mSˢ��ʱ����ʾ
		refresh_show_time();
		//������ָ��
		key = DetectKeys();		    
		if(key)
		{
			t=0;	
			switch(key)
			{
			case KEY_CMD_pow:
				power_off();		//�ֶ��ػ�
				break;

			case KEY_CMD_menu:
				key = Main_Menu();		//����˵�
				if(key==KEY_CMD_BTsw)
					bt_sw();			//���������л�
				else
				if(key==KEY_CMD_bro)
					browse();			//�������RFID��¼����
				RRT.time = RRT_TIME_10S;	
				break;

			case KEY_CMD_scan:
				rfid_scan();		//ɨ�迨Ƭ
				break;	

			case KEY_CMD_bro:
				browse();			//�������RFID��¼����
				RRT.time=RRT_TIME_10S;	
				break;

			case KEY_CMD_BTsw:
		  	    bt_sw();
				RRT.time = RRT_TIME_10S;			//���������л�	  fan ��
				break;

			default	:
				break;
			}
		}

		//���ͨѶָ��
		if(con_cmd())
			t=0;

		//���usb����
		if(USB_GetConnFlag())
			usb_app();
	}	   		  
}   

