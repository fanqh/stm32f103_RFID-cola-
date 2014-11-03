#ifndef __MYSHOW_
#define __MYSHOW_

typedef struct
{
	u8 locax;
	u8 locay;
	u8 fontw;
	u8 fonth;
}LOCATION;

#define  FIRSMALLFONTH  0,0,1,1

void MyShowOled(LOCATION *loca1, LOCATION *loca2, char *sting);
void OledClearandShow(u8 x, u8 y, u8 fontwidth, u8 fontheigth, char *sting, u8 ClearEnable);

#endif

