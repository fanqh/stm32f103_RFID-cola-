#include "DmaQueue.h"

//--------------------------------------------------------------------------
void QueueCreate(u8 *databuff, u16 space, QueuePrar *Qprar)
{
	Qprar->space = space;					//								
	Qprar->LastDmaCurrCnt = Qprar->space;	//DMA最后一次剩余的待传输数据数目
	Qprar->sta = databuff;								
	Qprar->out = databuff;					//数据输出地址
	Qprar->end = databuff + Qprar->space; 	//计算数据输出结束地址
	Qprar->ndata = 0;						//接收数据数目
}
//--------------------------------------------------------------------------
u16 QueueNdata(u16 DmaCurrCnt, QueuePrar *Qprar)
{
	long newdata;

	newdata = Qprar->LastDmaCurrCnt - DmaCurrCnt;	//实际传输数据数目
	Qprar->LastDmaCurrCnt = DmaCurrCnt;

	if(newdata<0)
		Qprar->ndata += ((u16)(newdata+Qprar->space));
	else
		Qprar->ndata += (u16)newdata;
	return Qprar->ndata;
}
//--------------------------------------------------------------------------
char QueueGetch(QueuePrar *Qprar)
{
	char ch;

   	ch = Qprar->out[0];				//数据出队
	Qprar->out++;					//调整出队指针
	if(Qprar->out >= Qprar->end)	//如果到达数据区的结束位置
		Qprar->out = Qprar->sta;	//指向数据区的开始位置
	return ch;
}
//--------------------------------------------------------------------------
bool QueueRead(u8 *databuff, QueuePrar *Qprar, u16 lenth)
{
	u32 len;

	if(lenth > Qprar->ndata) 			//空间不足
		return FALSE;
	
	len=lenth;
	while(len)
	{
    	*databuff = Qprar->out[0];		//数据出队
		databuff++;
		Qprar->out++;					//调整出队指针
		if(Qprar->out >= Qprar->end)
			Qprar->out = Qprar->sta;
		len--;
	}
	Qprar->ndata -= lenth;
	return TRUE;
}
//----不调整出队指针-----------------------------------------------------------------
bool QueueRead2(u8 *databuff, QueuePrar *Qprar, u16 lenth)
{
	u32 len;
	vu8 *up;

	if(!databuff)
		return FALSE;
	if(lenth > Qprar->ndata) 			//空间不足
		return FALSE;
	
	len=lenth;
	up=Qprar->out;
	while(len)
	{
    	*databuff = *up;		//数据出队
		databuff++;
		up++;					//调整出队指针
		if(up >= Qprar->end)
			up = Qprar->sta;
		len--;
	}
	return TRUE;
}
//----清除队列--------------------------------------------------------------
void QueueFlush(volatile u16 ndata, QueuePrar *Qprar)
{
	while(ndata)
	{
		if(Qprar->out >= Qprar->end)
			Qprar->out = Qprar->sta;
		Qprar->out++;
		ndata--;
	}
	Qprar->ndata = 0;
}

