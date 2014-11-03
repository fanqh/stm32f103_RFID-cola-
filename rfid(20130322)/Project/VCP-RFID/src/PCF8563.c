/*
改版说明

1. 日期格式为DD-MM-YYYY



*/

#include <stdio.h> 
#include <string.h> 
#include "PCF8563.h" 
////////////////////////////////////////////////////////////////////////////////// 	  


#define PCF8563_DEVIC 0xA2 		//PCF8563时钟芯片设备号
struct clock_ CLK;
struct _clock_ timer;
char time_str[12];	 //00:00:00
char date_str[12];	 //00-00-0000

//设置时钟
//把输入的时钟转换为秒钟
//以1970年1月1日为基准
//1970~2099年为合法年份
//返回值:0,成功;其他:错误代码.
//月份数据表											 
u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
//平年的月份日期表
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};


///////////////////////////////////////////////////////////////////////////////
//----BCD时间转换为时间字符串--------------------------------------------------
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
//----获取RTC时间字符串---------------------------------------------------------
char* GetRTCTime()
{
	//读取1307到时间结构
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
	//时间结构输出为时间字符串
	TimeStr();
	return time_str;
}
//----日期PCD码转换为日期字符串----------------------------------------------
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
//----获取RTC日期字符串---------------------------------------------------
char* GetRTCDate()
{
	//读取1307到时间结构
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
	//时间结构输出为日期字符串
	DateStr();
	return date_str;
}
/*
//----时间BCD码保存到PCF8563-------------------------------------------------
void writeTime()
{
   	IICwrite(PCF8563_DEVIC, 2, &CLK.sec, 3);
}
//----时间BCD码保存到PCF8563-------------------------------------------------
void writeTimeDate()
{
   	IICwrite(PCF8563_DEVIC, 2, &CLK.sec, 7);
}
*/

///////////////////////////////////////////////////////////////////////////////
//----修改时间------------------------------------------------------------
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
//----修改日期------------------------------------------------------------------
void DATEwrite(char *str)
{
	//日期字符串转化为BCD码
	CLK.day=(str[0]-'0')<<4;
	CLK.day+=(str[1]-'0');
	CLK.mon=(str[3]-'0')<<4;
	CLK.mon+=(str[4]-'0');
    CLK.year=(str[8]-'0')<<4;
	CLK.year+=(str[9]-'0');
   	IICwrite(PCF8563_DEVIC, 5, &CLK.day, 4);
}
//----初始化PCF8563-------------------------------------------------------------
void PCF8563_Init(void)
{
	IIC_Init();

	//读取1307到时间结构
   	IICread(PCF8563_DEVIC, 2, &CLK.sec, 3);
	CLK.sec &= 0x7F;
	CLK.min &= 0x7F;
	CLK.hour &= 0x3F;
	//检查时间的合法性
	if((CLK.sec > 0x59) || (CLK.min > 0x59) || (CLK.hour > 0x23))
	{
		CLK.sec = 0x00;
		CLK.min = 0x00;
		CLK.hour = 0x00;
   		IICwrite(PCF8563_DEVIC, 2, &CLK.sec, 3);
	}

	//读取1307到日期结构
   	IICread(PCF8563_DEVIC, 5, &CLK.day, 4);
	CLK.year &= 0xFF;
	CLK.mon &= 0x1F;
	CLK.day &= 0x3F;
	CLK.week &= 0x07;
	//检查日期的合法性
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
//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
u8 Is_Leap_Year(u16 year)
{			  
	if(year%4==0) //必须能被4整除
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)
				return 1;//如果以00结尾,还要能被400整除 	   
			else
				return 0;   
		}
		else 
			return 1;   
	}
	else 
		return 0;	
}	 			   
//----获得现在是星期几------------------------------------------------------
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日 
//返回值：星期号																						 
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{	
	u16 temp2;
	u8 yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// 如果为21世纪,年份数加100  
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
} 
//----秒钟数转换为时间结构----------------------------------------------------
void TimerCount_To_TimerStruct(u32 timecount)
{
//	static u16 daycnt=0;
	u32 temp;
	u16 temp1=0;	  
	   
	temp=timecount/86400;   //得到天数(秒钟数对应的)
//	if(daycnt!=temp)//超过一天了
	{	  
//		daycnt=temp;
		temp1=1970;	//从1970年开始
		while(temp>=365)
		{				 
			if(Is_Leap_Year(temp1))//是闰年
			{
				if(temp>=366)temp-=366;//闰年的秒钟数
				else break;  
			}
			else temp-=365;	  //平年 
			temp1++;  
		}   
		timer.w_year=temp1;//得到年份
		temp1=0;
		while(temp>=28)//超过了一个月
		{
			if(Is_Leap_Year(timer.w_year)&&temp1==1)//当年是不是闰年/2月份
			{
				if(temp>=29)temp-=29;//闰年的秒钟数
				else break; 
			}
			else 
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];//平年
				else break;
			}
			temp1++;  
		}
		timer.w_month=temp1+1;//得到月份
		timer.w_date=temp+1;  //得到日期 
	}
	temp=timecount%86400;     //得到秒钟数   	   
	timer.hour=temp/3600;     //小时
	timer.min=(temp%3600)/60; //分钟	
	timer.sec=(temp%3600)%60; //秒钟
