#ifndef __INTERNALFLASH_H
#define __INTERNALFLASH_H

u8  WriteFlashV16(u32 addr, u16 date);
u8  WriteFlashV32(u32 addr, u32 date);

void HalFlashWriteHalf(u32 addr, u16 hword);
#endif

