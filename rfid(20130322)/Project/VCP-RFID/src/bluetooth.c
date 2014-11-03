
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
//��DataTransfer.c�ж���
#define BT_CON	1

send_struct	SDB;	     //�������ṹ
bt_rx_buff 	BT_RX_BUFF;	 //�������ṹ
time_segment TIME_SEG;	 //ʱ��μ������ݽṹ dd-dd-dd tt:tt:tt dd-dd-dd tt:tt:tt

u8 BT_State=BT_STA_Null;	//����״̬
u32 BT_DeviceID;			//�����豸ID

extern char TxetTab[][20];

//----�����ӿڳ�ʼ��-------------------------------------------------
void BT_Init()
{
	//��Դ�ӿ�
	RCC->APB2ENR|=1<<3;    	//ʹ��PORTCʱ�� 
 	GPIOC->CRL&=0X0FFFFFFF;	//PC7�������
 	GPIOC->CRL|=0X70000000;	  //��©
	GPIOC->ODR|=(1<<7);	//����ߵ�ƽ
	//���Ͻӿ�
	RCC->APB2ENR|=1<<3;    	//ʹ��PORTCʱ�� 
 	GPIOC->CRL&=0XFFFFFFF0;	//PC0����
 	GPIOC->CRL|=0X30000008;
	GPIOC->ODR|=1;			//������ƽ

	com_init(COM2, 115200, 8, 1, 0); 
	delay_ms(10);
	com_clear(COM2);
}
//----����ָ��-------------------------------------------------------
void SendCmd(u16 cmd)
{

//	com_clear(COM2);
	com_write(COM2, (u8*)&cmd, 2);
}


//----�������ģ���ڲ�����-------------------------------------------
bool BT_GetErrPin()
{
	//�޹���
	if(BT_A1_PIN == 1)
		return FALSE;	
	//�й���
//	PowerBT_OFF();	//��������Դ		
	OLED_Clear();	
	OLED_ShowString(0,0,1,1,"�������� !!     ");		
	OLED_Refresh_Gram();	
	return TRUE;
}
//----�ȴ�ָ����㳬ʱ---------------------------------------------
u16 wait_cmd_time(vu32 t, bool bCancellEnable )
{
	u16 cmd;
	u8 key = 0;			//����

	while(t)
	{
		t--;	  
		delay_ms(1);
		if(com_package(COM2)>=2)	//���������ݰ�
		{
			com_read(COM2, (u8*)&cmd, 2);
			return cmd;		//��������ģ��ָ��
		}

		if( TRUE == bCancellEnable )
		{
			key=keys_scan(); 	 
			//ǿ���˳�
			if(key == KEY_CMD_back)
			{
				while(keys_scan()); 			//�ȴ���������
				return OPT_CANCEL;			    //ȡ������
			}
			else if((key == KEY_CMD_pow)&&(USB_GetConnFlag()!=TRUE))
			{
				power_off();  	//�ֶ��ػ�
				return KEY_CMD_pow;
			}	
		}

	}
	return TIME_OUT;			//���س�ʱ
}




/////////Ϊ��������������������״̬��������Թ�����ͻȻ�ػ����ڹػ�
//-------------------------------------------------------------------
u16 BT_GetCmd()
{
	u16 cmd;

	//�������ָ��
	if(com_package(COM2)>=2)	//���������ݰ�
	{
		com_read(COM2, (u8*)&cmd, 2);
		return cmd;
	}
	else
		return 0;
}

//----��������--------------------------------------------------------
bool BT_Connect_Prog(void)
{
	u16 sta;

	bool ret = FALSE;
	if(BT_State!=BT_STA_Null)
	{
		SendCmd(CmdConnect);
		sta = wait_cmd_time(10000, FALSE);			//�ȴ�ָ����㳬ʱ  ************120 �� 10000


		if(sta == CmdConnect_Ack)
		{ 
		 	BT_State = BT_STA_Connected;
			ret = TRUE;
		}
	}
	return ret;
}

