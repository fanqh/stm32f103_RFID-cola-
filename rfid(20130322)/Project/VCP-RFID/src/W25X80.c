
#include "string.h"
#include "spi.h"
#include "delay.h"
#include "W25X80.h"

////////////////////////////////////////////////////////////////////////////////
//˵����W25X80 
//PAGE����=0x100(256B)
//SECTER����=16*PAGE=0x1000(4,096B)
//BLOCK����=16*SECTOR=0x10000(65,536B)
//CHIP����=16*BLOCK=0x100000(1048,576B)  ��4096ҳ

#define W25X80_PAGE_LENTH     	256	
#define W25X80_Read	        	0x03	
#define W25X80_Enable	    	0x06
#define W25X80_Disable	    	0x04
#define W25X80_Erase4k	    	0x20
#define W25X80_Erase64k	    	0xD8
#define W25X80_EraseAll	    	0x60
#define W25X80_Prog     		0x02	//�������	
#define W25X80_Read_Sta_Reg  	0x05	//��״̬�Ĵ���
#define W25X80_Write_Sta_Reg 	0x01    //д״̬�Ĵ���
#define W25X80_Unpsec        	0x39    //ȡ������
#define W25X80_ChipErase     	0x60    //��ʽ������

//Ƭѡ�ӿ�
#define W25X80_CS_GPIO	GPIOC  	
#define W25X80_CS_PIN	GPIO_Pin_11  
#define W25X80_CSon()	GPIO_ResetBits(W25X80_CS_GPIO, W25X80_CS_PIN)
#define W25X80_CSoff()	GPIO_SetBits(W25X80_CS_GPIO, W25X80_CS_PIN)

#define W25X80_BLOCK_LENTH     	4096	
static u8  flash_block_buff[W25X80_BLOCK_LENTH];
#define EPROM_START_ADDR		0xC0000		//�û��洢��ʼ��ַ
#define EPROM_USER_LENTH		0x40000		//�û��洢ʹ�ÿռ�

//----���FALSH״̬��æ����1----------------------------------------------------
u8 flash_busy_status()
{
	u8  Sta_Reg;

	W25X80_CSon();
    SPI1_PutcGetc(W25X80_Read_Sta_Reg);		//��״̬�Ĵ���
    Sta_Reg=SPI1_PutcGetc(0);				//��״̬�Ĵ���
	W25X80_CSoff();
	return Sta_Reg & 1;
}
//----����FLASH��ַ-------------------------------------------------------------
void flash_addr(u32 addr)
{
    u8 str[4],*p;
	p=(u8 *)&addr;
	str[2]=*p; p++;
	str[1]=*p; p++;
	str[0]=*p;
	SPI1write(str, 3);					//���͵�ַ
}
//----��ȡFLASH����------------------------------------------------------------
u8  _FROMgetc(u32 addr)
{
	u8  ch;

	while(flash_busy_status());			//�ȴ�����
	W25X80_CSon();
    SPI1_PutcGetc(W25X80_Read);			//���Ͷ�����
	flash_addr(addr);
	ch=SPI1_PutcGetc(0);
	W25X80_CSoff();
	return ch;
} 
//----��ȡFLASH����------------------------------------------------------------
u8  EPROMgetc(u32 addr)
{
	if(addr > EPROM_USER_LENTH)
		return 0;
	addr+=EPROM_START_ADDR;
	return _FROMgetc(addr);
}
//----��ȡFLASH���ݴ�-----------------------------------------------------------
void _FROMread(u32 addr, u8 *buff, vu16 len)
{
	while(flash_busy_status());			//�ȴ�����
	W25X80_CSon();
    SPI1_PutcGetc(W25X80_Read);			//���Ͷ�����
	flash_addr(addr);
	SPI1read(buff, len);
	W25X80_CSoff();
} 
//----��ȡFLASH���ݴ�-----------------------------------------------------------
void EPROMread(u32 addr, u8 *buff, vu16 len)
{
	if(addr > EPROM_USER_LENTH)
		return;
	addr+=EPROM_START_ADDR;
	_FROMread(addr, buff, len);
} 

