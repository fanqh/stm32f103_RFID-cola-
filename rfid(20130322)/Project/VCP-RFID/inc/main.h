#ifndef __MAIN_
#define __MAIN_




typedef	struct
{
	u16 cmd;
	u16 lenth;
	u32 uid;
	u32 time;
	u8 crc1;
	u8 crc2;
	u16 eof;
}BTSEND;


typedef	struct
{
	u16 cmd;
	u16 lenth;
	u8 crc1;
	u8 crc2;
	u16 eof;
}BtReceiveNoDateCMD;

#endif



