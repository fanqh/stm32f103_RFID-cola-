#include "gpio_config.h"
#include "i2c.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
////////////////////////////////////////////////////////////////////////////////// 	  

u8 ack;

//----��ʼ��IIC-------------------------------------------------------------------
void IIC_Init(void)
{					     
 	RCC->APB2ENR|=1<<4;//��ʹ������IO PORTCʱ�� 							 
	GPIOC->CRH&=0X00FFFFFF;//PC14/15 �������
	GPIOC->CRH|=0X77000000;	   
	GPIOC->ODR|=3<<14;     //PC14,15 �����
}
//----����IIC��ʼ�ź�-------------------------------------------------------------
void IIC_Start(void)
{

	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(9); //9
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(9); //	9
	IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  
//----����IICֹͣ�ź�-------------------------------------------------------------
void IIC_Stop(void)
{
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(9);	//9
	IIC_SCL=1; 
	delay_us(9);	//9						   	
	IIC_SDA=1;//����I2C���߽����ź�
	delay_us(9);	//9						   	
}
//----һ��SCLʱ��----------------------------------------------------
u8 IIC_Clock()
{
	u8 sample;
	
	delay_us(9);   //9
	IIC_SCL=1;      		//��ʱ����Ϊ�ߣ�֪ͨ��������ʼ��������λ
	delay_us(9);		//��֤ʱ�Ӹߵ�ƽ���ڴ���4��s   9
	sample=READ_SDA;
	IIC_SCL=0; 
	return sample;
}
//----����һ���ֽ�----------------------------------------------------
void IIC_SendByte(u8 c)
{
	u8 BitCnt;
  
	//Ҫ���͵����ݳ���Ϊ8λ,
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
//----����һ���ֽ�----------------------------------------------------
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
//----Ӧ��----------------------------------------------------------
void IIC_Ack(u8 a)
{
   	IIC_SDA = a;     	//�ڴ˷���Ӧ����Ӧ���ź� 
	IIC_Clock();
	IIC_SDA = 1;
}

///////////////////////////////////////////////////////////////////
//----дһ���ֽ�----------------------------------------------------
bool IICputc(u8 sla, u8 c)
{
	IIC_Start();          //��������
	IIC_SendByte(sla);		//����������ַ
	if(ack==0)
    	return FALSE;

	IIC_SendByte(c);          //��������
	if(ack==0)            
    	return FALSE;

	IIC_Stop();           //�������� 
	return TRUE;
}
//----д���ӵ�ַ----------------------------------------------------
bool IICwrite(u8 sla, u8 suba, u8 *s, u8 no)
{
	u8 i;

	IIC_Start();          //��������
	IIC_SendByte(sla);        //����������ַ
	if(ack==0)           
    	return FALSE;

	IIC_SendByte(suba);       //���������ӵ�ַ
	if(ack==0)
	    return FALSE;

	for(i=0;i<no;i++)
    {   
    	IIC_SendByte(*s);      //��������
		if(ack==0)
       		return FALSE;
		s++;
    } 
	IIC_Stop();           //��������
	return TRUE;
}
//----д���ӵ�ַ----------------------------------------------------
bool IICwriteExt(u8 sla, u8 *s, u8 no)
{
	u8 i;

	IIC_Start();          //��������
	IIC_SendByte(sla);        //����������ַ
	if(ack==0)
    	return FALSE;

	for(i=0;i<no;i++)
    {   
    	IIC_SendByte(*s);      //��������
		if(ack==0)
			return FALSE;
		s++;
    } 
	IIC_Stop();           //�������� 
	return TRUE;
}
//----��һ���ֽ�----------------------------------------------------
bool IICgetc(u8 sla, u8 *c)
{
	IIC_Start();          //��������
	IIC_SendByte(sla+1);      //����������ַ
	if(ack==0)
		return FALSE;

	*c=IIC_RcvByte();         //��������
	IIC_Ack(1);           //�����꣬���ͷ�Ӧ��λ����������
	IIC_Stop();           //�������� 
	return TRUE;
}
//----�����ӵ�ַ----------------------------------------------------
bool IICread(u8 sla, u8 suba, u8 *s, u8 no)
{
	u8 i;

	IIC_Start();          //��������
	IIC_SendByte(sla);        //����������ַ
	if(ack==0)
    	return FALSE;

	IIC_SendByte(suba);       //���������ӵ�ַ
	if(ack==0)
    	return FALSE;
		
	IIC_Start();			//������������
  	IIC_SendByte(sla+1);
  	if(ack==0)
    	return FALSE;

	for(i=0;i<no-1;i++)   //�Ƚ���ǰ(no-1)�ֽ�
	{   
    	*s=IIC_RcvByte();      //��������
     	IIC_Ack(0);        //��δ�����꣬����Ӧ��λ  
     	s++;
   	} 
	*s=IIC_RcvByte();        //���յ�no�ֽ�
	IIC_Ack(1);          //�����꣬���ͷ�Ӧ��λ
	IIC_Stop();          //�������� 
	return TRUE;
}
//----�����ӵ�ַ----------------------------------------------------
bool IICreadExt(u8 sla, u8 *s, u8 no)
{
	u8 i;

	IIC_Start();
	IIC_SendByte(sla+1);		//R/Wѡ��λ��Ϊ1ʱΪ���� Ϊ0 ʱΪд
	if(ack==0)
		return FALSE;

	for(i=0;i<no-1;i++)   //�Ƚ���ǰ��no-1)���ֽ�
   	{   
    	*s=IIC_RcvByte();      //��������
     	IIC_Ack(0);        //δ��ȡ�꣬����Ӧ��λ  
     	s++;
	} 
   	*s=IIC_RcvByte();        //���յ�no�ֽ�
   	IIC_Ack(1);          //�����꣬���ͷ�Ӧ��λ
   	IIC_Stop();          //�������� 
   	return TRUE;
}


