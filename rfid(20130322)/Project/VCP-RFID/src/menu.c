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
	"1.蓝牙设置      ",		//"1.蓝牙设置      "
	"2.记录查询      ",		//"2.记录查询      "
//	"3.ID查询        ",		//"3.日期时间      "
	"3.设备序号      ",		
	"                ",		//"4.清除记录      "
	"6.选择卡片      ",	
	"5.清除蓝牙主机  ",	
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
			OLED_ShowString(0,0,1,1,"1.蓝牙(已关闭)  ");
		else
			OLED_ShowString(0,0,1,1,"1.蓝牙(已开启)  ");

		OLED_ShowString(0,16,1,1,"2.设置可发现    ");

		OLED_ShowString(120, item*16, 1,1, "<");	//显示新标记	
		OLED_Refresh_Gram();

		while(keys_scan());  	//等待按键脱离
		delay_ms(200);

		key=scan_key_time_power();
		//上移
		if(key==KEY_CMD_up)
		{
			if(item>0)
				item--;
		}
		else
		//下移
		if(key==KEY_CMD_down)
		{
			if(item<1)
				item++;
		}
		else
		//确认
		if(key==KEY_CMD_ok)
		{
			switch(item)
			{
			case 0:	bt_sw();			//蓝牙使用切换bt_menu();
					break;
			case 1:				//----可配对状态-----------------------------------------------------
				{
		 			if(bt_pair()==TRUE)
						return;
				}					
				break;
			}
		}
		//确认
		if((key==KEY_CMD_back)||(key==KEY_CMD_pow))
		{
			return;
		}
 	}  

}
//----主菜单---------------------------------------------------------
u8 Main_Menu()
{
    
	u8 key, ch=0;
	s16 i, item=0, selected_line=0, first_line=0;
	char buff[12];
	u8 platform;


	OledClearandShow(FIRSMALLFONTH, "      菜单      ", 1);
	delay_us(500);
	while(keys_scan());  	//等待按键脱离
		
LOOP1:
	OLED_Clear();
	for(i=0; i<LCD_ITEM_MAX; i++)			
		OLED_ShowString(0,i*16 ,1,1, (char*)menu_list[i+first_line]);	
	OLED_ShowString(120, selected_line*16, 1,1, "<");	//显示新标记	
	OLED_Refresh_Gram();	//刷新显示

	while(1)
	{
		while(keys_scan());  	//等待按键脱离
		delay_ms(200);

		key=scan_key_time_power();
		if(key == 0 )
			goto LOOP1;

		OLED_ShowString(120, selected_line*16, 1,1, " ");	//清除原标记	
		//上移
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
			OLED_ShowString(120, selected_line*16, 1,1, "<");	//显示新标记	
			OLED_Refresh_Gram();	//刷新显示
		}
		else
		//下移
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
			OLED_ShowString(120, selected_line*16, 1,1,"<");	//显示新标记	
			OLED_Refresh_Gram();	//刷新显示
		}
		else
		//确认
		if(key==KEY_CMD_ok)
		{
			switch(item)
			{
			case 0:	bt_menu();
					goto LOOP_END; 
			case 1:	ch=KEY_CMD_bro;	//检索刷卡记录
					goto LOOP_END; 
			case 2:	
					platform = getPlatformVersion();

					OLED_Clear();	  //查询设备ID
					sprintf(buff, "SN: %010lu", HWREG(FLASE_ID_ADDR));
					OLED_ShowString(0,0,1,1,buff);

					if(platform==PLATFORM_CCCL_BETA_V1)
						OLED_ShowString(0,16,1,1,"HW: V1.0");
					else if(platform==PLATFORM_CCCL_BETA_V3)
						OLED_ShowString(0,16,1,1,"HW: V3.0");
					else 
						OLED_ShowString(0,16,1,1,"HW: V4.0");

					OLED_ShowString(0,32,1,1,"SW: V1.4");

					OLED_Refresh_Gram();	//刷新显示
					while(1)  	//等待按键脱离
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
		//返回
		if((key==KEY_CMD_back)||(key==KEY_CMD_pow))
			break;

	}

LOOP_END:
	OLED_Clear();			//刷新屏幕
	OLED_Refresh_Gram();	//刷新显示	 
	while(keys_scan());  //等待按键脱离
	return ch;
}

