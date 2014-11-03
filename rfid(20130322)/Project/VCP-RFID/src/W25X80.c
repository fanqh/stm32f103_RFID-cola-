
#include "string.h"
#include "spi.h"
#include "delay.h"
#include "W25X80.h"

////////////////////////////////////////////////////////////////////////////////
//说明：W25X80 
//PAGE长度=0x100(256B)
//SECTER长度=16*PAGE=0x1000(4,096B)
//BLOCK长度=16*SECTOR=0x10000(65,536B)
//CHIP长度=16*BLOCK=0x100000(1048,576B)  共4096页

#define W25X80_PAGE_LENTH     	256	
#define W25X80_Read	        	0x03	
#define W25X80_Enable	    	0x06
#define W25X80_Disable	    	0x04
#define W25X80_Erase4k	    	0x20
#define W25X80_Erase64k	    	0xD8
#define W25X80_EraseAll	    	0x60
#define W25X80_Prog     		0x02	//编程命令	
#define W25X80_Read_Sta_Reg  	0x05	//读状态寄存器
#define W25X80_Write_Sta_Reg 	0x01    //写状态寄存器
#define W25X80_Unpsec        	0x39    //取消保护
#define W25X80_ChipErase     	0x60    //格式化命令

//片选接口
#define W25X80_CS_GPIO	GPIOC  	
#define W25X80_CS_PIN	GPIO_Pin_11  
#define W25X80_CSon()	GPIO_ResetBits(W25X80_CS_GPIO, W25X80_CS_PIN)
#define W25X80_CSoff()	GPIO_SetBits(W25X80_CS_GPIO, W25X80_CS_PIN)

#define W25X80_BLOCK_LENTH     	4096	
static u8  flash_block_buff[W25X80_BLOCK_LENTH];
#define EPROM_START_ADDR		0xC0000		//用户存储开始地址
#define EPROM_USER_LENTH		0x40000		//用户存储使用空间

