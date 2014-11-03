
#include "stm32f10x_flash.h"
#include "file.h" 
#include "PCF8563.h"
#include "InternalFlash.h"
#include "main.h"



u32 LastSaveTime = 0;	  //���һ�� 


RECORDINF  storeaddrlist;
RECORDINF  timeoutaddrlist;

////////////////////////////////////////////////////////////////////////////////
//----��ȡˢ����¼��------------------------------------------------------------
void AddrPoinitInit(RECORDINF* paddrlist)
{
	u16 cnt;
	u32 addr;

	//��ҳ�׵�ַ
	addr = paddrlist->baseaddr;		
	cnt = HWREGH(addr);		

	//��¼��0
	if( cnt == 0xFFFF )
	{
		paddrlist->RecordCount = 0;
		paddrlist->CountAddrOffset = 0;
		return;
	}

	addr += 2;
	//��ʼ��ѯ
	for( paddrlist->CountAddrOffset=2; paddrlist->CountAddrOffset<FLASH_PAGE_SPACE; paddrlist->CountAddrOffset+=2)
	{
		cnt = HWREGH(addr);		
		//�ҵ����һ������ֵ
		if( cnt == 0xFFFF )
		{
			addr -= 2;
			paddrlist->RecordCount = HWREGH(addr);	
			return;
		}
		addr += 2;
	}
	paddrlist->RecordCount = cnt;		  //�޸ģ���������bug�����������
	return;
}


//----����ˢ����¼�ļ�����------------------------------------------------------
void SaveDate(RECORDINF* paddrlist)
{
	u32 addr;
	 
	//����ѭ���ռ䣬����ռ䣬�ٴ�ͷ��ʼ
	if( ((paddrlist->CountAddrOffset) >= FLASH_PAGE_SPACE) )
	{
		FLASH_Unlock();
		FLASH_ErasePage( paddrlist->baseaddr );
		FLASH_Lock();
		paddrlist->CountAddrOffset = 0;
	}

	addr = paddrlist->baseaddr + (paddrlist->CountAddrOffset);
	(paddrlist->CountAddrOffset) += 2;		//����������ַ��λ��+2

	paddrlist->RecordCount ++;			//��¼������+1
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

//----��ȡˢ����¼��------------------------------------------------------------
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

	if((new_card.time<LastSaveTime)||(new_card.time<0x51308fe9)||(new_card.time>0x96c8df8d))	 // LastTime �޸�
	{
		ret = 3;						 //ʱ�����
	 	goto loop2;                      	
	}

	if(storeaddrlist.RecordCount>=RECORD_MAX_COUNT)
	{
		ret = 2;
		goto loop2;
	}

	if((timeoutaddrlist.RecordCount==0))  //�����������
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
			ret = 4	;			  /////��ַ�����쳣
			goto loop2;
		}	
		SPI_Flash_Read((u8*)&card, addr, sizeof(FILE_RECORD_STRUCT));
		if(((new_card.time - card.time)<=TIMEMAX))  //�ڱȽ�ʱ����
		{
			if(new_card.rfid==card.rfid)
			{
				ret = 1;
				goto loop2;			  //�����ظ�
			}		
		}
		else						 //��һ��ʱ�����������
		{
			ret = 0;
			finaladdr += sizeof(FILE_RECORD_STRUCT);
			goto loop2;				  //��Ҫ����
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
			SPI_Flash_WriteQ((u8*)&new_card, finaladdr, sizeof(FILE_RECORD_STRUCT));	 //�洢����

			SaveDate(&timeoutaddrlist);	
		}		

		return ret;	
}

/////////////////////////////////////////////////////////////////////////////
//----����BT����---------------------------------------------------------------
void save_para_BT_enable(u8 ch)
{
//	u8 para;

//	SPI_Flash_Read((u8*)&para, FILE_PARA_ADDR, sizeof(FILE_PARA_STRUCT)); 
//	para.BT_use = ch;
	//�������
	SPI_Flash_Write(&ch, FILE_PARA_ADDR, 1);		
}
//----��ȡBT����---------------------------------------------------------------
u8 read_para_BT_enable()
{
	u8 ch;

	SPI_Flash_Read((u8*)&ch, FILE_PARA_ADDR, 1); 
	return ch;
}

/////////////////////////////////////////////////////////////////////////////////
//STM32 FLASH ��������

//----��ȡ��ǰ��¼�кźͼ�����----------------------------------------------------
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

//----���ȫ��ˢ����¼-----------------------------------------------------
void clear_CardID()
{
	 char DateSt[12], TimeSt[12];

	storeaddrlist.RecordCount = 0;
	storeaddrlist.CountAddrOffset = 0;
	timeoutaddrlist.RecordCount = 0;
	timeoutaddrlist.CountAddrOffset = 0;
	LastSaveTime = GetRTC(DateSt, TimeSt);///��ȡ��ǰʱ�䣬���� //��Ҫ�Ż�;
	//�����¼��
  	FLASH_Unlock();
  	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(PLACELASTID); 
	FLASH_ErasePage(POINTOFSECTION);   
  	FLASH_Lock();
}




