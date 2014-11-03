#ifndef __PCF8563_H
#define __PCF8563_H

#include "i2c.h"   
////////////////////////////////////////////////////////////////////////////////// 	  
//时间结构体
struct _clock_ 
{
	u8 hour;
	u8 min;
	u8 sec;			
	//公历日月年周
	u16 w_year;
	u8  w_month;
	u8  w_date;
	u8  week;		 
};					 

//时间结构BCD码
struct clock_{
    u8 sec;
    u8 min;
    u8 hour;
    u8 day;
    u8 week;
    u8 mon;
    u8 year;
};
char* TimeStr(void);						//时间PCD码转换为时间字符串
char *DateStr(void);						//日期PCD码转换为日期字符串
void writeTimeDate(void);

void PCF8563_Init(void); //初始化IIC
//BOOL CLOCKstart(void);						//钟表启动
//void CLOCKstop(void);						//钟表停止
char* GetRTCTime(void);						//获取钟表时间
char* GetRTCDate(void);						//获取钟表日期
void TIMEwrite(char *str);					//修改钟表时间
void DATEwrite(char *str);					//修改钟表日期

u32 GetRTC(char* DateStr, char* TimeStr);	//获取RTC日期字符串
void TimerCount_To_DateTimerStr(char *DateStr, char *TimeStr, u32 timecount);	//秒钟数转换为字符串时间
void SetRTC(u32 timecount);					//保存RTC日期字符串



#endif
















