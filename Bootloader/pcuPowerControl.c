#include <string.h>
#include <stdio.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "i2c.h"
#include "timer.h"
#include "pcuPowerControl.h"
#include "mischardware.h"
#include "PCU.h"

static I2C_QUEUE_ENTRY myI2CQueueEntry;
static I2C_TRANSACTION myTransaction0;
static I2C_TRANSACTION myTransaction1;
static I2C_TRANSACTION myTransaction2;
static I2C_TRANSACTION myTransaction3;

static TIMERCTL sampleTimer;

/////
// set state of Power Control Outputs
// variables are PCA9634 POWEROUT bits
/////
static ePCU_POWER_CONTROL nStateDCU;
static ePCU_POWER_CONTROL nStateRadar;
static ePCU_POWER_CONTROL nStateAux0;
static ePCU_POWER_CONTROL nStateAux1;
static ePCU_POWER_CONTROL nStateLedVlow;
static ePCU_POWER_CONTROL nStateLedChrgr;
static ePCU_POWER_CONTROL nStateLedSyst;
static ePCU_POWER_CONTROL nStateLedAlarm;


static ePCU_POWER_CONTROL nStateRow0;
static ePCU_POWER_CONTROL nStateRow1;
static ePCU_POWER_CONTROL nStateRow2;
static ePCU_POWER_CONTROL nStateRow3;
static ePCU_POWER_CONTROL nStateFan0;
static ePCU_POWER_CONTROL nStateFan1;
static ePCU_POWER_CONTROL nStateGps;
static ePCU_POWER_CONTROL nStateModem;

static ePCU_POWER_CONTROL nNewDCU;
static ePCU_POWER_CONTROL nNewRadar;
static ePCU_POWER_CONTROL nNewAux0;
static ePCU_POWER_CONTROL nNewAux1;
static ePCU_POWER_CONTROL nNewLedVlow;
static ePCU_POWER_CONTROL nNewLedChrgr;
static ePCU_POWER_CONTROL nNewLedSyst;
static ePCU_POWER_CONTROL nNewLedAlarm;
                          
static ePCU_POWER_CONTROL nNewRow0 = ePOWER_OUTPUT_ON;
static ePCU_POWER_CONTROL nNewRow1;
static ePCU_POWER_CONTROL nNewRow2;
static ePCU_POWER_CONTROL nNewRow3;
static ePCU_POWER_CONTROL nNewFan0;
static ePCU_POWER_CONTROL nNewFan1;
static ePCU_POWER_CONTROL nNewGps;
static ePCU_POWER_CONTROL nNewModem;


/////
// these variables // are used to help stagger enabling power outputs
// so we can allow power to stabilize when bringing multiple outputs online
static TIMERCTL staggerPowerTimer;
static unsigned long bmPowerOnRequests = 0;
#define STAGGER_POWER_MS 50
//#define STAGGER_POWER_MS 2000


