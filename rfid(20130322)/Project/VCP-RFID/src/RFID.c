#include "RFID.h" 

u32 timeLength;
u8 currentLevel = 0;
u8 isFirstLevel;
u8 returncodeflag = 0; 

u16 tttl, ttth;			//检测脉冲宽度用


//////////////////////////////////////////////////////////////////////////////
//EM CARD 信号
//////////////////////////////////////////////////////////////////////////////
//----检测信号跳动方向1/0------------------------------------------------------
u8 Jump2270(void)
{
	s16 F, i;

	delay_us(375);
	F=SDOUT;
	//2mS内检测信号跳动
	for(i=0; i<5000; i++)
 	{
		if(F!=SDOUT)
			return F;
	    delay_us(1);
	}
 	return SOUT_TIMEOUT;
}

//----读取卡号-----------------------------------------------------------------
u32 Read2270(void)		//读取2270ID----8.000MHz
{
	u8 F, i, j, ptr[12], buff[12], Data, rtc;
	u8 count;

	//检测50次 连续9个“1”
	i=0;
	j=0;
	do
	{
		F=Jump2270();
		if(F==1) 
			i++;
		else
		if(F==SOUT_TIMEOUT)
			return 0;
		else
		{
			i=0;
			j++;
		}
		if(j>100) 
			return 0;
	}
	while(i<9);

	//读取ID，横向校验
	for(i=0; i<10; i++)
	{
		Data=0;
 		rtc=1;
 		for(j=0; j<4; j++)
		{
			Data<<=1;
			if((count=Jump2270())==1)
			{
				Data|=1;
		  		rtc^=1;
			}
			else
			{
		 		if(count==0xff)
		   			return 0;
		 		rtc^=0;
			} 
		} 
		rtc^=1;
		if(Jump2270()!=rtc) //校验错误
			return 0;	
		buff[i]=Data;
	}
	//取校验码
	Data=0;
	for(j=0; j<4; j++)
	{
		Data<<=1;
		if(Jump2270())
	 	Data|=1;
 	}
 	//纵向校验
	for(i=0; i<10; i++)
		Data^=buff[i];
	if(Data!=0)	  //校验错误
		return 0;

	//合并5个ID字节	
	for(i=0; i<5; i++)
 		buff[i]=(buff[i*2]<<4) + (buff[i*2+1]);
	for(i=0; i<5; i++)
		ptr[4-i]=buff[i];

	return *((u32*)ptr);
} 

/*/////////////////////////////////////////////////////////////////////////////
//2KBit/S AC码信号
//////////////////////////////////////////////////////////////////////////////
//----检测脉冲宽度-------------------------------------------------
u8 AC_PulsWidth()
{
	u32 t;

	//等待低电平
	StartSysTime(); 	//开始记时
	while(1)
	{
		if(SDOUT==0)
			break;
	    t = GetSysInterval();
		if(t>TIMEOUT)
		 	return SOUT_TIMEOUT;
	}
	//等待高电平
	StartSysTime(); //开始记时 
	while(1)
	{
		t = GetSysInterval();
		if(t>TIMEOUT)
		 	return SOUT_TIMEOUT;
		if(SDOUT==1)
			break;
	}
	if(t<LOWLENGTH)		//是窄脉冲
		return 1;
	if(t<HIGHLENGTH)		//是宽脉冲
		return 0;
	return SOUT_TIMEOUT;	//其余的无效
}

	
//----检测信号1/0--------------------------------------------------------------
//input:	non
//output:	non
//return:	1/0=Code，0xFF=超时
u8 AC_BitData()
{
	u8 F;

	F=AC_PulsWidth();
	if(F==1)
	{
		F=AC_PulsWidth();
		if(F==1)
			return 1;
		if(F==0)
			return 0;
		else
			return SOUT_TIMEOUT;
	}
	else
		return F;
}  */

 //----检测AC码第一个下降沿------------------------------------------------------
