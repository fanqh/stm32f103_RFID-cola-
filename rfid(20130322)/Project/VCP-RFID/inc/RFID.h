#ifndef __RFID_H
#define __RFID_H

#include <stdio.h> 
#include <string.h> 
#include "stm32f10x.h"
#include "delay.h"	
#include "HTRC110.h" 

#define erro         0xfe
#define SOUT_TIMEOUT 0xFF

///MC�������
#define SUBCYCLE       122	//��������
#define BITCYCLE       244	//bit����
#define UPLIMIT        60    //������120+60ʱ��Ϊ����������
#define DOWNLIMIT      60    //������120-60ʱ��Ϊ����Ϸ�
#define FIRSTCYCLE      10	 //��һ����Ч��ƽ����
//AC�������
#define LOWLENGTH    120+30
#define HIGHLENGTH	 240+30	  
#define TIMEOUT      5000  //5mS


#define AC_FIRST_TIMEOUT	5000	//����һ�������ó�ʱ5mS
#define AC_SHORT_TIMEOUT	181		//խ���峬ʱ   183
#define AC_LONG_TIMEOUT		732		//�����峬ʱ
#define AC_ERR			0xFF

u32 RFID_Read(u8 CardStyle);

#endif

 
