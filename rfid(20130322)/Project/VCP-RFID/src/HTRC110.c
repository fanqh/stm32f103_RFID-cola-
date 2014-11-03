#include "HTRC110.h" 
#include "RFID.h"
#include "hitagS.h"
#include "bitbuff.h"
#include "config.h"
#include "platform.h"


#define CMD_GET_SAMP_TIME	0x02 
#define CMD_GET_CFG_PAGE	0x04 
#define CMD_READ_PHASE		0x08 
#define CMD_READ_TAG		0xE0 
#define CMD_WRITE_TAG_N		0x10 
#define CMD_WRITE_TAG		0xC0 
#define CMD_SET_CFG_PAGE	0x40 
#define CMD_SET_SAMP_TIME	0x80 

u8 send_cmd[6] ;
u8 cNumEqBits;
u8 cReceiveBits;

//----��ʱ---------------------------------------------------------------------
void s_delay_us(vu16 us)
{
	vu16 nCount=20;

	while(us)
	{
		while(nCount)
			nCount--;
		us--;
	}
}
//----�����ź�---------------------------------------------------------------
void S_Start()
{
	SDIN=0;
	s_delay_us(20);
	SCLK=1;	
	s_delay_us(20);
	SDIN=1;
	s_delay_us(20);
	SCLK=0;
	s_delay_us(20);
}
//----ģ��SPIд��8λ����-------------------------------------------------------
char S_PutcGetc(char data, vu8 nBit)
{
	char ch=0;

	while(nBit)
	{
		//���һλ����
		if(data & 0x80)
			SDIN=1;
		else	
			SDIN=0;
		data <<= 1;
		s_delay_us(10);

		//���ʱ���źŸߵ�ƽ��
		SCLK=1;	
		s_delay_us(20);	//

		//����һλ����
		ch <<= 1;
		ch |= SDOUT;

		//���ʱ���źŵ͵�ƽ��
		SCLK=0;	
		s_delay_us(10);

		nBit--;
	}
	return ch;
}


//---------------------------------------------------------------------------
void Send_HTRC110_Cmd(u8 RcCmd)
{
	S_Start();		//�����ź�
	S_PutcGetc(RcCmd, 8);
}
//---------------------------------------------------------------------------
u8 Receive_HTRC110_Data(void)
{
	return S_PutcGetc(0, 8);
}


//---------------------------------------------------------------------------
u8 GET_CONFIG_PAGE(u8 Config_PageNo)
{
	Send_HTRC110_Cmd(0x04 | Config_PageNo);
	return Receive_HTRC110_Data ();
}
//---------------------------------------------------------------------------
u8 GET_SAMPLING_TIME(void)
{
	Send_HTRC110_Cmd(0x02);
	return Receive_HTRC110_Data ();
}
//---------------------------------------------------------------------------
u8 READ_PHASE(void)
{
	Send_HTRC110_Cmd(0x08);
	return Receive_HTRC110_Data ();
}

//---------------------------------------------------------------------------
bool Ast_Adjust(u8 t_offset)
{
	u8 t_antenna, ch;

	t_antenna = READ_PHASE();
	t_antenna <<= 1;
	t_antenna += t_offset;
	t_antenna &= 0x3F;
	SET_SAMPLING_TIME(t_antenna);
	ch=GET_SAMPLING_TIME();
	if(ch!=t_antenna)
		return FALSE;
	return TRUE;
}
//---------------------------------------------------------------------------
void READ_TAG(void)
{   
	S_Start();		
	S_PutcGetc(0xE0, 3);
	SDIN=0;
}
//---------------------------------------------------------------------------
void WRITE_TAG(void)
{
	S_Start();		
	S_PutcGetc(0xC0, 3);
	SDIN=0;
}
//---------------------------------------------------------------------------