static ePCU_POWER_CONTROL pcuPowerGetOutputSetting(ePCU_POWER_CONTROL eOnOff, ePCU_POWER_CONTROL_SET_OUTPUT eOutput)
{

	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		default:
			break;
		case eBoardRev2:
		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			switch(eOutput)
			{
				case ePCU_SET_ROW0_OUTPUT:	
				case ePCU_SET_ROW1_OUTPUT:		
				case ePCU_SET_ROW2_OUTPUT:		
				case ePCU_SET_ROW3_OUTPUT:
				case ePCU_SET_DCU_OUTPUT:	
				case ePCU_SET_RADAR_OUTPUT:
				case ePCU_SET_AUX0_OUTPUT:
				case ePCU_SET_AUX1_OUTPUT:
				case ePCU_SET_GPS_OUTPUT:
				case ePCU_SET_MODEM_OUTPUT:
				case ePCU_SET_FAN0_OUTPUT:		
				case ePCU_SET_FAN1_OUTPUT:	
					//printf("Inverting output[%d] [%d]\n", eOnOff, eOutput);
					// in REVE PCU, these have inverters, so switch the value of on/off
						if(ePOWER_OUTPUT_OFF == eOnOff)
						{
							eOnOff = ePOWER_OUTPUT_ON;
						}
						else
						{
							eOnOff = ePOWER_OUTPUT_OFF;
						}
						break;
	
				case ePCU_SET_LED_VLOW_OUTPUT:
				case ePCU_SET_LED_CHRGR_OUTPUT:
				case ePCU_SET_LED_SYST_OUTPUT:
				case ePCU_SET_LED_ALARM_OUTPUT:
					break;
			}
			break;
			
	}
	return eOnOff;
}
static void pcuPowerControlAddConfigureI2CQueueEntry()
{
		unsigned char nMode2Reg = SYS_POWER_CONTROL_MODEREG2_REVC;
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&myTransaction2);
		I2CQueueInitializeTransaction(&myTransaction3);
	
		switch(hwGetBoardRevision())
		{
			case eBoardRev1:
			default:
				break;
			case eBoardRev2:
			case eBoardRev3:
			case eBoardRev4:
			case eBoardRev5:
			case eBoardRev6:
			case eBoardRev7:
				nMode2Reg = SYS_POWER_CONTROL_MODEREG2_REVE;
				break;
		}
//printf("1-ppca\n");
/////==============================
		//============
		// PCA9634 U8
		//============
		myTransaction2.nSLA_W = PCA9634_SYS_POWER_CONTROL_0;
		myTransaction2.nSLA_R = 0;
		myTransaction2.cOutgoingData[0] = MODE1_INC;
		myTransaction2.cOutgoingData[1] = SYS_POWER_CONTROL_MODEREG1;
		myTransaction2.cOutgoingData[2] = nMode2Reg;

		myTransaction2.nOutgoingDataLength = 3;
		myTransaction2.nIncomingDataLength = 0;
	
		//============
		// PCA9634 U9
		//============
		myTransaction3.nSLA_W = PCA9634_SYS_POWER_CONTROL_1;
		myTransaction3.nSLA_R = 0;
		myTransaction3.cOutgoingData[0] = MODE1_INC;
		myTransaction3.cOutgoingData[1] = SYS_POWER_CONTROL_MODEREG1;
		myTransaction3.cOutgoingData[2] = nMode2Reg;

		myTransaction3.nOutgoingDataLength = 3;
		myTransaction3.nIncomingDataLength = 0;
	
		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction2);
		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction3);

}

static void pcuPowerControlAddOutputControlI2CQueueEntry()
{
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&myTransaction0);
		I2CQueueInitializeTransaction(&myTransaction1);
	//printf("1-pcpA[%d][%d][%d][%d]\n", nStateRow0, nStateRow1, nStateRow2, nStateRow3);
	//printf("2-pcpA[%d][%d][%d][%d]\n", nStateFan0, nStateFan1, nStateGps, nStateModem);
	
	//printf("3-pcpA[%d][%d][%d][%d]\n", nStateDCU, nStateRadar, nStateAux0, nStateAux1);
	//printf("4-pcpA[%d][%d][%d][%d]\n", nStateLedVlow, nStateLedChrgr, nStateLedSyst, nStateLedAlarm);
			
		//============
		// PCA9634 U8
		//============
		myTransaction0.nSLA_W = PCA9634_SYS_POWER_CONTROL_0;
		myTransaction0.nSLA_R = 0;
		myTransaction0.cOutgoingData[0] = LEDOUT0;							// auto inc bit is set
        
// MJB    
		myTransaction0.cOutgoingData[1] = nStateRow0|(nStateRow1<<2)|(nStateRow2<<4)|(nStateRow3<<6); 
