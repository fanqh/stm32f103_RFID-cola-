#ifndef __KEY_H
#define __KEY_H	 
//////////////////////////////////////////////////////////////////////////////////	 

#include "stm32f10x.h"


//����ָ��
#define KEY_CMD_pow		1	//��Դ

#define KEY_CMD_BTsw	2	//������������
#define KEY_CMD_up		2	//����

#define KEY_CMD_menu	0x04	//�˵�

#define KEY_CMD_scan	0X08	//ˢ��
//#define KEY_CMD_right	4	//����
#define KEY_CMD_ok		0X08	//ȷ��

#define KEY_CMD_back	0X10	//����

#define KEY_CMD_bro		0X20	//�����������
#define KEY_CMD_down	0X20	//����

#define KEY_TEST        0X25   //KEY3 ��KEY6���

u8 DetectKeys(void);	 						  
void keys_init(void);//IO��ʼ��
u8 keys_scan(void);  //����ɨ�躯��
u8 keys_wait(u16 time);
					    
// Key using interface 
u8 GetKey1(void);
u8 GetKey2(void);
u8 GetKey3(void);
u8 GetKey4(void);
u8 GetKey5(void);
u8 GetKey6(void);
	  
/****** TODO: add more key here ******/




//////////////////////////////////////////////////////////////////////////////////	 
#endif
