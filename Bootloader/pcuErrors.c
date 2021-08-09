#include <string.h>
#include <stdio.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "timer.h"
#include "i2c.h"
#include "pcuErrors.h"
#include "mischardware.h"
#include "actuator.h"
#include "lm73.h"
#include "display.h"


static TIMERCTL sampleTimer;

static I2C_QUEUE_ENTRY myI2CQueueEntry;
static I2C_TRANSACTION pcuErrorsTransaction;

static WORD nPCUErrorsData;

static void pcuErrorsAddI2CQueueEntry(unsigned char bConfig)
{
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&pcuErrorsTransaction);
	
		pcuErrorsTransaction.nSLA_W = PCA9555_PCU_ERRS_CONFIG;
	
		if(bConfig)
		{			
			pcuErrorsTransaction.nSLA_R = 0;						
			pcuErrorsTransaction.cOutgoingData[0] = CONP0;
			pcuErrorsTransaction.cOutgoingData[1] = ALL_INPUT;
			pcuErrorsTransaction.cOutgoingData[2] = ALL_INPUT;
			pcuErrorsTransaction.nOutgoingDataLength =3;
			pcuErrorsTransaction.nIncomingDataLength = 0;
		}
		else
		{			
			pcuErrorsTransaction.nSLA_R = PCA9555_PCU_ERRS_RD;			
			pcuErrorsTransaction.cOutgoingData[0] = INP0;
			pcuErrorsTransaction.nOutgoingDataLength = 1;
			pcuErrorsTransaction.nIncomingDataLength = 2;
		}
		
		I2CQueueAddTransaction(&myI2CQueueEntry, &pcuErrorsTransaction);
}

void pcuErrorsInit()
{
	nPCUErrorsData = 0;
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		default:
			/////
			// queue the configuration commands
			// to set these devices up as inputs
			/////
			pcuErrorsAddI2CQueueEntry(TRUE);
			I2CQueueAddEntry(&myI2CQueueEntry);
			startTimer(&sampleTimer, PCU_ERRORS_SAMPLE_TIME_MS);
			break;
		case eBoardRev2:
		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			break;
	}

}

void pcuErrorsDoWork()
{
	static int bConfig = 1;
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
			return;
	}
	switch(myI2CQueueEntry.eState)
	{
		case eI2C_TRANSFER_STATE_COMPLETE:
			{	
				/////
				// previous I2C queue entry is complete
				// grab the data
				//(first transfer is configuration)
				/////
				if(0 == bConfig)
				{
					nPCUErrorsData = (pcuErrorsTransaction.cIncomingData[0])<<8|pcuErrorsTransaction.cIncomingData[1];
				}
				bConfig = 0;
				myI2CQueueEntry.eState = eI2C_TRANSFER_STATE_IDLE;
			}
			break;

		case eI2C_TRANSFER_STATE_FAILED:
			/////
			// transfer failed for some reason
			// we will schedule another when the timer expires
			/////
			myI2CQueueEntry.eState = eI2C_TRANSFER_STATE_IDLE;
			break;
		case eI2C_TRANSFER_STATE_IDLE:
			break;
		case eI2C_TRANSFER_STATE_TRANSFERRING:
			break;
		default:
			break;
	}

	if((eI2C_TRANSFER_STATE_IDLE == myI2CQueueEntry.eState) && isTimerExpired(&sampleTimer))
	{
		/////
		// schedule another
		/////
		pcuErrorsAddI2CQueueEntry(FALSE);
		I2CQueueAddEntry(&myI2CQueueEntry);

		/////
		// and restart the timer
		/////
		startTimer(&sampleTimer, PCU_ERRORS_SAMPLE_TIME_MS);
	}
}

unsigned short pcuErrorsGetSwitchData()
{
	return hwReadModel();
}