//myTransaction0.cOutgoingData[1] = ePOWER_OUTPUT_OFF|(ePOWER_OUTPUT_OFF<<2)|(ePOWER_OUTPUT_OFF<<4)|(ePOWER_OUTPUT_OFF<<6);
    
        myTransaction0.cOutgoingData[2] = nStateFan0|(nStateFan1<<2)|(nStateGps<<4)|(nStateModem<<6);
		myTransaction0.nOutgoingDataLength = 3;
		myTransaction0.nIncomingDataLength = 0;
	
		//============
		// PCA9634 U9
		//============
		myTransaction1.nSLA_W = PCA9634_SYS_POWER_CONTROL_1;
		myTransaction1.nSLA_R = 0;
		myTransaction1.cOutgoingData[0] = LEDOUT0;								// auto inc bit is set
		myTransaction1.cOutgoingData[1] = nStateDCU|(nStateRadar<<2)|(nStateAux0<<4)|(nStateAux1<<6);
		myTransaction1.cOutgoingData[2] = nStateLedVlow|(nStateLedChrgr<<2)|(nStateLedSyst<<4)|(nStateLedAlarm<<6);
		myTransaction1.nOutgoingDataLength = 3;
		myTransaction1.nIncomingDataLength = 0;

		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction0);
		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction1);
}

void pcuPowerControlReset()
{
		/////
		// send the sequencec A5 5A to the software reset I2C address
		/////
		int i;
		int j;
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&myTransaction2);
		myTransaction2.nSLA_W = PCA9634_SYS_POWER_CONTROL_SWRST;
		myTransaction2.nSLA_R = 0;
		myTransaction2.cOutgoingData[0] = SWRST_BYTE_1;
		myTransaction2.cOutgoingData[1] = SWRST_BYTE_2;

		myTransaction2.nOutgoingDataLength = 2;
		myTransaction2.nIncomingDataLength = 0;
		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction2);
		I2CQueueAddEntry(&myI2CQueueEntry);
	
		// now, wait around for an arbitrary time
		// to let this transaction happen
		for(i=0;i<1000;i++)
		{
			I2CQueueDoWork();
			for(j=0;j<1000;j++)
			{
			}
		}
}
void pcuPowerControlInit()
{
    
	initTimer(&staggerPowerTimer);
	
	//============
	// PCA9634 U4
	//============
    
	nStateRow0  	 = ePOWER_OUTPUT_INIT;
	nStateRow1  	 = ePOWER_OUTPUT_INIT;
	nStateRow2  	 = ePOWER_OUTPUT_INIT;
	nStateRow3  	 = ePOWER_OUTPUT_INIT;

	nStateFan0  	 = pcuPowerGetOutputSetting(ePOWER_OUTPUT_OFF, ePCU_SET_FAN0_OUTPUT);
	nStateFan1  	 = pcuPowerGetOutputSetting(ePOWER_OUTPUT_OFF, ePCU_SET_FAN1_OUTPUT);
	nStateGps   	 = pcuPowerGetOutputSetting(ePOWER_OUTPUT_OFF, ePCU_SET_GPS_OUTPUT);
	nStateModem 	 = pcuPowerGetOutputSetting(ePOWER_OUTPUT_OFF, ePCU_SET_MODEM_OUTPUT);
	
	//============
	// PCA9634 U5
	//============
	nStateDCU      = pcuPowerGetOutputSetting(ePOWER_OUTPUT_OFF, ePCU_SET_MODEM_OUTPUT);
	nStateRadar    = pcuPowerGetOutputSetting(ePOWER_OUTPUT_OFF, ePCU_SET_RADAR_OUTPUT);
	nStateAux0     = pcuPowerGetOutputSetting(ePOWER_OUTPUT_OFF, ePCU_SET_AUX0_OUTPUT);
	nStateAux1     = pcuPowerGetOutputSetting(ePOWER_OUTPUT_OFF, ePCU_SET_AUX1_OUTPUT);
	nStateLedVlow  = pcuPowerGetOutputSetting(ePOWER_OUTPUT_OFF, ePCU_SET_LED_VLOW_OUTPUT);
	nStateLedChrgr = pcuPowerGetOutputSetting(ePOWER_OUTPUT_OFF, ePCU_SET_LED_CHRGR_OUTPUT);
	nStateLedSyst  = pcuPowerGetOutputSetting(ePOWER_OUTPUT_OFF, ePCU_SET_LED_SYST_OUTPUT);
	nStateLedAlarm = pcuPowerGetOutputSetting(ePOWER_OUTPUT_OFF, ePCU_SET_LED_ALARM_OUTPUT);
	
	initTimer(&sampleTimer);
	startTimer(&sampleTimer, POWER_CONTROL_SAMPLE_TIME_MS);
	
	// schedule I2C transaction to configure the driver
	// leave all drivers off for now
	
	pcuPowerControlAddConfigureI2CQueueEntry();
	I2CQueueAddEntry(&myI2CQueueEntry);
}


