/////#FIle Name: Log.c

#include <stdarg.h>
#include <string.h>
#include"log.h"
#include"ASCT.h"
#include "MyUsart.h"
#include "bluetooth.h"
#include "file.h"
#include "flash.h"
#include "key.h"
#include "beep.h"
#include "oled.h"
#include "delay.h"
#include "RFID.h"


#define __TEST_MODE__
#define __UART_AVALIBLE__

#ifdef __TEST_MODE__
#ifdef __UART_AVALIBLE__ 

static INT8U sbbuf[256];


u32 vsbuild( INT8U *buffer, const char *format, va_list args)
{
    u32 blen = 0;
    u32 ch;
    u32 flag = 0;
    
    INT8U  buf[16];
    while( (ch = *format++) != 0 )
    {
        if (ch != '%')
        {
            *buffer++ = ch;
            blen++;
            if( blen>250 ) break;
        }
        else
        {
           {
            u32 pre=0;
            u32 num=0;
            u32 m=0;
   
            while( (ch=*format) != 0 )
            { 
               if( num==0 )
               {
                  if( ch=='0' ) BITSET(pre,7);	///////////////////////
               }
               if( (ch>='0') && (ch<='9') )
               {
                  num = num*10+(ch-'0');
                  format++;
               }
               else
               {
                  if( ch=='l' || ch=='L' )
                  {
                     BITSET(pre,6);
                     format++;
                  }
                  else
                  if( ch=='.' )
                  {
                     format++; 
                     while( (ch=*format) != 0 )
                     {
                        if( (ch>='0') && (ch<='9') )
                        {
                           m = m*10+(ch-'0');
                           format++;
                        }
                        else
                           break;
                     }
                  }
                  break;
               }
            }
            flag = pre+num+(m<<8);
            }
            ch = *format++;
            switch ( ch )
            {
               case 'u':
               case 'U':
               case 'd':
               case 'D':
                    if( ch=='d' || ch=='D' )
                    {
                       ch = va_arg(args, u32);
                       if( ch&0x80000000 )
                       {
                          ch = -ch;
                          *buffer++ = '-';
                          blen++;
                       }
                    }
                    else
                       ch = va_arg(args, u32);
                    
                    TO_DEC_ASC( ch, buf );
                    add_dot( buf, flag );
                    ch = remove_pre0( buf, buffer, flag );
                    blen += ch;
                    buffer += ch;
                    
                    break;

                case 'x':
                case 'X':
                case 'p':
                case 'P':
                    ch = va_arg(args, u32);
                    
                    TO_HEX_ASC( ch, buf );
                    
                    ch = remove_pre0( buf, buffer, flag );
                    blen += ch;
                    buffer += ch;

                    break;
                    
                case 's':
                case 'S':
                    ch = va_arg(args, u32);
                    while( *((INT8U*)ch) )
                    {
                       *buffer++ = *((INT8U*)ch);
                       ch++;
                       blen++;
                    }
                    break;
                
                case 'b':
                case 'B':
                    ch = va_arg(args, u32);
                    TO_BCD_ASC( ch, buffer );
                    ch = 2;
                    buffer += ch;
                    blen += ch;
                    break;
                
                case 'g':
                case 'G':
                    ch = va_arg(args, u32);
                    
                    ch = TRAN_G( (INT8U*)ch, buffer, flag );
                    
                    buffer += ch;
                    blen += ch;
                    break;

                case 'c':
                case 'C':
                    ch = va_arg(args, u32);
                     
                //case '%':
                default:
                    *buffer++ = ch;
                    blen++;
                    break;
            }
            if( blen>240 ) break;
            if( ch == 0 ) break;   
        }
    }
    *buffer = 0;
    return blen;
}



void MyDebugPrint( char *fm, ...)
{
   va_list  args;
   va_start( args,fm );
   vsbuild( sbbuf, fm, args );
   com_clear(COM3);
   com_write(COM3, sbbuf, STRLEN(sbbuf));

}




void TestModel(void)
{

	u8 KeyNum =0 ;
	u8 Key4Num = 0;
	u16 sta;
	u32 id;

	beep(1, 100, 1);
	//MyDebugPrint("KEY1 Passed\r\n");
	MyDebugPrint("KEY3 Passed\r\n");
	MyDebugPrint("KEY6 Passed\r\n");

	OLED_Clear();			
		OLED_ShowString(0,1*16 ,1,1, "  TEST MODE");		
	OLED_Refresh_Gram();	//刷新显示

	while(1)
	{
		while(keys_scan());
		while(1)
		{
			KeyNum = keys_scan();
			if(KeyNum!=0)
			break;
		}
	    beep(1, 50, 1);
		switch(KeyNum)
		{
			case KEY_CMD_up:
				
				MyDebugPrint("KEY2 Passed\r\n");
				sta = BT_bootup();
				if((sta==WithNoHostAdd)||(sta==WithHostAdd))
					MyDebugPrint("KEY-12 Passed\r\n");
				else
					MyDebugPrint("KEY-12 ERROR!!!!!!!!!!\r\n");
					
		        break;

			case KEY_CMD_back:
				
				MyDebugPrint("KEY5 Passed\r\n");
				if(Key4Num==0)
				{
					Key4Num ++;
					OLED_whiteback();	
				}
				else
				{	 
					Key4Num = 0;
					OLED_Clear();
					OLE_blackback();
					OLED_Refresh_Gram();	//刷新显示
				}
				break;
			case KEY_CMD_scan :

				MyDebugPrint("KEY4 Passed\r\n");

				id = RFID_Read(1);
				if(id)
					MyDebugPrint("A :  %08X\r\n", id);
				else
				{
					id = RFID_Read(2);
					if(id)
						MyDebugPrint("B :  %08X\r\n", id);	
				}
				break;

			case KEY_CMD_menu :

				MyDebugPrint("KEY3 Passed\r\n");
				break;

			case KEY_CMD_down :

				MyDebugPrint("KEY6 Passed\r\n");
				break;

			case KEY_CMD_pow :

				MyDebugPrint("KEY1 Passed\r\n");
				break;

			default	:
				break;			   	
		}
	while(keys_scan());	

	}

}

#endif

#endif