//----检测FALSH状态，忙返回1----------------------------------------------------
u8 flash_busy_status()
{
	u8  Sta_Reg;

	W25X80_CSon();
    SPI1_PutcGetc(W25X80_Read_Sta_Reg);		//读状态寄存器
    Sta_Reg=SPI1_PutcGetc(0);				//读状态寄存器
	W25X80_CSoff();
	return Sta_Reg & 1;
}
//----发送FLASH地址-------------------------------------------------------------
void flash_addr(u32 addr)
{
    u8 str[4],*p;
	p=(u8 *)&addr;
	str[2]=*p; p++;
	str[1]=*p; p++;
	str[0]=*p;
	SPI1write(str, 3);					//发送地址
}
//----读取FLASH数据------------------------------------------------------------
u8  _FROMgetc(u32 addr)
{
	u8  ch;

	while(flash_busy_status());			//等待空闲
	W25X80_CSon();
    SPI1_PutcGetc(W25X80_Read);			//发送读命令
	flash_addr(addr);
	ch=SPI1_PutcGetc(0);
	W25X80_CSoff();
	return ch;
} 
//----读取FLASH数据------------------------------------------------------------
u8  EPROMgetc(u32 addr)
{
	if(addr > EPROM_USER_LENTH)
		return 0;
	addr+=EPROM_START_ADDR;
	return _FROMgetc(addr);
}
//----读取FLASH数据串-----------------------------------------------------------
void _FROMread(u32 addr, u8 *buff, vu16 len)
{
	while(flash_busy_status());			//等待空闲
	W25X80_CSon();
    SPI1_PutcGetc(W25X80_Read);			//发送读命令
	flash_addr(addr);
	SPI1read(buff, len);
	W25X80_CSoff();
} 
//----读取FLASH数据串-----------------------------------------------------------
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
    SPI1_PutcGetc(W25X80_Read_Sta_Reg);		//读状态寄存器
    SPI1_PutcGetc(0);					//读状态寄存器
	W25X80_CSoff();
	return (u8 )F_READ_DATA;
}
*/
//----使能或失能FLASH写入-----------------------------------------------------
void flash_write_enable(u8  f)
{
	while(flash_busy_status());			//等待空闲
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
//----FLASH块擦除(绝对地址)-----------------------------------------------
void _FROMerase4k(u32 addr)
{
	while(flash_busy_status());			//等待空闲
    flash_write_enable(1);
	W25X80_CSon();
    SPI1_PutcGetc(W25X80_Erase4k);		//擦除扇区命令
	flash_addr(addr & 0xFFFFF000);
	W25X80_CSoff();
	delay_ms(150);
}
//----FLASH块擦除(用户地址)-----------------------------------------------
void EPROMerase4k(u32 addr)
{
	if(addr > EPROM_USER_LENTH)
		return;
	if((addr + 4096) > EPROM_USER_LENTH)
		return;
	addr+=EPROM_START_ADDR;
	_FROMerase4k(addr);
}
//----随机写FLASH(绝对地址)-----------------------------------------------
void _FROMwrite(u32 addr, u8* buff, vu16 bufflen)
{
	u32 addr_block_top;				//块的第一个字节的地址
	u32 offset;						//地址偏移
	u16 lenth;						//复制字节数
	int page, len=bufflen;
	u8 *p;

	addr_block_top = addr & 0xFFFFF000;
	offset = addr-addr_block_top;
	while(len>0)
	{
		_FROMread(addr_block_top, flash_block_buff, W25X80_BLOCK_LENTH);	//读取块数据
		lenth = W25X80_BLOCK_LENTH - offset;
		if(len<lenth)
			lenth=len;
		memcpy((flash_block_buff + offset), buff, lenth);				//改写数据
    	_FROMerase4k(addr_block_top);								//擦除块
		p=flash_block_buff;
		for(page=0; page<16; page++)
		{
			while(flash_busy_status());			//等待空闲
			flash_write_enable(1);
			W25X80_CSon();
    		SPI1_PutcGetc(W25X80_Prog);			//编程命令
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
//----随机写FLASH(用户地址)------------------------------------------------
void EPROMwrite(u32 addr, u8 * buff, vu16 bufflen)
{
	if(addr > EPROM_USER_LENTH)
		return;
	if((addr + bufflen) >= (EPROM_START_ADDR+EPROM_USER_LENTH))
		return;
	addr+=EPROM_START_ADDR;
	_FROMwrite(addr, buff, bufflen);
}
//----快速写FLASH(绝对地址)-----------------------------------------------
void _FROMwriteQ(u32 addr, u8 * buff, vu16 bufflen, u8  EraseFlag)
{
	u16 lenth;						//复制字节数
	u16 len=bufflen;

	lenth = W25X80_PAGE_LENTH - (addr & 0x000000FF);	//当前页的数据长度
	while(len>0)
	{
		//有自动擦除标志，擦除4K块
		if(EraseFlag && (addr & (u32)0x0FFF)==0 )
			_FROMerase4k(addr);
		if(len<lenth)
			lenth=len;
		while(flash_busy_status());			//等待空闲
		flash_write_enable(1);
		W25X80_CSon();
   		SPI1_PutcGetc(W25X80_Prog);		//编程命令
		flash_addr(addr);
		SPI1write(buff, lenth);
		W25X80_CSoff();
		addr += lenth;
		buff += lenth;
		len -= lenth;
		lenth = W25X80_PAGE_LENTH;
	}
}
//----快速写FLASH(用户地址)---------------------------------------------------
void EPROMwriteQ(u32 addr, u8 * buff, vu16 bufflen, u8  EraseFlag)
{
	if(addr > EPROM_USER_LENTH)
		return;
	if((addr + bufflen) > EPROM_USER_LENTH)
		return;
	addr+=EPROM_START_ADDR;
	_FROMwriteQ(addr, buff, bufflen, EraseFlag);
}
//----随机写FLASH字符(用户地址)-----------------------------------------------
void EPROMputc(u32 addr, u8  ch)
{
	if(addr > EPROM_USER_LENTH)
		return;
	addr+=EPROM_START_ADDR;
	_FROMwrite(addr, &ch, 1);
}
//----随机写FLASH字符(绝对地址)-----------------------------------------------
void _FROMputc(u32 addr, u8  ch)
{
	EPROMwrite(addr, &ch, 1);
}

//----读取芯片ID W25X16的ID---------------------------------------------------
u16 W25X80_ReadID(void)
{
	u16 Temp = 0;	  

	W25X80_CSon();
	SPI1_PutcGetc(0x90);//发送读取ID命令	    
	SPI1_PutcGetc(0x00); 	    
	SPI1_PutcGetc(0x00); 	    
	SPI1_PutcGetc(0x00); 	 			   
	Temp|=SPI1_PutcGetc(0xFF)<<8;  
	Temp|=SPI1_PutcGetc(0xFF);	 
	W25X80_CSoff();
	return Temp;
}   		    

//----初始化W25X80-------------------------------------------------------------
bool FROM_init()
{
	u16 id;
	u16 t;
	GPIO_InitTypeDef GPIO_InitStructure;	//定义1个结构体变量
  	//Enable SPI and GPIO clocks
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	//W25X80 CS
	GPIO_InitStructure.GPIO_Pin = W25X80_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(W25X80_CS_GPIO, &GPIO_InitStructure);
	W25X80_CSoff();

	spi1_init();			 	//初始化SPI口

	// 等待BUSY位清空
	while (flash_busy_status())   
	{
		delay_ms(1);
		t++;
		if(t>100)
			return FALSE;
	}
	flash_Write_status(0);			 						//写保护无效

	//读取芯片ID W25X16的ID:0XEF14
	id=W25X80_ReadID();
	if(id==0 || id==0xFF)
		return FALSE;

	return TRUE;
}
/*
//----FLASH块擦除---------------------------------------------------------
void _FROMerase64k(u32 addr)
{
    flash_write_enable(1);
	W25X80_CSon();
    SPI1_PutcGetc(W25X80_Erase64k);		//擦除
	flash_addr(addr & 0xFFFF0000);
	W25X80_CSoff();
//	delay_ms(20000);
}
//----FLASH块擦除---------------------------------------------------------
void _FROMeraseAll()
{
    flash_write_enable(1);
	W25X80_CSon();
    SPI1_PutcGetc(W25X80_EraseAll);		//擦除
	W25X80_CSoff();
//	delay_ms(200000);
}
*/

