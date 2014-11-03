#ifndef __HITAGS_H
#define __HITAGS_H

#include "HTRC110.h" 
//#include "RFID.h"

void HITAGS_Reast(void);				//HITAGSø®µÙµÁ∏¥Œª
void HITAGS_UID_Request(void);			//«Î«ÛUID
void field_switch(u8 *sw, vu8 nBit);


void EM_Reast(void);

#endif

