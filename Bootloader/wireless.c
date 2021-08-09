#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lpc23xx.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "timer.h"

#include "wireless.h"
#include "serial.h"
#include "spiflash.h"
#include "watchdog.h"

static ePLATFORM_TYPE eMyPlatformType;
static unsigned int nOurESN;
#if 0
typedef enum ePullup
{
	ePULLUP_OFF,
	ePULLUP_HIGH,
	ePULLUP_LOW
}ePULLUP_STATE;
static void wirelessSetIOPullup(ePULLUP_STATE ePullup)
{
	switch(eMyPlatformType)
	{
		case ePLATFORM_TYPE_ARROW_BOARD:
			switch(ePullup)
			{
				case ePULLUP_OFF:
					PINMODE1 &= (~ARROW_BOARD_RF_IO_PINMODE_MASK);
					PINMODE1 |= ARROW_BOARD_RF_IO_PULLUP_OFF;
					break;
				case ePULLUP_HIGH:
					PINMODE1 &= (~ARROW_BOARD_RF_IO_PINMODE_MASK);
					PINMODE1 |= ARROW_BOARD_RF_IO_PULLUP_ENABLE;
					break;
				case ePULLUP_LOW:
					PINMODE1 &= (~ARROW_BOARD_RF_IO_PINMODE_MASK);
					PINMODE1 |= ARROW_BOARD_RF_IO_PULLDOWN_ENABLE;
					break;
			}
			break;
		case ePLATFORM_TYPE_HANDHELD:	
			switch(ePullup)
			{
				case ePULLUP_OFF:
					PINMODE3 &= (~HANDHELD_RF_IO_PINMODE_MASK);
					PINMODE3 |= HANDHELD_RF_IO_PULLUP_OFF;
					break;
				case ePULLUP_HIGH:	
					PINMODE3 &= (~HANDHELD_RF_IO_PINMODE_MASK);
					PINMODE3 |= HANDHELD_RF_IO_PULLUP_ENABLE;
					break;
				case ePULLUP_LOW:
					PINMODE3 &= (~HANDHELD_RF_IO_PINMODE_MASK);
					PINMODE3 |= HANDHELD_RF_IO_PULLDOWN_ENABLE;
					break;
			}			
			break;
	}		
}
#endif
static void wirelessSetLRNDirection(int nDirection)
{
	switch(eMyPlatformType)
	{
		case ePLATFORM_TYPE_ARROW_BOARD:
			if(nDirection)
			{
				FIO0DIR |= (OUT << ARROW_BOARD_RF_LRN);
			}
			else
			{
				FIO0DIR &= ~(OUT<<ARROW_BOARD_RF_LRN);
			}
			break;
		case ePLATFORM_TYPE_HANDHELD:
			if(nDirection)
			{
				FIO1DIR |= (OUT << HANDHELD_RF_LRN);
			}
			else
			{
				FIO1DIR &= ~(OUT<<HANDHELD_RF_LRN);
			}
			break;
		}
}
static void wirelessSetLRNState(int nState)
{
	switch(eMyPlatformType)
	{
		case ePLATFORM_TYPE_ARROW_BOARD:
			if(nState)
			{
				FIO0SET = (1<<ARROW_BOARD_RF_LRN);
			}
			else
			{
				FIO0CLR = (1<<ARROW_BOARD_RF_LRN);
			}
			break;
		case ePLATFORM_TYPE_HANDHELD:
			if(nState)
			{
				FIO1SET = (1<<HANDHELD_RF_LRN);
			}
			else
			{
				FIO1CLR = (1<<HANDHELD_RF_LRN);
			}
			break;
	}
}
#if 0
static void wirelessSetLRNPullup(ePULLUP_STATE ePullup)
{
	switch(eMyPlatformType)
	{
		case ePLATFORM_TYPE_ARROW_BOARD:
			switch(ePullup)
			{
				case ePULLUP_OFF:
					PINMODE1 &= (~ARROW_BOARD_RF_LRN_PINMODE_MASK);
					PINMODE1 |=  ARROW_BOARD_RF_LRN_PULLUP_OFF;
					break;
				case ePULLUP_HIGH:
					PINMODE1 &= (~ARROW_BOARD_RF_LRN_PINMODE_MASK);
					PINMODE1 |=  ARROW_BOARD_RF_LRN_PULLUP_ENABLE;
					break;
				case ePULLUP_LOW:
					PINMODE1 &= (~ARROW_BOARD_RF_LRN_PINMODE_MASK);
					PINMODE1 |=  ARROW_BOARD_RF_LRN_PULLDOWN_ENABLE;
					break;
			}
			break;
		case ePLATFORM_TYPE_HANDHELD:	
			switch(ePullup)
			{
				case ePULLUP_OFF:
					PINMODE3 &= (~HANDHELD_RF_LRN_PINMODE_MASK);
					PINMODE3 |=  HANDHELD_RF_LRN_PULLUP_OFF;
					break;
				case ePULLUP_HIGH:
					PINMODE3 &= (~HANDHELD_RF_LRN_PINMODE_MASK);
					PINMODE3 |=  HANDHELD_RF_LRN_PULLUP_ENABLE;				
					break;
				case ePULLUP_LOW:
					PINMODE3 &= (~HANDHELD_RF_LRN_PINMODE_MASK);
					PINMODE3 |=  HANDHELD_RF_LRN_PULLDOWN_ENABLE;
					break;
			}			
			break;
	}		
}
#endif
static void wirelessAssertReset(BOOL bState)
{
	switch(eMyPlatformType)
	{
		case ePLATFORM_TYPE_ARROW_BOARD:
			if(bState)
			{
				FIO0CLR = (1<<ARROW_BOARD_RF_xRST);
			}
			else
			{
				FIO0SET = (1<<ARROW_BOARD_RF_xRST);
			}
			break;
		case ePLATFORM_TYPE_HANDHELD:	
			if(bState)
			{
				FIO1CLR = (1<<HANDHELD_RF_xRST);
			}
			else
			{
				FIO1SET = (1<<HANDHELD_RF_xRST);
			}		
			break;
	}	
}

