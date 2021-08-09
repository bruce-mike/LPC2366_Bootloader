#include <string.h>
#include <stdio.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "timer.h"
#include "i2c.h"

#include "ADS7828.h"

// these are ref'd to 'by name' via #define's in ads7828.h.
//                            ch0   ch1   ch2   ch3   ch4   ch5   ch6   ch7
// ch0 = ID1
// ch1 = ID2 etc\
// power down mode and single-ended comversion mode is encoded here in channel cmd
const char ADS7828_CMDS[] = {ADC_ID1, ADC_ID2, ADC_IB, ADC_IX, ADC_GND, ADC_IA, ADC_IL, ADC_IS};	

//single-ended, internal ref ON, ADC ON.
//#define ADC_ID1			0x8C 	// Unused
//#define ADC_ID2 		0xCC	// Unused
//#define ADC_IB 			0x9C	// Unused
//#define ADC_IX 			0xDC	// Unused
//#define ADC_GND 		0xAC	// Ground
//#define ADC_IA 			0xEC	// Unused
//#define ADC_IL 			0xBC	// Solar
//#define ADC_IS 			0xFC	// 12v subsystem (lamp drivers, beacon, indicator, user, hour, lbar, strobe
static TIMERCTL sampleTimer;
static I2C_QUEUE_ENTRY myI2CQueueEntry;
static I2C_TRANSACTION myTransaction;

static int nADC_IL;
static int nADC_IS;

static void ADS7828AddI2CQueueEntry(int nChannelNum)
{
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&myTransaction);
		myTransaction.nSLA_W = ADS7828_BASE;
		myTransaction.nSLA_R = ADS7828_RD;
		myTransaction.cOutgoingData[0] = ADS7828_CMDS[nChannelNum];
		myTransaction.nOutgoingDataLength = 1;
		myTransaction.nIncomingDataLength = 2;

		I2CQueueAddTransaction(&myI2CQueueEntry, &myTransaction);
}

void ADS7828Init()
{
	nADC_IL = 0;
	nADC_IS = 0;
	ADS7828AddI2CQueueEntry(1);
	I2CQueueAddEntry(&myI2CQueueEntry);
	startTimer(&sampleTimer, ADS7828_SAMPLE_TIME_MS);
}

void ADS7828DoWork()
{
	#define MAX_SYSTEM_CURRENT_SAMPLES_FOR_AVERAGE 10
	#define MAX_LINE_CURRENT_SAMPLES_FOR_AVERAGE 10
	
	static int nSystemCurrentSampleArray[MAX_SYSTEM_CURRENT_SAMPLES_FOR_AVERAGE];
	static int nSystemCurrentSampleIndex = 0;
	static int nLineCurrentSampleArray[MAX_LINE_CURRENT_SAMPLES_FOR_AVERAGE];
	static int nLineCurrentSampleIndex = 0;
	
	static BOOL bInit = TRUE;
	static int nChannelNum = 6;
	
	int nAverage;
	int nADCValue = 0;
	int i;
	
	if(bInit)
	{
		for(i=0;i<MAX_SYSTEM_CURRENT_SAMPLES_FOR_AVERAGE;i++)
		{
			nSystemCurrentSampleArray[i] = 0;
		}
		
		for(i=0;i<MAX_LINE_CURRENT_SAMPLES_FOR_AVERAGE;i++)
		{
			nLineCurrentSampleArray[i] = 0;
		}
		
		bInit = FALSE;
	}
	
	switch(myI2CQueueEntry.eState)
	{
		case eI2C_TRANSFER_STATE_COMPLETE:
			{
				nADCValue = (myTransaction.cIncomingData[0])<<8|myTransaction.cIncomingData[1];
				//printf("nADCValue[%d]\n", nADCValue);
				/////
				// previous I2C queue entry is complete
				// grab the data
				// and schedule another
				/////
				switch(nChannelNum)
				{
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
						break;
					case 5:
					case 6:
						if(4095 <= nADCValue)
						{
							nChannelNum = 7;
							break;
						}
						nLineCurrentSampleArray[nLineCurrentSampleIndex++] = nADCValue;
						
						if(MAX_LINE_CURRENT_SAMPLES_FOR_AVERAGE <= nLineCurrentSampleIndex)
						{
							nLineCurrentSampleIndex = 0;
						}
						nAverage = 0;
						for(i=0;i<MAX_LINE_CURRENT_SAMPLES_FOR_AVERAGE;i++)
						{
								nAverage += nLineCurrentSampleArray[i];
						}
						nAverage /= MAX_LINE_CURRENT_SAMPLES_FOR_AVERAGE;
						nADC_IL = nAverage;
						nChannelNum = 7;
						break;
					case 7:
						if(4095 <= nADCValue)
						{
							//printf("BAD 7-nADCValue[%d]\n", nADCValue);
							nChannelNum = 6;
							break;
						}
						nSystemCurrentSampleArray[nSystemCurrentSampleIndex++] = nADCValue;
						if(MAX_SYSTEM_CURRENT_SAMPLES_FOR_AVERAGE <= nSystemCurrentSampleIndex)
						{
							nSystemCurrentSampleIndex = 0;
						}
						nAverage = 0;
						for(i=0;i<MAX_SYSTEM_CURRENT_SAMPLES_FOR_AVERAGE;i++)
						{
								nAverage += nSystemCurrentSampleArray[i];
						}
						nAverage /= MAX_SYSTEM_CURRENT_SAMPLES_FOR_AVERAGE;
						nADC_IS = nAverage;
						nChannelNum = 6;
						break;
				}
				myI2CQueueEntry.eState = eI2C_TRANSFER_STATE_IDLE;
				break;
			case eI2C_TRANSFER_STATE_FAILED:
				myI2CQueueEntry.eState = eI2C_TRANSFER_STATE_IDLE;
				break;
			default:
				break;
		}
	}
	
	/////
	// if sample time expired
	// then schedule another transfer
	/////
	if((eI2C_TRANSFER_STATE_IDLE == myI2CQueueEntry.eState) && (TRUE == isTimerExpired(&sampleTimer)))
	{
		ADS7828AddI2CQueueEntry(nChannelNum);
		I2CQueueAddEntry(&myI2CQueueEntry);
		startTimer(&sampleTimer, ADS7828_SAMPLE_TIME_MS);
	}
}


/////
// line current
/////
unsigned short nADS7828GetIL(void)
{
	unsigned short nAmps = 0;

	float fAmps = AMPS_PER_BIT_ALT_AND_BATT * nADC_IL;
	nAmps = fAmps *1000;
	//printf("nADC_IL[%04X] nAmps[%d]\n",nADC_IL, nAmps);
	return nAmps;
}

/////
// system current
/////
unsigned short nADS7828GetIS(void)
{
	unsigned short nAmps = 0;

	float fAmps = AMPS_PER_BIT_ALT_AND_BATT * nADC_IS;
	nAmps = fAmps *1000;
	//printf("nADC_IS[%04X] nAmps[%d]\n",nADC_IS, nAmps);
	return nAmps;
}
