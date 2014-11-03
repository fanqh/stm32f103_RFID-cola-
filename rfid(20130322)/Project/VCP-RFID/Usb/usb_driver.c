
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "stm32f10x_it.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
//#include "hw_config.h"
//#include "platform_config.h"
#include "usb_pwr.h"

#define USB_DISCONNECT            GPIOA  						
#define USB_DISCONNECT_PIN        GPIO_Pin_12					
#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOA


extern uint8_t ubs_rx_buff[272];
extern __IO uint32_t usb_rx_cnt;
extern __IO uint32_t bDeviceState;
/*******************************************************************************
* Function Name  : Set_USBClock
* Description    : Configures USB Clock input (48MHz)
* Input          : None.
* Return         : None.
*******************************************************************************/
void USB_SetClock(void)
{
  /* USBCLK = PLLCLK / 1.5 */
  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
  /* Enable USB clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}


/*******************************************************************************
* Function Name  : USB_Interrupts_Config
* Description    : Configures the USB interrupts
* Input          : None.
* Return         : None.
*******************************************************************************/
void USB_Interrupts_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : USB_Cable_Config
* Description    : Software Connection/Disconnection of USB Cable
* Input          : None.
* Return         : Status
*******************************************************************************/
void usb_cable(FunctionalState NewState)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	if (NewState == DISABLE)	//改
  	{
    	GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
  		/* Enable USB_DISCONNECT GPIO clock */
  		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);
  		/* Configure USB pull-up pin */
  		GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
  		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//改
  		GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);
  	}
  	else
  	{
    	GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
  	}
}

/*******************************************************************************
* Function Name  : Enter_LowPowerMode
* Description    : Power-off system clocks and power while entering suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void)
{
  /* Set the device state to suspend */
  bDeviceState = SUSPENDED;
}

/*******************************************************************************
* Function Name  : Leave_LowPowerMode
* Description    : Restores system clocks and power while exiting suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void)
{
  DEVICE_INFO *pInfo = &Device_Info;

  /* Set the device state to the correct state */
  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
  else
  {
    bDeviceState = ATTACHED;
  }
}

/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_SerialNum(void)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

  Device_Serial0 = *(__IO uint32_t*)(0x1FFFF7E8);
  Device_Serial1 = *(__IO uint32_t*)(0x1FFFF7EC);
  Device_Serial2 = *(__IO uint32_t*)(0x1FFFF7F0);

  if (Device_Serial0 != 0)
  {
    Virtual_Com_Port_StringSerial[2] = (uint8_t)(Device_Serial0 & 0x000000FF);
    Virtual_Com_Port_StringSerial[4] = (uint8_t)((Device_Serial0 & 0x0000FF00) >> 8);
    Virtual_Com_Port_StringSerial[6] = (uint8_t)((Device_Serial0 & 0x00FF0000) >> 16);
    Virtual_Com_Port_StringSerial[8] = (uint8_t)((Device_Serial0 & 0xFF000000) >> 24);

    Virtual_Com_Port_StringSerial[10] = (uint8_t)(Device_Serial1 & 0x000000FF);
    Virtual_Com_Port_StringSerial[12] = (uint8_t)((Device_Serial1 & 0x0000FF00) >> 8);
    Virtual_Com_Port_StringSerial[14] = (uint8_t)((Device_Serial1 & 0x00FF0000) >> 16);
    Virtual_Com_Port_StringSerial[16] = (uint8_t)((Device_Serial1 & 0xFF000000) >> 24);

    Virtual_Com_Port_StringSerial[18] = (uint8_t)(Device_Serial2 & 0x000000FF);
    Virtual_Com_Port_StringSerial[20] = (uint8_t)((Device_Serial2 & 0x0000FF00) >> 8);
    Virtual_Com_Port_StringSerial[22] = (uint8_t)((Device_Serial2 & 0x00FF0000) >> 16);
    Virtual_Com_Port_StringSerial[24] = (uint8_t)((Device_Serial2 & 0xFF000000) >> 24);
  }
}



void usb_init()
{
	USB_SetClock();
  	USB_Interrupts_Config();
  	USB_Init();
}



//----检测USB接收数据----------------------------------------------------------
u16 usb_ndata()
{
    if ((usb_rx_cnt != 0) && (bDeviceState == CONFIGURED))
		return 	usb_rx_cnt;
	return 0;
}
//----USB接收------------------------------------------------------------------
void usb_read(u8 *buff, vu16 lenth)
{

    if ((usb_rx_cnt != 0) && (bDeviceState == CONFIGURED))
    {
//		cnt=usb_rx_cnt;
		memcpy(buff, ubs_rx_buff, lenth);
		usb_rx_cnt = 0;
	}
}


////王师傅修改///////////////////////////////////////////////////////

void usb_tx_delay()
{
 u16 i;
 for(i=0; i<7000; i++);
}

//////////////////////////////////////////////////////////

#define OUT_SIZE   (VIRTUAL_COM_PORT_DATA_SIZE-2)
//----USB发送------------------------------------------------------------------
bool usb_write(u8 *buff, u16 lenth)
{
	 u16 len;
	 u16 sta;
	 u16 t;
	 while(1)
	 {
	  //等待上个数据包发送完成
	  t=0;
	  while(1)
	  {
	   	sta=_GetEPTxStatus(ENDP1);
	   	if( (sta & EP_TX_STALL) == 0 )
	    	break;
	   	usb_tx_delay();
	   	t++;
	   	if(t>2000)
	    return FALSE;
	  }
	  //限制数据包长度
	  if(lenth>OUT_SIZE)
	   len=OUT_SIZE;
	  else
	   len=lenth;
	  lenth-=len;
	  //发送数据包
	    UserToPMABufferCopy(buff, ENDP1_TXADDR, len);
	    SetEPTxCount(ENDP1, len);
	    SetEPTxValid(ENDP1);
	
	  if(lenth==0)
	   break;
	 }
	 return TRUE;
}

//----清除USB接收区--------------------------------------------------------------
void usb_clear()
{
	usb_rx_cnt = 0;
}


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
