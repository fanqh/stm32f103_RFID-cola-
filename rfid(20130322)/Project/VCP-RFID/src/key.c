#include "gpio_config.h"
#include "key.h"
#include "delay.h"
#include "platform.h"
//#include "config.h"
/*
#ifdef USER_BAORD_V30
	#define KEY_1 	PCin(13)   
	#define KEY_2	PBin(12)
	#define KEY_3	PBin(9) 
	#define KEY_4	PBin(13)
	#define KEY_5 	PBin(14)
	#define KEY_6	PBin(8)
#else	  
	#define KEY_1 	PCin(13)   
	#define KEY_2	PBin(12) 
	#define KEY_3	PBin(8)
	#define KEY_4	PBin(9) 
	#define KEY_5	PBin(13)
	#define KEY_6 	PBin(14) 
#endif  
 */
								    
//按键初始化函数
void keys_init(void)
{
	RCC->APB2ENR|=1<<3;     	//使能PORTB时钟
	RCC->APB2ENR|=1<<4;     	//使能PORTC时钟

	GPIOB->CRH&=0XF000FF00;		//PB8、9、12、13、14设置成输入	  
	GPIOB->CRH|=0X08880088;  
	GPIOB->ODR|=0x73<<8;	 	//PB8、9、12、13、14上拉
	//KEY_POW电源 
	GPIOC->CRH&=0XFF0FFFFF;		//PC13设置成输入	  
	GPIOC->CRH|=0X00800000; 				   
//	GPIOC->ODR|=1<<13;	   		//PA13上拉
} 

//按键处理函数
//返回按键值
//0，没有任何按键按下
u8 keys_scan(void)
{	 
	if(GetKey1()==1 || GetKey2()==0 || GetKey3()==0 || GetKey4()==0 || GetKey5()==0 || GetKey6()==0)
	{
		delay_ms(10);//去抖动 
		if(GetKey1()==1)
			return KEY_CMD_pow;
		if(GetKey2()==0)
			return KEY_CMD_up;
		if(GetKey3()==0)
			return KEY_CMD_menu;
		if(GetKey4()==0)
			return KEY_CMD_scan;
		if(GetKey5()==0)
			return KEY_CMD_back;
		if(GetKey6()==0)
			return KEY_CMD_down;
	}		
	return 0;	// 无按键按下
}


u8 DetectKeys(void)
{
	u8 KeyNum1=0;
	u8 KeyNum2=0;

	if(GetKey1()==1) KeyNum1 |= 1<<0;
	if(GetKey2()==0) KeyNum1 |= 1<<1;
	if(GetKey3()==0) KeyNum1 |= 1<<2;
	if(GetKey4()==0) KeyNum1 |= 1<<3;
	if(GetKey5()==0) KeyNum1 |= 1<<4;
	if(GetKey6()==0) KeyNum1 |= 1<<5;
	if(KeyNum1)
	{
	    delay_ms(10);
	   	if(GetKey1()==1) KeyNum2 |= 1<<0;
	    if(GetKey2()==0) KeyNum2 |= 1<<1;
		if(GetKey3()==0) KeyNum2 |= 1<<2;
		if(GetKey4()==0) KeyNum2 |= 1<<3;
		if(GetKey5()==0) KeyNum2 |= 1<<4;
		if(GetKey6()==0) KeyNum2 |= 1<<5;  
			
		if(KeyNum1==KeyNum2)
			return KeyNum1;
	} 
		return 0;		
}


u8 keys_wait(u16 time)
{
	u8 key;
	
	//无限等待
	if(time==0)
	{
		while(1)
		{
			key=keys_scan();
			if(key!=0)
				return key;
		}
	}
	//超时退出
	while(1)
	{
		delay_ms(1);
		key=keys_scan();
		if(key!=0)
			return key;
		time--;
		if(time==0)
			return 0;
	}
}


u8 GetKey1()
{ 
	u8 key = 0xff; 
	if(g_Platform_Param.key1)
		key =(g_Platform_Param.key1)();
	return  key;
}
u8 GetKey2()
{ 
	u8 key = 0xff; 
	if(g_Platform_Param.key2)
		key =(g_Platform_Param.key2)();
	return  key;
}

u8 GetKey3()
{ 
	u8 key = 0xff; 
	if(g_Platform_Param.key3)
		key =(g_Platform_Param.key3)();
	return  key;
}
u8 GetKey4()
{ 
	u8 key = 0xff; 
	if(g_Platform_Param.key4)
		key =(g_Platform_Param.key4)();
	return  key;
}
u8 GetKey5()
{ 
	u8 key = 0xff; 
	if(g_Platform_Param.key5)
		key =(g_Platform_Param.key5)();
	return  key;
}
u8 GetKey6()
{ 
	u8 key = 0xff; 
	if(g_Platform_Param.key6)
		key =(g_Platform_Param.key6)();
	return  key;
}