//=================
// PCA9634 U4 & U5
//=================

static void pcuPowerControlDoSetOutput(ePCU_POWER_CONTROL epcuPowerControl, ePCU_POWER_CONTROL_SET_OUTPUT epcuPowerControlSetOutput)
{
	// invert the input/output logic if necessary
    
	epcuPowerControl = pcuPowerGetOutputSetting(epcuPowerControl, epcuPowerControlSetOutput);
    
	switch(epcuPowerControlSetOutput)
	{
		case ePCU_SET_ROW0_OUTPUT:
			nNewRow0 = epcuPowerControl;	
			break;
		case ePCU_SET_ROW1_OUTPUT:	
			nNewRow1 = epcuPowerControl;	
			break;
		case ePCU_SET_ROW2_OUTPUT:	
			nNewRow2 = epcuPowerControl;	
			break;
		case ePCU_SET_ROW3_OUTPUT:	
			nNewRow3 = epcuPowerControl;	
			break;
		case ePCU_SET_FAN0_OUTPUT:	
			nNewFan0 = epcuPowerControl;
			break;
		case ePCU_SET_FAN1_OUTPUT:	
			nNewFan1 = epcuPowerControl;
			break;
		case ePCU_SET_GPS_OUTPUT:	
			nNewGps = epcuPowerControl;
			break;
		case ePCU_SET_MODEM_OUTPUT:	
			nNewModem = epcuPowerControl;
			break;
		case ePCU_SET_DCU_OUTPUT:	
			nNewDCU = epcuPowerControl;
			break;
		case ePCU_SET_RADAR_OUTPUT:	
			nNewRadar = epcuPowerControl;
			break;
		case ePCU_SET_AUX0_OUTPUT:	
			nNewAux0 = epcuPowerControl;
			break;
		case ePCU_SET_AUX1_OUTPUT:	
			nNewAux1 = epcuPowerControl;
			break;
		case ePCU_SET_LED_VLOW_OUTPUT:	
			nNewLedVlow = epcuPowerControl;
			break;
		case ePCU_SET_LED_CHRGR_OUTPUT:	
			nNewLedChrgr = epcuPowerControl;
			break;
		case ePCU_SET_LED_SYST_OUTPUT:	
			nNewLedSyst = epcuPowerControl;
			break;
		case ePCU_SET_LED_ALARM_OUTPUT:	
			nNewLedAlarm = epcuPowerControl;
			break;
		default:
			break;
	};
}
void pcuPowerControlSetOutput(ePCU_POWER_CONTROL epcuPowerControl, ePCU_POWER_CONTROL_SET_OUTPUT epcuPowerControlSetOutput)
{

	switch(epcuPowerControlSetOutput)
	{
		case ePCU_SET_LED_VLOW_OUTPUT:
		case ePCU_SET_LED_CHRGR_OUTPUT:
		case ePCU_SET_LED_SYST_OUTPUT:
		case ePCU_SET_LED_ALARM_OUTPUT:
				/////
				// do these right now
				/////
				pcuPowerControlDoSetOutput(epcuPowerControl, epcuPowerControlSetOutput);
				break;
		default:
				if(ePOWER_OUTPUT_OFF == epcuPowerControl)
				{
					// if power off is requested, do it right away
					pcuPowerControlDoSetOutput(epcuPowerControl, epcuPowerControlSetOutput);
				}
				else
				{
					// power being switched on
					
					// so schedule it so that we can give power time
					// to settle between outputs being switched on
					bmPowerOnRequests |= (1<<epcuPowerControlSetOutput);
					//printf("SO[%X]\n", epcuPowerControlSetOutput);
				}
				break;
	}
}