//////////////////////////////////////////////////////////////////////////////
/*
//----------------------------------------------------------------------------
u8  flash_read_status()
{
	W25X80_CSon();
	//DelayUS(10);
    SPI1_PutcGetc(W25X80_Read_Sta_Reg);		//��״̬�Ĵ���
    SPI1_PutcGetc(0);					//��״̬�Ĵ���
	W25X80_CSoff();
	return (u8 )F_READ_DATA;
}
*/
//----ʹ�ܻ�ʧ��FLASHд��-----------------------------------------------------
void flash_write_enable(u8  f)
{
	while(flash_busy_status());			//�ȴ�����
	W25X80_CSon();
	if(f)
        SPI1_PutcGetc(W25X80_Enable);		
	else
        SPI1_PutcGetc(W25X80_Disable);		
	W25X80_CSoff();
} 
//------------------------------------------------------------------------------
void flash_Write_status(u32 ch)
{
	flash_write_enable(1);
	W25X80_CSon();
    SPI1_PutcGetc(W25X80_Write_Sta_Reg);		
    SPI1_PutcGetc(ch);		
	W25X80_CSoff();
}		
//----FLASH�����(���Ե�ַ)-----------------------------------------------
void _FROMerase4k(u32 addr)
{
	while(flash_busy_status());			//�ȴ�����
    flash_write_enable(1);
	W25X80_CSon();
    SPI1_PutcGetc(W25X80_Erase4k);		//������������
	flash_addr(addr & 0xFFFFF000);
	W25X80_CSoff();
	delay_ms(150);
}
//----FLASH�����(�û���ַ)-----------------------------------------------
void EPROMerase4k(u32 addr)
{
	if(addr > EPROM_USER_LENTH)
		return;
	if((addr + 4096) > EPROM_USER_LENTH)
		return;
	addr+=EPROM_START_ADDR;
	_FROMerase4k(addr);
}
//----���дFLASH(���Ե�ַ)-----------------------------------------------
void _FROMwrite(u32 addr, u8* buff, vu16 bufflen)
{
	u32 addr_block_top;				//��ĵ�һ���ֽڵĵ�ַ
	u32 offset;						//��ַƫ��
	u16 lenth;						//�����ֽ���
	int page, len=bufflen;
	u8 *p;

	addr_block_top = addr & 0xFFFFF000;
	offset = addr-addr_block_top;
	while(len>0)
	{
		_FROMread(addr_block_top, flash_block_buff, W25X80_BLOCK_LENTH);	//��ȡ������
		lenth = W25X80_BLOCK_LENTH - offset;
		if(len<lenth)
			lenth=len;
		memcpy((flash_block_buff + offset), buff, lenth);				//��д����
    	_FROMerase4k(addr_block_top);								//������
		p=flash_block_buff;
		for(page=0; page<16; page++)
		{
			while(flash_busy_status());			//�ȴ�����
			flash_write_enable(1);
			W25X80_CSon();
    		SPI1_PutcGetc(W25X80_Prog);			//�������
			flash_addr(addr_block_top);
			SPI1write(p, W25X80_PAGE_LENTH);
			W25X80_CSoff();
			delay_ms(2);
			addr_block_top+=W25X80_PAGE_LENTH;
			p+=W25X80_PAGE_LENTH;
		}

		offset=0;
		buff += lenth;
		len -= lenth;
	}
}
//----���дFLASH(�û���ַ)------------------------------------------------
void EPROMwrite(u32 addr, u8 * buff, vu16 bufflen)
{
	if(addr > EPROM_USER_LENTH)
		return;
	if((addr + bufflen) >= (EPROM_START_ADDR+EPROM_USER_LENTH))
		return;
	addr+=EPROM_START_ADDR;
	_FROMwrite(addr, buff, bufflen);
}
//----����дFLASH(���Ե�ַ)-----------------------------------------------
void _FROMwriteQ(u32 addr, u8 * buff, vu16 bufflen, u8  EraseFlag)
{
	u16 lenth;						//�����ֽ���
	u16 len=bufflen;

	lenth = W25X80_PAGE_LENTH - (addr & 0x000000FF);	//��ǰҳ�����ݳ���
	while(len>0)
	{
		//���Զ�������־������4K��
		if(EraseFlag && (addr & (u32)0x0FFF)==0 )
			_FROMerase4k(addr);
		if(len<lenth)
			lenth=len;
		while(flash_busy_status());			//�ȴ�����
		flash_write_enable(1);
		W25X80_CSon();
   		SPI1_PutcGetc(W25X80_Prog);		//�������
		flash_addr(addr);
		SPI1write(buff, lenth);
		W25X80_CSoff();
		addr += lenth;
		buff += lenth;
		len -= lenth;
		lenth = W25X80_PAGE_LENTH;
	}
}
//----����дFLASH(�û���ַ)---------------------------------------------------
void EPROMwriteQ(u32 addr, u8 * buff, vu16 bufflen, u8  EraseFlag)
{
	if(addr > EPROM_USER_LENTH)
		return;
	if((addr + bufflen) > EPROM_USER_LENTH)
		return;
	addr+=EPROM_START_ADDR;
	_FROMwriteQ(addr, buff, bufflen, EraseFlag);
}
//----���дFLASH�ַ�(�û���ַ)-----------------------------------------------
void EPROMputc(u32 addr, u8  ch)
{
	if(addr > EPROM_USER_LENTH)
		return;
	addr+=EPROM_START_ADDR;
	_FROMwrite(addr, &ch, 1);
}
//----���дFLASH�ַ�(���Ե�ַ)-----------------------------------------------
void _FROMputc(u32 addr, u8  ch)
{
	EPROMwrite(addr, &ch, 1);
}

