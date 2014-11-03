#ifndef __CONFIG_H_
#define __CONFIG_H_


/////////////////////////////////////////
//客户名称
#define K_COSTUMER_COCACOLA	


////////////////////////////////////////
//蓝牙图标显示坐标
#define KEY_ICON_XY 120,0   	//按钮图标位置
#define BT_ICON_XY 96,0   		//蓝牙图标位置
#define BATT_ICON_XY 112,0   	//电池图标位置


///////////////////////////////////////////////////////////
//w25x16中地址分配
#define FILE_PARA_ADDR		0x0000	//参数地址
#define FILE_DATA_ADDR		0x1000	//文件数据地址
#define FILE_DATE_ENDADDR   FILE_DATA_ADDR + (RECORD_MAX_COUNT-1)*sizeof(FILE_RECORD_STRUCT)//数据最后地址

#define FILE_SETION_START_DATA_ADDR    (FILE_DATE_ENDADDR / 4096 + 1) * 4096 
#define FILE_SETION_END_DATA_ADDR	    FILE_SETION_START_DATA_ADDR + 4096 * 10	 ///   10

#define SEARCH_MAX_NUM     100

////////////////////////////////////////////////////////////
//STM32 FLASH 作计数器
#define PLACELASTID	  	 0x801F000	 //记录最后一次存入id的Id地址
#define POINTOFSECTION   0X801FC00	 //一分钟表格区指针

#define FLASE_ID_ADDR 	 0x801F800	 //本机可修改ID地址



///////////////////////////////////
//电池电量图标显示参考电压
#define BATT_HIGH  4000
#define BATT_MID_H 3850
#define BATT_MID   3600
#define BATT_MID_L 3350
#define BATT_LOW   3300
#define CHARGEFULL   500	 //电池充电满，chrg电平	 0.5v
#define TIMECHARGFLASH  50   //电池充电。闪烁时间

////////////////////////////////////
//系统时间定义
#ifndef K_COSTUMER_COCACOLA
	
	#define TIME30S             120000       //2min无操作时关机时间
	#define TIMEMAX             0            //60秒，ram中数据保持时间	
#else
	#define TIME30S 	 		30000*2*5   //5min无操作时关机时间	30000*2*5
	#define TIMEMAX             60          //60秒，ram中数据保持时间
#endif
		

#define TIME10S        		    10000      //ID号查询，显示ID号10s内无操作关机
#define RRT_TIME_10S	        10000      //10s钟 刷新屏幕时间
#define LISTSIZE	            100	       //记录的循环计数器的最大计数值（LISTSIZE>(TIMEMAX时间内扫描的最多id个数))
 
#define RECORD_MAX_COUNT	    9999       //最大记录数
#define SCAN_TIME_OUT           20	       //刷卡超时次数，定义次数内扫描不到数据，宣布失败


#endif


