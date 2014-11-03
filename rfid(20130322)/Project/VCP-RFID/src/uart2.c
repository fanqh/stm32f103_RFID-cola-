#include "uart2.h"
 

//////////////////////////////////////////////////////////////////


#define USART2_RX_MAX	128 
u8 USART2_RX_BUFF[USART2_RX_MAX];		//���ջ���,���64���ֽ�.
u16 USART2_RX_CNT;
static u32 Bound;

//----USART2�ж�---------------------------------------------------
void USART2_IRQHandler(void)
{
	u8 res;	    
	if(USART2->SR & (1<<5))		//���յ�����
	{	 
		res=USART2->DR; 
		if(USART2_RX_CNT<USART2_RX_MAX)
		{	
			USART2_RX_BUFF[USART2_RX_CNT]=res;
			USART2_RX_CNT++;
		}  		 									     
	}  											 
} 
//----USART2��ʼ��-------------------------------------------------
void USART2_Init(u32 pclk2,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	 

	//����Ӳ���ӿ�
	RCC->APB2ENR|=1<<2;   		//ʹ��PORTA��ʱ��  
	GPIOA->CRL&=0XFFFF00FF; 
	GPIOA->CRL|=0X00008B00;		//IO״̬����
		  
	RCC->APB1ENR |= 1<<17;  	//ʹ�ܴ���ʱ�� 
	RCC->APB1RSTR |= 1<<17;   	//��λ����
	RCC->APB1RSTR &= ~(1<<17);	//ֹͣ��λ	   	   
	  
	//���㲨����
	temp=(float)(pclk2/2*1000000)/(bound*16);//�õ�USARTDIV
	mantissa=temp;				 //�õ���������
	fraction=(temp-mantissa)*16; //�õ�С������	 
    mantissa<<=4;
	mantissa+=fraction; 
 	USART2->BRR=mantissa; 		//����������	 
	USART2->CR1|=0X200C;		//1λֹͣ,��У��λ.

	//ʹ�ܽ����ж�
	USART2->CR1|=1<<8;    		//PE�ж�ʹ��
	USART2->CR1|=1<<5;    		//���ջ������ǿ��ж�ʹ��	    	
	MY_NVIC_Init(2,3,USART2_IRQChannel,2);	//��2���ε͵����ȼ� 

	Bound = bound;
	USART2_RX_CNT=0;
}

//----USART2�������ݰ�----------------------------------------------------
u16 USART2_Package()
{
	static u8 t=0;
	u8 t_bound;				//����bound�������ʱ
	static u16 cnt=0;

	//δ���յ�����
	if(USART2_RX_CNT==0)
		return 0;

	//δ�����ʱ��
	t++;
	t_bound = 11000/Bound + 2;		//11000pbs�����ʣ�����Ҫ��1mS 
	if(t<=t_bound)
		return 0;
	t=0;

	//���ڽ�����
	if(cnt != USART2_RX_CNT)
	{
		cnt=USART2_RX_CNT;
		return 0;
	}

	cnt=0;
	return  USART2_RX_CNT;
}

//----USART2������--------------------------------------------------------
void USART2_Read(u8 *buff, vu16 cnt)
{
	memcpy((char*)buff, (char*)USART2_RX_BUFF, cnt);
	USART2_RX_CNT=0;
}

//----USART2����һ���ֽ�--------------------------------------------------
void USART2_Putc(u8 ch)
{      
	USART2->DR = ch;      
	while((USART2->SR & (1<<7))==0);	//ֱ���������   
}

//----USART2����----------------------------------------------------------
void USART2_Write(u8 *buff, vu16 cnt)
{
	while(cnt)
	{
		USART2_Putc(*buff);
		buff++;
		cnt--;
	}
}

//----USART2���----------------------------------------------------------
void USART2_Clear()
{
	USART2_RX_CNT=0;			//���������
}


