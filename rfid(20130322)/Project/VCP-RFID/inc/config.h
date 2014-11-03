#ifndef __CONFIG_H_
#define __CONFIG_H_


/////////////////////////////////////////
//�ͻ�����
#define K_COSTUMER_COCACOLA	


////////////////////////////////////////
//����ͼ����ʾ����
#define KEY_ICON_XY 120,0   	//��ťͼ��λ��
#define BT_ICON_XY 96,0   		//����ͼ��λ��
#define BATT_ICON_XY 112,0   	//���ͼ��λ��


///////////////////////////////////////////////////////////
//w25x16�е�ַ����
#define FILE_PARA_ADDR		0x0000	//������ַ
#define FILE_DATA_ADDR		0x1000	//�ļ����ݵ�ַ
#define FILE_DATE_ENDADDR   FILE_DATA_ADDR + (RECORD_MAX_COUNT-1)*sizeof(FILE_RECORD_STRUCT)//��������ַ

#define FILE_SETION_START_DATA_ADDR    (FILE_DATE_ENDADDR / 4096 + 1) * 4096 
#define FILE_SETION_END_DATA_ADDR	    FILE_SETION_START_DATA_ADDR + 4096 * 10	 ///   10

#define SEARCH_MAX_NUM     100

////////////////////////////////////////////////////////////
//STM32 FLASH ��������
#define PLACELASTID	  	 0x801F000	 //��¼���һ�δ���id��Id��ַ
#define POINTOFSECTION   0X801FC00	 //һ���ӱ����ָ��

#define FLASE_ID_ADDR 	 0x801F800	 //�������޸�ID��ַ



///////////////////////////////////
//��ص���ͼ����ʾ�ο���ѹ
#define BATT_HIGH  4000
#define BATT_MID_H 3850
#define BATT_MID   3600
#define BATT_MID_L 3350
#define BATT_LOW   3300
#define CHARGEFULL   500	 //��س������chrg��ƽ	 0.5v
#define TIMECHARGFLASH  50   //��س�硣��˸ʱ��

////////////////////////////////////
//ϵͳʱ�䶨��
#ifndef K_COSTUMER_COCACOLA
	
	#define TIME30S             120000       //2min�޲���ʱ�ػ�ʱ��
	#define TIMEMAX             0            //60�룬ram�����ݱ���ʱ��	
#else
	#define TIME30S 	 		30000*2*5   //5min�޲���ʱ�ػ�ʱ��	30000*2*5
	#define TIMEMAX             60          //60�룬ram�����ݱ���ʱ��
#endif
		

#define TIME10S        		    10000      //ID�Ų�ѯ����ʾID��10s���޲����ػ�
#define RRT_TIME_10S	        10000      //10s�� ˢ����Ļʱ��
#define LISTSIZE	            100	       //��¼��ѭ����������������ֵ��LISTSIZE>(TIMEMAXʱ����ɨ������id����))
 
#define RECORD_MAX_COUNT	    9999       //����¼��
#define SCAN_TIME_OUT           20	       //ˢ����ʱ���������������ɨ�費�����ݣ�����ʧ��


#endif


