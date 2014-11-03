#ifndef __W25X80_H_
#define __W25X80_H_
////////////////////////////////////////////////////////////////////////////

#include "stm32f10x.h"


////////////////////////////////////////////////////////////////////////////
//FROM�洢���ڲ�����
////////////////////////////////////////////////////////////////////////////
bool FROM_init(void);										//��ʼ��W25X80
u8 _FROMgetc(u32 addr);								//��_FROM�ж�ȡһ���ֽ�
void _FROMread(u32 addr, u8  *buff, vu16 lenth);		//��_FROM�ж�ȡһ������
void _FROMputc(u32 addr, u8  ch);						//��_FROM���������һ���ֽ�
void _FROMwrite(u32 addr, u8  *buff, vu16 lenth);		//��_FROM���������һ������
void _FROMerase4k(u32 addr);								//����_FROM�е�4096�ֽڿռ�
void _FROMerase64k(u32 addr);								//����_FROM�е�65536�ֽڿռ�
void _FROMeraseAll(void);									//����ȫ��_FROM�ռ�
//��_FROM�п��ٱ�������,���������־��Ч�����Զ����������4K��
void FROMwriteQ(u32 addr, u8 * buff, vu16 lenth, u8  EraseFlag);



//------------------------------------------------------------------------------
#endif
