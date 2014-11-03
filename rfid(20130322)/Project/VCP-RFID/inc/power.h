#ifndef __POWER_H
#define __POWER_H	 
//////////////////////////////////////////////////////////////////////////////////	 

#include "stm32f10x.h"
#include "gpio_config.h"

#define PowerCPU_On()	(PCout(4)=1)		//CPU电源		   
#define PowerCPU_Off()	(PCout(4)=0)		//CPU电源		   
#define PowerBT_ON()	(PCout(7)=1)		//蓝牙电源
#define PowerBT_OFF()	(PCout(7)=0)		//蓝牙电源

void POWER_Init(void);	//电源管理接口初始化
u16 BatteryGet(void);	//获取电池电压	 
u8 scan_key_time_power(void);
void power_off(void);
void power_off2(void);
bool test_v5(void);		//检测V5电压

u16 Get_Adc(u8 ch);


					    
//////////////////////////////////////////////////////////////////////////////////	 
#endif
