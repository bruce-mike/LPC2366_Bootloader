#include <string.h>
#include <stdio.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "timer.h"
#include "i2c.h"
#include "errorinput.h"
#include "mischardware.h"
#include "actuator.h"
#include "adc.h"
#include "lm73.h"
#include "display.h"
#include "storedconfig.h"

static TIMERCTL sampleTimer;

static I2C_QUEUE_ENTRY myI2CQueueEntry;
static I2C_TRANSACTION driveErrorTransaction;

static unsigned char nMiscErrorData;
static WORD nDriveErrorData;

static void errorInputAddI2CQueueEntry(unsigned char bConfig)
{
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&driveErrorTransaction);
	
		driveErrorTransaction.nSLA_W = PCA9555_DRV_ERRS;
	
		if(bConfig)
		{			
			driveErrorTransaction.nSLA_R = 0;						
			driveErrorTransaction.cOutgoingData[0] = CONP0;
			driveErrorTransaction.cOutgoingData[1] = ALL_INPUT;
			driveErrorTransaction.cOutgoingData[2] = ALL_INPUT;
			driveErrorTransaction.nOutgoingDataLength =3;
			driveErrorTransaction.nIncomingDataLength = 0;
		}
		else
		{			
			driveErrorTransaction.nSLA_R = PCA9555_DRV_ERRS_RD;			
			driveErrorTransaction.cOutgoingData[0] = INP0;
			driveErrorTransaction.nOutgoingDataLength = 1;
			driveErrorTransaction.nIncomingDataLength = 2;
		}
		
		I2CQueueAddTransaction(&myI2CQueueEntry, &driveErrorTransaction);
}

void errorInputInit()
{
	nDriveErrorData = 0;
	
	/////
	// queue the configuration commands
	// to set these devices up as inputs
	/////
	errorInputAddI2CQueueEntry(TRUE);
	I2CQueueAddEntry(&myI2CQueueEntry);
	startTimer(&sampleTimer, ERROR_INPUT_SAMPLE_TIME_MS);
}

void errorInputDoWork()
{
	static int bConfig = 1;
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
					nDriveErrorData = (driveErrorTransaction.cIncomingData[0])<<8|driveErrorTransaction.cIncomingData[1];
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
		errorInputAddI2CQueueEntry(FALSE);
		I2CQueueAddEntry(&myI2CQueueEntry);

		/////
		// and restart the timer
		/////
		startTimer(&sampleTimer, ERROR_INPUT_SAMPLE_TIME_MS);
	}
}
unsigned short errorInputGetMiscErrors()
{
	/////
	// low is active error
	// so flip this data
	/////
	return ((nMiscErrorData)&0xff)^0xFF;
}

unsigned short errorInputGetSwitchData()
{
	unsigned short nBitmap = 0;
	int nModeSwitches  = hwReadSwitches();
	
	if(!(nModeSwitches&0x01))
	{
		nBitmap |= SWITCH_BITMAP_MODE_1;
	}
	
	if(!(nModeSwitches&0x02))
	{
		nBitmap |= SWITCH_BITMAP_MODE_2;
	}
	
	if(!(nModeSwitches&0x04))
	{
		nBitmap |= SWITCH_BITMAP_MODE_4;
	}
	
	if(!(nModeSwitches&0x08))
	{
		nBitmap |= SWITCH_BITMAP_MODE_8;
	}
	
	return nBitmap;
}

unsigned short errorInputGetDriveErrors()
{
	/////
	// low is active error
	// so flip this data
	/////
	return nDriveErrorData^0xffff;
}

unsigned short errorInputGetAlarms()
{
	unsigned short nAlarms = ALARM_NONE;
	unsigned short nDriveErrorData = errorInputGetDriveErrors();
	
	printf("errorInputGetAlarms: nDriveErrorData[%X]\n",nDriveErrorData);
	
	if(ADCGetLineVoltage() > lineVoltageHighLimit || ADCGetLineVoltage() < lineVoltageLowLimit)
	{
		nAlarms |= ALARM_BITMAP_LOW_LINE_VOLTAGE;
	}
	
	if(eAUX_BATTERY_NONE != storedConfigGetAuxBatteryType())
	{
		if(ADCGetBatteryVoltage() > batteryVoltageHighLimit || ADCGetBatteryVoltage() < batteryVoltageLowLimit)
		{
			nAlarms |= ALARM_BITMAP_LOW_BATTERY;
		}

		if(!hwIsVBPosPolarity())
		{
			nAlarms |= ALARM_BITMAP_RVBV;
		}
	}
	
	if(!hwIsVLPosPolarity())
	{
		nAlarms |= ALARM_BITMAP_RVLV;		
	}
		
	if(lm73GetDegreesC() > temperatureHighLimit || lm73GetDegreesC() < temperatureLowLimit)
	{
		nAlarms |= ALARM_BITMAP_OVER_TEMP;
	}
	
	if(hwGetLVDStatus())
	{
		nAlarms |= ALARM_BITMAP_LVD;
	}
	
	if(hwGetSolarChargeStatus())
	{
		nAlarms |= ALARM_BITMAP_CHARGER_ON;
	}
	
	if(nDriveErrorData != 0x00)
	{
		nAlarms |= ALARM_BITMAP_DISPLAY_ERRORS;
	}
	
	if(errorInputGetPcuErrors() & AUX_ERROR_AUX)
	{
		nAlarms |= ALARM_BITMAP_AUX_ERRORS;
	}
	
	return nAlarms;
}

BOOL errorInputIsVbOK()
{
	BOOL bRetVal = FALSE;
	unsigned char nErrors = errorInputGetMiscErrors();
	/////
	// 0 is good
	/////
	//printf("errorInputIsVBOK()[%X][%X][%d][%d]\n", nErrors, (1<<IN_VOK_VB), ADCGetLineVoltage(), ADCGetBatteryVoltage());
	if(0 == ((1<<I2_IN_VOK_VB) & nErrors))
	{
		printf("1-\n");
		bRetVal = TRUE;
	}
	return bRetVal;
}
BOOL errorInputIsVlOK()
{
	BOOL bRetVal = FALSE;
	unsigned char nErrors = errorInputGetMiscErrors();
	/////
	// 0 is good
	/////
	//printf("errorInputIsVLOK()[%X][%X][%d][%d]\n", nErrors, (1<<IN_VOK_VL), ADCGetLineVoltage(), ADCGetBatteryVoltage());
	if(0 == ((1<<I2_IN_VOK_VL) &  nErrors))
	{
		bRetVal = TRUE;
	}
	return bRetVal;
}

unsigned short errorInputGetPcuErrors()
{
	unsigned short nErrors = 0;
	unsigned short nAllErrors = errorInputGetMiscErrors();
	
	if(nAllErrors& (1<<I2_xERR_ROW0))
	{
		nErrors |= AUX_ERROR_INDR;
	}
	if(nAllErrors&(1<<I2_xERR_ROW1))
	{
		nErrors |= AUX_ERROR_INDL;
	}
	if(nAllErrors&(1<<I2_xERR_ROW2))
	{
		nErrors |= AUX_ERROR_AUX;
	}
	return nErrors;
}