void Fast_Settling(void)
/******************************************************************************
*                                                                             *
* Description:                                                                *
* Implementation of the general fast settling sequence described in the       *
* application note AN97070.                                                   *
*                                                                             *
* Parameters: none                                                            *
* Return: none                                                                *
*                                                                             *
******************************************************************************/
{
	SET_CONFIG_PAGE(CF_PAGE_2+THRESET+FREEZE1+FREEZE0);
	delay_ms(5);
   	SET_CONFIG_PAGE(CF_PAGE_2+THRESET);
	delay_ms(1);
	SET_CONFIG_PAGE(CF_PAGE_2);
}  
///////////////////////////////////////////////////////////////////////////////
void Write_Settling (void)
/******************************************************************************
*                                                                             *
* Description:                                                                *
* Implementation of the after-write fast settling sequence described in the   *
* application note AN97070. Before calling the routine the following          *
* operations should have been executed:                                       *
*  SET_CONFIG_PAGE(CF_PAGE_2+THRSET1+FREEZE0);                                *
*  WRITE_TAG(_N)();                                                           *
*  "sending bits to transponder;n"                                            *
*                                                                             *
* Parameters: none                                                            *
* Return: none                                                                *
*                                                                             *
******************************************************************************/
{
	delay_us(200);
	SET_CONFIG_PAGE(CF_PAGE_2+THRESET+FREEZE1+FREEZE0);
	delay_us(250);
	SET_CONFIG_PAGE(CF_PAGE_2);
}

//----HTRC110ΪHITAG׼���ö��ź�------------------------------------------------
void HTRC110_TitagReady()  
{
	WRITE_TAG_N(0); //fan
	HITAGS_Reast();			//HITAGS�����縴λ
	delay_us(200);
		                                             	
	HITAGS_UID_Request();	//����UIDָ��
	SET_CONFIG_PAGE(CF_PAGE_2+THRESET+FREEZE1);
	delay_us(640);
	SET_CONFIG_PAGE(CF_PAGE_2+THRESET+FREEZE1+FREEZE0);	  
	delay_us(480);
	SET_CONFIG_PAGE(CF_PAGE_2);
	READ_TAG();
}


//----HTRC110 select HITAG����------------------------------------------------
void SelectUID_Ready(u8 *id)  
{
	
	u8 *pid;
	pid = id;  
	               
    WRITE_TAG_N(6); //fan 
	delay_us(200);                                         

	HitagSSelectUid(pid, send_cmd);	//����UID��ָ��
	field_switch(send_cmd, 45);	//����select_id����
	delay_us(480); //200
	SET_CONFIG_PAGE(CF_PAGE_2+THRESET+FREEZE1);
	delay_us(640); //200
	SET_CONFIG_PAGE(CF_PAGE_2+THRESET+FREEZE1+FREEZE0);
	delay_us(480); //200
	SET_CONFIG_PAGE(CF_PAGE_2);
	READ_TAG();
}
//----HTRC110ΪEM��׼���ö��ź�------------------------------------------------
void HTRC110_EmCardReady()  
{
	WRITE_TAG();                                             
	EM_Reast();			//HITAGS�����縴λ
	READ_TAG();
}



void HitagSSelectUid(u8 *uid, u8 *cSendData)//u8 * pcUid, u8 * pcPage1
{
  u8 cTemp[1];
                                // temporary buffer command & CRC byte
  BitBufferInit( cSendData );
  cTemp[0] = HITAGS_SELECT_UID << 3;
  BitBufferStoreBlock( cTemp, 5 );     // store the five command bits

  BitBufferStoreBlock( uid, 32 );      // append UID

  cTemp[0] = crc8( cSendData, 37, CRC_PRESET );               // calculate CRC byte
  BitBufferStoreBlock( cTemp, 8 );            // append CRC byte
}





