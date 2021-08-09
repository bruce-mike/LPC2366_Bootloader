#include <string.h>
#include <stdio.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "i2c.h"
#include "timer.h"
#include "pcuStatusBoard.h"

static I2C_QUEUE_ENTRY myI2CQueueEntry;
static I2C_TRANSACTION myTransaction;

static TIMERCTL sampleTimer;
static BOOL radarLEDToggle = FALSE;
	
static ePCU_STATUS_BOARD_CONTROL nStateSchedule;
static ePCU_STATUS_BOARD_CONTROL nStateRadar;
static ePCU_STATUS_BOARD_CONTROL nStateCharger;
                  
static ePCU_STATUS_BOARD_CONTROL nNewStateSchedule;
static ePCU_STATUS_BOARD_CONTROL nNewStateRadar;
static ePCU_STATUS_BOARD_CONTROL nNewStateCharger;

static void pcuStatusBoardAddConfigureI2CQueueEntry()
{
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&myTransaction);
	
		//============
		// PCA9531 U1
		//============
		myTransaction.nSLA_W = PCA9531_STATUS_BOARD_CONTROL;
		myTransaction.nSLA_R = 0;
		myTransaction.cOutgoingData[0] = PCA9531_AUTO_INC;
		myTransaction.nOutgoingDataLength = 1;
		myTransaction.nIncomingDataLength = 0;
	
		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction);
}

static void pcuStatusBoardAddOutputI2CQueueEntry()
{
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&myTransaction);

		//============
		// PCA9531 U1
		//============
		myTransaction.nSLA_W = PCA9531_STATUS_BOARD_CONTROL;
		myTransaction.nSLA_R = 0;
		myTransaction.cOutgoingData[0] = PCA9531_LEDSEL0;	// auto inc bit is set
		myTransaction.cOutgoingData[1] = (nStateRadar << 4)|(nStateSchedule << 7);
		myTransaction.cOutgoingData[2] =  nStateCharger;
		myTransaction.nOutgoingDataLength = 3;
		myTransaction.nIncomingDataLength = 0;
	
		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction);
}

void pcuStatusBoardInit()
{
	//============
	// PCA9531 U1
	//============
	nStateSchedule	 	 = ePCU_STATUS_BOARD_OUTPUT_ON;
	nStateRadar			 	 = ePCU_STATUS_BOARD_OUTPUT_ON;
	nStateCharger		 	 = ePCU_STATUS_BOARD_OUTPUT_ON;

	//============
	// PCA9531 U1
	//============
	nStateSchedule	 	 = ePCU_STATUS_BOARD_OUTPUT_OFF;
	nStateRadar			 	 = ePCU_STATUS_BOARD_OUTPUT_OFF;
	nStateCharger		 	 = ePCU_STATUS_BOARD_OUTPUT_OFF;
	
	initTimer(&sampleTimer);
	startTimer(&sampleTimer, PCU_STATUS_BOARD_SAMPLE_TIME_MS);
	
	/////
	// schedule I2C transaction to configure the driver
	// leave all drivers off for now
	/////
	pcuStatusBoardAddConfigureI2CQueueEntry();
	I2CQueueAddEntry(&myI2CQueueEntry);
}

void pcuStatusBoardDoWork()
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
				
				//============
				// PCA9531 U1
				//============				
				if(nNewStateSchedule != nStateSchedule)
				{
					nStateSchedule = nNewStateSchedule;
					bNew = TRUE;
				}
				
				if(nNewStateRadar != nStateRadar)
				{
					nStateRadar = nNewStateRadar;
					bNew = TRUE;
				}
                
				
				if(nNewStateCharger != nStateCharger)
				{
					nStateCharger = nNewStateCharger;
					bNew = TRUE;
				}
				
				if(bNew)
				{
					pcuStatusBoardAddOutputI2CQueueEntry();
					I2CQueueAddEntry(&myI2CQueueEntry);
					startTimer(&sampleTimer, PCU_STATUS_BOARD_SAMPLE_TIME_MS);
				}
	}
}

//=================
// PCA9531 U1
//=================
void pcuStatusBoardSetStatus(ePCU_STATUS_BOARD_CONTROL ePcuStatusBoardControl, ePCU_STATUS_BOARD_SET_STATUS ePcuStatusBoardSetStatus)
{
	switch(ePcuStatusBoardSetStatus)
	{
		case ePCU_STATUS_BOARD_SET_SCHEDULE_STATUS:	
			nNewStateSchedule = ePcuStatusBoardControl;
			break;

		case ePCU_STATUS_BOARD_SET_RADAR_STATUS:	
			nNewStateRadar = ePcuStatusBoardControl;	
			break;
		
		case ePCU_STATUS_BOARD_SET_CHARGER_STATUS:	
			nNewStateCharger = ePcuStatusBoardControl;	
			break;
		
		default:
			break;
	};
}

//=================
// PCA9531 U1
//=================
unsigned char pcuStatusBoardGetStatus(ePCU_STATUS_BOARD_GET_STATUS ePcuStatusBoardGetStatus)
{
	unsigned char nStatus = 0;
	
	switch(ePcuStatusBoardGetStatus)
	{
		case ePCU_STATUS_BOARD_GET_SCHEDULE_STATUS:			
			nStatus = nNewStateSchedule;
			break;
		case ePCU_STATUS_BOARD_GET_RADAR_STATUS:			
			nStatus = nNewStateRadar;
			break;
		case ePCU_STATUS_BOARD_GET_CHARGER_STATUS:			
			nStatus = nNewStateCharger;
			break;
		default:
			break;			
	};
		
	return nStatus;
}

void pcuStatusBoardToggleRadarLED(void)
{
	if(radarLEDToggle)
	{			
		pcuStatusBoardSetStatus(ePCU_STATUS_BOARD_OUTPUT_ON,ePCU_STATUS_BOARD_SET_RADAR_STATUS);
		radarLEDToggle = FALSE;
	}
	else
	{
		pcuStatusBoardSetStatus(ePCU_STATUS_BOARD_OUTPUT_OFF,ePCU_STATUS_BOARD_SET_RADAR_STATUS);
		radarLEDToggle = TRUE;
	}
}