void pcuPowerControlDoWork()
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
         
				// PCA9634 U4
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
					bNew = TRUE;
				}
				if(nNewFan1 != nStateFan1)
				{
					nStateFan1 = nNewFan1;
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
				if(nNewDCU != nStateDCU)
				{
					nStateDCU = nNewDCU;
					bNew = TRUE;
				}
				if(nNewRadar != nStateRadar)
				{
					nStateRadar = nNewRadar;
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
					pcuPowerControlAddOutputControlI2CQueueEntry();
					I2CQueueAddEntry(&myI2CQueueEntry);
					startTimer(&sampleTimer, POWER_CONTROL_SAMPLE_TIME_MS);
				}
	}
	
	// stagger enabling power outputs
	// to allow power to stabilize
	if(isTimerExpired(&staggerPowerTimer))
	{
		if(0 != bmPowerOnRequests)
		{
			int i;
			//printf("TO1[%lX]\n", bmPowerOnRequests);
			/////
			// we have a request to turn a power output on
			//
			// so do that and set the timer
			/////

			for(i=0;i<32;i++)
			{
				if(bmPowerOnRequests & (1<<i))
				{
				   //printf("TO2[%X]\n", i);
					////
					// remove the request
					/////
					bmPowerOnRequests ^= (1<<i);
					
					/////
					// enable the power output
					/////
					pcuPowerControlDoSetOutput(ePOWER_OUTPUT_ON, (ePCU_POWER_CONTROL_SET_OUTPUT)i);
					
					/////
					// and restart the timer
					/////
					startTimer(&staggerPowerTimer, STAGGER_POWER_MS);
					break;
				}
			}
		}
	}
}

//=================
// PCA9634 U4 & U5
//=================
unsigned char pcuPowerControlGetStatus(ePCU_POWER_CONTROL_GET_STATUS ePcuPowerControlGetStatus)
{
	unsigned char nStatus = 0;
	
	switch(ePcuPowerControlGetStatus)
	{
		case ePCU_GET_ROW0_STATUS:
			nStatus = nNewRow0;
			break;
		case ePCU_GET_ROW1_STATUS:			
			nStatus = nNewRow1;
			break;
		case ePCU_GET_ROW2_STATUS:			
			nStatus = nNewRow2;
			break;
		case ePCU_GET_ROW3_STATUS:			
			nStatus = nNewRow3;
			break;
		case ePCU_GET_FAN0_STATUS:			
			nStatus = nNewFan0;
			break;
		case ePCU_GET_FAN1_STATUS:			
			nStatus = nNewFan1;
			break;
		case ePCU_GET_GPS_STATUS:			
			nStatus = nNewGps;
			break;
		case ePCU_GET_MODEM_STATUS:			
			nStatus = nNewModem;
			break;
		case ePCU_GET_DCU_STATUS:		
			nStatus = nNewDCU;
			break;
		case ePCU_GET_RADAR_STATUS:		
			nStatus = nNewRadar;
			break;
		case ePCU_GET_AUX0_STATUS:		
			nStatus = nNewAux0;
			break;
		case ePCU_GET_AUX1_STATUS:	
			nStatus = nNewAux1;
			break;
		case ePCU_GET_LED_VLOW_STATUS:	
			nStatus = nNewLedVlow;
			break;
		case ePCU_GET_LED_CHRGR_STATUS:	
			nStatus = nNewLedChrgr;
			break;
		case ePCU_GET_LED_SYST_STATUS:	
			nStatus = nNewLedSyst;
			break;
		case ePCU_GET_LED_ALARM_STATUS:
		nStatus = nNewLedAlarm;
			break;
		default:
			break;			
	};

	// invert the input/output logic if necessary
	nStatus = pcuPowerGetOutputSetting((ePCU_POWER_CONTROL)nStatus, (ePCU_POWER_CONTROL_SET_OUTPUT)ePcuPowerControlGetStatus);
    
	return nStatus;
}

