#ifndef __W25X80_H_
#define __W25X80_H_
////////////////////////////////////////////////////////////////////////////

#include "stm32f10x.h"


////////////////////////////////////////////////////////////////////////////
//FROM存储器内部控制
////////////////////////////////////////////////////////////////////////////
bool FROM_init(void);										//初始化W25X80
u8 _FROMgetc(u32 addr);								//从_FROM中读取一个字节
void _FROMread(u32 addr, u8  *buff, vu16 lenth);		//从_FROM中读取一串数据
void _FROMputc(u32 addr, u8  ch);						//在_FROM中随机保存一个字节
void _FROMwrite(u32 addr, u8  *buff, vu16 lenth);		//在_FROM中随机保存一串数据
void _FROMerase4k(u32 addr);								//擦除_FROM中的4096字节空间
void _FROMerase64k(u32 addr);								//擦除_FROM中的65536字节空间
void _FROMeraseAll(void);									//擦除全部_FROM空间
//在_FROM中快速保存数据,如果擦除标志有效，将自动擦除后面的4K块
void FROMwriteQ(u32 addr, u8 * buff, vu16 lenth, u8  EraseFlag);



//------------------------------------------------------------------------------
#endif
