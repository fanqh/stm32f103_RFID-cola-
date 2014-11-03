#ifndef __MyUsart_H
#define __MyUsart_H

#include "stm32f10x.h"

////////////////////////////////////////////////////////////////////////////////////
//串行通讯函数
////////////////////////////////////////////////////////////////////////////////////
#define COM1 0x00
#define COM2 0x01
#define COM3 0x02

//RS232通讯
void com_init(u8 com, u32 baud, u8 width, u8 stop, u8 par);
u16 com_package(u8 com);					//检测UART1接收数据包
u16 com_ndata(u8 com);
u8 com_getc(u8 com);
bool com_read(u8 com, u8 *buff, vu16 lenth);
bool com_read2(u8 com, u8 *buff, vu16 lenth);
void com_clear(u8 com);
void com_putc(u8 com, u8 ch);
void com_write(u8 com, u8* buff, vu16 lenth);
void com_autoclear(u8 com, bool f);				//发送完毕自动清除（半双工485方式）


//////////////////////////////////////////////////////////////////////////////////	 
#endif
