#include "uart2.h"
 

//////////////////////////////////////////////////////////////////


#define USART2_RX_MAX	128 
u8 USART2_RX_BUFF[USART2_RX_MAX];		//接收缓冲,最大64个字节.
u16 USART2_RX_CNT;
static u32 Bound;

//----USART2中断---------------------------------------------------
void USART2_IRQHandler(void)
{
	u8 res;	    
	if(USART2->SR & (1<<5))		//接收到数据
	{	 
		res=USART2->DR; 
		if(USART2_RX_CNT<USART2_RX_MAX)
		{	
			USART2_RX_BUFF[USART2_RX_CNT]=res;
			USART2_RX_CNT++;
		}  		 									     
	}  											 
} 
//----USART2初始化-------------------------------------------------
void USART2_Init(u32 pclk2,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	 

	//设置硬件接口
	RCC->APB2ENR|=1<<2;   		//使能PORTA口时钟  
	GPIOA->CRL&=0XFFFF00FF; 
	GPIOA->CRL|=0X00008B00;		//IO状态设置
		  
	RCC->APB1ENR |= 1<<17;  	//使能串口时钟 
	RCC->APB1RSTR |= 1<<17;   	//复位串口
	RCC->APB1RSTR &= ~(1<<17);	//停止复位	   	   
	  
	//计算波特率
	temp=(float)(pclk2/2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分	 
    mantissa<<=4;
	mantissa+=fraction; 
 	USART2->BRR=mantissa; 		//波特率设置	 
	USART2->CR1|=0X200C;		//1位停止,无校验位.

	//使能接收中断
	USART2->CR1|=1<<8;    		//PE中断使能
	USART2->CR1|=1<<5;    		//接收缓冲区非空中断使能	    	
	MY_NVIC_Init(2,3,USART2_IRQChannel,2);	//组2，次低低优先级 

	Bound = bound;
	USART2_RX_CNT=0;
}

//----USART2接收数据包----------------------------------------------------
u16 USART2_Package()
{
	static u8 t=0;
	u8 t_bound;				//根据bound计算的延时
	static u16 cnt=0;

	//未接收到数据
	if(USART2_RX_CNT==0)
		return 0;

	//未到间隔时间
	t++;
	t_bound = 11000/Bound + 2;		//11000pbs的速率，至少要用1mS 
	if(t<=t_bound)
		return 0;
	t=0;

	//正在接收中
	if(cnt != USART2_RX_CNT)
	{
		cnt=USART2_RX_CNT;
		return 0;
	}

	cnt=0;
	return  USART2_RX_CNT;
}

//----USART2读数据--------------------------------------------------------
void USART2_Read(u8 *buff, vu16 cnt)
{
	memcpy((char*)buff, (char*)USART2_RX_BUFF, cnt);
	USART2_RX_CNT=0;
}

//----USART2发送一个字节--------------------------------------------------
void USART2_Putc(u8 ch)
{      
	USART2->DR = ch;      
	while((USART2->SR & (1<<7))==0);	//直到发送完毕   
}

//----USART2发送----------------------------------------------------------
void USART2_Write(u8 *buff, vu16 cnt)
{
	while(cnt)
	{
		USART2_Putc(*buff);
		buff++;
		cnt--;
	}
}

//----USART2清除----------------------------------------------------------
void USART2_Clear()
{
	USART2_RX_CNT=0;			//清除计数器
}


