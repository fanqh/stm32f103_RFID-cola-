#include "stm32f10x.h"
#include "power.h"
#include "delay.h"
#include "beep.h"
#include "bluetooth.h"
#include "oled.h"
#include "key.h"
#include "MyUsart.h"
#include "DataTransfer.h"
//#include "usb_driver.h"
#include "usb_istr.h"
#include "file.h"
#include "config.h"
#include "log.h"

#define V5_GPIO     GPIOA  						
#define V5_PIN		GPIO_Pin_10
#define V5_Periph	RCC_APB2Periph_GPIOA

//----监测电池电压-------------------------------------------------------
void battery_show(void);
//----USB应用-------------------------------------------------------------------
void usb_app(void);



/////////////////////////////////////////////////////////////////////////

//我们默认将开启通道0~3																	   
void  Adc_Init(void)
{    
  
	//先初始化IO口
 	RCC->APB2ENR|=1<<4;    //使能PORTC口时钟 
	//RCC->APB2ENR|=1<<0;	  ///复用开启

	GPIOC->CRL&=0XfF0FF0FF;//PC2 pc5 anolog输入
	//通道10/11设置			 
	RCC->APB2ENR|=1<<9;    //ADC1时钟使能	  
	RCC->APB2RSTR|=1<<9;   //ADC1复位
	RCC->APB2RSTR&=~(1<<9);//复位结束	    
	RCC->CFGR&=~(3<<14);   //分频因子清零	
	//SYSCLK/DIV2=12M ADC时钟设置为12M,ADC最大时钟不能超过14M!
	//否则将导致ADC准确度下降! 
	RCC->CFGR|=2<<14;      	 

	ADC1->CR1&=0XF0FFFF;   //工作模式清零
	ADC1->CR1|=0<<16;      //独立工作模式  
	ADC1->CR1&=~(1<<8);    //非扫描模式	  
	ADC1->CR2&=~(1<<1);    //单次转换模式
	ADC1->CR2&=~(7<<17);	   
	ADC1->CR2|=7<<17;	   //软件控制转换  
	ADC1->CR2|=1<<20;      //使用用外部触发(SWSTART)!!!	必须使用一个事件来触发
	ADC1->CR2&=~(1<<11);   //右对齐	 
	ADC1->SQR1&=~(0XF<<20);
	ADC1->SQR1&=0<<20;     //1个转换在规则序列中 也就是只转换规则序列1 			   
	//设置通道15的采样时间
	ADC1->SMPR1 &= 0XFFFC7e3F;//通道15采样时间清空	  
	ADC1->SMPR1 |= 7<<15;     //通道15  239.5周期,提高采样时间可以提高精确度
	ADC1->SMPR1 |= 7<<6;     //通道12  239.5周期,提高采样时间可以提高精确度		 

	ADC1->CR2|=1<<0;	    //开启AD转换器	 
	ADC1->CR2|=1<<3;        //使能复位校准  
	while(ADC1->CR2&1<<3);  //等待校准结束 			 
    //该位由软件设置并由硬件清除。在校准寄存器被初始化后该位将被清除。 		 
	ADC1->CR2|=1<<2;        //开启AD校准	   
	while(ADC1->CR2&1<<2);  //等待校准结束

	//该位由软件设置以开始校准，并在校准结束时由硬件清除  
}				  
//----获得ADC值----------------------------------------------------------
//ch:通道值
u16 BatteryGet()
{
	u32 vol;

	//设置转换序列	  		 
	ADC1->SQR3 &= 0XFFFFFFE0;//规则序列1 通道ch
	ADC1->SQR3 |= 15;		//通道15		  			    
	ADC1->CR2|=1<<22;       //启动规则转换通道 
	while(!(ADC1->SR&1<<1));//等待转换结束	 	   
	vol = ADC1->DR;		//返回adc值	
	vol = vol * 4920 / 3143;	//校正
	return vol;
}


