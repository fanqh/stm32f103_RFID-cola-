#ifndef __POWER_H
#define __POWER_H	 
//////////////////////////////////////////////////////////////////////////////////	 

#include "stm32f10x.h"
#include "gpio_config.h"

#define PowerCPU_On()	(PCout(4)=1)		//CPU��Դ		   
#define PowerCPU_Off()	(PCout(4)=0)		//CPU��Դ		   
#define PowerBT_ON()	(PCout(7)=1)		//������Դ
#define PowerBT_OFF()	(PCout(7)=0)		//������Դ

void POWER_Init(void);	//��Դ����ӿڳ�ʼ��
u16 BatteryGet(void);	//��ȡ��ص�ѹ	 
u8 scan_key_time_power(void);
void power_off(void);
void power_off2(void);
bool test_v5(void);		//���V5��ѹ

u16 Get_Adc(u8 ch);


					    
//////////////////////////////////////////////////////////////////////////////////	 
#endif