static unsigned char wirelessGetLRNPin()
{
	BOOL bRetVal = FALSE;
	switch(eMyPlatformType)
	{
		case ePLATFORM_TYPE_ARROW_BOARD:
			if(0 != (FIO0PIN & (1<<ARROW_BOARD_RF_LRN)))
			{
				bRetVal = TRUE;
			}
			break;
		case ePLATFORM_TYPE_HANDHELD:
			if(0 != (FIO1PIN & (1<<HANDHELD_RF_LRN)))
			{
				bRetVal = TRUE;
			}
			break;
	}
	return bRetVal;
}
static unsigned char wirelessEraseESNS()
{
	BOOL bRetVal = TRUE;
	unsigned char nLRNPinValue = FALSE;
	
	TIMERCTL timerWait;
	initTimer(&timerWait);
	
	/////
	// bring LRN high
	/////
	wirelessSetLRNDirection(1);
	wirelessSetLRNState(1);
	
	/////
	// wait 10 seconds
	/////
	startTimer(&timerWait, 12*1000);	
	
	while(1)
	{
		if(isTimerExpired(&timerWait))
		{
			break;
		}
		watchdogFeed();
	}
	/////
	// bring LRN low
	/////
	wirelessSetLRNState(0);
	
	/////
	// wait for a few ms (> 10)
	// to give the RFD time to recognize this
	/////
	startTimer(&timerWait, 20);	
	while(1)
	{
		if(isTimerExpired(&timerWait))
		{
			break;
		}
		watchdogFeed();
	}
	
	/////
	// then make LRN an input
	// so we can tell when RFD is done
	/////
	wirelessSetLRNDirection(0);
		
	/////
	// monitor RF_LRN for up to 3 seconds
	//
	// if no change in that time then the erase failed
	/////
	startTimer(&timerWait, 3*1000);	
	nLRNPinValue = FALSE;
	while(1)
	{
		unsigned char nNewLRNValue = wirelessGetLRNPin();
		if(nLRNPinValue != nNewLRNValue)
		{
			/////
			// LRN pin changed
			// so ESNs are erased
			/////
			printf("ESNs Erased\n");
			break;
		}
		nLRNPinValue = nNewLRNValue;

		if(isTimerExpired(&timerWait))
		{
			/////
			// timeout, no change
			// so ESNs are not erased
			/////
			bRetVal = FALSE;
			printf("ESNs Not Erased\n");
			break;

		}
		watchdogFeed();
	}
	
	/////
	// leave LRN pin low
	/////
	wirelessSetLRNDirection(1);
	wirelessSetLRNState(0);
	return bRetVal;
}

