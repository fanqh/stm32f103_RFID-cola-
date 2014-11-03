#include "bitbuff.h"

u8  * pcBitBufferAddr; /* Pointer to currently addressed byte   */
u8 cBitBufferMask;          /* Bit mask for currently addressed byte */




u8 crc8( u8 * pcIn, u8 cBitCount, u8 cCRC )
/******************************************************************************
*                                                                             *
* Description:                                                                *
*   Calculates the CRC of a of byte array. Starting with the MSB.             *
*   The number of bits which are used for the calculation is defined with     *
*   cBitcount.                                                                *
*   Due to the run time optimization for 8 bit controller and to the fact     *
*   that the algorithmus is byte oriented, all remainder bits have to be set  *
*   to zero, as the crc calculation is done byte wise.                        *
*                                                                             *
* Global variables:                                                           *
*                                                                             *
* Parameters:                                                                 *
*   *cCrc:      pointer to the byte which contains the actual crc value       *
*   *pcIn:      pointer to the input array of data bytes for calculation      *
*   cBitCount:  defines the number of relevant bits for CRC calculation       *
*                                                                             *
* Result: none                                                                *
*                                                                             *
******************************************************************************/
{
  u8 cCurrentBit;
  u8 cNoBits;
  u8 cCurrentByte;
  u8 cLastElement;
  
  const u8 _MASK_TABLE[8] = {0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
  
  cNoBits = 8;
  cLastElement = (cBitCount+7)/8-1;                                       // calculate index value to last element
  cBitCount %=8;
  pcIn[cLastElement] = pcIn[cLastElement] & _MASK_TABLE[cBitCount];   // set all remainder bits of last element to zero

  for( cCurrentByte = 0; cCurrentByte < (u8)(cLastElement+1); cCurrentByte++ )
  {
    cCRC ^= pcIn[cCurrentByte];
    
    if(cCurrentByte == cLastElement )
    {
      if( cBitCount != 0 )
        cNoBits = cBitCount;
    }

    for( cCurrentBit = 0; cCurrentBit < cNoBits; cCurrentBit++ )
    {
      if( cCRC & 0x80 )
      {
        cCRC <<=1;
        cCRC ^= CRC_POLYNOM;
      }
      else
      {
        cCRC <<=1;
      }
    }
  }
  return cCRC;
}

void BitBufferInit( u8 * pcAddr )
/******************************************************************************
*                                                                             *
* Description:                                                                *
*   Initialization of bit operations (bit_store, bit_xor, bit_get).           *
*                                                                             *
* Parameters:                                                                 *
*   pcAddr = base address of bit buffer.                                      *
*                                                                             *
* Result: none                                                                *
*                                                                             *
******************************************************************************/
{
  pcBitBufferAddr = pcAddr;
  cBitBufferMask = 0x80;    /* Initialize bit mask for first bit */
}


void BitBufferStoreBlock( u8 * pcAddr, u8 cBitCount )
/******************************************************************************
*                                                                             *
* Description:                                                                *
*   Stores a (MSB aligned) data block, beginning at current bit location      *
*   (pcBitBufferAddr, cBitBufferMask), and advances to first bit location     *
*   after the block end. Data is stored MSB-first, starting at the address    *
*   that was given by BitBufferInit().                                        *
*                                                                             *
* NOTE: for performance reasons, in the case of a partial last byte of a      *
*   block (cBitCount not multiple of 8), the function will still modify       *
*   the locations of the bit buffer that would receive these bits. In such    *
*   a case, it may be necessary to extend the size of the bit buffer by one   *
*   "dummy" byte.                                                             *
*                                                                             *
* Parameters:                                                                 *
*   pcAddr    = address of first byte of data block to store.                 *
*   cBitCount = number of bits to copy.                                       *
*                                                                             *
* Result: none                                                                *
*                                                                             *
******************************************************************************/
{
  u8 cByteCount;
  const u8 _MASK_TABLE[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};


  cByteCount = (cBitCount+7) / 8; /* calculate number of bytes */

  /* Calculate number of bits in possibly partial last byte. We will use this  */
  /* information to compute a new bit-pointer mask value after copying.        */
  cBitCount %= 8;
  if( cBitCount == 0 )
  {
    cBitCount = 8;    /* we need to copy an integer multiple of bytes */
  }

  /* Copy bytes, shifted accordingly to match current bit-buffer bit position. */
  /* Since the 8051 (as most 8-bit controllers) does not efficiently support   */
  /* arbitrary bit shifts, we copy the algorithm for each possible shift count.*/
  switch( cBitBufferMask )
  {
    case 0x80:
      do
      {
        *pcBitBufferAddr = *pcAddr;
        pcBitBufferAddr++;
        pcAddr++;
      }
      while( --cByteCount );
      break;

    case 0x40:
      do
      {
        *pcBitBufferAddr = ( *pcBitBufferAddr & 0x80 ) | ( *pcAddr >> 1 );
        pcBitBufferAddr++;
        *pcBitBufferAddr = *pcAddr << 7;
        pcAddr++;
      }
      while( --cByteCount );
      cBitCount += 1;
      break;

    case 0x20:
      do
      {
        *pcBitBufferAddr = ( *pcBitBufferAddr & 0xC0 ) | ( *pcAddr >> 2 );
        pcBitBufferAddr++;
        *pcBitBufferAddr = *pcAddr << 6;
        pcAddr++;
      }
      while( --cByteCount );
      cBitCount += 2;
      break;

    case 0x10:
      do
      {
        *pcBitBufferAddr = ( *pcBitBufferAddr & 0xE0 ) | ( *pcAddr >> 3 );
        pcBitBufferAddr++;
        *pcBitBufferAddr = *pcAddr << 5;
        pcAddr++;
      }
      while( --cByteCount );
      cBitCount += 3;
      break;

    case 0x08:
      do
      {
        *pcBitBufferAddr = ( *pcBitBufferAddr & 0xF0 ) | ( *pcAddr >> 4 );
        pcBitBufferAddr++;
        *pcBitBufferAddr = *pcAddr << 4;
        pcAddr++;
      }
      while( --cByteCount );
      cBitCount += 4;
      break;

    case 0x04:
      do
      {
        *pcBitBufferAddr = ( *pcBitBufferAddr & 0xF8 ) | ( *pcAddr >> 5 );
        pcBitBufferAddr++;
        *pcBitBufferAddr = *pcAddr << 3;
        pcAddr++;
      }
      while( --cByteCount );
      cBitCount += 5;
      break;

    case 0x02:
      do
      {
        *pcBitBufferAddr = ( *pcBitBufferAddr & 0xFC ) | ( *pcAddr >> 6 );
        pcBitBufferAddr++;
        *pcBitBufferAddr = *pcAddr << 2;
        pcAddr++;
      }
      while( --cByteCount );
      cBitCount += 6;
      break;

    case 0x01:
      do
      {
        *pcBitBufferAddr = ( *pcBitBufferAddr & 0xFE ) | ( *pcAddr >> 7 );
        pcBitBufferAddr++;
        *pcBitBufferAddr = *pcAddr << 1;
        pcAddr++;
      }
      while( --cByteCount );
      cBitCount += 7;
      break;
  }

  /* Calculate mask value of new bit position within bit-buffer. */
  cBitBufferMask = _MASK_TABLE[ cBitCount % 8 ];

  /* If the last copied byte was incomplete, then the bit-buffer byte pointer */
  /* has advanced beyond the end of data. Correct it in this case.            */
  if( cBitCount < 8 )
  {
    pcBitBufferAddr--;
  }

}
