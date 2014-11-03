#ifndef __DmaQueue_H
#define __DmaQueue_H

////////////////////////////////////////////////////////////////////////////////////
#include "stm32f10x.h"

//DMA���нṹ
typedef struct {
    vu8	*out; 		//ָ���������λ�� 
    vu8	*sta;		//ָ��Buf�Ŀ�ʼλ��
    vu8	*end;		//ָ��Buf�Ľ���λ��
    vu16 ndata;		//��������Ч�����ֽ���   
    vu16 space;		//���е������ֽڿռ�
	vu16 LastDmaCurrCnt;	//DMA���һ��ʣ��Ĵ�����������Ŀ
}QueuePrar;

//--------------------------------------------------------------------------
void QueueCreate(u8 *databuff, u16 space, QueuePrar *Qprar);
u16 QueueNdata(u16 DmaCurrCnt, QueuePrar *Qprar);
char QueueGetch(QueuePrar *Qprar);
bool QueueRead(u8 *databuff, QueuePrar *Qprar, vu16 lenth);
bool QueueRead2(u8 *databuff, QueuePrar *Qprar, vu16 lenth);
void QueueFlush(vu16 ndata, QueuePrar *Qprar);										//�������

//////////////////////////////////////////////////////////////////////////////////	 
#endif
