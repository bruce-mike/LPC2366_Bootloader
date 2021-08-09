#include <string.h>
#include <stdio.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "i2c.h"
#include "timer.h"

#include "lm73.h"
static TIMERCTL sampleTimer;

static I2C_QUEUE_ENTRY myI2CQueueEntry;
static I2C_TRANSACTION myTransaction;

static int nTempReadingC;
static int nTempReadingF;

static union 
{
	WORD wTemperatureCounts;
	BYTE bTemperatureCounts[2];
}TEMP_COUNTS;

static void lm73AddI2CConfigQueueEntry()
{
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&myTransaction);
		myTransaction.nSLA_W = LM73_WRITE_ADDR;
		myTransaction.nSLA_R = 0;
		myTransaction.cOutgoingData[0] = CONTROL;
		myTransaction.cOutgoingData[1] = LM73_RESOLUTION_11_BITS;
		myTransaction.nOutgoingDataLength = 2;
		myTransaction.nIncomingDataLength = 0;
		//printf("lm73AddI2CConfigQueueEntry myTransaction[%X] [%X][%X][%X][%X] nSLA_W[%X] nSLA_R[%X]\n\r", (int)&myTransaction, myTransaction.cOutgoingData[0], myTransaction.cOutgoingData[1], myTransaction.cOutgoingData[2], myTransaction.cOutgoingData[3], myTransaction.nSLA_W, myTransaction.nSLA_R);
		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction);
}
static void lm73AddI2CReadQueueEntry()
{
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&myTransaction);
		myTransaction.nSLA_W = LM73_WRITE_ADDR;
		myTransaction.nSLA_R = LM73_READ_ADDR;
		myTransaction.cOutgoingData[0] = TEMP_REG;
		myTransaction.nOutgoingDataLength = 1;
		myTransaction.nIncomingDataLength = 2;
		//printf("lm73AddI2CQueueEntry myTransaction[%X] [%X][%X][%X][%X]\n\r", (int)&myTransaction, myTransaction.cOutgoingData[0], myTransaction.cOutgoingData[1], myTransaction.cOutgoingData[2], myTransaction.cOutgoingData[3]);
		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction);
}
void lm73Init()
{
	TEMP_COUNTS.wTemperatureCounts = 0;
	lm73AddI2CConfigQueueEntry();
	//printf("lm73Init myI2CQueueEntry[%X]\n\r", (int)&myI2CQueueEntry);
	I2CQueueAddEntry(&myI2CQueueEntry);
	startTimer(&sampleTimer, LM73_SAMPLE_TIME_MS);
}


static void lm73CalculateTempReading()
{
    ////////////////////////////////////////////////
    // 11-bit value (left justified)
    // 150C: 0x4B00
    //  25C: 0x0C80
    //   1C: 0x0080
    // .25C: 0x0020
    //   0C: 0x0000
    //-.25C: 0xFFE0
    //  -1C: 0xFF80
    // -25C: 0xF380
    // -40C: 0xEC00
    ////////////////////////////////////////////////
    
	float fTempC = 0;
	BYTE isNegative = FALSE;
    

	// Check to see if the Negative Temperature Bit is Set
	if(TEMP_COUNTS.wTemperatureCounts & 0x8000)
	{				
		isNegative = TRUE;
        TEMP_COUNTS.wTemperatureCounts = ~TEMP_COUNTS.wTemperatureCounts + 1;           
    }    

    fTempC += (float)(TEMP_COUNTS.wTemperatureCounts/128);


    if(TEMP_COUNTS.wTemperatureCounts & 0x0020)
	{
		fTempC += 0.25;
	}
    if(TEMP_COUNTS.wTemperatureCounts & 0x0040)
	{
        fTempC += 0.50;
	}

    if(isNegative == TRUE)
	{
        fTempC = 0.0 - fTempC - 0.5;
    }
    else
    {
        fTempC += 0.5;
    }
    
    nTempReadingC = (int)fTempC;
    nTempReadingF = (int)((fTempC * 1.8) + 32.0);
}


void lm73DoWork()
{
	switch(myI2CQueueEntry.eState)
	{
		case eI2C_TRANSFER_STATE_COMPLETE:
			{
				/////
				// previous I2C queue entry is complete
				// grab the data
				// and schedule another
				/////
				TEMP_COUNTS.wTemperatureCounts = (myTransaction.cIncomingData[0]<<8)|myTransaction.cIncomingData[1];
                lm73CalculateTempReading();                
			}
			myI2CQueueEntry.eState = eI2C_TRANSFER_STATE_IDLE;
			break;

		case eI2C_TRANSFER_STATE_FAILED:
			myI2CQueueEntry.eState = eI2C_TRANSFER_STATE_IDLE;
			break;
		case eI2C_TRANSFER_STATE_IDLE:
		default:
			break;
	}
	if((eI2C_TRANSFER_STATE_IDLE == myI2CQueueEntry.eState) && isTimerExpired(&sampleTimer))
	{
		lm73AddI2CReadQueueEntry();
		I2CQueueAddEntry(&myI2CQueueEntry);
		startTimer(&sampleTimer, LM73_SAMPLE_TIME_MS);
	}
}

int lm73GetTempReading(BYTE tempUnit)
{
	if(DEG_C == tempUnit)
	{
		return nTempReadingC;
	}

	return nTempReadingF;
}
