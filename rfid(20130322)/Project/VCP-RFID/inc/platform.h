#ifndef __PLATFORM_H__
#define __PLATFORM_H__


#include <stdio.h> 
#include "stm32f10x.h"
#include "gpio_config.h"

#define PLATFORM_CCCL_BETA_V1 	0x22
#define PLATFORM_CCCL_BETA_V3 	0
#define PLATFORM_CCCL_BETA_V4 	0x01

#define PLATFORM_CCCL_ERROR 	0XFF


/********  Beta V3 Keys Define *******/
#define Beta_V3_KEY_1 	PCin(13)   
#define Beta_V3_KEY_2	PBin(12)  
#define Beta_V3_KEY_3	PBin(9) 
#define Beta_V3_KEY_4	PBin(13)
#define Beta_V3_KEY_5 	PBin(14) 
#define Beta_V3_KEY_6	PBin(8)

/********  Beta V1 Keys Define *******/
#define Beta_V1_KEY_1 	PCin(13)   
#define Beta_V1_KEY_2	PBin(12) 
#define Beta_V1_KEY_3	PBin(8)
#define Beta_V1_KEY_4	PBin(9) 
#define Beta_V1_KEY_5	PBin(13)
#define Beta_V1_KEY_6 	PBin(14) 

						   
typedef u8 (*fnKey)();
/*
u8 Beta_V3_GetKEY_1();
u8 Beta_V3_GetKEY_2();
u8 Beta_V3_GetKEY_3();
u8 Beta_V3_GetKEY_4();
u8 Beta_V3_GetKEY_5();
u8 Beta_V3_GetKEY_6();

u8 Beta_V1_GetKEY_1();
u8 Beta_V1_GetKEY_2();
u8 Beta_V1_GetKEY_3();
u8 Beta_V1_GetKEY_4();
u8 Beta_V1_GetKEY_5();
u8 Beta_V1_GetKEY_6();
*/


typedef struct
{
	fnKey key1;
	fnKey key2;
	fnKey key3;
	fnKey key4;
	fnKey key5;
	fnKey key6;
	u8 powerHTRC110_Level;
	u32 valuevcc5config;
}Platform_Param;
						
extern Platform_Param g_Platform_Param;
bool initPlatform(void);
u8 getPowerHTRC110_Level(void);
u32 getValuev5Config(void);
u8 getPlatformVersion(void);




#endif  

