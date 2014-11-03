#ifndef __OLED_H
#define __OLED_H	
//------------------------------------------------------------------------
		  	 
#include "stm32f10x.h"
#include "gpio_config.h"
#include "stdlib.h"

//OLEDģʽ����
//0:4�ߴ���ģʽ
//1:����8080ģʽ
#define OLED_MODE 1
		    						  
//-----------------OLED�˿ڶ���----------------  					   
//#define OLED_CS PCout(9)		ֱ�ӽӵ�
//#define OLED_RST  PBout(14)//��MINISTM32��ֱ�ӽӵ���STM32�ĸ�λ�ţ�	
#define OLED_RS PCout(11)
#define OLED_WR PCout(12)		  
#define OLED_RD PDout(2)

//PB0~7,��Ϊ������
#define DATAOUT(x) GPIOB->ODR=(GPIOB->ODR&0xff00)|(x&0x00FF); //���
  
//ʹ��4�ߴ��нӿ�ʱʹ�� 
#define OLED_SCLK PBout(0)
#define OLED_SDIN PBout(1)
		     
#define OLED_CMD  0	//д����
#define OLED_DATA 1	//д����
//OLED�����ú���
void OLED_WR_Byte(u8 dat,u8 cmd);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Refresh_Gram(void);		   
							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);
void OLED_ShowChar(u8 x,u8 y, u8 fontwidth, u8 fontheight, u8 chr);
//void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);
void OLED_ShowString(u8 x,u8 y, u8 fontwidth, u8 fontheight, char *str);
void OLED_whiteback(void);// ������ʾ�������ã�fan	
void OLE_blackback(void);//	������ʾ�������ã�fan
//----------------------------------------------------------------------- 
#endif  
	 