//获得ADC值
//ch:通道值 0~3
u16 Get_Adc(u8 ch)   
{
	//设置转换序列	  		 
	ADC1->SQR3&=0XFFFFFFE0;//规则序列1 通道ch
	ADC1->SQR3|=ch;		  			    
	ADC1->CR2|=1<<22;       //启动规则转换通道 
	while(!(ADC1->SR&1<<1));//等待转换结束	 	   
	return ADC1->DR;		//返回adc值	
}

//----初始化---------------------------------------------------------------					    
void POWER_Init(void)
{ 	 				 	 					    
	RCC->APB2ENR|=1<<4;    //使能PORTC时钟 
	//CPU
 	GPIOC->CRL&=0XFFF0FFFF;	//PC4推挽输出
 	GPIOC->CRL|=0X00030000;
	GPIOC->ODR&=~(1<<4);	//输出低电平
	//BT
 	GPIOC->CRL&=0X0FFFFFFF;	//PC4推挽输出
 	GPIOC->CRL|=0X30000000;
	GPIOC->ODR&=~(1<<7);	//输出低电平

	Adc_Init();	  //采样初始化ad采样通道15，12；
}

//----关机---------------------------------------------------------------------
void power_off()
{
	u8 key;
	u32 timecount;

	beep(2, 100, 100);
	BT_Stop_Prog();			//蓝牙关闭 
	OLED_Clear();		
	OLED_ShowString(0,16,1,1,"   正在关机...  ");	//正在关机	
	OLED_Refresh_Gram();	 
	delay_s(2);
	PowerCPU_Off();			//关CPU电源	
	//----检测V5电压------------------------------------------------------------------
	if(test_v5())
	{
		OLED_Clear();		
		while(1)
		{
			timecount++;
			delay_ms(10);
			if(timecount>=TIMECHARGFLASH)
			{
				timecount = 0;
				battery_show();
			    OLED_Refresh_Gram();					//刷新显示
			}

			key=DetectKeys();
			if(key == KEY_CMD_pow)
			{
				PowerCPU_On();			//开CPU电源	
				delay_ms(50);
		   		beep(1, 100, 100);
				while(keys_scan());  				//等待按键脱离
			                                        
				BT_ClrState();						//清除蓝牙状态
	            if(read_para_BT_enable()==1)			//读取参数
		            BT_Start_Prog();				//蓝牙启动
				break;
			}
			else
			if(key == 0x25)
				TestModel();
		}
	}
	else
		while(1);	   
}
//----关机2---------------------------------------------------------------------
void power_off2()
{
	PowerCPU_Off();			//关CPU电源	
	//----检测V5电压------------------------------------------------------------------
	if(test_v5())
	{
		OLED_Clear();		
		while(1)
		{
			delay_ms(10);
			battery_show();
			OLED_Refresh_Gram();  //刷新显示
		}
	}
}
//----扫描键盘、超时关机----------------------------------------------------------					    
u8 scan_key_time_power()
{
	u8 key;
	u32 t=0;

	//扫描键盘，如果没有键盘操作，30秒后关闭电源（节电）
	while(1)
	{
		delay_ms(1);
		key=keys_scan();
		if(key == KEY_CMD_pow)
			power_off();		//关机
		if(key)
			break;

		t++;
		if(t>TIME30S)		//30S时间到
		{
			power_off();		//关机
			t=0;
			key=0;
		}

		if(con_cmd())	
			t=0;

		//检测usb连接
		if(USB_GetConnFlag())
		{
			usb_app();
			return 0;
		}
	}
	return key;
}
//----检测V5电压------------------------------------------------------------------
bool test_v5()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(V5_Periph, ENABLE);
	GPIO_InitStructure.GPIO_Pin = V5_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(V5_GPIO, &GPIO_InitStructure);

	if(GPIO_ReadInputDataBit(V5_GPIO, V5_PIN))
		return TRUE;
	return FALSE;
}

