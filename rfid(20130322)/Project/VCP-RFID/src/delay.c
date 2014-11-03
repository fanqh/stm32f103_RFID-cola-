#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//±¾³ÌÐòÖ»¹©Ñ§Ï°Ê¹ÓÃ£¬Î´¾­×÷ÕßÐí¿É£¬²»µÃÓÃÓÚÆäËüÈÎºÎÓÃÍ¾
//Mini STM32¿ª·¢°å
//Ê¹ÓÃSysTickµÄÆÕÍ¨¼ÆÊýÄ£Ê½¶ÔÑÓ³Ù½øÐÐ¹ÜÀí
//°üÀ¨delay_us,delay_ms
//ÕýµãÔ­×Ó@ALIENTEK
//¼¼ÊõÂÛÌ³:www.openedv.com
//ÐÞ¸ÄÈÕÆÚ:2010/5/27
//°æ±¾£ºV1.2
//°æÈ¨ËùÓÐ£¬µÁ°æ±Ø¾¿¡£
//Copyright(C) ÕýµãÔ­×Ó 2009-2019
//All rights reserved
//********************************************************************************
//V1.2ÐÞ¸ÄËµÃ÷
//ÐÞÕýÁËÖÐ¶ÏÖÐµ÷ÓÃ³öÏÖËÀÑ­»·µÄ´íÎó
//·ÀÖ¹ÑÓÊ±²»×¼È·,²ÉÓÃdo while½á¹¹!
//////////////////////////////////////////////////////////////////////////////////	 
static u8  fac_us=0;//usÑÓÊ±±¶³ËÊý
static u16 fac_ms=0;//msÑÓÊ±±¶³ËÊý
//³õÊ¼»¯ÑÓ³Ùº¯Êý
//SYSTICKµÄÊ±ÖÓ¹Ì¶¨ÎªHCLKÊ±ÖÓµÄ1/8
//SYSCLK:ÏµÍ³Ê±ÖÓ
void delay_init()
{
	u8 SYSCLK=72;

	SysTick->CTRL&=0xfffffffb;//bit2Çå¿Õ,Ñ¡ÔñÍâ²¿Ê±ÖÓ  HCLK/8
	fac_us=SYSCLK/8;		    
	fac_ms=(u16)fac_us*1000;
}								    
//ÑÓÊ±nms
//×¢ÒânmsµÄ·¶Î§
//SysTick->LOADÎª24Î»¼Ä´æÆ÷,ËùÒÔ,×î´óÑÓÊ±Îª:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLKµ¥Î»ÎªHz,nmsµ¥Î»Îªms
//¶Ô72MÌõ¼þÏÂ,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;//Ê±¼ä¼ÓÔØ(SysTick->LOADÎª24bit)
	SysTick->VAL =0x00;           //Çå¿Õ¼ÆÊýÆ÷
	SysTick->CTRL=0x01 ;          //¿ªÊ¼µ¹Êý  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//µÈ´ýÊ±¼äµ½´ï   
	SysTick->CTRL=0x00;       //¹Ø±Õ¼ÆÊýÆ÷
	SysTick->VAL =0X00;       //Çå¿Õ¼ÆÊýÆ÷	  	    
}   
//ÑÓÊ±nus
//nusÎªÒªÑÓÊ±µÄusÊý.		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; //Ê±¼ä¼ÓÔØ	  		 
	SysTick->VAL=0x00;        //Çå¿Õ¼ÆÊýÆ÷
	SysTick->CTRL=0x01 ;      //¿ªÊ¼µ¹Êý 	 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//µÈ´ýÊ±¼äµ½´ï   
	SysTick->CTRL=0x00;       //¹Ø±Õ¼ÆÊýÆ÷
	SysTick->VAL =0X00;       //Çå¿Õ¼ÆÊýÆ÷	 
}

void delay_s(vu16 s)
{
	while(s)
	{
		delay_ms(1000);
		s--;
	}
}
void StartSysTime(void)
{
// 	u32 temp;	    	 
	SysTick->LOAD=SETTIME*fac_us; //Ê±¼ä¼ÓÔØ	  		 
	SysTick->VAL=0x00;        //Çå¿Õ¼ÆÊýÆ÷
	SysTick->CTRL=0x01 ;      //¿ªÊ¼µ¹ý  
}

u32 GetSysInterval(void)
{
  //if(SETTIME*fac_us>=SysTick->VAL)
      return ((u32)(SETTIME*fac_us)-SysTick->VAL)/9;
//  else
//  	  return 0;
}

/*
  #include "cortexm3_macro.h"
  #include "stm32f10x_systick.h"


void SysTick_Configuration(void)
{
	SysTick->CTRL = 0;
	SysTick->LOAD = 72000-1;                                    
	NVIC_SystemHandlerPriorityConfig(SystemHandler_SysTick, 3, 0);
	SysTick->VAL = 0;                                                           
	SysTick->CTRL = 0x07; 	
}

void SysTick_ISR(void)
{
	int c;
	c=0;
	c=c;
}
  */





