//	timer.week=RTC_Get_Week(timer.w_year,timer.w_month,timer.w_date);//获取星期   
}


//----数值数转换为BCD码-----------------------------------------------------
u8 IntToBCD(u8 val)
{
	u8 bcd;

	bcd=(val/10)<<4;
	bcd+=(val%10);
	return bcd;	
}
//----BCD码数转换为数值-----------------------------------------------------
u8 BCDToInt(u8 bcd)
{  
	u8 val;

	val=(bcd>>4)*10 + (bcd & 0x0F);
	return val;	
}

/////////////////////////////////////////////////////////////////////////////
//----BCD时间转换为时间结构--------------------------------------------------
void DateTimerBCD_To_TimerStruct()
{
	timer.w_year=BCDToInt(CLK.year)+2000;
	timer.w_month=BCDToInt(CLK.mon);
	timer.w_date=BCDToInt(CLK.day);
	timer.hour=BCDToInt(CLK.hour);
	timer.min=BCDToInt(CLK.min);	
	timer.sec=BCDToInt(CLK.sec);
}
//----时间结构转换为BCD时间--------------------------------------------------
void TimerStruct_To_DateTimerBCD()
{
	CLK.sec=IntToBCD(timer.sec);
	CLK.min=IntToBCD(timer.min);
	CLK.hour=IntToBCD(timer.hour);
	CLK.day=IntToBCD(timer.w_date);
	CLK.mon=IntToBCD(timer.w_month);
    CLK.year=IntToBCD(timer.w_year-2000);
}

//----秒钟数转换为字符串时间--------------------------------------------------
void TimerCount_To_DateTimerStr(char *DateStr, char *TimeStr, u32 timecount)
{
	TimerCount_To_TimerStruct(timecount);	//秒钟数转换为时间结构

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
//----时间结构转换为秒钟数----------------------------------------------------
u32 TimerStruct_To_TimerCount()
{
	u16 t;
	u32 seccount=0;
	if(timer.w_year<1970 || timer.w_year>2099)
		return 1;	   
	for(t=1970; t<timer.w_year; t++)		//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t))
			seccount+=31622400;				//闰年的秒钟数
		else
			seccount+=31536000;				//平年的秒钟数
	}
	for(t=0; t<timer.w_month-1; t++)	 	//把前面月份的秒钟数相加
	{
		seccount+=(u32)mon_table[t]*86400;	//月份秒钟数相加
		if(Is_Leap_Year(timer.w_year) && t == 1)
			seccount+=86400;				//闰年2月份增加一天的秒钟数	   
	}
	seccount+=(u32)(timer.w_date-1)*86400;	//把前面日期的秒钟数相加 
	seccount+=(u32)timer.hour*3600;			//小时秒钟数
    seccount+=(u32)timer.min*60;	 		//分钟秒钟数
	seccount+=timer.sec;					//最后的秒钟加上去
	return seccount;	    
}

/////////////////////////////////////////////////////////////////////////////
//----获取RTC日期字符串---------------------------------------------------
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
//----保存RTC时间----------------------------------------------------------
void SetRTC(u32 timecount)
{
	
	TimerCount_To_TimerStruct(timecount);	//秒钟数转换为时间结构
	TimerStruct_To_DateTimerBCD();			//时间结构转换为BCD时间
   	IICwrite(PCF8563_DEVIC, 2, &CLK.sec, 7);

}




