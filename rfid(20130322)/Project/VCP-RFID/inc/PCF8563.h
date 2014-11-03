#ifndef __PCF8563_H
#define __PCF8563_H

#include "i2c.h"   
////////////////////////////////////////////////////////////////////////////////// 	  
//ʱ��ṹ��
struct _clock_ 
{
	u8 hour;
	u8 min;
	u8 sec;			
	//������������
	u16 w_year;
	u8  w_month;
	u8  w_date;
	u8  week;		 
};					 

//ʱ��ṹBCD��
struct clock_{
    u8 sec;
    u8 min;
    u8 hour;
    u8 day;
    u8 week;
    u8 mon;
    u8 year;
};
char* TimeStr(void);						//ʱ��PCD��ת��Ϊʱ���ַ���
char *DateStr(void);						//����PCD��ת��Ϊ�����ַ���
void writeTimeDate(void);

void PCF8563_Init(void); //��ʼ��IIC
//BOOL CLOCKstart(void);						//�ӱ�����
//void CLOCKstop(void);						//�ӱ�ֹͣ
char* GetRTCTime(void);						//��ȡ�ӱ�ʱ��
char* GetRTCDate(void);						//��ȡ�ӱ�����
void TIMEwrite(char *str);					//�޸��ӱ�ʱ��
void DATEwrite(char *str);					//�޸��ӱ�����

u32 GetRTC(char* DateStr, char* TimeStr);	//��ȡRTC�����ַ���
void TimerCount_To_DateTimerStr(char *DateStr, char *TimeStr, u32 timecount);	//������ת��Ϊ�ַ���ʱ��
void SetRTC(u32 timecount);					//����RTC�����ַ���



#endif
