static unsigned char wirelessLearnESNS()
{
	BOOL bRetVal = TRUE;
	unsigned char nLRNPinValue = FALSE;
	
	TIMERCTL timerWait;
	initTimer(&timerWait);
	/////
	// bring LRN high
	// enable RF_LRN pullup
	/////
printf("1-wirelessLearnESNS\n");
	/////
	// wait awhile so I can start the logic analizer.
	/////
	startTimer(&timerWait, 5*1000);	
	
	while(1)
	{
		if(isTimerExpired(&timerWait))
		{
			break;
		}
		watchdogFeed();
	}
printf("1A-wirelessLearnESNS X\n");
	wirelessAssertReset(TRUE);
printf("1A-wirelessLearnESNS X\n");
	wirelessSetLRNState(1);
	wirelessAssertReset(FALSE);
	wirelessSetLRNDirection(1);


	
	/////
	// wait 20 ms
	// for RFD to recognize this
	/////
	startTimer(&timerWait, 20);	
	
	while(1)
	{
		if(isTimerExpired(&timerWait))
		{
			break;
		}
		watchdogFeed();
	}
	/////
	// bring RF_LRN down
	// module should enter learning mode
	/////
	wirelessSetLRNState(0);
	
	//////
	// give the RFD time to recognize this state change
	/////
	startTimer(&timerWait, 20);	
	while(1)
	{
		if(isTimerExpired(&timerWait))
		{
			break;
		}
		watchdogFeed();
	}

	//////
	// then set LRN as input
	/////
	wirelessSetLRNDirection(0);
	
	/////
	// monitor RF_LRN for up to 10 seconds
	//
	// if no change in that time then the learn failed
	/////
	startTimer(&timerWait, 10*1000);	
	nLRNPinValue = FALSE;
	while(1)
	{
		unsigned char nNewLRNValue = wirelessGetLRNPin();
		if(nLRNPinValue != nNewLRNValue)
		{
			/////
			// LRN pin changed
			// so ESN was learned
			/////
			printf("2-wirelessLearnESNS\n");
			break;
		}
		nLRNPinValue = nNewLRNValue;

		if(isTimerExpired(&timerWait))
		{
			/////
			// timeout, no change
			// so ESNs not learned
			/////
			bRetVal = FALSE;
			printf("3-wirelessLearnESNS\n");
			break;
		}
		watchdogFeed();
	}
	
	/////
	// leave LRN pin low
	/////
	wirelessSetLRNDirection(1);
	wirelessSetLRNState(0);
	printf("X-wirelessLearnESNS\n");	
	return bRetVal;
}
static unsigned long wirelessReadOurESN()
{
	unsigned long nESN=0;
	BOOL bFound = FALSE;
	char szReadESN[100];
	unsigned char nData;
	int nIndex;
	int nLen;

	TIMERCTL timerWait;
	initTimer(&timerWait);
	serialDisableInterface(eINTERFACE_WIRELESS);

	/////
	// hold RF_xRST low
	// bring RF_LEARN high
	// then release reset
	/////
	wirelessAssertReset(TRUE);	
	wirelessSetLRNDirection(1);
	wirelessSetLRNState(1);
	wirelessAssertReset(FALSE);

	/////
	// wait 250 ms
	/////
	startTimer(&timerWait, 251);	
	
	while(1)
	{
		if(isTimerExpired(&timerWait))
		{
			
			break;
		}
		watchdogFeed();
	}
	/////
	// bring LRN back low
	/////
	wirelessSetLRNState(0);
	
	/////
	// now, send the string "READ ESN" to the wireless module
	/////
	strcpy(szReadESN, "READ ESN");
	nLen = strlen(szReadESN);
	for(nIndex=0;nIndex<nLen;nIndex++)
	{
			serialWriteWireless(szReadESN[nIndex]);
	}
	/////
	// read the received string
	// in the form:
	// 314CE686:RFDP8 v1.2 11/18/08 08:45:16$
	/////
	startTimer(&timerWait, 1000);		
	nIndex = 0;
	memset(szReadESN, 0, sizeof(szReadESN));
	while(1)
	{
		if(serialReadWireless(&nData))
		{
			szReadESN[nIndex++] = nData;
			if('$' == nData)
			{
				bFound = TRUE;
				break;
			}
		}
		if(isTimerExpired(&timerWait))
		{
			break;
		}
		watchdogFeed();
	}
	
	if(bFound)
	{
		/////
		// grab the ESN
		// Keil strtol can't handle this large value directly
		// so break it into two peices
		/////
		char szESN1[5];
		char szESN2[5];

		strncpy(szESN1, szReadESN, 4);
		szESN1[4] = '\0';
		strncpy(szESN2, &szReadESN[4], 4);
		szESN2[4] = '\0';
		nESN = strtol(szESN1, NULL, 16);
		nESN <<= 16;
		nESN |= strtol(szESN2, NULL, 16);
		printf("ESN[%s][%lX]\n", szReadESN, nESN);
	}

	serialEnableInterface(eINTERFACE_WIRELESS);
	
	/////
	// leave LRN pin low
	/////
	wirelessSetLRNDirection(1);
	wirelessSetLRNState(0);	
	
	//nESN = wirelessSwapDword(nESN);
	
	//printf("ESN[%s][%lX]\n", szReadESN, nESN);
	
	return nESN;
}
void wirelessSetUARTMode()
{
	///////
	// initialize RF Digital module
	// RF_xRST, RF_M0, RF_M1, RF_M2 are outputs
	// RF_LRN & RF_IO are inputs
	// 
	// RF_xRST = 1	// take out of reset
	// RF_IO=0			// logic I/O pin low
	//
	// set UART (Non network mode mode
	// RF_M0=0
	// RF_M1=1
	// RF_M2=0
	/////
	switch(eMyPlatformType)
	{
		case ePLATFORM_TYPE_ARROW_BOARD:
			FIO0CLR = (1<<ARROW_BOARD_RF_xRST);
			FIO0CLR = (1<<ARROW_BOARD_RF_M0);
			FIO0SET = (1<<ARROW_BOARD_RF_M1);
			FIO0CLR = (1<<ARROW_BOARD_RF_M2);
			FIO0CLR = (1<<ARROW_BOARD_RF_IO);
			FIO0SET = (1<<ARROW_BOARD_RF_xRST);

			break;
		case ePLATFORM_TYPE_HANDHELD:
			FIO1CLR = (1<<HANDHELD_RF_xRST);
			FIO1CLR = (1<<HANDHELD_RF_M0);
			FIO1SET = (1<<HANDHELD_RF_M1);
			FIO1CLR = (1<<HANDHELD_RF_M2);
			FIO1CLR = (1<<HANDHELD_RF_IO);
			FIO1SET = (1<<HANDHELD_RF_xRST);			
			break;
	}

}