//----��ȡоƬID W25X16��ID---------------------------------------------------
u16 W25X80_ReadID(void)
{
	u16 Temp = 0;	  

	W25X80_CSon();
	SPI1_PutcGetc(0x90);//���Ͷ�ȡID����	    
	SPI1_PutcGetc(0x00); 	    
	SPI1_PutcGetc(0x00); 	    
	SPI1_PutcGetc(0x00); 	 			   
	Temp|=SPI1_PutcGetc(0xFF)<<8;  
	Temp|=SPI1_PutcGetc(0xFF);	 
	W25X80_CSoff();
	return Temp;
}   		    

//----��ʼ��W25X80-------------------------------------------------------------
bool FROM_init()
{
	u16 id;
	u16 t;
	GPIO_InitTypeDef GPIO_InitStructure;	//����1���ṹ�����
  	//Enable SPI and GPIO clocks
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	//W25X80 CS
	GPIO_InitStructure.GPIO_Pin = W25X80_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//�������ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(W25X80_CS_GPIO, &GPIO_InitStructure);
	W25X80_CSoff();

	spi1_init();			 	//��ʼ��SPI��

	// �ȴ�BUSYλ���
	while (flash_busy_status())   
	{
		delay_ms(1);
		t++;
		if(t>100)
			return FALSE;
	}
	flash_Write_status(0);			 						//д������Ч

	//��ȡоƬID W25X16��ID:0XEF14
	id=W25X80_ReadID();
	if(id==0 || id==0xFF)
		return FALSE;

	return TRUE;
}
/*
//----FLASH�����---------------------------------------------------------
void _FROMerase64k(u32 addr)
{
    flash_write_enable(1);
	W25X80_CSon();
    SPI1_PutcGetc(W25X80_Erase64k);		//����
	flash_addr(addr & 0xFFFF0000);
	W25X80_CSoff();
//	delay_ms(20000);
}
//----FLASH�����---------------------------------------------------------
void _FROMeraseAll()
{
    flash_write_enable(1);
	W25X80_CSon();
    SPI1_PutcGetc(W25X80_EraseAll);		//����
	W25X80_CSoff();
//	delay_ms(200000);
}
*/

