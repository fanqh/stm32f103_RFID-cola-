#ifndef __RFID_H
#define __RFID_H

#include <stdio.h> 
#include <string.h> 
#include "stm32f10x.h"
#include "delay.h"	
#include "HTRC110.h" 

#define erro         0xfe
#define SOUT_TIMEOUT 0xFF

///MC解码参数
#define SUBCYCLE       122	//子码周期
#define BITCYCLE       244	//bit周期
#define UPLIMIT        60    //当大于120+60时认为有两个子码
#define DOWNLIMIT      60    //当大于120-60时认为子码合法
#define FIRSTCYCLE      10	 //第一个有效电平长度
//AC解码参数
#define LOWLENGTH    120+30
#define HIGHLENGTH	 240+30	  
#define TIMEOUT      5000  //5mS


#define AC_FIRST_TIMEOUT	5000	//检测第一个脉冲用超时5mS
#define AC_SHORT_TIMEOUT	181		//窄脉冲超时   183
#define AC_LONG_TIMEOUT		732		//宽脉冲超时
#define AC_ERR			0xFF

u32 RFID_Read(u8 CardStyle);

#endif

 
