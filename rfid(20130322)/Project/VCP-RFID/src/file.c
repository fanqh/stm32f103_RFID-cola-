
#include "stm32f10x_flash.h"
#include "file.h" 
#include "PCF8563.h"
#include "InternalFlash.h"
#include "main.h"



u32 LastSaveTime = 0;	  //最后一个 


RECORDINF  storeaddrlist;
RECORDINF  timeoutaddrlist;

////////////////////////////////////////////////////////////////////////////////
//----获取刷卡记录数------------------------------------------------------------
void AddrPoinitInit(RECORDINF* paddrlist)
{
	u16 cnt;
	u32 addr;

	//从页首地址
	addr = paddrlist->baseaddr;		
	cnt = HWREGH(addr);		

	//记录是0
	if( cnt == 0xFFFF )
	{
		paddrlist->RecordCount = 0;
		paddrlist->CountAddrOffset = 0;
		return;
	}

	addr += 2;
	//开始查询
	for( paddrlist->CountAddrOffset=2; paddrlist->CountAddrOffset<FLASH_PAGE_SPACE; paddrlist->CountAddrOffset+=2)
	{
		cnt = HWREGH(addr);		
		//找到最后一个计数值
		if( cnt == 0xFFFF )
		{
			addr -= 2;
			paddrlist->RecordCount = HWREGH(addr);	
			return;
		}
		addr += 2;
	}
	paddrlist->RecordCount = cnt;		  //修改，曾经出现bug，后果很严重
	return;
}


//----保存刷卡记录的计数器------------------------------------------------------
void SaveDate(RECORDINF* paddrlist)
{
	u32 addr;
	 
	//超出循环空间，清除空间，再从头开始
	if( ((paddrlist->CountAddrOffset) >= FLASH_PAGE_SPACE) )
	{
		FLASH_Unlock();
		FLASH_ErasePage( paddrlist->baseaddr );
		FLASH_Lock();
		paddrlist->CountAddrOffset = 0;
	}

	addr = paddrlist->baseaddr + (paddrlist->CountAddrOffset);
	(paddrlist->CountAddrOffset) += 2;		//计数器（地址）位置+2

	paddrlist->RecordCount ++;			//记录计数器+1
	HalFlashWriteHalf(addr, paddrlist->RecordCount);
	
}


void FileInit(void)
{
	
	storeaddrlist.baseaddr   = PLACELASTID;
	timeoutaddrlist.baseaddr = POINTOFSECTION;
	
	AddrPoinitInit(&storeaddrlist);	
	AddrPoinitInit(&timeoutaddrlist);
	
}
 
////////////////////////////////////////////////////////////////////////////////

//----获取刷卡记录数------------------------------------------------------------
u16 GetStoreCount(void)
{
	return storeaddrlist.RecordCount;
}

RECORDINF* GetStoreListInfo(void)
{
	return  &storeaddrlist;	
}


u8 SearchId(FILE_RECORD_STRUCT new_card)
{
	u32 addr;
	u32 finaladdr;
	u16 i;
	FILE_RECORD_STRUCT card;
	u8 ret = 0;

	if((new_card.time<LastSaveTime)||(new_card.time<0x51308fe9)||(new_card.time>0x96c8df8d))	 // LastTime 修改
	{
		ret = 3;						 //时间错误
	 	goto loop2;                      	
	}

	if(storeaddrlist.RecordCount>=RECORD_MAX_COUNT)
	{
		ret = 2;
		goto loop2;
	}

	if((timeoutaddrlist.RecordCount==0))  //表格内无数据
	{
		ret = 0;
		finaladdr = FILE_SETION_START_DATA_ADDR;
		goto loop2;		  	
	}

	finaladdr = FILE_SETION_START_DATA_ADDR + (timeoutaddrlist.RecordCount-1)*sizeof(FILE_RECORD_STRUCT);
	addr = finaladdr;
	   
	for(i=0; i<SEARCH_MAX_NUM; i++)
	{

		if((addr<FILE_SETION_START_DATA_ADDR)||(addr>FILE_SETION_END_DATA_ADDR))
		{
			ret = 4	;			  /////地址出现异常
			goto loop2;
		}	
		SPI_Flash_Read((u8*)&card, addr, sizeof(FILE_RECORD_STRUCT));
		if(((new_card.time - card.time)<=TIMEMAX))  //在比较时间内
		{
			if(new_card.rfid==card.rfid)
			{
				ret = 1;
				goto loop2;			  //卡号重复
			}		
		}
		else						 //在一定时间段内无数据
		{
			ret = 0;
			finaladdr += sizeof(FILE_RECORD_STRUCT);
			goto loop2;				  //需要保存
		}
						
		if(addr==FILE_SETION_START_DATA_ADDR)
			addr = FILE_SETION_END_DATA_ADDR;
		else
			addr = addr -  sizeof(FILE_RECORD_STRUCT);
				
	}	
	loop2:
		if(ret!= 3) 
			LastSaveTime = new_card.time;
		if(ret==0)
		{
			if(finaladdr>=FILE_SETION_END_DATA_ADDR)
				finaladdr = FILE_SETION_START_DATA_ADDR;
			SPI_Flash_WriteQ((u8*)&new_card, finaladdr, sizeof(FILE_RECORD_STRUCT));	 //存储数据

			SaveDate(&timeoutaddrlist);	
		}		

		return ret;	
}

/////////////////////////////////////////////////////////////////////////////
//----保存BT参数---------------------------------------------------------------
void save_para_BT_enable(u8 ch)
{
//	u8 para;

//	SPI_Flash_Read((u8*)&para, FILE_PARA_ADDR, sizeof(FILE_PARA_STRUCT)); 
//	para.BT_use = ch;
	//保存参数
	SPI_Flash_Write(&ch, FILE_PARA_ADDR, 1);		
}
//----读取BT参数---------------------------------------------------------------
u8 read_para_BT_enable()
{
	u8 ch;

	SPI_Flash_Read((u8*)&ch, FILE_PARA_ADDR, 1); 
	return ch;
}

/////////////////////////////////////////////////////////////////////////////////
//STM32 FLASH 作计数器

//----获取当前记录行号和计数器----------------------------------------------------
//u16 Get_Recode_Count()
//{
//	u32 addr;
//
//	addr = HWREG(PLACELASTID); 
//    if((addr<FILE_DATA_ADDR)||(addr>FILE_DATE_ENDADDR))
//        return 0;
//    else
//       return 	 (HWREG(PLACELASTID)-FILE_DATA_ADDR)/sizeof(FILE_RECORD_STRUCT)+1;
//}

//----清除全部刷卡记录-----------------------------------------------------
void clear_CardID()
{
	 char DateSt[12], TimeSt[12];

	storeaddrlist.RecordCount = 0;
	storeaddrlist.CountAddrOffset = 0;
	timeoutaddrlist.RecordCount = 0;
	timeoutaddrlist.CountAddrOffset = 0;
	LastSaveTime = GetRTC(DateSt, TimeSt);///获取当前时间，秒数 //需要优化;
	//清除记录区
  	FLASH_Unlock();
  	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(PLACELASTID); 
	FLASH_ErasePage(POINTOFSECTION);   
  	FLASH_Lock();
}




