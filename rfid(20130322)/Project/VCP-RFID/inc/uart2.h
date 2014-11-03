#ifndef __UART2_H
#define __UART2_H

#include "stm32f10x.h"
#include "stdio.h"	 
#include "string.h"	 


void USART2_Init(u32 pclk2,u32 bound);
//----COM�������ݰ�----------------------------------------------------
//----USART2�������ݰ�----------------------------------------------------
u16 USART2_Package(void);
//----USART2�������ݰ�----------------------------------------------------
void USART2_Read(u8 *buff, vu16 cnt);
//----USART2����һ���ֽ�--------------------------------------------------
void USART2_Putc(u8 ch);
//----USART2�������ݰ�----------------------------------------------------
void USART2_Write(u8 *buff, vu16 cnt);
//----USART2���----------------------------------------------------------
void USART2_Clear(void);

#endif	   
















