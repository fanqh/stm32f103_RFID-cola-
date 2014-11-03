#ifndef __KEY_H
#define __KEY_H	 
//////////////////////////////////////////////////////////////////////////////////	 

#include "stm32f10x.h"


//键盘指令
#define KEY_CMD_pow		1	//电源

#define KEY_CMD_BTsw	2	//启用蓝牙开关
#define KEY_CMD_up		2	//向上

#define KEY_CMD_menu	0x04	//菜单

#define KEY_CMD_scan	0X08	//刷卡
//#define KEY_CMD_right	4	//向右
#define KEY_CMD_ok		0X08	//确认

#define KEY_CMD_back	0X10	//返回

#define KEY_CMD_bro		0X20	//进入检索界面
#define KEY_CMD_down	0X20	//向下

#define KEY_TEST        0X25   //KEY3 和KEY6组合

u8 DetectKeys(void);	 						  
void keys_init(void);//IO初始化
u8 keys_scan(void);  //按键扫描函数
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
