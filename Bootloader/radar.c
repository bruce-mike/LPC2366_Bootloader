// radar.c

#include "shareddefs.h"


// Wanco radar data comes in following order

#define PRE_FIRST	0x02
#define PRE_SECOND  0x84
#define PRE_THIRD	0x01
// MPH 
#define POST_FIRST	0x01
#define POST_SECOND	0xAA
#define POST_THIRD	0x03



static BYTE bytePos = 0;

BYTE mph = 0;


BYTE radarParseMessage(BYTE data)
{
    BYTE bComplete = FALSE;
    
	switch(bytePos)
	{
		case 0:
			if(PRE_FIRST == data)
				bytePos = 1;
			else
				bytePos = 0;
			break;

		case 1:
			if(PRE_SECOND == data)
				bytePos = 2;
			else
				bytePos = 0;
			break;

		case 2:
			if(PRE_THIRD == data)
				bytePos = 3;
			else
				bytePos = 0;
			break;
		
		case 3:
			mph = data;
			bytePos = 0;
            bComplete = TRUE;        
			break;

		default:
			bytePos = 0;
			break;
	}
    
    return bComplete;
}


BYTE radarGetMPH()
{        
    return mph;
}

		