BOOL pcuErrorsGetAlarm(ePCU_ALARMS ePcuAlarm)
{
	BOOL bPcuAlarm = FALSE;
	
	switch(ePcuAlarm)
	{
		case ePCU_VLINE_ALARM: 
			if(ADCGetLineVoltage() < lineVoltageLowLimit)
			{
				bPcuAlarm = TRUE;
			}
			break;
			
		case ePCU_VBATT_ALARM: 	
			if(	ADCGetBatteryVoltage() < hwGetPowerControlData(ePCU_COMMAND_GET_PWR_WARN_VOLTAGE) )
			{
				bPcuAlarm = TRUE;
			}
			break;
		
		case ePCU_RVLV_ALARM:  	
			if(!hwIsVLPosPolarity())
			{
				bPcuAlarm = TRUE;;		
			}
			break;
			
		case ePCU_RVBV_ALARM:	 	
			if(!hwIsVBPosPolarity())
			{
				bPcuAlarm = TRUE;
			}
			break;
		
		case ePCU_TEMP_ALARM:  	
			if(lm73GetTempReading(DEG_C) > temperatureHighLimitC || lm73GetTempReading(DEG_C) < temperatureLowLimitC)
			{
				bPcuAlarm = TRUE;
			}
			break;
		
		case ePCU_LVD_ALARM:   	
			if(hwGetLVDStatus())
			{
				bPcuAlarm = TRUE;;
			}
			break;
		
		case ePCU_CHARGER_ALARM:		
			if(hwGetSolarChargeStatus())
			{
				bPcuAlarm = TRUE;
			}
			break;
		case ePCU_PHOTOCELL_ALARM:
			break;
		
		default:
			break;
	}
	
	return bPcuAlarm;
}

BOOL pcuErrorsGetError(ePCU_ERRORS ePcuError)
{
	BOOL bPcuError = FALSE;
	switch(ePcuError)
	{
		case ePCU_ROW0_ERROR:
			if(!(nPCUErrorsData& (1<<I2_xERR_ROW0))) // LOW TRUE ERROR SIGNALS
			{
				bPcuError = TRUE;
			}
			break;	
			
		case ePCU_ROW1_ERROR:  
			if(!(nPCUErrorsData& (1<<I2_xERR_ROW1)))
			{
				bPcuError = TRUE;
			}
			break;	
			
		case ePCU_ROW2_ERROR:  
			if(!(nPCUErrorsData& (1<<I2_xERR_ROW2)))
			{
				bPcuError = TRUE;
			}
			break;
			
		case ePCU_ROW3_ERROR:  
			if(!(nPCUErrorsData& (1<<I2_xERR_ROW3)))
			{
				bPcuError = TRUE;
			}
			break;
			
		case ePCU_FAN0_ERROR:  
			if(!(nPCUErrorsData& (1<<I2_xERR_FAN0)))
			{
				bPcuError = TRUE;
			}
			break;	
			
		case ePCU_FAN1_ERROR:  
			if(!(nPCUErrorsData& (1<<I2_xERR_FAN1)))
			{
				bPcuError = TRUE;
			}
			break;
			
		case ePCU_GPS_ERROR:   
			if(!(nPCUErrorsData& (1<<I2_xERR_GPS)))
			{
				bPcuError = TRUE;
			}
			break;	
			
		case ePCU_MODEM_ERROR: 
			if(!(nPCUErrorsData& (1<<I2_xERR_MODEM)))
			{
				bPcuError = TRUE;
			}
			break;
			
		case ePCU_DCU_ERROR: 
			if(!(nPCUErrorsData& (1<<I2_xERR_DCU)))
			{
				bPcuError = TRUE;
			}
			break;
			
		case ePCU_RADAR_ERROR:   
			if(!(nPCUErrorsData& (1<<I2_xERR_RADAR)))
			{
				bPcuError = TRUE;
			}
			break;
			
		case ePCU_AUX0_ERROR:  
			if(!(nPCUErrorsData& (1<<I2_xERR_AUX0)))
			{
				bPcuError = TRUE;
			}
			break;	
			
		case ePCU_AUX1_ERROR:  
			if(!(nPCUErrorsData& (1<<I2_xERR_AUX1)))
			{
				bPcuError = TRUE;
			}
			break;	
			
		case ePCU_VOK_VL_ERROR:
			if(!(nPCUErrorsData& (1<<I2_IN_VOK_VL)))
			{
				bPcuError = TRUE;
			}
			break;		
			
		case ePCU_VOK_VB_ERROR:
			if(!(nPCUErrorsData& (1<<I2_IN_VOK_VB)))
			{
				bPcuError = TRUE;
			}
			break;	
			
		default:
			break;			
	};

	return bPcuError;
}
