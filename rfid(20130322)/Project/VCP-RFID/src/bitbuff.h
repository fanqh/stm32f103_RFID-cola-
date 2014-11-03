#ifndef __BITBUFF_H_INCLUDED
#define __BITBUFF_H_INCLUDED

#include "stm32f10x.h"

#define HITAGS_UID_REQ_STD    (0x06)
//#define HITAGS_UID_REQ_ADV    (0x18)  // only valid for HITAG S
#define HITAGS_UID_REQ_ADV    (0x19)
#define HITAGS_UID_REQ_FADV   (0x1A)    // only valid for HITAG S
#define HITAGS_SELECT_UID     (0x00)


#define CRC_POLYNOM 0x1D
#define CRC_PRESET 0xFF

u8 crc8( u8 * pcIn, u8 cBitCount, u8 cCRC );
void BitBufferInit( u8 * pcAddr );

void BitBufferStoreBlock( u8 * pcAddr, u8 cBitCount );

#endif 