bool AC_First_Failling()
{
	//如果是低电平，返回无效
	if(SDOUT == 0)
 		return TRUE;	

	//如果是高电平，要等待低电平作为开始
	StartSysTime(); 	//开始记时
	while(1)
	{
		if(SDOUT==0)	//检测到下降沿!!!
			break;
    	ttth = GetSysInterval();
		if(ttth > AC_FIRST_TIMEOUT)
	 		return FALSE;	//超时，没有下降沿
	}
	
	return TRUE;
}
//----检测信号1/0--------------------------------------------------------------
//return:	1/0=Code，0xFE=错误
u8 AC_BitData()
{
	//如果是高电平，返回错误
	if(SDOUT == 1)
 		return AC_ERR;
		
	//检测低电平宽度
	StartSysTime(); 	//开始记时 
	while(1)
	{
		tttl = GetSysInterval();
		if(tttl > AC_LONG_TIMEOUT)
		 	return AC_ERR;		//超时返回错误
		if(SDOUT == 1)
			break;
	}
		
	//是前个低电平窄脉冲，继续检测
	if( tttl < AC_SHORT_TIMEOUT )
	{
		//检测前个高电平窄脉冲
		StartSysTime(); 	//开始记时 
		while(1)
		{
			ttth = GetSysInterval();
			if( ttth > AC_SHORT_TIMEOUT )
			 	return AC_ERR;		//超时返回错误
			if(SDOUT == 0)
				break;
		}	
		//检测后个低电平窄脉冲
		StartSysTime(); 	//开始记时 
		while(1)
		{
			tttl = GetSysInterval();
			if( tttl > AC_SHORT_TIMEOUT )
			 	return AC_ERR;		//超时返回错误
			if(SDOUT == 1)
				break;
		}	
		//检测后个高电平窄脉冲
		StartSysTime(); 	//开始记时 
		while(1)
		{
			ttth = GetSysInterval();
			if( (SDOUT == 0) || (ttth > AC_SHORT_TIMEOUT) )
			 	return 1;			//1 code
		}	
	}	

	//是宽脉冲，等待下降沿或超时
	else	
	{
		StartSysTime(); 	//开始记时 
		while(1)
		{
			ttth = GetSysInterval();
			if( (SDOUT == 0) || (ttth > AC_LONG_TIMEOUT) )
			 	return 0;			//0 code
		}
	}
}  

//----读取卡号-----------------------------------------------------------------
void ReadTitagUID(u8 *pback, u8 bytenum, u8 sof_num)
{
	u8 F;

	u16 i;
	u8 temp;
	u8 lenth;
	u8 *p;
	
	temp = sof_num;
	lenth = bytenum;
	p = pback;

	//检测AC码第一个下降沿
	if( !AC_First_Failling() )
		goto loop_err;

	//检测SOF,连续n个“1”
	while(temp--) //sof_num
	{
		F = AC_BitData();
		if( F != 1 ) 
			goto loop_err;
	}
	//读取UID
	while(lenth--)
	{
		for(i=0; i<8; i++)
		{
			(*pback) <<= 1;
			F = AC_BitData();
			if(F == AC_ERR )
				goto loop_err;
			if(F==1) 
				(*pback)|=1;
		}
		pback++;
    }
	return;

loop_err:
	memset( p, 0, bytenum );
} 
/*
//----读取卡号-----------------------------------------------------------------
void ReadTitagUID(u8 *pback, u8 bytenum, u8 sof_num)
{
	u16 i;
	u8 F;
	u8 temp;
	temp = sof_num;

	//检测SOF,连续n个“1”
	while(temp) //sof_num
	{
		if(AC_BitData()!=1) 
			return ;
		temp--;
	}
	//读取UID
	while(bytenum--)
	{
		for(i=0; i<8; i++)
		{
			(*pback)<<=1;
			F=AC_BitData();
			if(F==1) 
				(*pback)|=1;
			else
			if(F==SOUT_TIMEOUT)
				return ;
		}
		pback++;
    }
}  */
u8 confirmEdge(u8 beforelevel)
{
	u8 i;
	u32 time;
	u8 levelAdd = 0;

	for(i=0;i<3;i++)
	{ 
		time = GetSysInterval() + 1;
		while(GetSysInterval()<time) ; //延时1us
		if(SDOUT==1) 
			levelAdd++;					     
	}

	if(((levelAdd==3)&&(beforelevel==0))||((levelAdd==0)&&(beforelevel==1)))//确认为跳变
		return 1;
	else 
		return 0;
}
//////////////////////////////////////////////////////////////////////////////
//4KBit/S MC码信号	   周期为244us
//////////////////////////////////////////////////////////////////////////////
u8 DetectEdge(void)
{	
	u16 t;
	u8 level;

	if(isFirstLevel==0)
	{
		t = 0;

		while(1)
		{
			if(SDOUT==0)  //等待第一个有效低电平
			{
				currentLevel = 0;
				break;	 
			}
			delay_us(1);
			t++;
			if(t>TIMEOUT)
				return SOUT_TIMEOUT; //本帧数据失败	 
		}
		StartSysTime(); 	//开始记时
	} 
	
    while(1)  //跳变扫描，时间扫描
	{

		timeLength = GetSysInterval();

		if(timeLength > BITCYCLE+UPLIMIT)	 //超时	  如果大于1个周期没有边沿跳变240+30
		{
		//	StartSysTime(); 	//开始记时
			return SOUT_TIMEOUT; 
		}
		if((timeLength>SUBCYCLE+UPLIMIT)&&(returncodeflag==0))  //有连续子码 返回一个子码120+30
		{
		    
			returncodeflag = 1;
			return currentLevel;
		}
	
		if(SDOUT!=currentLevel) //检测到跳变	     				  
		{	
	    	if(confirmEdge(currentLevel)==1)//确认为跳变
			{  
				returncodeflag = 0; 	                                                                                            	
				if(((timeLength>FIRSTCYCLE)&&(isFirstLevel==0))||((isFirstLevel!=0)&&( timeLength>SUBCYCLE-DOWNLIMIT )))	
				{	 
					StartSysTime();
					
					isFirstLevel = 1;
					level = currentLevel;
					currentLevel = (~currentLevel)&0x01;	   //电平反转
						return level; //1个有效位
				}
			} 
		}
    }			 
}


 
u8 Receivebit(void)
{ 
  u8 sta;
  u8 buffer;
  u8 decodbyte;

  buffer = 0;
  for(sta=0; sta<2; sta++)
  {	
    decodbyte = DetectEdge();
	buffer = buffer<<1;
	if(decodbyte==SOUT_TIMEOUT)	
	   return erro;
	else
      buffer = (buffer|decodbyte)&0x3;	///解码bit
  }
      
  switch (buffer)
  {
	  case 1:	  //01
	       sta = 1;
	       break;
	  case 2 :   //10
	       sta = 0;
	       break;
	 default :
	       sta = erro;
		   break;
  }
  buffer = 0;
  return sta;
}

