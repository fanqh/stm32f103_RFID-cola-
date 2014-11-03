/*
�İ�˵��

1. ���ڸ�ʽΪDD-MM-YYYY



*/

#include <stdio.h> 
#include <string.h> 
#include "PCF8563.h" 
////////////////////////////////////////////////////////////////////////////////// 	  


#define PCF8563_DEVIC 0xA2 		//PCF8563ʱ��оƬ�豸��
struct clock_ CLK;
struct _clock_ timer;
char time_str[12];	 //00:00:00
char date_str[12];	 //00-00-0000

//����ʱ��
//�������ʱ��ת��Ϊ����
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//����ֵ:0,�ɹ�;����:�������.
//�·����ݱ�											 
u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�	  
//ƽ����·����ڱ�
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};


///////////////////////////////////////////////////////////////////////////////
//----BCDʱ��ת��Ϊʱ���ַ���--------------------------------------------------
char* TimeStr()
{
    char *p;
	p=time_str;
	*p='0'+(CLK.hour>>4);	  	 	p++;
	*p='0'+(CLK.hour & 0x0F); 	  	p++;
	*p=':';				  			p++;
	*p='0'+(CLK.min>>4);			p++;
	*p='0'+(CLK.min & 0x0F);		p++;
	*p=':';				  			p++;
	*p='0'+(CLK.sec>>4);			p++;
	*p='0'+(CLK.sec & 0x0F);		p++;
	*p=0;
	return time_str;
}
//----��ȡRTCʱ���ַ���---------------------------------------------------------
char* GetRTCTime()
{
	//��ȡ1307��ʱ��ṹ
   	IICread(PCF8563_DEVIC, 2, &CLK.sec, 3);
	CLK.sec &= 0x7F;
	if(CLK.sec>0x59)
		CLK.sec = 0;
	CLK.min &= 0x7F;
	if(CLK.min>0x59)
		CLK.min = 0;	
	CLK.hour &= 0x3F;
	if(CLK.hour>0x23)
		CLK.hour = 0;
	//ʱ��ṹ���Ϊʱ���ַ���
	TimeStr();
	return time_str;
}
//----����PCD��ת��Ϊ�����ַ���----------------------------------------------
char *DateStr()
{
    char *p;
	p=date_str;
	*p='0'+(CLK.day>>4);			p++;
	*p='0'+(CLK.day & 0x0F);		p++;
	*p='-';				  			p++;
	*p='0'+(CLK.mon>>4);			p++;
	*p='0'+(CLK.mon & 0x0F);		p++;
	*p='-';				  			p++;
	*p='2';	  	 	p++;
	*p='0'; 	  	p++;
	*p='0'+(CLK.year>>4);	  	 	p++;
	*p='0'+(CLK.year & 0x0F); 	  	p++;
	*p=0;


//	*p='.';				  			p++;
//	*p='0'+CLK.week;				p++;
//	*p=0;
	return date_str;
}
//----��ȡRTC�����ַ���---------------------------------------------------
char* GetRTCDate()
{
	//��ȡ1307��ʱ��ṹ
   	IICread(PCF8563_DEVIC, 5, &CLK.day, 4);
	CLK.year &= 0xFF;
	if(CLK.year>0x99)
		CLK.year = 0;
	CLK.mon &= 0x1F;
	if(CLK.mon>0x12)
		CLK.mon=1;
	CLK.day &= 0x3F;
	if(CLK.day>0x31)
		CLK.day = 1;
	//ʱ��ṹ���Ϊ�����ַ���
	DateStr();
	return date_str;
}
/*
//----ʱ��BCD�뱣�浽PCF8563-------------------------------------------------
void writeTime()
{
   	IICwrite(PCF8563_DEVIC, 2, &CLK.sec, 3);
}
//----ʱ��BCD�뱣�浽PCF8563-------------------------------------------------
void writeTimeDate()
{
   	IICwrite(PCF8563_DEVIC, 2, &CLK.sec, 7);
}
*/

