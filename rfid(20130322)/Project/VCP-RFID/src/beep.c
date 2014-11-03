#include "beep.h"
#include "delay.h"
#include "platform.h"

#define BEEP_GPIO       GPIOC  						
#define BEEP_PIN		GPIO_Pin_6					
#define BEEP_Periph		RCC_APB2Periph_GPIOC
/*	 
////////////////////////////////////////////////////////////////////////////////////
//----延迟120nS---------------------------------------------------------------------
void Delay(volatile u32 nCount)
{
// __IO 就是volatile, 加上这个后可以避免延迟函数被编译器优化掉 

	for(; nCount; nCount--);
}

//----延迟uS------------------------------------------------------------------------
void DelayUS(volatile u32 nCount)
{
	for(; nCount != 0; nCount--)
		Delay(7);
}
//----延迟mS------------------------------------------------------------------------
void DelayMS(volatile u32 nCount)
{
	for(; nCount != 0; nCount--)
		DelayUS(1000);
}
  */
//----beep---------------------------------------------------------------------					    
void beep(u16 cnt, u16 timeOn, u16 timeOff)
{

	u32 i;
	u32 t ;
	GPIO_InitTypeDef GPIO_InitStructure;

	t = (u32)timeOn*100/37;
	RCC_APB2PeriphClockCmd(BEEP_Periph, ENABLE);
	GPIO_InitStructure.GPIO_Pin = BEEP_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//改
	GPIO_Init(BEEP_GPIO, &GPIO_InitStructure);


	if(PLATFORM_CCCL_BETA_V3==getPlatformVersion()||(PLATFORM_CCCL_BETA_V1==getPlatformVersion()))
	{
		if(timeOn > 1800)
			timeOn = 1800;
		else
		if(timeOn == 0)
			timeOn = 1;

		if(timeOff > 1800)
			timeOff = 1800;
		else
		if(timeOff == 0)
			timeOff = 1;	 
		while(cnt)
		{
	    	GPIO_SetBits(BEEP_GPIO, BEEP_PIN);
	//		BEEP_SW = 1;
			delay_ms(timeOn);
	    	GPIO_ResetBits(BEEP_GPIO, BEEP_PIN);
	//		BEEP_SW = 0;
			delay_ms(timeOff);
			cnt--;
		}
	}
	else if(PLATFORM_CCCL_BETA_V4==getPlatformVersion())
	{
		while(cnt--)
		{
			for(i=0; i<t; i++)
			{
		    	GPIO_SetBits(BEEP_GPIO, BEEP_PIN);
		//		BEEP_SW = 1;
				delay_us(185);
		    	GPIO_ResetBits(BEEP_GPIO, BEEP_PIN);
		//		BEEP_SW = 0;
				delay_us(185);
			}
	
			delay_ms(timeOff);
		}

	}		



}
