#include "gpio_config.h"
#include "hitagS.h"


void field_switch(u8 *sw, vu8 nBit)
{
   	u8 send_bit = 0;
	while(nBit)
	{
		
        send_bit++; 
		//���һλ����
		SDIN=1;
		delay_us(46);  //56
		SDIN=0;
		if((*sw) & 0x80)
			delay_us(168+10);	//�ų��߼�1
		else	
			delay_us(104+10);	//�ų��߼�0
		(*sw)<<=1;
		nBit--;
		if(send_bit>=8)
		{
		   send_bit = 0;
		   sw++;
		}
		  
	}
	//�ų�EOF
	SDIN=1;
	delay_us(46);			
	SDIN=0;
	delay_us(46);			
}
//----HITAGS�����縴λ-----------------------------------------
void HITAGS_Reast(void)
{
	//�رմų�
	SDIN=1;
	delay_ms(10);			// >4.8 mS			
	//�򿪴ų�
	SDIN=0;
	delay_ms(5);			// (280~5000)*8 uS
}	




void EM_Reast(void)
{
	//�رմų�
	SDIN=1;
	delay_ms(100);			// >4.8 mS			
	//�򿪴ų�
	SDIN=0;
	delay_ms(5);			// (280~5000)*8 uS
}			
//----����UID--------------------------------------------------
void HITAGS_UID_Request(void)
{
    u8 cmd = 0xc0; 
   	field_switch(&cmd, 5);	//11000xxx
}			
