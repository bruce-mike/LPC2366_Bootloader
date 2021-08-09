#include <string.h>
#include <stdio.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "i2c.h"
#include "timer.h"
#include "leddriver.h"
#include "mischardware.h"
#include "PCU.h"

static I2C_QUEUE_ENTRY myI2CQueueEntry;
static I2C_TRANSACTION myTransaction;
static I2C_TRANSACTION myTransaction1;

static TIMERCTL sampleTimer;

static PCUPACKET pcuPacket;

/////
// set state of LEDs
// variables are PCA9634 LEDOUT bits
/////
static eLED_CONFIG_BITS nStateComps;
static eLED_CONFIG_BITS nStateRDR;
static eLED_CONFIG_BITS nStateAux0;
static eLED_CONFIG_BITS nStateAux1;
static eLED_CONFIG_BITS nStateLedVlow;
static eLED_CONFIG_BITS nStateLedChrgr;
static eLED_CONFIG_BITS nStateLedSyst;
static eLED_CONFIG_BITS nStateLedAlarm;

static eLED_CONFIG_BITS nStateRow0;
static eLED_CONFIG_BITS nStateRow1;
static eLED_CONFIG_BITS nStateRow2;
static eLED_CONFIG_BITS nStateRow3;
static eLED_CONFIG_BITS nStateFan0;
static eLED_CONFIG_BITS nStateFan1;
static eLED_CONFIG_BITS nStateGps;
static eLED_CONFIG_BITS nStateModem;


static eLED_CONFIG_BITS nNewComps;
static eLED_CONFIG_BITS nNewRDR;
static eLED_CONFIG_BITS nNewAux0;
static eLED_CONFIG_BITS nNewAux1;
static eLED_CONFIG_BITS nNewLedVlow;
static eLED_CONFIG_BITS nNewLedChrgr;
static eLED_CONFIG_BITS nNewLedSyst;
static eLED_CONFIG_BITS nNewLedAlarm;

static eLED_CONFIG_BITS nNewRow0;
static eLED_CONFIG_BITS nNewRow1;
static eLED_CONFIG_BITS nNewRow2;
static eLED_CONFIG_BITS nNewRow3;
static eLED_CONFIG_BITS nNewFan0;
static eLED_CONFIG_BITS nNewFan1;
static eLED_CONFIG_BITS nNewGps;
static eLED_CONFIG_BITS nNewModem;

static void ledDriverAddConfigureI2CQueueEntry()
{
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&myTransaction);
		I2CQueueInitializeTransaction(&myTransaction1);
	
		//============
		// PCA9634 U3
		//============
		myTransaction.nSLA_W = PCA9634_SYS_LEDS_AND_PWM_RD;
		myTransaction.nSLA_R = 0;
		myTransaction.cOutgoingData[0] = MODE1_INC;
		myTransaction.cOutgoingData[1] = SYS_LEDS_CONFIG_MODEREG1;
		myTransaction.cOutgoingData[2] = SYS_LEDS_CONFIG_MODEREG2;
		myTransaction.nOutgoingDataLength = 3;
		myTransaction.nIncomingDataLength = 0;

		//============
		// PCA9634 U2
		//============
		myTransaction1.nSLA_W = PCA9634_SYS_LEDS_AND_PWM;
		myTransaction1.nSLA_R = 0;
		myTransaction1.cOutgoingData[0] = MODE1_INC;
		myTransaction1.cOutgoingData[1] = SYS_LEDS_CONFIG_MODEREG1;
		myTransaction1.cOutgoingData[2] = SYS_LEDS_CONFIG_MODEREG2;
		myTransaction1.nOutgoingDataLength = 3;
		myTransaction1.nIncomingDataLength = 0;
	
		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction);
		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction1);
}

static void ledDriverAddLedControlI2CQueueEntry()
{
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&myTransaction);
		I2CQueueInitializeTransaction(&myTransaction1);
	
		//============
		// PCA9634 U3
		//============
		myTransaction.nSLA_W = PCA9634_SYS_LEDS_AND_PWM_RD;
		myTransaction.nSLA_R = 0;
		myTransaction.cOutgoingData[0] = LEDOUT0;								// auto inc bit is set
		myTransaction.cOutgoingData[1] = nStateComps|(nStateRDR<<2)|(nStateAux0<<4)|(nStateAux1<<6);
		myTransaction.cOutgoingData[2] = nStateLedVlow|(nStateLedChrgr<<2)|(nStateLedSyst<<4)|(nStateLedAlarm<<6);
		myTransaction.nOutgoingDataLength = 3;
		myTransaction.nIncomingDataLength = 0;
			
		//============
		// PCA9634 U2
		//============
		myTransaction1.nSLA_W = PCA9634_SYS_LEDS_AND_PWM;
		myTransaction1.nSLA_R = 0;
		myTransaction1.cOutgoingData[0] = LEDOUT0;								// auto inc bit is set
		myTransaction1.cOutgoingData[1] = nStateRow0|(nStateRow1<<2)|(nStateRow2<<4)|(nStateRow3<<6);
		myTransaction1.cOutgoingData[2] = nStateFan0|(nStateFan1<<2)|(nStateGps<<4)|(nStateModem<<6);
		myTransaction1.nOutgoingDataLength = 3;
		myTransaction1.nIncomingDataLength = 0;
		
		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction);
		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction1);
}