void wirelessSetESNTeachMode()
{
	///////
	// initialize RF Digital module
	// RF_xRST, RF_M0, RF_M1, RF_M2 are outputs
	// RF_LRN & RF_IO are inputs
	// 
	// RF_xRST = 1	// take out of reset
	// RF_IO=0			// logic I/O pin low
	//
	// set Mode 0, send ESN every 2 seconds
	// RF_M0=0
	// RF_M1=0
	// RF_M2=0
	/////
	switch(eMyPlatformType)
	{
		case ePLATFORM_TYPE_ARROW_BOARD:
			FIO0CLR = (1<<ARROW_BOARD_RF_xRST);
			FIO0SET = (1<<ARROW_BOARD_RF_M0|1<<ARROW_BOARD_RF_M1);
			FIO0CLR = (1<<ARROW_BOARD_RF_M2);
			FIO0CLR = (1<<ARROW_BOARD_RF_IO);
			FIO0SET = (1<<ARROW_BOARD_RF_xRST);

			break;
		case ePLATFORM_TYPE_HANDHELD:
			FIO1CLR = (1<<HANDHELD_RF_xRST);
			FIO1SET = ((1<<HANDHELD_RF_M0)|(1<<HANDHELD_RF_M1));
			FIO1CLR = (1<<HANDHELD_RF_M2);
			FIO1CLR = (1<<HANDHELD_RF_IO);
			FIO1SET = (1<<HANDHELD_RF_xRST);			
			break;
	}
}

