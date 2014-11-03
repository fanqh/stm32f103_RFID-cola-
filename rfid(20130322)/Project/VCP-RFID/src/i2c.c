#include "gpio_config.h"
#include "i2c.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
////////////////////////////////////////////////////////////////////////////////// 	  

u8 ack;

//----初始化IIC-------------------------------------------------------------------
void IIC_Init(void)
{					     
 	RCC->APB2ENR|=1<<4;//先使能外设IO PORTC时钟 							 
	GPIOC->CRH&=0X00FFFFFF;//PC14/15 推挽输出
	GPIOC->CRH|=0X77000000;	   
	GPIOC->ODR|=3<<14;     //PC14,15 输出高
}
//----产生IIC起始信号-------------------------------------------------------------
void IIC_Start(void)
{

	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(9); //9
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(9); //	9
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//----产生IIC停止信号-------------------------------------------------------------
void IIC_Stop(void)
{
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(9);	//9
	IIC_SCL=1; 
	delay_us(9);	//9						   	
	IIC_SDA=1;//发送I2C总线结束信号
	delay_us(9);	//9						   	
}
//----一个SCL时钟----------------------------------------------------
u8 IIC_Clock()
{
	u8 sample;
	
	delay_us(9);   //9
	IIC_SCL=1;      		//置时钟线为高，通知被控器开始接收数据位
	delay_us(9);		//保证时钟高电平周期大于4μs   9
	sample=READ_SDA;
	IIC_SCL=0; 
	return sample;
}
//----发送一个字节----------------------------------------------------
void IIC_SendByte(u8 c)
{
	u8 BitCnt;
  
	//要传送的数据长度为8位,
	for(BitCnt=0; BitCnt<8; BitCnt++)  
   	{
        IIC_SDA = (c & 0x80) >> 7;
		c<<=1;
		IIC_Clock();
	}
	if(IIC_Clock())
    	ack=0;
	else 
    	ack=1;
}
//----接收一个字节----------------------------------------------------
u8 IIC_RcvByte()
{
	u8 retc;
	u8 BitCnt;
  
	retc=0; 
	for(BitCnt=0;BitCnt<8;BitCnt++)
    {
     	retc=retc<<1;
		if(IIC_Clock())
        	retc++;
    }
	return retc;
}
//----应答----------------------------------------------------------
void IIC_Ack(u8 a)
{
   	IIC_SDA = a;     	//在此发出应答或非应答信号 
	IIC_Clock();
	IIC_SDA = 1;
}

///////////////////////////////////////////////////////////////////
//----写一个字节----------------------------------------------------
bool IICputc(u8 sla, u8 c)
{
	IIC_Start();          //启动总线
	IIC_SendByte(sla);		//发送器件地址
	if(ack==0)
    	return FALSE;

	IIC_SendByte(c);          //发送数据
	if(ack==0)            
    	return FALSE;

	IIC_Stop();           //结束总线 
	return TRUE;
}
//----写有子地址----------------------------------------------------
bool IICwrite(u8 sla, u8 suba, u8 *s, u8 no)
{
	u8 i;

	IIC_Start();          //启动总线
	IIC_SendByte(sla);        //发送器件地址
	if(ack==0)           
    	return FALSE;

	IIC_SendByte(suba);       //发送器件子地址
	if(ack==0)
	    return FALSE;

	for(i=0;i<no;i++)
    {   
    	IIC_SendByte(*s);      //发送数据
		if(ack==0)
       		return FALSE;
		s++;
    } 
	IIC_Stop();           //结束总线
	return TRUE;
}
//----写无子地址----------------------------------------------------
bool IICwriteExt(u8 sla, u8 *s, u8 no)
{
	u8 i;

	IIC_Start();          //启动总线
	IIC_SendByte(sla);        //发送器件地址
	if(ack==0)
    	return FALSE;

	for(i=0;i<no;i++)
    {   
    	IIC_SendByte(*s);      //发送数据
		if(ack==0)
			return FALSE;
		s++;
    } 
	IIC_Stop();           //结束总线 
	return TRUE;
}
//----读一个字节----------------------------------------------------
bool IICgetc(u8 sla, u8 *c)
{
	IIC_Start();          //启动总线
	IIC_SendByte(sla+1);      //发送器件地址
	if(ack==0)
		return FALSE;

	*c=IIC_RcvByte();         //接收数据
	IIC_Ack(1);           //接收完，发送非应答位，结束总线
	IIC_Stop();           //结束总线 
	return TRUE;
}
//----读有子地址----------------------------------------------------
bool IICread(u8 sla, u8 suba, u8 *s, u8 no)
{
	u8 i;

	IIC_Start();          //启动总线
	IIC_SendByte(sla);        //发送器件地址
	if(ack==0)
    	return FALSE;

	IIC_SendByte(suba);       //发送器件子地址
	if(ack==0)
    	return FALSE;
		
	IIC_Start();			//重新启动总线
  	IIC_SendByte(sla+1);
  	if(ack==0)
    	return FALSE;

	for(i=0;i<no-1;i++)   //先接收前(no-1)字节
	{   
    	*s=IIC_RcvByte();      //接收数据
     	IIC_Ack(0);        //还未接收完，发送应答位  
     	s++;
   	} 
	*s=IIC_RcvByte();        //接收第no字节
	IIC_Ack(1);          //接收完，发送非应答位
	IIC_Stop();          //结束总线 
	return TRUE;
}
//----读无子地址----------------------------------------------------
bool IICreadExt(u8 sla, u8 *s, u8 no)
{
	u8 i;

	IIC_Start();
	IIC_SendByte(sla+1);		//R/W选择位，为1时为读， 为0 时为写
	if(ack==0)
		return FALSE;

	for(i=0;i<no-1;i++)   //先接收前（no-1)个字节
   	{   
    	*s=IIC_RcvByte();      //接收数据
     	IIC_Ack(0);        //未读取完，发送应答位  
     	s++;
	} 
   	*s=IIC_RcvByte();        //接收第no字节
   	IIC_Ack(1);          //接收完，发送非应答位
   	IIC_Stop();          //结束总线 
   	return TRUE;
}


