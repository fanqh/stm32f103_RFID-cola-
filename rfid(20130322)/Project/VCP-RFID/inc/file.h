#ifndef __FILE_H
#define __FILE_H
	 
#include <string.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include "stm32f10x.h"
#include "flash.h" 
#include "config.h" 

#define HWREG(x)	(*((volatile unsigned long *)(x)))
#define HWREGH(x) 	(*((volatile unsigned short *)(x)))
#define HWREGB(x)	(*((volatile unsigned char *)(x)))


//参数结构
typedef struct
{
	u8 BT_use;			//BT启用标志 1==启用
}FILE_PARA_STRUCT;

//刷卡记录结构
//dd-dd-dd tt:tt:tt: nnnnnnnnnn-;--
typedef struct
{
	u32 time;		//刷卡时间
	u32 rfid;		//卡号
//	u32 sendflag;   //0x55aa：数据曾经发送过标记，其他数值表示此数据未曾发送过
//	u32 nouse;
}FILE_RECORD_STRUCT;

//ram中id信息
typedef struct
{
	u32 timeout; 	
	u32 ID;		 

}TIMEOUTBUFF;

//ram中id信息表格指针
typedef struct
{
	u16 PointStart; 
	u16 PointEnd;
	u16 ListLen;
	u8 Polarity;

}LISTPARA;

typedef enum
{
	savedate = 0,
	repeatid,
	memoryfull,
	timeerror,
	systemerror
	
}IDPROCESS;

typedef struct
{
	u32 baseaddr;
	u16 RecordCount;		//卡片记录 计数器
	u16 CountAddrOffset;	//计数器（地址）位置
}RECORDINF;


extern u32 LastSaveTime ;	  //最后一个 
#define FLASH_PAGE_SPACE 1024


u8 SearchId(FILE_RECORD_STRUCT new_card);


u16 Get_Recode_Count(void);	              //获取当前记录行号和计数器

void clear_CardID(void);		          //清除全部刷卡记录

void save_para_BT_enable(u8 ch);	//保存参数
u8 read_para_BT_enable(void);		//读取参数
void save_para_Card(u8 ch);			//保存卡片参数
u8 read_para_Card(void);			//读取卡片参数

RECORDINF* GetStoreListInfo(void); //获取存储表格内的地址信息
void FileInit(void);	           //数据存储表格初始化
u16 GetStoreCount(void);
void SaveDate(RECORDINF* paddrlist);


#endif
