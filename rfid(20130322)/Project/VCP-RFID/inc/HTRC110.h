#ifndef __HTRC110_H
#define __HTRC110_H
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h> 
#include <string.h> 
#include "stm32f10x.h"
#include "gpio_config.h"
#include "delay.h"	

/////////////////////////////////////////////////////////////////////////////
#define SCLK PAout(8) 
#define SDIN PCout(9) 
#define SDOUT PCin(8) 
#define PowerHTRC110 PBout(15) 


/*
#define GET_SAMP_TIME	0x02 
#define GET_CFG_PAGE	0x04 
#define READ_PHASE		0x08 
#define READ_TAG		0xE0 
#define WRITE_TAG_N		0x10 
#define WRITE_TAG		0xC0 
#define SET_CFG_PAGE	0x40 
#define SET_SAMP_TIME	0x80 
*/


/* Constants for 'Config_PageData' */
/* Page No. */
#define CF_PAGE_0    (0)
#define CF_PAGE_1   (16)
#define CF_PAGE_2   (32)
#define CF_PAGE_3   (48)

/*Constants for configuration page 0*/
#define GAIN1        (8)
#define GAIN0        (4)
#define FILTERH      (2)
#define FILTERL      (1)

/*Constants for configuration page 1*/
#define PD_MODE      (8)
#define PD           (4)
#define HYSTERESIS   (2)
#define TXDIS        (1)

/*Constants for configuration page 2*/
#define THRESET      (8)
#define ACQAMP       (4)
#define FREEZE1      (2)
#define FREEZE0      (1)

/*Constants for configuration page 3*/
#define DISLP1       (8)
#define DISSMARTCOMP (4)
#define FSEL1        (2)
#define FSEL0        (1) 


void Send_HTRC110_Cmd(u8 RcCmd);
u8 Receive_HTRC110_Data( void );
void HitagSSelectUid(u8 *uid, u8 *cSendData);
void SelectUID_Ready(u8 *pid);

#define WRITE_TAG_N( N ) Send_HTRC110_Cmd( 0x10| (N) )
#define SET_CONFIG_PAGE(Config_PageData) Send_HTRC110_Cmd( 0x40| (Config_PageData) )
#define SET_SAMPLING_TIME( SampleTime ) Send_HTRC110_Cmd( 0x80| (SampleTime) )

u8 GET_CONFIG_PAGE(u8 Config_PageNo);
u8 GET_SAMPLING_TIME(void);
u8 READ_PHASE(void);
bool Ast_Adjust(u8 t_offset);
void Fast_Settling(void);
void Write_Settling (void);
void READ_TAG(void);    
void WRITE_TAG(void);

u8 HTRC110_Init(u8 CardStyle);		//初始化HTRC110
void HTRC110_Close(void);			//关闭HTRC110

 

void HTRC110_EmCardReady(void);		//HTRC110为EM卡准备好读信号  
void HTRC110_TitagReady(void);
//////////////////////////////////////////////////////////////////////
#endif
