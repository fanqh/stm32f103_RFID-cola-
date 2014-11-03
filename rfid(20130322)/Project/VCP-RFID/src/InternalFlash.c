#include "stm32f10x_flash.h"
#include "file.h" 
#include "stm32f10x.h"
#include "delay.h"

/////////���ַ��д��32λdate
////////���� 1:д����ȷ   0��д�����
u8  WriteFlashV16(u32 addr, u16 date)
{
	FLASH_Unlock();	  
	FLASH_ErasePage(addr);
	FLASH_ProgramHalfWord(addr, date);
//	FLASH_ProgramHalfWord(addr+2, (u16)((date>>16)&0xffff));	
	if(HWREGH(addr)==date)
	{
		FLASH_Lock();
	    return 1;
	}
	else
	{
	    FLASH_Lock();
	    return 0;  
	}  
}

 u8  WriteFlashV32(u32 addr, u32 date)
{
	FLASH_Unlock();	            
	FLASH_ErasePage(addr);
	FLASH_ProgramHalfWord(addr, (u16)date&0xffff);
	FLASH_ProgramHalfWord(addr+2, (u16)((date>>16)&0xffff));
	 
	
	if(HWREG(addr)==date)
	{
		FLASH_Lock();
	    return 1;
	}
	else
	{
	    FLASH_Lock();
	    return 0;  
	}
}


//----2�ֽ�д��flash������------------------------------------------------------
void HalFlashWriteHalf(u32 addr, u16 hword)
{
	FLASH_Unlock();	  
	FLASH_ProgramHalfWord(addr, hword);
    FLASH_Lock();
}




