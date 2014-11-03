#ifndef __USB_DRIVER_H
#define __USB_DRIVER_H

/* Includes ------------------------------------------------------------------*/
#include "usb_type.h"
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void  Get_SerialNum(void);

void usb_init(void);
//----检测USB接收数据----------------------------------------------------------
u16 usb_ndata(void);
//----USB接收------------------------------------------------------------------
void usb_read(u8 *buff, vu16 lenth);
//----USB发送------------------------------------------------------------------
bool usb_write(u8 *buff, u16 lenth);
//----清除USB接收区--------------------------------------------------------------
void usb_clear(void);

void usb_cable(FunctionalState NewState);
//void USB_SetClock(void);
//void USB_Interrupts_Config(void);
//void Get_SerialNum(void);




#endif  

