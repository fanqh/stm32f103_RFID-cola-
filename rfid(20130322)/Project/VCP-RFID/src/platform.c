

#include "platform.h"  
#include "gpio_config.h"
#include "delay.h"

Platform_Param g_Platform_Param;
u8 hardWareVersion;

u8 Beta_V3_GetKEY_1(){ return Beta_V3_KEY_1; };
u8 Beta_V3_GetKEY_2(){ return Beta_V3_KEY_2; };
u8 Beta_V3_GetKEY_3(){ return Beta_V3_KEY_3; };
u8 Beta_V3_GetKEY_4(){ return Beta_V3_KEY_4; };
u8 Beta_V3_GetKEY_5(){ return Beta_V3_KEY_5; };
u8 Beta_V3_GetKEY_6(){ return Beta_V3_KEY_6; };

u8 Beta_V1_GetKEY_1(){ return Beta_V1_KEY_1; };
u8 Beta_V1_GetKEY_2(){ return Beta_V1_KEY_2; };
u8 Beta_V1_GetKEY_3(){ return Beta_V1_KEY_3; };
u8 Beta_V1_GetKEY_4(){ return Beta_V1_KEY_4; };
u8 Beta_V1_GetKEY_5(){ return Beta_V1_KEY_5; };
u8 Beta_V1_GetKEY_6(){ return Beta_V1_KEY_6; };	   

u8 GetPinSw1(void)
{
 	u8 sta;
	RCC->APB2ENR|=1<<2;     	//使能PORTA时钟
	GPIOA->CRH&=0X0fffffff;		//P15设置成输入
	GPIOA->CRH|=0X80000000; 
	GPIOA->ODR|=1<<15;	 	//PA15 上拉输入
	delay_us(10);
	
	sta = PAin(15); 
	if(sta ==0 )
		return 0;
	else
	{
		GPIOA->ODR&=~(1<<15);	 	//PA15 下拉输入
		delay_us(10);
		sta = PAin(15); 
		if(sta==1)
			return 1;
		else
			return 2;	
	}

}


u8 GetPinSw2(void)
{
 	u8 sta;

	RCC->APB2ENR|=1<<4;     	//使能PORTC时钟
	GPIOC->CRH&=0Xfffff0ff;		//PC10设置成输入
	GPIOC->CRH|=0X00000800; 
	GPIOC->ODR|=1<<10;	 	//PC10 上拉输入
	delay_us(10);
	
	sta = PCin(10); 
	if(sta ==0 )
		return 0;
	else
	{
		GPIOC->ODR&=~(1<<10);	 	//PC10 下拉输入
		delay_us(10);
		sta = PCin(10); 
		if(sta==1)
			return 1;
		else
			return 2;	
	}

}
u8 getPlatformVersion()
{
	u8 sw1;
	u8 sw2;

	sw1 = GetPinSw1();
	sw2 = GetPinSw2();

	hardWareVersion = sw1 |(sw2<<4);
	return 	hardWareVersion;
}

 /*****************
   Platform_BetaV3_Keys definition
 */
static Platform_Param Platform_BetaV3_Keys = {
	Beta_V3_GetKEY_1,
	Beta_V3_GetKEY_2,
	Beta_V3_GetKEY_3,
	Beta_V3_GetKEY_4,
	Beta_V3_GetKEY_5,
	Beta_V3_GetKEY_6,
	0,
	0X70000000
};	 

 /*****************
   Platform_BetaV1_Keys definition
 */
static Platform_Param Platform_BetaV1_Keys = {
	Beta_V1_GetKEY_1,
	Beta_V1_GetKEY_2,
	Beta_V1_GetKEY_3,
	Beta_V1_GetKEY_4,
	Beta_V1_GetKEY_5,
	Beta_V1_GetKEY_6,
	1,
	0X30000000	 //	  0X30000000
};	 

bool initPlatform()
{
    bool ret = FALSE;

	getPlatformVersion();

	if ( PLATFORM_CCCL_BETA_V3 == hardWareVersion )
	{
		g_Platform_Param = Platform_BetaV3_Keys;
		ret = TRUE;
	}
	else if ( PLATFORM_CCCL_BETA_V1 == hardWareVersion )
	{
		g_Platform_Param = Platform_BetaV1_Keys; 
		ret = TRUE;
	}
	else if	( PLATFORM_CCCL_BETA_V4 == hardWareVersion )

	{
		g_Platform_Param = Platform_BetaV3_Keys;
		ret = TRUE;
	}
	
	return ret;	
}

u8 getPowerHTRC110_Level(void)
{
	return g_Platform_Param.powerHTRC110_Level;
}

u32 getValuev5Config(void)
{
	return g_Platform_Param.valuevcc5config;
}
