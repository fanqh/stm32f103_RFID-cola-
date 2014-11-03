#ifndef __LOG_
#define __LOG_

#include "stm32f10x.h"

#define BIT0             0x01UL
#define BIT1             0x02
#define BIT2             0x04
#define BIT3             0x08
#define BIT4             0x10
#define BIT5             0x20
#define BIT6             0x40
#define BIT7             0x80

#define BITSET( p, n )   ( p |=  (BIT0<<n) )
#define BITCLR( p, n )   ( p &= ~(BIT0<<n) )
#define BITTST( p, n )   ( p &   (BIT0<<n) )
#define BITCPL( p, n )   ( p ^=  (BIT0<<n) )


typedef u32  INT32U ;
typedef u16  INT16U ;
typedef u8  INT8U ;

typedef struct 
{
	INT8U head;
	INT8U cmd;
	INT8U data[3];
}TESTTYPE;

void MyDebugPrint( char *fm, ...);
void TestModel(void);
void OledTest(void);

#endif

