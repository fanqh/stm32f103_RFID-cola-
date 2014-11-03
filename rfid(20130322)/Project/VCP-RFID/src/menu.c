#include <string.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include "menu.h" 
#include "PCF8563.h"
#include "oled.h"
#include "key.h"
#include "delay.h"
#include "file.h"
#include "bluetooth.h"
#include "power.h" 
#include "MyShow.h"
#include "platform.h"

extern u8 CardStyle;
const char menu_list[][20]={
	"1.��������      ",		//"1.��������      "
	"2.��¼��ѯ      ",		//"2.��¼��ѯ      "
//	"3.ID��ѯ        ",		//"3.����ʱ��      "
	"3.�豸���      ",		
	"                ",		//"4.�����¼      "
	"6.ѡ��Ƭ      ",	
	"5.�����������  ",	
};

//----------------------------------------------------------------
void bt_menu(void)
{
	u8 item=0;
	u8 key;
	
	while(1)
	{
		OLED_Clear();	
		if(BT_GetState()==BT_STA_Null)
			OLED_ShowString(0,0,1,1,"1.����(�ѹر�)  ");
		else
			OLED_ShowString(0,0,1,1,"1.����(�ѿ���)  ");

		OLED_ShowString(0,16,1,1,"2.���ÿɷ���    ");

		OLED_ShowString(120, item*16, 1,1, "<");	//��ʾ�±��	
		OLED_Refresh_Gram();

		while(keys_scan());  	//�ȴ���������
		delay_ms(200);

		key=scan_key_time_power();
		//����
		if(key==KEY_CMD_up)
		{
			if(item>0)
				item--;
		}
		else
		//����
		if(key==KEY_CMD_down)
		{
			if(item<1)
				item++;
		}
		else
		//ȷ��
		if(key==KEY_CMD_ok)
		{
			switch(item)
			{
			case 0:	bt_sw();			//����ʹ���л�bt_menu();
					break;
			case 1:				//----�����״̬-----------------------------------------------------
				{
		 			if(bt_pair()==TRUE)
						return;
				}					
				break;
			}
		}
		//ȷ��
		if((key==KEY_CMD_back)||(key==KEY_CMD_pow))
		{
			return;
		}
 	}  

}
//----���˵�---------------------------------------------------------
u8 Main_Menu()
{
    
	u8 key, ch=0;
	s16 i, item=0, selected_line=0, first_line=0;
	char buff[12];
	u8 platform;


	OledClearandShow(FIRSMALLFONTH, "      �˵�      ", 1);
	delay_us(500);
	while(keys_scan());  	//�ȴ���������
		
LOOP1:
	OLED_Clear();
	for(i=0; i<LCD_ITEM_MAX; i++)			
		OLED_ShowString(0,i*16 ,1,1, (char*)menu_list[i+first_line]);	
	OLED_ShowString(120, selected_line*16, 1,1, "<");	//��ʾ�±��	
	OLED_Refresh_Gram();	//ˢ����ʾ

	while(1)
	{
		while(keys_scan());  	//�ȴ���������
		delay_ms(200);

		key=scan_key_time_power();
		if(key == 0 )
			goto LOOP1;

		OLED_ShowString(120, selected_line*16, 1,1, " ");	//���ԭ���	
		//����
		if(key==KEY_CMD_up)
		{
			if(item>0)
			{
				item--;
				selected_line--;
				if(selected_line<0)
				{
					selected_line=0;
					first_line--;
					goto LOOP1;
				}
			}
			OLED_ShowString(120, selected_line*16, 1,1, "<");	//��ʾ�±��	
			OLED_Refresh_Gram();	//ˢ����ʾ
		}
		else
		//����
		if(key==KEY_CMD_down)
		{
			if(item<(MENU_ITEM_MAX-1))
			{
				item++;
				selected_line++;
				if(selected_line>(LCD_ITEM_MAX-1))
				{
					selected_line=(LCD_ITEM_MAX-1);
					first_line++;
					goto LOOP1;
				}
			}
			OLED_ShowString(120, selected_line*16, 1,1,"<");	//��ʾ�±��	
			OLED_Refresh_Gram();	//ˢ����ʾ
		}
		else
		//ȷ��
		if(key==KEY_CMD_ok)
		{
			switch(item)
			{
			case 0:	bt_menu();
					goto LOOP_END; 
			case 1:	ch=KEY_CMD_bro;	//����ˢ����¼
					goto LOOP_END; 
			case 2:	
					platform = getPlatformVersion();

					OLED_Clear();	  //��ѯ�豸ID
					sprintf(buff, "SN: %010lu", HWREG(FLASE_ID_ADDR));
					OLED_ShowString(0,0,1,1,buff);

					if(platform==PLATFORM_CCCL_BETA_V1)
						OLED_ShowString(0,16,1,1,"HW: V1.0");
					else if(platform==PLATFORM_CCCL_BETA_V3)
						OLED_ShowString(0,16,1,1,"HW: V3.0");
					else 
						OLED_ShowString(0,16,1,1,"HW: V4.0");

					OLED_ShowString(0,32,1,1,"SW: V1.4");

					OLED_Refresh_Gram();	//ˢ����ʾ
					while(1)  	//�ȴ���������
					{
						key=keys_wait(TIME10S);
						if(key==KEY_CMD_back || key==0)
							break;
					}
					goto LOOP_END; 
			}
			goto LOOP1;
		}
		else
		//����
		if((key==KEY_CMD_back)||(key==KEY_CMD_pow))
			break;

	}

LOOP_END:
	OLED_Clear();			//ˢ����Ļ
	OLED_Refresh_Gram();	//ˢ����ʾ	 
	while(keys_scan());  //�ȴ���������
	return ch;
}

