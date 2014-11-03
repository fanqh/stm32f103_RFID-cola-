#ifndef __I2C_H
#define __I2C_H
//////////////////////////////////////////////////////////////////////////////////	 


#include "stm32f10x.h"


//IO��������	 
#define IIC_SCL    PCout(15) //SCL
#define IIC_SDA    PCout(14) //SDA	 
#define READ_SDA   PCin(14)  //����SDA 

//IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��
bool IICputc(u8 sla, u8 c);
bool IICwrite(u8 sla, u8 suba, u8 *s, u8 no);
bool IICwriteExt(u8 sla, u8 *s, u8 no);
bool IICgetc(u8 sla, u8 *c);
bool IICread(u8 sla, u8 suba, u8 *s, u8 no);
bool IICreadExt(u8 sla, u8 *s, u8 no);


//////////////////////////////////////////////////////////////////////////////////	 
#endif
