u8 ReadHitagselect(u8 *pback, u8 sof_num)
{
	u8 F;
	u8 temp;
	u16 t;
	u8 ByteMax;
	u8 ByteCount;

	t = 0;
	ByteCount = 0;
	ByteMax = 8; ///////////////最多接收字节数
	isFirstLevel = 0;
	temp = sof_num;	

	//检测SOF,连续n个“1”
	while(temp) //sof_num		
	{
		if(Receivebit()==1) 
			temp--;
		else 
		{
		    temp = sof_num;
		    isFirstLevel = 0;
			t++;
		}
		if(t>5)  //1000
		    return 0;
    }
	//读取UID
   while(ByteMax--)
   {

		for(t=0; t<8; t++)
		{
			(*pback)<<=1;
			F = Receivebit();
			if(F==1) 
				(*pback) |= 1 ;
			else
			if(F==erro)
				return  ByteCount;	
		}

		pback++;
	    ByteCount++;
   }
   return ByteCount;
}
 
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//----读取UID--------------------------------------------------------------
u32 RFID_Read(u8 CardStyle)	
{
	u8 uid[4];
	u8 uid0[4];
	u8 con[5];
	u8 state=0;
	u32 ID;
	u32 ID0;
	u32 id1,id2,id3;

	//EM卡
	if(CardStyle==1)
	{
		if(HTRC110_Init(CardStyle)==FALSE)
		    return 0; //初始化失败
	    delay_ms(50);				    //延时很重要，要等待磁场建立
		id1=Read2270();
		id2=Read2270();

		if(id1==0 && id2==0)  //&& id3==0
			return 0;
		if(id1==0xFFFFFFFF)
			return 0;
		ID=0;
		if(id1==id2)
			ID=id1;
		else
		{
			id3=Read2270();	
			if((id2==id3)||(id3==id1))
				ID=id3;
		}
	   	if(ID!=0)
		   return ID;
	}

	//Titag卡
	else
	if(CardStyle==2)
	{
		HTRC110_Init(CardStyle);
		HTRC110_TitagReady();		//HTRC110为HITAG准备好读信号
		ReadTitagUID(uid, 4, 3);
	
		HTRC110_TitagReady();		//HTRC110为HITAG准备好读信号
		ReadTitagUID(uid0, 4, 3);

		ID = ((u32)uid[0]<<24)|((u32)uid[1]<<16)|((u32)uid[2]<<8)|((u32)uid[3]);
		ID0 = ((u32)uid0[0]<<24)|((u32)uid0[1]<<16)|((u32)uid0[2]<<8)|((u32)uid0[3]);
		if((ID!=ID0)||(ID==0xFFFFFFFF))
		{
			return 0;
		}
		SelectUID_Ready(uid);
	    state = ReadHitagselect(con, 6);
	    if((state==4))	
		{
		  return ID;
		}
		  
	}

	return 0;
}