///////////////////////////////////////////////////////////////////////////////
//----�޸�ʱ��------------------------------------------------------------
void TIMEwrite(char *str)
{
	CLK.hour=(str[0]-'0')<<4;
	CLK.hour+=(str[1]-'0');
	CLK.min=(str[3]-'0')<<4;
	CLK.min+=(str[4]-'0');
	CLK.sec=(str[6]-'0')<<4;
	CLK.sec+=(str[7]-'0');
   	IICwrite(PCF8563_DEVIC, 2, &CLK.sec, 3);
}
//----�޸�����------------------------------------------------------------------
void DATEwrite(char *str)
{
	//�����ַ���ת��ΪBCD��
	CLK.day=(str[0]-'0')<<4;
	CLK.day+=(str[1]-'0');
	CLK.mon=(str[3]-'0')<<4;
	CLK.mon+=(str[4]-'0');
    CLK.year=(str[8]-'0')<<4;
	CLK.year+=(str[9]-'0');
   	IICwrite(PCF8563_DEVIC, 5, &CLK.day, 4);
}
//----��ʼ��PCF8563-------------------------------------------------------------
void PCF8563_Init(void)
{
	IIC_Init();

	//��ȡ1307��ʱ��ṹ
   	IICread(PCF8563_DEVIC, 2, &CLK.sec, 3);
	CLK.sec &= 0x7F;
	CLK.min &= 0x7F;
	CLK.hour &= 0x3F;
	//���ʱ��ĺϷ���
	if((CLK.sec > 0x59) || (CLK.min > 0x59) || (CLK.hour > 0x23))
	{
		CLK.sec = 0x00;
		CLK.min = 0x00;
		CLK.hour = 0x00;
   		IICwrite(PCF8563_DEVIC, 2, &CLK.sec, 3);
	}

	//��ȡ1307�����ڽṹ
   	IICread(PCF8563_DEVIC, 5, &CLK.day, 4);
	CLK.year &= 0xFF;
	CLK.mon &= 0x1F;
	CLK.day &= 0x3F;
	CLK.week &= 0x07;
	//������ڵĺϷ���
	if((CLK.year > 0x99) || (CLK.mon > 0x12) || (CLK.day > 0x31) || (CLK.week > 0x06))
	{
		CLK.year &= 0x00;
		CLK.mon &= 0x01;
		CLK.day &= 0x01;
		CLK.week &= 0x00;
   		IICwrite(PCF8563_DEVIC, 5, &CLK.day, 4);
	}
}



//////////////////////////////////////////////////////////////////////////////
//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//���:������ǲ�������.1,��.0,����
u8 Is_Leap_Year(u16 year)
{			  
	if(year%4==0) //�����ܱ�4����
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)
				return 1;//�����00��β,��Ҫ�ܱ�400���� 	   
			else
				return 0;   
		}
		else 
			return 1;   
	}
	else 
		return 0;	
}	 			   
//----������������ڼ�------------------------------------------------------
//��������:���빫�����ڵõ�����(ֻ����1901-2099��)
//������������������� 
//����ֵ�����ں�																						 
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{	
	u16 temp2;
	u8 yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// ���Ϊ21����,�������100  
	if (yearH>19)yearL+=100;
	// ����������ֻ��1900��֮���  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
} 
//----������ת��Ϊʱ��ṹ----------------------------------------------------
void TimerCount_To_TimerStruct(u32 timecount)
{
//	static u16 daycnt=0;
	u32 temp;
	u16 temp1=0;	  
	   
	temp=timecount/86400;   //�õ�����(��������Ӧ��)
//	if(daycnt!=temp)//����һ����
	{	  
//		daycnt=temp;
		temp1=1970;	//��1970�꿪ʼ
		while(temp>=365)
		{				 
			if(Is_Leap_Year(temp1))//������
			{
				if(temp>=366)temp-=366;//�����������
				else break;  
			}
			else temp-=365;	  //ƽ�� 
			temp1++;  
		}   
		timer.w_year=temp1;//�õ����
		temp1=0;
		while(temp>=28)//������һ����
		{
			if(Is_Leap_Year(timer.w_year)&&temp1==1)//�����ǲ�������/2�·�
			{
				if(temp>=29)temp-=29;//�����������
				else break; 
			}
			else 
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];//ƽ��
				else break;
			}
			temp1++;  
		}
		timer.w_month=temp1+1;//�õ��·�
		timer.w_date=temp+1;  //�õ����� 
	}
	temp=timecount%86400;     //�õ�������   	   
	timer.hour=temp/3600;     //Сʱ
	timer.min=(temp%3600)/60; //����	
	timer.sec=(temp%3600)%60; //����
//	timer.week=RTC_Get_Week(timer.w_year,timer.w_month,timer.w_date);//��ȡ����   
}


