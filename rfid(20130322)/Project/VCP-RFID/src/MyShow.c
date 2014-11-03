#include <string.h>
#include "stm32f10x.h"
#include "oled.h"
#include "MyShow.h"
#include "delay.h"


void MyShowOled(LOCATION *loca1, LOCATION *loca2, char *sting)
{
	u8 x,y;
	u8 fontwidth,fontheigth;

	x = loca1->locax;
	y = loca1->locay;
	fontwidth  = loca1->fontw;
	fontheigth = loca1->fonth;
	OLED_ShowString(x,y,fontwidth,fontheigth,"        ");

	x = loca2->locax;
	y = loca2->locay;
	fontwidth  = loca2->fontw;
	fontheigth = loca2->fonth;
	OLED_ShowString(x,y,fontwidth,fontheigth,sting );

	OLED_Refresh_Gram();
}

void OledClearandShow(u8 x, u8 y, u8 fontwidth, u8 fontheigth, char *sting, u8 ClearEnable)
{
	if(ClearEnable)
		OLED_Clear();
	OLED_ShowString(x, y, fontwidth, fontheigth, sting);		
	OLED_Refresh_Gram();
}







