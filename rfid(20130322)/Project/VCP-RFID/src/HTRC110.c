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

//----延时---------------------------------------------------------------------
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
//----启动信号---------------------------------------------------------------
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
//----模拟SPI写读8位数据-------------------------------------------------------
char S_PutcGetc(char data, vu8 nBit)
{
	char ch=0;

	while(nBit)
	{
		//输出一位数据
		if(data & 0x80)
			SDIN=1;
		else	
			SDIN=0;
		data <<= 1;
		s_delay_us(10);

		//输出时钟信号高电平↑
		SCLK=1;	
		s_delay_us(20);	//

		//输入一位数据
		ch <<= 1;
		ch |= SDOUT;

		//输出时钟信号低电平↓
		SCLK=0;	
		s_delay_us(10);

		nBit--;
	}
	return ch;
}


//---------------------------------------------------------------------------
void Send_HTRC110_Cmd(u8 RcCmd)
{
	S_Start();		//启动信号
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

//----HTRC110为HITAG准备好读信号------------------------------------------------
void HTRC110_TitagReady()  
{
	WRITE_TAG_N(0); //fan
	HITAGS_Reast();			//HITAGS卡掉电复位
	delay_us(200);
		                                             	
	HITAGS_UID_Request();	//请求UID指令
	SET_CONFIG_PAGE(CF_PAGE_2+THRESET+FREEZE1);
	delay_us(640);
	SET_CONFIG_PAGE(CF_PAGE_2+THRESET+FREEZE1+FREEZE0);	  
	delay_us(480);
	SET_CONFIG_PAGE(CF_PAGE_2);
	READ_TAG();
}


//----HTRC110 select HITAG命令------------------------------------------------
void SelectUID_Ready(u8 *id)  
{
	
	u8 *pid;
	pid = id;  
	               
    WRITE_TAG_N(6); //fan 
	delay_us(200);                                         

	HitagSSelectUid(pid, send_cmd);	//根据UID求指令
	field_switch(send_cmd, 45);	//发送select_id命令
	delay_us(480); //200
	SET_CONFIG_PAGE(CF_PAGE_2+THRESET+FREEZE1);
	delay_us(640); //200
	SET_CONFIG_PAGE(CF_PAGE_2+THRESET+FREEZE1+FREEZE0);
	delay_us(480); //200
	SET_CONFIG_PAGE(CF_PAGE_2);
	READ_TAG();
}
//----HTRC110为EM卡准备好读信号------------------------------------------------
void HTRC110_EmCardReady()  
{
	WRITE_TAG();                                             
	EM_Reast();			//HITAGS卡掉电复位
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





//----初始化HTRC110通讯接口----------------------------------------------------
void HTRC110_GBIO_Init(void)  
{  
	RCC->APB2ENR|=1<<2;    		//使能PORTA时钟	   	 
	RCC->APB2ENR|=1<<3;    		//使能PORTB时钟	 
	RCC->APB2ENR|=1<<4;    		//使能PORTC时钟	  
	//uC SCLK输出 	 
	GPIOA->CRH&=0XFFFFFFF0;		//PA8，用于输出
	GPIOA->CRH|=0X00000007;		//3OP输出 	  
	//uC SDIN输出 	 
	GPIOC->CRH&=0XFFFFFF0F;		//PC9，用于输出
	GPIOC->CRH|=0X00000070;		//3OP输出 	  
	//uC SDOUT输入
	GPIOC->CRH&=0XFFFFFFF0;		//PC9，用于输入
	GPIOC->CRH|=0X00000008;		//8输入上拉或下拉 	  
	GPIOC->ODR|=(0x0001<<8);			//上拉
	//VCC 5V电源输出 	 
	GPIOB->CRH&=0X0FFFFFFF;		//PB15，用于输出

	GPIOB->CRH|=getValuev5Config();		//7OP输出  getValuev5Config()
}
//----设置4.000M/4.288M时钟输出----------------------------------------------------------
void PWM_Init(u8 CardStyle)  
{  

	RCC->APB2ENR|=1<<2;    		//使能PORTA时钟	   	 
	RCC->APB2ENR|=1<<11;       	//使能TIM1时钟    
	GPIOA->CRH&=0XFFFFFF0F;		//PA9，用于PWM输出
	GPIOA->CRH|=0X000000B0;		//B复用推挽输出 	  

	//TIM1-PWM模式
  	TIM1->CR1 &= 0x00FF;		//分割时间
  	TIM1->CR1 &= 0x038F;		//边沿对齐方式

	TIM1->PSC=0;				//时钟预分频
    TIM1->RCR = 0;				//预装载更新速率
	TIM1->EGR = 1;				//重新初始化计数器          
	TIM1->CCMR1 &= 0xFCFF;  	//CH2配置为输出	
		 
	TIM1->CCMR1 &= 0x8FFF;	 	//CH2 PWM1模式
	TIM1->CCMR1 |= 6<<12;  		//CH2 PWM1模式

	TIM1->CCER |= 1<<5;			//CH2输出反向极性 
	TIM1->CCMR1 |= 1<<11; 		//使能:OC2预装载PWM脉冲宽度	

	//4.000M
	if(CardStyle==1)
	{
		TIM1->ARR = 17;			//设置TIMx计数器自动重装载值（定时器周期）	4M
		TIM1->CCR2 = 17/2;
	}
	else
	{
		TIM1->ARR = 16;			//设置TIMx计数器自动重装载值（定时器周期）	4.288M
		TIM1->CCR2 = 16/2;
	}


	//下面这句话对于TIM1和TIM8是必须的，对于TIM2-TIM6则不必要
	TIM1->CCER |= 1<<4; 		//使能OC2在引脚上输出	   
	TIM1->CR1 |= 0x01;    		//开启计数器  
	TIM1->BDTR |= 1<<15;		//开启OC输出
}
//----关射频信号源---------------------------------------------------------
void PWM_Close(void)  
{  
	TIM1->CR1 &= ~1;    		//停止计数器 										  
	TIM1->BDTR &= ~(1<<15);		//关闭OC输出
}

//----初始化HTRC110------------------------------------------------------------
bool HTRC110_Init(u8 CardStyle)  
{
	u8 ch;
	  
	HTRC110_GBIO_Init();	//初始化HTRC110通讯接口
	
	PowerHTRC110 = getPowerHTRC110_Level();			//HTRC110电源开getPowerHTRC110_Level()

	PWM_Init(CardStyle);	//设置4.000M/4.288M时钟输出
	delay_ms(10);

   	SET_CONFIG_PAGE(CF_PAGE_3);
	delay_ms(1);
	ch=GET_CONFIG_PAGE(3);
	if((ch & 0x0F) != 0)	//检测HTRC110芯片
		return FALSE;
	delay_ms(10);

	//初始化  EM方式
	if(CardStyle==1)
	{
		SET_CONFIG_PAGE(CF_PAGE_0 + 0x0c + 3);
		delay_ms(1);
		SET_SAMPLING_TIME(0);
   		SET_CONFIG_PAGE(CF_PAGE_2);	
	 }
	//初始化  HITAG方式
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
//----关HTRC110电源-----------------------------------------------------------
void HTRC110_Close()
{
	PWM_Close();  
	PowerHTRC110=getPowerHTRC110_Level();			//HTRC110电源开

}