//----��ֵ��ת��ΪBCD��-----------------------------------------------------
u8 IntToBCD(u8 val)
{
	u8 bcd;

	bcd=(val/10)<<4;
	bcd+=(val%10);
	return bcd;	
}
//----BCD����ת��Ϊ��ֵ-----------------------------------------------------
u8 BCDToInt(u8 bcd)
{  
	u8 val;

	val=(bcd>>4)*10 + (bcd & 0x0F);
	return val;	
}

/////////////////////////////////////////////////////////////////////////////
//----BCDʱ��ת��Ϊʱ��ṹ--------------------------------------------------
void DateTimerBCD_To_TimerStruct()
{
	timer.w_year=BCDToInt(CLK.year)+2000;
	timer.w_month=BCDToInt(CLK.mon);
	timer.w_date=BCDToInt(CLK.day);
	timer.hour=BCDToInt(CLK.hour);
	timer.min=BCDToInt(CLK.min);	
	timer.sec=BCDToInt(CLK.sec);
}
//----ʱ��ṹת��ΪBCDʱ��--------------------------------------------------
void TimerStruct_To_DateTimerBCD()
{
	CLK.sec=IntToBCD(timer.sec);
	CLK.min=IntToBCD(timer.min);
	CLK.hour=IntToBCD(timer.hour);
	CLK.day=IntToBCD(timer.w_date);
	CLK.mon=IntToBCD(timer.w_month);
    CLK.year=IntToBCD(timer.w_year-2000);
}

//----������ת��Ϊ�ַ���ʱ��--------------------------------------------------
void TimerCount_To_DateTimerStr(char *DateStr, char *TimeStr, u32 timecount)
{
	TimerCount_To_TimerStruct(timecount);	//������ת��Ϊʱ��ṹ

	sprintf((char*)&TimeStr[0], "%02d", timer.hour);
	TimeStr[2]=':';
	sprintf((char*)&TimeStr[3], "%02d", timer.min);
	TimeStr[5]=':';
	sprintf((char*)&TimeStr[6], "%02d", timer.sec);
	TimeStr[8]=0;

	sprintf((char*)&DateStr[0], "%02d", timer.w_date);
	DateStr[2]='-';
	sprintf((char*)&DateStr[3], "%02d", timer.w_month);
	DateStr[5]='-';
	sprintf((char*)&DateStr[6], "%04d", timer.w_year);
	DateStr[10]=0;
}	 
//----ʱ��ṹת��Ϊ������----------------------------------------------------
u32 TimerStruct_To_TimerCount()
{
	u16 t;
	u32 seccount=0;
	if(timer.w_year<1970 || timer.w_year>2099)
		return 1;	   
	for(t=1970; t<timer.w_year; t++)		//��������ݵ��������
	{
		if(Is_Leap_Year(t))
			seccount+=31622400;				//�����������
		else
			seccount+=31536000;				//ƽ���������
	}
	for(t=0; t<timer.w_month-1; t++)	 	//��ǰ���·ݵ����������
	{
		seccount+=(u32)mon_table[t]*86400;	//�·����������
		if(Is_Leap_Year(timer.w_year) && t == 1)
			seccount+=86400;				//����2�·�����һ���������	   
	}
	seccount+=(u32)(timer.w_date-1)*86400;	//��ǰ�����ڵ���������� 
	seccount+=(u32)timer.hour*3600;			//Сʱ������
    seccount+=(u32)timer.min*60;	 		//����������
	seccount+=timer.sec;					//�������Ӽ���ȥ
	return seccount;	    
}

/////////////////////////////////////////////////////////////////////////////
//----��ȡRTC�����ַ���---------------------------------------------------
u32 GetRTC(char* DateStr, char* TimeStr)
{
	GetRTCTime();
	strcpy(TimeStr, time_str);	 //00:00:00
	GetRTCDate();
	strcpy(DateStr, date_str);	 //00-00-0000
	//
	DateTimerBCD_To_TimerStruct();
	return TimerStruct_To_TimerCount();
}
//----����RTCʱ��----------------------------------------------------------
void SetRTC(u32 timecount)
{
	
	TimerCount_To_TimerStruct(timecount);	//������ת��Ϊʱ��ṹ
	TimerStruct_To_DateTimerBCD();			//ʱ��ṹת��ΪBCDʱ��
   	IICwrite(PCF8563_DEVIC, 2, &CLK.sec, 7);

}