static BOOL bInit;

void ledDriverInit()
{
	//============
	// PCA9634 U4
	//============
	nStateRow0 		 = eLED_ON;
	nStateRow1 		 = eLED_ON;
	nStateRow2 		 = eLED_ON;
	nStateRow3 		 = eLED_ON;
	nStateFan0 		 = eLED_ON;
	nStateFan1 		 = eLED_ON;
	nStateGps   	 = eLED_ON;
	nStateModem 	 = eLED_ON;
	
	//============
	// PCA9634 U5
	//============
	nStateComps    = eLED_ON;
	nStateRDR      = eLED_ON;
	nStateAux0     = eLED_ON;
	nStateAux1     = eLED_ON;
	nStateLedVlow  = eLED_ON;
	nStateLedChrgr = eLED_ON;
	nStateLedSyst  = eLED_ON;
	nStateLedAlarm = eLED_ON;
	
	//============
	// PCA9634 U4
	//============
	nStateRow0  	 = eLED_OFF;
	nStateRow1  	 = eLED_OFF;
	nStateRow2  	 = eLED_OFF;
	nStateRow3  	 = eLED_OFF;
	nStateFan0  	 = eLED_OFF;
	nStateFan1  	 = eLED_OFF;
	nStateGps   	 = eLED_OFF;
	nStateModem 	 = eLED_OFF;
	
	//============
	// PCA9634 U5
	//============
	nStateComps    = eLED_OFF;
	nStateRDR      = eLED_OFF;
	nStateAux0     = eLED_OFF;
	nStateAux1     = eLED_OFF;
	nStateLedVlow  = eLED_OFF;
	nStateLedChrgr = eLED_OFF;
	nStateLedSyst  = eLED_OFF;
	nStateLedAlarm = eLED_OFF;
	
	bInit = TRUE;
	
	initTimer(&sampleTimer);
	startTimer(&sampleTimer, LEDDRIVER_SAMPLE_TIME_MS);
	/////
	// schedule I2C transaction to configure the driver
	// leave all drivers off for now
	/////
	ledDriverAddConfigureI2CQueueEntry();
	I2CQueueAddEntry(&myI2CQueueEntry);
}

