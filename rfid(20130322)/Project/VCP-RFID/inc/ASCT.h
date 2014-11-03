
#ifndef __ASCI__
#define __ASCI__


u32 TRAN_G( INT8U *s, INT8U *p, u32 flag );
void TO_BCD_ASC( INT32U n, INT8U *p );
void TO_HEX_ASC( INT32U n, INT8U *p );
u32 remove_pre0( INT8U *ps, INT8U *pd, u32 flag );
void add_dot( INT8U *ps, u32 flag );
void TO_DEC_ASC( INT32U n, INT8U *p );
INT16U STRLEN( INT8U *s );

#endif