unsigned long wirelessGetOurESN()
{
	return nOurESN;
}

unsigned short wirelessGetConfig()
{
	unsigned short nConfig = 0;
	return nConfig;
}
void wirelessInit(ePLATFORM_TYPE ePlatformType)
{
	eMyPlatformType = ePlatformType;

	
	switch(eMyPlatformType)
	{
		case ePLATFORM_TYPE_ARROW_BOARD:
			FIO0DIR |= (OUT<<ARROW_BOARD_RF_xRST)|(OUT<<ARROW_BOARD_RF_M0)|(OUT<<ARROW_BOARD_RF_M1)|(OUT<<ARROW_BOARD_RF_M2);
			/////
			// io pin output
			// hold it low
			/////
			FIO0DIR |= (1<<ARROW_BOARD_RF_IO);
			FIO0CLR = (1<<ARROW_BOARD_RF_IO);
			break;
		case ePLATFORM_TYPE_HANDHELD:
			FIO1DIR |= (OUT<<HANDHELD_RF_xRST)|(OUT<<HANDHELD_RF_M0)|(OUT<<HANDHELD_RF_M1)|(OUT<<HANDHELD_RF_M2);
			/////
			// io pin output
			// hold it low
			/////	
			FIO1DIR |= (1<<HANDHELD_RF_IO);	
			FIO1CLR = (1<<HANDHELD_RF_IO);
			break;
	}	
	
		/////
	// setup for UART mode
	/////
	wirelessSetUARTMode();
	
	/////
	// read our ESN
	/////
	nOurESN = wirelessReadOurESN();
	
	printf("X-wirelessInit [%08X]\n", nOurESN);
}

void wirelessDoWork()
{
	
}
BOOL wirelessDoBondLearn()
{
	BOOL bRetVal = FALSE;
	serialDisableInterface(eINTERFACE_WIRELESS);
	if(wirelessEraseESNS())
	{
		if(wirelessLearnESNS())
		{
			bRetVal = TRUE;
		}
	}
	serialEnableInterface(eINTERFACE_WIRELESS);
	return bRetVal;
}

DWORD wirelessSwapDword( DWORD Data ) 
{
	DWORD swappedData;
	
  swappedData = ((( Data & 0xFF000000 ) >> 24 ) |
								 (( Data & 0x00FF0000 ) >>  8 ) |
								 (( Data & 0x0000FF00 ) <<  8 ) |
								 (( Data & 0x000000FF ) << 24 ));
	
	return (swappedData);
}



