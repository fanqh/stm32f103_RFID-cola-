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

//----����ص�ѹ-------------------------------------------------------
void battery_show(void);
//----USBӦ��-------------------------------------------------------------------
void usb_app(void);



/////////////////////////////////////////////////////////////////////////

//����Ĭ�Ͻ�����ͨ��0~3																	   
void  Adc_Init(void)
{    
  
	//�ȳ�ʼ��IO��
 	RCC->APB2ENR|=1<<4;    //ʹ��PORTC��ʱ�� 
	//RCC->APB2ENR|=1<<0;	  ///���ÿ���

	GPIOC->CRL&=0XfF0FF0FF;//PC2 pc5 anolog����
	//ͨ��10/11����			 
	RCC->APB2ENR|=1<<9;    //ADC1ʱ��ʹ��	  
	RCC->APB2RSTR|=1<<9;   //ADC1��λ
	RCC->APB2RSTR&=~(1<<9);//��λ����	    
	RCC->CFGR&=~(3<<14);   //��Ƶ��������	
	//SYSCLK/DIV2=12M ADCʱ������Ϊ12M,ADC���ʱ�Ӳ��ܳ���14M!
	//���򽫵���ADC׼ȷ���½�! 
	RCC->CFGR|=2<<14;      	 

	ADC1->CR1&=0XF0FFFF;   //����ģʽ����
	ADC1->CR1|=0<<16;      //��������ģʽ  
	ADC1->CR1&=~(1<<8);    //��ɨ��ģʽ	  
	ADC1->CR2&=~(1<<1);    //����ת��ģʽ
	ADC1->CR2&=~(7<<17);	   
	ADC1->CR2|=7<<17;	   //�������ת��  
	ADC1->CR2|=1<<20;      //ʹ�����ⲿ����(SWSTART)!!!	����ʹ��һ���¼�������
	ADC1->CR2&=~(1<<11);   //�Ҷ���	 
	ADC1->SQR1&=~(0XF<<20);
	ADC1->SQR1&=0<<20;     //1��ת���ڹ��������� Ҳ����ֻת����������1 			   
	//����ͨ��15�Ĳ���ʱ��
	ADC1->SMPR1 &= 0XFFFC7e3F;//ͨ��15����ʱ�����	  
	ADC1->SMPR1 |= 7<<15;     //ͨ��15  239.5����,��߲���ʱ�������߾�ȷ��
	ADC1->SMPR1 |= 7<<6;     //ͨ��12  239.5����,��߲���ʱ�������߾�ȷ��		 

	ADC1->CR2|=1<<0;	    //����ADת����	 
	ADC1->CR2|=1<<3;        //ʹ�ܸ�λУ׼  
	while(ADC1->CR2&1<<3);  //�ȴ�У׼���� 			 
    //��λ��������ò���Ӳ���������У׼�Ĵ�������ʼ�����λ��������� 		 
	ADC1->CR2|=1<<2;        //����ADУ׼	   
	while(ADC1->CR2&1<<2);  //�ȴ�У׼����

	//��λ����������Կ�ʼУ׼������У׼����ʱ��Ӳ�����  
}				  
//----���ADCֵ----------------------------------------------------------
//ch:ͨ��ֵ
u16 BatteryGet()
{
	u32 vol;

	//����ת������	  		 
	ADC1->SQR3 &= 0XFFFFFFE0;//��������1 ͨ��ch
	ADC1->SQR3 |= 15;		//ͨ��15		  			    
	ADC1->CR2|=1<<22;       //��������ת��ͨ�� 
	while(!(ADC1->SR&1<<1));//�ȴ�ת������	 	   
	vol = ADC1->DR;		//����adcֵ	
	vol = vol * 4920 / 3143;	//У��
	return vol;
}


//���ADCֵ
//ch:ͨ��ֵ 0~3
u16 Get_Adc(u8 ch)   
{
	//����ת������	  		 
	ADC1->SQR3&=0XFFFFFFE0;//��������1 ͨ��ch
	ADC1->SQR3|=ch;		  			    
	ADC1->CR2|=1<<22;       //��������ת��ͨ�� 
	while(!(ADC1->SR&1<<1));//�ȴ�ת������	 	   
	return ADC1->DR;		//����adcֵ	
}

//----��ʼ��---------------------------------------------------------------					    
void POWER_Init(void)
{ 	 				 	 					    
	RCC->APB2ENR|=1<<4;    //ʹ��PORTCʱ�� 
	//CPU
 	GPIOC->CRL&=0XFFF0FFFF;	//PC4�������
 	GPIOC->CRL|=0X00030000;
	GPIOC->ODR&=~(1<<4);	//����͵�ƽ
	//BT
 	GPIOC->CRL&=0X0FFFFFFF;	//PC4�������
 	GPIOC->CRL|=0X30000000;
	GPIOC->ODR&=~(1<<7);	//����͵�ƽ

	Adc_Init();	  //������ʼ��ad����ͨ��15��12��
}

//----�ػ�---------------------------------------------------------------------
void power_off()
{
	u8 key;
	u32 timecount;

	beep(2, 100, 100);
	BT_Stop_Prog();			//�����ر� 
	OLED_Clear();		
	OLED_ShowString(0,16,1,1,"   ���ڹػ�...  ");	//���ڹػ�	
	OLED_Refresh_Gram();	 
	delay_s(2);
	PowerCPU_Off();			//��CPU��Դ	
	//----���V5��ѹ------------------------------------------------------------------
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
			    OLED_Refresh_Gram();					//ˢ����ʾ
			}

			key=DetectKeys();
			if(key == KEY_CMD_pow)
			{
				PowerCPU_On();			//��CPU��Դ	
				delay_ms(50);
		   		beep(1, 100, 100);
				while(keys_scan());  				//�ȴ���������
			                                        
				BT_ClrState();						//�������״̬
	            if(read_para_BT_enable()==1)			//��ȡ����
		            BT_Start_Prog();				//��������
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
//----�ػ�2---------------------------------------------------------------------
void power_off2()
{
	PowerCPU_Off();			//��CPU��Դ	
	//----���V5��ѹ------------------------------------------------------------------
	if(test_v5())
	{
		OLED_Clear();		
		while(1)
		{
			delay_ms(10);
			battery_show();
			OLED_Refresh_Gram();  //ˢ����ʾ
		}
	}
}
//----ɨ����̡���ʱ�ػ�----------------------------------------------------------					    
u8 scan_key_time_power()
{
	u8 key;
	u32 t=0;

	//ɨ����̣����û�м��̲�����30���رյ�Դ���ڵ磩
	while(1)
	{
		delay_ms(1);
		key=keys_scan();
		if(key == KEY_CMD_pow)
			power_off();		//�ػ�
		if(key)
			break;

		t++;
		if(t>TIME30S)		//30Sʱ�䵽
		{
			power_off();		//�ػ�
			t=0;
			key=0;
		}

		if(con_cmd())	
			t=0;

		//���usb����
		if(USB_GetConnFlag())
		{
			usb_app();
			return 0;
		}
	}
	return key;
}
//----���V5��ѹ------------------------------------------------------------------
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