void ledDriverDoWork()
{
	switch(myI2CQueueEntry.eState)
	{

		case eI2C_TRANSFER_STATE_COMPLETE:
		case eI2C_TRANSFER_STATE_FAILED:
			{
				myI2CQueueEntry.eState = eI2C_TRANSFER_STATE_IDLE;
			}
			break;
		case eI2C_TRANSFER_STATE_IDLE:
			break;
		default:
			break;
	}
	
	if((eI2C_TRANSFER_STATE_IDLE == myI2CQueueEntry.eState) && isTimerExpired(&sampleTimer))
	{
				BOOL bNew = FALSE;
		
				/////
				// previous I2C queue entry is complete
				// if anything has changed
				// then schedule another
				/////
				if(bInit)
				{
					hwEnableSystemLedsAndIndicators();
					bInit = FALSE;
				}
				
				//============
				// PCA9634 U4
				//============
				if(nNewRow0 != nStateRow0)
				{
					nStateRow0 = nNewRow0;
					bNew = TRUE;
				}
				if(nNewRow1 != nStateRow1)
				{
					nStateRow1 = nNewRow1;
					bNew = TRUE;
				}
				if(nNewRow2 != nStateRow2)
				{
					nStateRow2 = nNewRow2;
					bNew = TRUE;
				}
				if(nNewRow3 != nStateRow3)
				{
					nStateRow3 = nNewRow3;
					bNew = TRUE;
				}
				if(nNewFan0 != nStateFan0)
				{
					nStateFan0 = nNewFan0;
					pcuPacket.pcuStatus.pcuFan0Status = nStateRow0;
					printf("FAN0 State[%d]\n",pcuPacket.pcuStatus.pcuFan0Status);
					bNew = TRUE;
				}
				if(nNewFan1 != nStateFan1)
				{
					nStateFan1 = nNewFan1;
					printf("FAN1 State[%d]\n",nStateFan1);
					bNew = TRUE;
				}
				if(nNewGps != nStateGps)
				{
					nStateGps = nNewGps;
					bNew = TRUE;
				}
				if(nNewModem != nStateModem)
				{
					nStateModem = nNewModem;
					bNew = TRUE;
				}
				
				//============
				// PCA9634 U5
				//============				
				if(nNewComps != nStateComps)
				{
					nStateComps = nNewComps;
					bNew = TRUE;
				}
				if(nNewRDR != nStateRDR)
				{
					nStateRDR = nNewRDR;
					bNew = TRUE;
				}
				if(nNewAux0 != nStateAux0)
				{
					nStateAux0 = nNewAux0;
					bNew = TRUE;
				}
				if(nNewAux1 != nStateAux1)
				{
					nStateAux1 = nNewAux1;
					bNew = TRUE;
				}
				if(nNewLedVlow != nStateLedVlow)
				{
					nStateLedVlow = nNewLedVlow;
					bNew = TRUE;
				}
				if(nNewLedChrgr != nStateLedChrgr)
				{
					nStateLedChrgr = nNewLedChrgr;
					bNew = TRUE;
				}
				if(nNewLedSyst != nStateLedSyst)
				{
					nStateLedSyst = nNewLedSyst;
					bNew = TRUE;
				}
				if(nNewLedAlarm != nStateLedAlarm)
				{
					nStateLedAlarm = nNewLedAlarm;
					bNew = TRUE;
				}

				if(bNew)
				{
					ledDriverAddLedControlI2CQueueEntry();
					I2CQueueAddEntry(&myI2CQueueEntry);
					startTimer(&sampleTimer, LEDDRIVER_SAMPLE_TIME_MS);
				}
	}
}

//============
// PCA9634 U4
//============
void ledDriverSetRow0(eLED_CONFIG_BITS eNew)
{
	nNewRow0 = eNew;
}
void ledDriverSetRow1(eLED_CONFIG_BITS eNew)
{
	nNewRow1 = eNew;
}
void ledDriverSetRow2(eLED_CONFIG_BITS eNew)
{
	nNewRow2 = eNew;
}
void ledDriverSetRow3(eLED_CONFIG_BITS eNew)
{
	nNewRow3 = eNew;
}
void ledDriverSetFan0(eLED_CONFIG_BITS eNew)
{
	nNewFan0 = eNew;
}
void ledDriverSetFan1(eLED_CONFIG_BITS eNew)
{
	nNewFan1 = eNew;
}
void ledDriverSetGps(eLED_CONFIG_BITS eNew)
{
	nNewGps = eNew;
}
void ledDriverSetModem(eLED_CONFIG_BITS eNew)
{
	nNewModem = eNew;
}

//============
// PCA9634 U5
//============
void ledDriverSetComps(eLED_CONFIG_BITS eNew)
{
	nNewComps = eNew;
}
void ledDriverSetRDR(eLED_CONFIG_BITS eNew)
{
	nNewRDR = eNew;
}
void ledDriverSetAux0(eLED_CONFIG_BITS eNew)
{
	nNewAux0 = eNew;
}
void ledDriverSetAux1(eLED_CONFIG_BITS eNew)
{
	nNewAux1 = eNew;
}
void ledDriverSetLedVlow(eLED_CONFIG_BITS eNew)
{
	nNewLedVlow = eNew;
}
void ledDriverSetLedChrgr(eLED_CONFIG_BITS eNew)
{
	nNewLedChrgr = eNew;
}
void ledDriverSetLedSyst(eLED_CONFIG_BITS eNew)
{
	nNewLedSyst = eNew;
}
void ledDriverSetLedAlarm(eLED_CONFIG_BITS eNew)
{
	nNewLedAlarm = eNew;
}

unsigned short ledDriverGetAuxStatus()
{
	unsigned short nStatus = 0;
	
	if(eLED_OFF != nNewComps)
	{
		nStatus |= AUX_STATUS_COMPS;
	}
	if(eLED_OFF != nNewRDR)
	{
		nStatus |= AUX_STATUS_RDR;
	}
	if(eLED_OFF != nNewAux0)
	{
		nStatus |= AUX_STATUS_AUX0;
	}
	if(eLED_OFF != nNewAux1)
	{
		nStatus |= AUX_STATUS_AUX1;
	}
	return nStatus;
}
