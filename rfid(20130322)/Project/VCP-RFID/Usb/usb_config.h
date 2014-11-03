#ifndef __USB_CONFIG_H
#define __USB_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "usb_type.h"

void Set_USBClock(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config (FunctionalState NewState);

bool usb_write(u8 *buff, vu16 lenth);	//USB·¢ËÍ



#endif  /*__USB_CONFIG_H*/

