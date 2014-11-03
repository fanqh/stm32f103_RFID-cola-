//ASCII转换程序
/*
修改记录

*/
#include"log.h"


static INT8U const HEX[]="0123456789ABCDEF";
static INT8U const BCD[]="0123456789------";

INT8U * STRCPY( INT8U *s, INT8U *d )
{
   while( *s )
   {
      *d++ = *s++;
   }
   
   *d = 0;
   
   return d;
}
void TO_BCD_ASC( INT32U n, INT8U *p )
{
   
   p[1] = BCD[n&0x0f];
   n >>= 4;
   p[0] = BCD[n&0x0f];

}
void TO_HEX_ASC( INT32U n, INT8U *p )
{
   INT32U i;
   
   p[10] = 0;
   
   p += 9;
   
   for( i=0; i<8; i++ )
   {
      *p-- = HEX[n&0x0f];
      n >>= 4;
   }
   
   *p-- = '0';
   *p-- = '0';
}

void BTOBCDS( INT32U n , INT8U *p )
{
   n = n/10*16 + n%10 ;
   p[1] = HEX[n&0x0f];
   n >>= 4;
   p[0] = HEX[n];
   
}
void TO_DEC_ASC( INT32U n, INT8U *p )
{

   INT32U b;
   INT32U c,d,e;
   
   b = n % 100000000;
   n = n / 100000000;
   
   c = b / 10000;
   d = b % 10000;
   
   b = c / 100;
   c = c % 100;
   
   e = d % 100;
   d = d / 100;
   
   BTOBCDS( n, p );
   p += 2;
   
   BTOBCDS( b, p );
   p += 2;
   
   BTOBCDS( c, p );
   p += 2;
   
   BTOBCDS( d, p );
   p += 2;
   
   BTOBCDS( e, p );
   p += 2;
   
   *p = 0;

}

INT32U GET_C( INT8U *p )
{
   INT32U c;
   c = *p;
   if( (c<'0') || (c>'9') ) return 0xff;
   
   c -= '0';
   
   return c;

}

//读十进制字符串
INT32U DS_TO_D( INT8U *p )
{
   INT32U c,d;
   
   d = 0;
   
   while( (c=GET_C(p))<=9 )
   {
      d = d*10+c;
      p++;
   }
   
   return d;
}
//读十六进制字符串
INT32U HS_TO_D( INT8U *p )
{
   INT32U c,d;
   
   d = 0;
   
   while( (c=GET_C(p))<=9 )
   {
      d = (d<<4)+c;
      p++;
   }
   
   return d;
}     
void T_DATE( INT8U *s, INT8U *p )
{

   static INT8U DATE1[]="20  \x0b  \x0c  \x14";
	
   STRCPY( DATE1, p );
   p += 2;
   
   BTOBCDS( s[6], p );      //year
   p += 3;
   
   BTOBCDS( s[5], p );      //month
   p += 3;
   
   BTOBCDS( s[4], p );      //date
   p += 3;
   
   *p = 0;  
}
void T_TIME( INT8U *s, INT8U *p )
{
	
   static INT8U  TIME1[]="00:00:00\x0e\x0f ";
   
   static INT8U const WEEKMAP[]={ 0x14,'1','2','3','4','5','6',
                                  0x14,'-','-','-','-','-','-',
                                  '-','-' };
   STRCPY( TIME1, p );
   
   BTOBCDS( s[2], p );      //hour
   p += 3;

   BTOBCDS( s[1], p );      //minute
   p += 3;

   BTOBCDS( s[0], p );      //second
   p += 4;

   *p++ = WEEKMAP[s[3]&0x0f]; //week
   
   *p = 0;  
}
//00-00:00
void T_TIME1( INT8U *s, INT8U *p )
{

   TO_BCD_ASC( s[2], p );
   p += 2;
   
   *p++ = '-';
   
   TO_BCD_ASC( s[1], p );
   p += 2;
   
   *p++ = ':';
   
   TO_BCD_ASC( s[0], p );
   p += 2;
   
   *p = 0;
}

INT32U ISDC( INT8U c )
{
   
   if( (c>='0') && (c<='9') ) return 1;
   else                       return 0;

}
//返回转换长度
// flag
//
//11g - DATE
//12g - TIME
//13g - TIME1
//
INT32U TRAN_G( INT8U *s, INT8U *p, INT32U flag )
{
   INT32U len;
   
   switch( flag&0x3f )
   {
      case 11:
        T_DATE( s, p );
        len = 11;
        break;
                         
      case 12:
        T_TIME( s, p );
        len = 11;
        break;
      
      case 13:
        T_TIME1( s, p );
        len = 8;
        break;
        
      default:
        len = 0;
        break;
   }
   s = p;
   if( !BITTST(flag,7) )      //去掉前缀0
   {
      while( *p==' ' ) p++;   //过滤前面的空格
      
      if( (p[0]=='-') && (p[1]!='-') ) p++;
      
      while( *p )
      {
         if( (p[0]=='0') && ISDC(p[1]) ) *p++=' ';
         else break;
      }
   }   
   while( *s )                
   {
      if( (s[0]=='-') && (s[1]==' ') )    //'-'号向后移
      {
         s[0] = ' ';
         s[1] = '-';
         s++;
      }
      else break;
   }  
   return len;
}

u32 remove_pre0( INT8U *ps, INT8U *pd, u32 flag )
{
   
   INT8U *p = ps;
   u32 l = 0;
   if( !BITTST(flag,7) )
   {
      while( *p )             //去掉前缀0
      {
         if( (p[0]=='0') && ( (p[1]!='.') && p[1] ) ) *p++=' ';
         else break;
      }
   }
   flag &= 0x1f;
   if( flag>10 ) flag=10;
   
   if( flag ) p = ps+10-flag;
   while( *p )
   {
      *pd++ = *p++;
      l++;
   }
   
   return l;  
}
void add_dot( INT8U *ps, u32 flag )
{
   u32 m;
   
   m = flag>>8;               //小数位数
   if( m==0 || m>8 ) return;
   m = 9-m;
   while( m )
   {
      ps[0] = ps[1];
      ps++;
      m--;
   }
   ps[0] = '.';
}

//字符串长度,不包含结尾0
INT16U STRLEN( INT8U *s )
{
   INT16U c=0;
   
   while( *s++ ) c++;
   
   return c;
}

