//----��ʼ��HTRC110ͨѶ�ӿ�----------------------------------------------------
void HTRC110_GBIO_Init(void)  
{  
	RCC->APB2ENR|=1<<2;    		//ʹ��PORTAʱ��	   	 
	RCC->APB2ENR|=1<<3;    		//ʹ��PORTBʱ��	 
	RCC->APB2ENR|=1<<4;    		//ʹ��PORTCʱ��	  
	//uC SCLK��� 	 
	GPIOA->CRH&=0XFFFFFFF0;		//PA8���������
	GPIOA->CRH|=0X00000007;		//3OP��� 	  
	//uC SDIN��� 	 
	GPIOC->CRH&=0XFFFFFF0F;		//PC9���������
	GPIOC->CRH|=0X00000070;		//3OP��� 	  
	//uC SDOUT����
	GPIOC->CRH&=0XFFFFFFF0;		//PC9����������
	GPIOC->CRH|=0X00000008;		//8�������������� 	  
	GPIOC->ODR|=(0x0001<<8);			//����
	//VCC 5V��Դ��� 	 
	GPIOB->CRH&=0X0FFFFFFF;		//PB15���������

	GPIOB->CRH|=getValuev5Config();		//7OP���  getValuev5Config()
}
//----����4.000M/4.288Mʱ�����----------------------------------------------------------
void PWM_Init(u8 CardStyle)  
{  

	RCC->APB2ENR|=1<<2;    		//ʹ��PORTAʱ��	   	 
	RCC->APB2ENR|=1<<11;       	//ʹ��TIM1ʱ��    
	GPIOA->CRH&=0XFFFFFF0F;		//PA9������PWM���
	GPIOA->CRH|=0X000000B0;		//B����������� 	  

	//TIM1-PWMģʽ
  	TIM1->CR1 &= 0x00FF;		//�ָ�ʱ��
  	TIM1->CR1 &= 0x038F;		//���ض��뷽ʽ

	TIM1->PSC=0;				//ʱ��Ԥ��Ƶ
    TIM1->RCR = 0;				//Ԥװ�ظ�������
	TIM1->EGR = 1;				//���³�ʼ��������          
	TIM1->CCMR1 &= 0xFCFF;  	//CH2����Ϊ���	
		 
	TIM1->CCMR1 &= 0x8FFF;	 	//CH2 PWM1ģʽ
	TIM1->CCMR1 |= 6<<12;  		//CH2 PWM1ģʽ

	TIM1->CCER |= 1<<5;			//CH2��������� 
	TIM1->CCMR1 |= 1<<11; 		//ʹ��:OC2Ԥװ��PWM������	

	//4.000M
	if(CardStyle==1)
	{
		TIM1->ARR = 17;			//����TIMx�������Զ���װ��ֵ����ʱ�����ڣ�	4M
		TIM1->CCR2 = 17/2;
	}
	else
	{
		TIM1->ARR = 16;			//����TIMx�������Զ���װ��ֵ����ʱ�����ڣ�	4.288M
		TIM1->CCR2 = 16/2;
	}


	//������仰����TIM1��TIM8�Ǳ���ģ�����TIM2-TIM6�򲻱�Ҫ
	TIM1->CCER |= 1<<4; 		//ʹ��OC2�����������	   
	TIM1->CR1 |= 0x01;    		//����������  
	TIM1->BDTR |= 1<<15;		//����OC���
}
//----����Ƶ�ź�Դ---------------------------------------------------------
void PWM_Close(void)  
{  
	TIM1->CR1 &= ~1;    		//ֹͣ������ 										  
	TIM1->BDTR &= ~(1<<15);		//�ر�OC���
}

//----��ʼ��HTRC110------------------------------------------------------------
bool HTRC110_Init(u8 CardStyle)  
{
	u8 ch;
	  
	HTRC110_GBIO_Init();	//��ʼ��HTRC110ͨѶ�ӿ�
	
	PowerHTRC110 = getPowerHTRC110_Level();			//HTRC110��Դ��getPowerHTRC110_Level()

	PWM_Init(CardStyle);	//����4.000M/4.288Mʱ�����
	delay_ms(10);

   	SET_CONFIG_PAGE(CF_PAGE_3);
	delay_ms(1);
	ch=GET_CONFIG_PAGE(3);
	if((ch & 0x0F) != 0)	//���HTRC110оƬ
		return FALSE;
	delay_ms(10);

	//��ʼ��  EM��ʽ
	if(CardStyle==1)
	{
		SET_CONFIG_PAGE(CF_PAGE_0 + 0x0c + 3);
		delay_ms(1);
		SET_SAMPLING_TIME(0);
   		SET_CONFIG_PAGE(CF_PAGE_2);	
	 }
	//��ʼ��  HITAG��ʽ
	else
	if(CardStyle==2)
	{
		SET_CONFIG_PAGE(CF_PAGE_0 + 0x0c + 3);
		delay_ms(1);
		Ast_Adjust(0x3F);
   		SET_CONFIG_PAGE(CF_PAGE_2 + THRESET + FREEZE0);	
		WRITE_TAG_N(6);
	}
	delay_ms(10);
	READ_TAG();

	return TRUE;
}
//----��HTRC110��Դ-----------------------------------------------------------
void HTRC110_Close()
{
	PWM_Close();  
	PowerHTRC110=getPowerHTRC110_Level();			//HTRC110��Դ��

}