//----������״̬------------------------------------------------------
bool BT_Connectable_Prog()
{
	u16 sta;
	
	OLED_Clear();	
	OLED_ShowString(0,0,1,1,TxetTab[7]);	//��������	
	OLED_Refresh_Gram();			

	SendCmd(CmdGoConnected);			//����Connectable(������)״̬
	sta = wait_cmd_time(2000, FALSE);		    //�ȴ�ָ����㳬ʱ

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
//----�����״̬-----------------------------------------------------
bool BT_Pairable_Prog()
{
	u16 sta;
	bool flag;
	
	OLED_Clear();	
	OLED_ShowString(0,0,1,1,TxetTab[4]);		//�������	
	OLED_Refresh_Gram();	
	
	SendCmd(CmdPair);				//���ͽ���Pairable(���)״ָ̬��
	sta = wait_cmd_time(5000, FALSE);		//�ȴ�Ӧ��ָ����㳬ʱ 1000
	if(sta != CmdPair_Ack)
	{
			
		OLED_ShowString(0,0,1,1,"    ���ʧ��    ");
		flag=FALSE;
		goto loop_end;
	}

	sta = wait_cmd_time(120000, TRUE);		//�ȴ�ָ����㳬ʱ 120000
	if(sta == CmdPair_OK)  			//Pairable(���)�ɹ�
	{
		BT_State = BT_STA_Initial;
//	  	BT_State = BT_STA_Connected;
		OLED_ShowString(0,0,1,1,"    ��Գɹ�    ");	
		flag=TRUE;
		goto loop_end;
	}
	if(sta == CmdPair_TOut)
	{
			
		OLED_ShowString(0,0,1,1,"    ��Գ�ʱ    ");	
		flag=FALSE;
		goto loop_end;
	}
	if(sta == OPT_CANCEL)
	{			
		SendCmd(CmdQuitBT);				//�˳�����ģʽ
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
//		PowerBT_OFF();			//��������Դ
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

	SendCmd(CmdDisconnect);	//���ͶϿ�����ָ��
	sta = wait_cmd_time(2000, FALSE);		//�ȴ�Ӧ��ָ����㳬ʱ 2000MS
	if(sta==CmdDisconnect_Ack)
		ret = TRUE;	
	return ret; 
}

//----��ȡ����״̬-----------------------------------------------------
u8 BT_GetState()
{
	return BT_State;
}
//----�������״̬-----------------------------------------------------
void BT_ClrState()
{
	BT_State = BT_STA_Null;
}
//----BT��������-------------------------------------------------
bool BT_SendDatas(u8* buf, u16 lenth)
{
	u16 sta;
//	u8 rceivedconfirm = 1;

	SendCmd(CmdSend);						//����ָ��
	com_write(COM2, (u8*)&lenth, 2);		//�������ݳ���  *************����**************

	com_write(COM2, buf, lenth);			//��������
	while(1)
	{
		sta = wait_cmd_time(1000, FALSE);		//�ȴ�ָ���ʱ100mS **************1500 ��1000
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
	
//----�����ر�----------------------------------------------------------
void BT_Stop_Prog()
{
	u16 sta;

	if(BT_State != BT_STA_Null)
	{	
		OLED_Clear();	
		OLED_ShowString(0,0,1,1,TxetTab[6]);	//�ر�����	
		OLED_Refresh_Gram();	
	 	SendCmd(CmdQuitBT);		//�����˳�����ָ��
		if(BT_State == BT_STA_Connected)
		{
			sta = wait_cmd_time(2000, FALSE);				//�ȴ�ָ���ʱ2S ********ֻ��������״̬�£��ȴ���Ӧ����ʱ�ػ�
		}	 
		sta = sta;

	//	delay_s(2);
		OLED_ShowString(0,0,1,1,"                ");	
		OLED_Refresh_Gram();
		
	}
//	PowerBT_OFF();			//��������Դ
	BT_State = BT_STA_Null;		
}

///������λ
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
	sta = wait_cmd_time(5000, TRUE);	//�ȴ�ָ���ʱ100mS
	sta = sta;
	BT_State = BT_STA_Null;
	
}

u16 AppBTPowerOn(void)
{
  	u16 sta;

	BTReset();
	com_clear(COM2);
	sta = wait_cmd_time(5000, FALSE);	//�ȴ�ָ���ʱ100mS
	if((sta!=WithNoHostAdd)&&(sta!=WithHostAdd))
	{
		BT_State = BT_STA_Null;
//		PowerBT_OFF();			//��������Դ	
		OLED_ShowString(0,0,1,1,"   ��������ʧ��   ");	
		OLED_Refresh_Gram();
		delay_s(1);	
		
	}
	else
	{
		BT_State = BT_STA_Initial;	
	}

	return sta;
	
}

//----��������----------------------------------------------------------
u16 BT_bootup()
{
	u16 sta = TIME_OUT;

	OLED_Clear();			//ˢ����Ļ
	OLED_ShowString(0,0,1,1,TxetTab[9]);	//��������
	OLED_Refresh_Gram();
	delay_s(1);	

	com_init(COM2, 115200, 8, 1, 0);
	sta = AppBTPowerOn();
//	if((sta!=WithNoHostAdd)&&(sta!=WithHostAdd))
//	{
//		BT_State = BT_STA_Null;
//		save_para_BT_enable(0);		//�������
////		PowerBT_OFF();			  //��������Դ	
//		OLED_ShowString(0,0,1,1,"   ��������ʧ��   ");	
//		OLED_Refresh_Gram();
//		delay_s(1);	
//	}
	return sta;	 //

}

//==============================================================
//----����ID-----------------------------------------------------------------------
bool SendInitDeviceID()
{
	u16 cmd = CmdInitDeviceID;
	u32 id= HWREG(FLASE_ID_ADDR);
	u16 sta;	

	com_clear(COM2);
	com_write(COM2, (u8*)&cmd, 2);
	com_write(COM2, (u8*)&id, 4);

	sta = wait_cmd_time(5000, FALSE);	//�ȴ�ָ���ʱ100mS
	if( sta == CmdInitDeviceID_Ack)
		return TRUE;
	else
	{
		BT_State = BT_STA_Null;
//		PowerBT_OFF();			//��������Դ	
		OLED_ShowString(0,0,1,1,"   ��������ʧ��   ");	
		OLED_Refresh_Gram();
		delay_s(1);	
	}
	return FALSE;
}
//===============================================================


//----��������---------------------------------------------------------------------
bool BT_Start_Prog()
{
    u16 sta = BT_bootup();
	bool ret = FALSE;

	//��������ַ���������
	if(sta == WithNoHostAdd)
	{
		if(SendInitDeviceID()==TRUE)
		{	
			if(BT_Pairable_Prog()==TRUE)
				ret = TRUE;
			else
				save_para_BT_enable(0);		//�������
		}
	}
   //����-������Թ�
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
//----����ʹ���л�---------------------------------------------------------------		  		  
void bt_sw()
{
	//δʹ������������
	if(BT_GetState()==BT_STA_Null)
	{
		if(BT_Start_Prog()==TRUE)			//��������
		{
		 	if(BT_GetState()!=BT_STA_Null)
				save_para_BT_enable(1);	//�������
		}
		else
		{
//			PowerBT_OFF();			//��������Դ
			BT_State = BT_STA_Null;	
			save_para_BT_enable(0);		//�������		
		}
	
	}
	//��ʹ����������ر�
	else
	{
		BT_Stop_Prog();				//�����ر�
		save_para_BT_enable(0);		//�������
	}
}
//----�������---------------------------------------------------------------------		  		  
bool bt_pair()
{
	u16 sta;
	bool ret = FALSE;

	//save_para_BT_enable(0);		//�������
	//����������
	if(BT_GetState()==BT_STA_Connected)
	{
		
		if(BT_Disconnect_Prog()==TRUE)
		{
			BT_State = BT_STA_Initial;
			if(BT_Pairable_Prog()==TRUE)//�������
			{
				save_para_BT_enable(1);		//�������
				return TRUE;
			}
			else
			{  
			 	save_para_BT_enable(0);		//�������
				return FALSE;
			}	
		}
		else
		{
			AppBTPowerOff();
			save_para_BT_enable(0);		//�������
		}	
	}					
	//δʹ������������
	if(BT_GetState()==BT_STA_Null)
	{
		sta=BT_bootup();
		if((sta!=WithNoHostAdd)&&(sta!=WithHostAdd) )//����ʧ��
		{
			save_para_BT_enable(0);		//�������
			return FALSE;
		}
		else
		{
			if(SendInitDeviceID()==FALSE)
			{
				save_para_BT_enable(0);		//�������
				return FALSE;
			}
		}
	}
	if(BT_Pairable_Prog()==TRUE)
	{
		ret = TRUE;
		save_para_BT_enable(1);	//�������	
	}
	else
	{
		ret = FALSE;
		save_para_BT_enable(0);		//�������
	}

    return ret;		//������� 
}

//----���������豸ID-------------------------------------------------------
u32 BT_Browse_DeviceID()
{
	return BT_DeviceID;
}
