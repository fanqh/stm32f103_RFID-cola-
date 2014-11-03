#include "stm32f10x.h"


//JTAGģʽ���ö���
#define JTAG_SWD_DISABLE   0X02
#define SWD_ENABLE         0X01
#define JTAG_SWD_ENABLE    0X00	
//----------------------------
void JTAG_Set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     //��������ʱ��	   
	AFIO->MAPR&=0XF8FFFFFF; //���MAPR��[26:24]
	AFIO->MAPR|=temp;       //����jtagģʽ
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

  	RCC_PCLK1Config(RCC_HCLK_Div1);			//���õ���AHBʱ��
  	RCC_PCLK2Config(RCC_HCLK_Div1);			//���ø���AHBʱ��
	JTAG_Set(SWD_ENABLE);		//��
  
}


//ϵͳ��λ
//CHECK OK
//091209
void Sys_Soft_Reset(void)
{   
	SCB->AIRCR =0X05FA0000|(u32)0x04;	  
} 

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
