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


//�����ṹ
typedef struct
{
	u8 BT_use;			//BT���ñ�־ 1==����
}FILE_PARA_STRUCT;

//ˢ����¼�ṹ
//dd-dd-dd tt:tt:tt: nnnnnnnnnn-;--
typedef struct
{
	u32 time;		//ˢ��ʱ��
	u32 rfid;		//����
//	u32 sendflag;   //0x55aa�������������͹���ǣ�������ֵ��ʾ������δ�����͹�
//	u32 nouse;
}FILE_RECORD_STRUCT;

//ram��id��Ϣ
typedef struct
{
	u32 timeout; 	
	u32 ID;		 

}TIMEOUTBUFF;

//ram��id��Ϣ���ָ��
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
	u16 RecordCount;		//��Ƭ��¼ ������
	u16 CountAddrOffset;	//����������ַ��λ��
}RECORDINF;


extern u32 LastSaveTime ;	  //���һ�� 
#define FLASH_PAGE_SPACE 1024


u8 SearchId(FILE_RECORD_STRUCT new_card);


u16 Get_Recode_Count(void);	              //��ȡ��ǰ��¼�кźͼ�����

void clear_CardID(void);		          //���ȫ��ˢ����¼

void save_para_BT_enable(u8 ch);	//�������
u8 read_para_BT_enable(void);		//��ȡ����
void save_para_Card(u8 ch);			//���濨Ƭ����
u8 read_para_Card(void);			//��ȡ��Ƭ����

RECORDINF* GetStoreListInfo(void); //��ȡ�洢����ڵĵ�ַ��Ϣ
void FileInit(void);	           //���ݴ洢����ʼ��
u16 GetStoreCount(void);
void SaveDate(RECORDINF* paddrlist);


#endif
