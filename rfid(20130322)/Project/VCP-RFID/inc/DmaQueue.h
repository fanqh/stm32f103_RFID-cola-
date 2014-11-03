#ifndef __DmaQueue_H
#define __DmaQueue_H

////////////////////////////////////////////////////////////////////////////////////
#include "stm32f10x.h"

//DMA队列结构
typedef struct {
    vu8	*out; 		//指向数据输出位置 
    vu8	*sta;		//指向Buf的开始位置
    vu8	*end;		//指向Buf的结束位置
    vu16 ndata;		//队列中有效数据字节数   
    vu16 space;		//队列的数据字节空间
	vu16 LastDmaCurrCnt;	//DMA最后一次剩余的待传输数据数目
}QueuePrar;

//--------------------------------------------------------------------------
void QueueCreate(u8 *databuff, u16 space, QueuePrar *Qprar);
u16 QueueNdata(u16 DmaCurrCnt, QueuePrar *Qprar);
char QueueGetch(QueuePrar *Qprar);
bool QueueRead(u8 *databuff, QueuePrar *Qprar, vu16 lenth);
bool QueueRead2(u8 *databuff, QueuePrar *Qprar, vu16 lenth);
void QueueFlush(vu16 ndata, QueuePrar *Qprar);										//清除队列

//////////////////////////////////////////////////////////////////////////////////	 
#endif
