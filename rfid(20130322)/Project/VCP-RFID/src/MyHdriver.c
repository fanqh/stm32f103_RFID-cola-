#include "stm32f10x.h"


//JTAG模式设置定义
#define JTAG_SWD_DISABLE   0X02
#define SWD_ENABLE         0X01
#define JTAG_SWD_ENABLE    0X00	
//----------------------------
void JTAG_Set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     //开启辅助时钟	   
	AFIO->MAPR&=0XF8FFFFFF; //清除MAPR的[26:24]
	AFIO->MAPR|=temp;       //设置jtag模式
} 

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Set_System
* Description    : Configures Main system clocks & power
* Input          : None.
* Return         : None.
*******************************************************************************/
void Hdriver_init(void)
{

  /* Setup the microcontroller system. Initialize the Embedded Flash Interface,  
     initialize the PLL and update the SystemFrequency variable. */
	SystemInit();

  	RCC_PCLK1Config(RCC_HCLK_Div1);			//设置低速AHB时钟
  	RCC_PCLK2Config(RCC_HCLK_Div1);			//设置高速AHB时钟
	JTAG_Set(SWD_ENABLE);		//加
  
}


//系统软复位
//CHECK OK
//091209
void Sys_Soft_Reset(void)
{   
	SCB->AIRCR =0X05FA0000|(u32)0x04;	  
} 

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
