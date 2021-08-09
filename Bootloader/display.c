#include <stdlib.h>
#include <stdio.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "timer.h"
#include "i2c.h"

#include "commands.h"
#include "mischardware.h"
#include "display.h"
#include "leddriver.h"
#include "errorinput.h"
#include "pwm.h"
#include "watchdog.h"
#include "adc.h"


static eMODEL eModel;

static I2C_QUEUE_ENTRY myI2CQueueEntry;
static I2C_TRANSACTION group_1_8_Transaction;
static I2C_TRANSACTION group_9_16_Transaction;

static TIMERCTL patternTimer;
static int nCurrentDisplayPatternIndex;
static int nDisplayPatterns;
static int nCurrentIndicatorPatternIndex;
static int nIndicatorPatterns;
static DISPLAY_PATTERN displayPatterns[MAX_DISPLAY_PATTERNS];
static eLED_CONFIG_BITS indicatorLeft[MAX_DISPLAY_PATTERNS];
static eLED_CONFIG_BITS indicatorRight[MAX_DISPLAY_PATTERNS];
static eDISPLAY_TYPES eCurrentDisplaySelection;
static eDISPLAY_TYPES eInitialDisplaySelection;
static unsigned char bInit = TRUE;
///////////////////
// the patterns defined here 
// have group bit definitions for 
// group 1-4
// group 5-8
// gtoup 9-12
// group 13-16
////////////////////////

/////////////////
// 25 light sequential
/////////////////
static unsigned char Pattern_Blank[] =
{
	0,
	0,
	0,
	0
};
static unsigned char Pattern_AllOn_25_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3|DISPLAY_GROUP4,
	DISPLAY_GROUP5|DISPLAY_GROUP6|DISPLAY_GROUP7|DISPLAY_GROUP8,
	DISPLAY_GROUP9|DISPLAY_GROUP10|DISPLAY_GROUP11|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP15|DISPLAY_GROUP16
};
static unsigned char Pattern_DoubleArrow_1_25_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3,
	0,
	DISPLAY_GROUP9|DISPLAY_GROUP10|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP16
};
static unsigned char Pattern_Bar_1_25_light[] = 
{
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	DISPLAY_GROUP5,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP16
};
static unsigned char Pattern_FourCorner_1_25_light[] =
{
	DISPLAY_GROUP3,
	0,
	0,
	DISPLAY_GROUP14
};
static unsigned char Pattern_RightArrow_1_25_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	0,
	DISPLAY_GROUP9|DISPLAY_GROUP10|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP16
};
static unsigned char Pattern_LeftArrow_1_25_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3,
	DISPLAY_GROUP5,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP16
};	
static unsigned char Pattern_RightStemArrow_1_25_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	0,
	DISPLAY_GROUP12,
	0
};
static unsigned char Pattern_RightStemArrow_2_25_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	0,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13
};
static unsigned char Pattern_RightStemArrow_3_25_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	0,
	DISPLAY_GROUP9|DISPLAY_GROUP10|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP16	
};

static unsigned char Pattern_LeftStemArrow_1_25_light[] =
{
	0,
	DISPLAY_GROUP5,
	0,
	DISPLAY_GROUP16	
};
static unsigned char Pattern_LeftStemArrow_2_25_light[] =
{
	0,
	DISPLAY_GROUP5,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP16
};
static unsigned char Pattern_LeftStemArrow_3_25_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3,
	DISPLAY_GROUP5,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP16
};
static unsigned char Pattern_RightWalkingArrow_1_25_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP3|DISPLAY_GROUP4,
	DISPLAY_GROUP8,
	DISPLAY_GROUP12,
	0
};	
static unsigned char Pattern_RightWalkingArrow_2_25_light[] =
{	
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	DISPLAY_GROUP7,
	DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP15
};
static unsigned char Pattern_RightWalkingArrow_3_25_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	0,
	DISPLAY_GROUP9|DISPLAY_GROUP10|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP16
};
static unsigned char Pattern_LeftWalkingArrow_1_25_light[] =
{ 
	0,
	DISPLAY_GROUP5,
	DISPLAY_GROUP11,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP16
};	
static unsigned char Pattern_LeftWalkingArrow_2_25_light[] =
{
	0,
	DISPLAY_GROUP5|DISPLAY_GROUP6,
	DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP15|DISPLAY_GROUP16
};
static unsigned char Pattern_LeftWalkingArrow_3_25_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3,
	DISPLAY_GROUP5,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP16
};
static unsigned char Pattern_RightChevron_1_25_light[] =
{
	DISPLAY_GROUP3,
	DISPLAY_GROUP8,
	DISPLAY_GROUP12,
	0
};	
static unsigned char Pattern_RightChevron_2_25_light[] =
{ 
	DISPLAY_GROUP3,
	DISPLAY_GROUP7|DISPLAY_GROUP8,
	DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP15
};	
static unsigned char Pattern_RightChevron_3_25_light[] =
{
	DISPLAY_GROUP3,
	DISPLAY_GROUP7|DISPLAY_GROUP8,
	DISPLAY_GROUP10|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP15|DISPLAY_GROUP16
};	
static unsigned char Pattern_LeftChevron_1_25_light[] =
{
	0,
	0,
	DISPLAY_GROUP11,
	DISPLAY_GROUP13|DISPLAY_GROUP14	
};
static unsigned char Pattern_LeftChevron_2_25_light[] =
{
	0,
	DISPLAY_GROUP6,
	DISPLAY_GROUP11|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP15	
};
static unsigned char Pattern_LeftChevron_3_25_light[] =
{ 
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3,
	DISPLAY_GROUP6,
	DISPLAY_GROUP11|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP15
};
static unsigned char Pattern_DoubleDiamond_1_25_light[] =
{ 
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3,
	DISPLAY_GROUP8,
	DISPLAY_GROUP12,
	0		
};
static unsigned char Pattern_DoubleDiamond_2_25_light[] =
{ 
	0,
	0,
	DISPLAY_GROUP10|DISPLAY_GROUP11,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP16
};

/////////////////
// 15 light sequential
// note: the grouping is the same
// as for the 25 light sequential
/////////////////
static unsigned char Pattern_AllOn_sequential_15_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3|DISPLAY_GROUP4,
	DISPLAY_GROUP5|DISPLAY_GROUP6|DISPLAY_GROUP7|DISPLAY_GROUP8,
	DISPLAY_GROUP9|DISPLAY_GROUP10|DISPLAY_GROUP11|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP15|DISPLAY_GROUP16
};
static unsigned char Pattern_FourCorner_1_sequential_15_light[] =
{
	DISPLAY_GROUP3,
	0,
	0,
	DISPLAY_GROUP14
};	

static unsigned char Pattern_DoubleArrow_1_sequential_15_light[] =
{ 
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3,
	0,
	DISPLAY_GROUP9|DISPLAY_GROUP10|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP16,
};	
static unsigned char Pattern_Bar_1_sequential_15_light[] =
{ 
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	DISPLAY_GROUP5,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP16,
};

static unsigned char Pattern_RightArrow_1_sequential_15_light[] =
{ 
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	0,
	DISPLAY_GROUP9|DISPLAY_GROUP10|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP16,
};

static unsigned char Pattern_LeftArrow_1_sequential_15_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3,
	DISPLAY_GROUP5,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP16,
};

static unsigned char Pattern_RightStemArrow_1_sequential_15_light[] =
{ 
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	0,
	0,
	0
};
static unsigned char Pattern_RightStemArrow_2_sequential_15_light[] =
{	
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	0,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13,
};	
	static unsigned char Pattern_RightStemArrow_3_sequential_15_light[] =
{ 
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	0,
	DISPLAY_GROUP9|DISPLAY_GROUP10|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP16,
};

static unsigned char Pattern_LeftStemArrow_1_sequential_15_light[] =
{ 
	0,
	DISPLAY_GROUP5,
	0,
	DISPLAY_GROUP16,
};	
static unsigned char Pattern_LeftStemArrow_2_sequential_15_light[] =
{  
	0,
	DISPLAY_GROUP5,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP16,
};
static unsigned char Pattern_LeftStemArrow_3_sequential_15_light[] =
{  
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3,
	DISPLAY_GROUP5,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP16,
};
/////////////////
// 15 light flashing
// note: the grouping is the same
// as for the 25 light sequential
/////////////////
static unsigned char Pattern_AllOn_flashing_15_light[] =
{
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3|DISPLAY_GROUP4,
	DISPLAY_GROUP5|DISPLAY_GROUP6|DISPLAY_GROUP7|DISPLAY_GROUP8,
	DISPLAY_GROUP9|DISPLAY_GROUP10|DISPLAY_GROUP11|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP15|DISPLAY_GROUP16
};
static unsigned char Pattern_FourCorner_1_flashing_15_light[] =
{ 
	DISPLAY_GROUP3,
	0,
	0,
	DISPLAY_GROUP14
};
static unsigned char Pattern_DoubleArrow_1_flashing_15_light[] =
{ 
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3,
	0,
	DISPLAY_GROUP9|DISPLAY_GROUP10|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP16,
};	
static unsigned char Pattern_Bar_1_flashing_15_light[] =
{ 
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	DISPLAY_GROUP5,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP16,
};
static unsigned char Pattern_RightArrow_1_flashing_15_light[] =
{ 
	DISPLAY_GROUP1|DISPLAY_GROUP4,
	0,
	DISPLAY_GROUP9|DISPLAY_GROUP10|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP14|DISPLAY_GROUP16,
};	
static unsigned char Pattern_LeftArrow_1_flashing_15_light[] =
{ 
	DISPLAY_GROUP1|DISPLAY_GROUP2|DISPLAY_GROUP3,
	DISPLAY_GROUP5,
	DISPLAY_GROUP9|DISPLAY_GROUP12,
	DISPLAY_GROUP13|DISPLAY_GROUP16,
};

typedef enum eIndicatorPatterns
{
	eINDICATOR_PATTERN_OFF,
	eINDICATOR_PATTERN_RIGHT,
	eINDICATOR_PATTERN_LEFT,
	eINCICATOR_PATTERN_BOTH,
	eINDICATOR_PATTERN_MOVING_RIGHT,
	eINDICATOR_PATTERN_MOVING_LEFT,
	eINDICATOR_PATTERN_WIG_WAG,
}eINDICATOR_PATTERNS;

/////
// indicators installed on all trailer signs
// but not on vehicle mount signs
/////
BOOL displayHasIndicators()
{
	BOOL bRetVal = FALSE;
	switch(eModel)
	{
	////////////////////////////////////////
	//// Vehicle Mount
	////////////////////////////////////////
		case eMODEL_VEHICLE_25_LIGHT_SEQUENTIAL:
		case eMODEL_VEHICLE_15_LIGHT_SEQUENTIAL:
		case eMODEL_VEHICLE_15_LIGHT_FLASHING:
		case eMODEL_NONE:
		default:
			break;
	
	//////////////////////////////////////////
	//// Solar Trailer
	//////////////////////////////////////////
		case eMODEL_TRAILER_25_LIGHT_SEQUENTIAL:
		case eMODEL_TRAILER_15_LIGHT_SEQUENTIAL:
		case eMODEL_TRAILER_15_LIGHT_FLASHING:
				bRetVal = TRUE;
				break;
	}
	return bRetVal;
}
void displaySetIndicatorPatterns(eINDICATOR_PATTERNS eIndicatorPattern)
{
	switch(eIndicatorPattern)
	{
		case eINDICATOR_PATTERN_OFF:
			nIndicatorPatterns = 1;
			indicatorLeft[0] = eLED_OFF;
			indicatorRight[0] = eLED_OFF;
			break;
		case eINDICATOR_PATTERN_RIGHT:
			nIndicatorPatterns = 2;
			indicatorLeft[0] = eLED_OFF;
			indicatorRight[0] = eLED_ON;
			indicatorLeft[1] = eLED_OFF;
			indicatorRight[1] = eLED_OFF;
			break;
		case eINDICATOR_PATTERN_LEFT:
			nIndicatorPatterns = 2;
			indicatorLeft[0] = eLED_ON;
			indicatorRight[0] = eLED_OFF;
			indicatorLeft[1] = eLED_OFF;
			indicatorRight[1] = eLED_OFF;
			break;
		case eINCICATOR_PATTERN_BOTH:
			nIndicatorPatterns = 2;
			indicatorLeft[0] = eLED_ON;
			indicatorRight[0] = eLED_ON;
			indicatorLeft[1] = eLED_OFF;
			indicatorRight[1] = eLED_OFF;
			break;
		case eINDICATOR_PATTERN_MOVING_RIGHT:
			nIndicatorPatterns = 4;
			indicatorLeft[0] = eLED_ON;
			indicatorRight[0] = eLED_OFF;
			indicatorLeft[1] = eLED_OFF;
			indicatorRight[1] = eLED_ON;
			indicatorLeft[2] = eLED_OFF;
			indicatorRight[2] = eLED_ON;
			indicatorLeft[3] = eLED_OFF;
			indicatorRight[3] = eLED_OFF;
			break;
		case eINDICATOR_PATTERN_MOVING_LEFT:
			nIndicatorPatterns = 4;
			indicatorLeft[0] = eLED_OFF;
			indicatorRight[0] = eLED_ON;
			indicatorLeft[1] = eLED_ON;
			indicatorRight[1] = eLED_OFF;
			indicatorLeft[2] = eLED_ON;
			indicatorRight[2] = eLED_OFF;
			indicatorLeft[3] = eLED_OFF;
			indicatorRight[3] = eLED_OFF;
			break;
		case eINDICATOR_PATTERN_WIG_WAG:
			nIndicatorPatterns = 2;
			indicatorLeft[0] = eLED_ON;
			indicatorRight[0] = eLED_OFF;
			indicatorLeft[1] = eLED_OFF;
			indicatorRight[1] = eLED_ON;
			break;
	}
}


static void displayAddConfigureI2CQueueEntry()
{
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&group_1_8_Transaction);
		I2CQueueInitializeTransaction(&group_9_16_Transaction);
	
		group_1_8_Transaction.nSLA_W = PCA9634_G1_8_SLA;
		group_1_8_Transaction.nSLA_R = 0;
		group_1_8_Transaction.cOutgoingData[0] = MODE1_INC;
		group_1_8_Transaction.cOutgoingData[1] = DISPLAY_CONFIG_MODEREG1;
		group_1_8_Transaction.cOutgoingData[2] = DISPLAY_CONFIG_MODEREG2;
		group_1_8_Transaction.nOutgoingDataLength = 3;
		group_1_8_Transaction.nIncomingDataLength = 0;
	
		group_9_16_Transaction.nSLA_W = PCA9634_G9_16_SLA;
		group_9_16_Transaction.nSLA_R = 0;
		group_9_16_Transaction.cOutgoingData[0] = MODE1_INC;
		group_9_16_Transaction.cOutgoingData[1] = DISPLAY_CONFIG_MODEREG1;
		group_9_16_Transaction.cOutgoingData[2] = DISPLAY_CONFIG_MODEREG2;
		group_9_16_Transaction.nOutgoingDataLength = 3;
		group_9_16_Transaction.nIncomingDataLength = 0;
	
		I2CQueueAddTransaction(&myI2CQueueEntry, &group_1_8_Transaction);
		I2CQueueAddTransaction(&myI2CQueueEntry, &group_9_16_Transaction);
}


static void displayAddI2CQueueEntry()
{
		I2CQueueEntryInitialize(&myI2CQueueEntry);
		I2CQueueInitializeTransaction(&group_1_8_Transaction);
		I2CQueueInitializeTransaction(&group_9_16_Transaction);

		group_1_8_Transaction.nSLA_W = PCA9634_G1_8_SLA;
		group_1_8_Transaction.nSLA_R = 0;
		group_1_8_Transaction.cOutgoingData[0] = LEDOUT0;
		group_1_8_Transaction.cOutgoingData[1] = displayPatterns[nCurrentDisplayPatternIndex].nGroup1_4Bits;
		group_1_8_Transaction.cOutgoingData[2] = displayPatterns[nCurrentDisplayPatternIndex].nGroup5_8Bits;
		group_1_8_Transaction.nOutgoingDataLength = 3;
		group_1_8_Transaction.nIncomingDataLength = 0;
	
		group_9_16_Transaction.nSLA_W = PCA9634_G9_16_SLA;
		group_9_16_Transaction.nSLA_R = 0;
		group_9_16_Transaction.cOutgoingData[0] = LEDOUT0;
		group_9_16_Transaction.cOutgoingData[1] = displayPatterns[nCurrentDisplayPatternIndex].nGroup9_12Bits;
		group_9_16_Transaction.cOutgoingData[2] = displayPatterns[nCurrentDisplayPatternIndex].nGroup13_16Bits;
		group_9_16_Transaction.nOutgoingDataLength = 3;
		group_9_16_Transaction.nIncomingDataLength = 0;
//printf("1-displayAddI2CQueueEntry nSLA_W[%X] nSLA_R[%X]\n", group_1_8_Transaction.nSLA_W, group_1_8_Transaction.nSLA_R);
//printf("1A-[%X][%X][%X]\n\r",group_1_8_Transaction.cOutgoingData[0], group_1_8_Transaction.cOutgoingData[1], group_1_8_Transaction.cOutgoingData[2]);
//printf("2-displayAddI2CQueueEntry nSLA_W[%X] nSLA_R[%X]\n", group_9_16_Transaction.nSLA_W, group_9_16_Transaction.nSLA_R);
//printf("2A-[%X][%X][%X]\n\r",group_9_16_Transaction.cOutgoingData[0], group_9_16_Transaction.cOutgoingData[1], group_9_16_Transaction.cOutgoingData[2]);
	
		I2CQueueAddTransaction(&myI2CQueueEntry, &group_1_8_Transaction);
		I2CQueueAddTransaction(&myI2CQueueEntry, &group_9_16_Transaction);
}

static void displaySetPatternData(int nPatternIndex, int nPatternTime, unsigned char* pPatternData)
{
			displayPatterns[nPatternIndex].nGroup1_4Bits		= pPatternData[0];
			displayPatterns[nPatternIndex].nGroup5_8Bits		= pPatternData[1];
			displayPatterns[nPatternIndex].nGroup9_12Bits		= pPatternData[2];
			displayPatterns[nPatternIndex].nGroup13_16Bits	= pPatternData[3];
			displayPatterns[nPatternIndex].nPatternTimeMS		= nPatternTime;
}

static BOOL isPatternBlank(int nIndex)
{
	BOOL bRetVal = TRUE;
	if((displayPatterns[nIndex].nGroup1_4Bits != 0) || 
		(displayPatterns[nIndex].nGroup5_8Bits != 0) ||
		(displayPatterns[nIndex].nGroup9_12Bits	!= 0) ||
		(displayPatterns[nIndex].nGroup13_16Bits != 0))
	{
		bRetVal = FALSE;
	}
	return bRetVal;
}	
static ePACKET_STATUS displayChange25Light(eDISPLAY_TYPES eDisplaySelection)
{
	ePACKET_STATUS eRetVal = ePACKET_STATUS_SUCCESS;

	switch(eDisplaySelection)
	{
		case eDISPLAY_TYPE_BLANK:
			nDisplayPatterns = 1;

			displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_OFF);

			break;
		case eDISPLAY_TYPE_FOUR_CORNER:
			nDisplayPatterns = 2;
		
			displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_FourCorner_1_25_light);
			displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINCICATOR_PATTERN_BOTH);
			break;
		case eDISPLAY_TYPE_DOUBLE_ARROW:
			nDisplayPatterns = 2;

		  displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_DoubleArrow_1_25_light);
		  displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINCICATOR_PATTERN_BOTH);
			break;
		case eDISPLAY_TYPE_BAR:
			nDisplayPatterns = 2;

		  displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_Bar_1_25_light);
		  displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINCICATOR_PATTERN_BOTH);
			break;
		case eDISPLAY_TYPE_RIGHT_ARROW:
			nDisplayPatterns = 2;

		  displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_RightArrow_1_25_light);
		  displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_RIGHT);
			break;
		case eDISPLAY_TYPE_LEFT_ARROW:
			nDisplayPatterns = 2;

		  displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_LeftArrow_1_25_light);
		  displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_LEFT);
			break;
		case eDISPLAY_TYPE_RIGHT_STEM_ARROW:
			nDisplayPatterns = 4;

		  displaySetPatternData(0, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_RightStemArrow_1_25_light);
		  displaySetPatternData(1, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_RightStemArrow_2_25_light);
		  displaySetPatternData(2, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_RightStemArrow_3_25_light);
		  displaySetPatternData(3, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_MOVING_RIGHT);
			break;
		case eDISPLAY_TYPE_LEFT_STEM_ARROW:
			nDisplayPatterns = 4;

		  displaySetPatternData(0, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_LeftStemArrow_1_25_light);
		  displaySetPatternData(1, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_LeftStemArrow_2_25_light);
		  displaySetPatternData(2, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_LeftStemArrow_3_25_light);
		  displaySetPatternData(3, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_MOVING_LEFT);
			break;
		case eDISPLAY_TYPE_RIGHT_WALKING_ARROW:
			nDisplayPatterns = 4;
	
		  displaySetPatternData(0, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_RightWalkingArrow_1_25_light);
		  displaySetPatternData(1, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_RightWalkingArrow_2_25_light);
		  displaySetPatternData(2, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_RightWalkingArrow_3_25_light);
		  displaySetPatternData(3, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_MOVING_RIGHT);
			break;
		case eDISPLAY_TYPE_LEFT_WALKING_ARROW:
			nDisplayPatterns = 4;

		  displaySetPatternData(0, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_LeftWalkingArrow_1_25_light);
		  displaySetPatternData(1, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_LeftWalkingArrow_2_25_light);
		  displaySetPatternData(2, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_LeftWalkingArrow_3_25_light);
		  displaySetPatternData(3, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_MOVING_LEFT);
			break;
		case eDISPLAY_TYPE_RIGHT_CHEVRON:
			nDisplayPatterns = 4;
		
		  displaySetPatternData(0, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_RightChevron_1_25_light);
		  displaySetPatternData(1, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_RightChevron_2_25_light);
		  displaySetPatternData(2, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_RightChevron_3_25_light);
		  displaySetPatternData(3, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_MOVING_RIGHT);
			break;
		case eDISPLAY_TYPE_LEFT_CHEVRON:
			nDisplayPatterns = 4;
		
		  displaySetPatternData(0, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_LeftChevron_1_25_light);
		  displaySetPatternData(1, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_LeftChevron_2_25_light);
		  displaySetPatternData(2, DISPLAY_SEQUENCE_4_TIME_MS	, Pattern_LeftChevron_3_25_light);
		  displaySetPatternData(3, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_MOVING_LEFT);
			break;
		case eDISPLAY_TYPE_DOUBLE_DIAMOND:
			nDisplayPatterns = 2;

		  displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_DoubleDiamond_1_25_light);
		  displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_DoubleDiamond_2_25_light);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_WIG_WAG);
			break;
		case eDISPLAY_TYPE_ALL_LIGHTS_ON:
			nDisplayPatterns = 1;

		  displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_AllOn_25_light);
			displaySetIndicatorPatterns(eINCICATOR_PATTERN_BOTH);
			break;
	}
	
	/////
	// disable the lamps
	/////
	pwmSetDriver(PWM_SUSPEND);
	
	/////
	// schedule the first pattern
	/////
	nCurrentDisplayPatternIndex = 0;
	nCurrentIndicatorPatternIndex = 0;
	if(displayHasIndicators())
	{
		ledDriverSetIndR(indicatorRight[nCurrentDisplayPatternIndex]);
		ledDriverSetIndL(indicatorLeft[nCurrentDisplayPatternIndex]);
	}
	displayAddI2CQueueEntry();
	I2CQueueAddEntry(&myI2CQueueEntry);
	startTimer(&patternTimer, displayPatterns[0].nPatternTimeMS);
	return eRetVal;
}
static ePACKET_STATUS displayChange15LightSequential(eDISPLAY_TYPES eDisplaySelection)
{
	ePACKET_STATUS eRetVal = ePACKET_STATUS_SUCCESS;
	switch(eDisplaySelection)
	{
		case eDISPLAY_TYPE_BLANK:
			nDisplayPatterns = 1;
	
			displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_OFF);
			break;
		case eDISPLAY_TYPE_FOUR_CORNER:
			nDisplayPatterns = 2;

			displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_FourCorner_1_sequential_15_light);
			displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINCICATOR_PATTERN_BOTH);
			break;
		case eDISPLAY_TYPE_DOUBLE_ARROW:
			nDisplayPatterns = 2;
		
			displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_DoubleArrow_1_sequential_15_light);
			displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINCICATOR_PATTERN_BOTH);
			break;
		case eDISPLAY_TYPE_BAR:
			nDisplayPatterns = 2;
		
			displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_Bar_1_sequential_15_light);
			displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINCICATOR_PATTERN_BOTH);
			break;
		case eDISPLAY_TYPE_RIGHT_ARROW:
			nDisplayPatterns = 2;
		
			displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_RightArrow_1_sequential_15_light);
			displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_RIGHT);
			break;
		case eDISPLAY_TYPE_LEFT_ARROW:
			nDisplayPatterns = 2;
		
			displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_LeftArrow_1_sequential_15_light);
			displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_LEFT);
			break;
		case eDISPLAY_TYPE_RIGHT_STEM_ARROW:
			nDisplayPatterns = 4;
		
			displaySetPatternData(0, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_RightStemArrow_1_sequential_15_light);
			displaySetPatternData(1, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_RightStemArrow_2_sequential_15_light);
			displaySetPatternData(2, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_RightStemArrow_3_sequential_15_light);
			displaySetPatternData(3, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_MOVING_RIGHT);
			break;
		case eDISPLAY_TYPE_LEFT_STEM_ARROW:
			nDisplayPatterns = 4;
		
			displaySetPatternData(0, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_LeftStemArrow_1_sequential_15_light);
			displaySetPatternData(1, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_LeftStemArrow_2_sequential_15_light);
			displaySetPatternData(2, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_LeftStemArrow_3_sequential_15_light);
			displaySetPatternData(3, DISPLAY_SEQUENCE_4_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_MOVING_LEFT);
			break;
		case eDISPLAY_TYPE_ALL_LIGHTS_ON:
			nDisplayPatterns = 1;

		  displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_AllOn_sequential_15_light);
			displaySetIndicatorPatterns(eINCICATOR_PATTERN_BOTH);
			break;
		case eDISPLAY_TYPE_RIGHT_WALKING_ARROW:
		case eDISPLAY_TYPE_LEFT_WALKING_ARROW:
		case eDISPLAY_TYPE_RIGHT_CHEVRON:
		case eDISPLAY_TYPE_LEFT_CHEVRON:
		case eDISPLAY_TYPE_DOUBLE_DIAMOND:
			eRetVal = ePACKET_STATUS_NOT_SUPPORTED;
			break;
	}
	
	if(ePACKET_STATUS_SUCCESS == eRetVal)
	{
		/////
		// disable the lamps
		/////
		pwmSetDriver(PWM_SUSPEND);
	
		/////
		// schedule the first pattern
		/////
		nCurrentDisplayPatternIndex = 0;
		nCurrentIndicatorPatternIndex = 0;
		if(displayHasIndicators())
		{
			ledDriverSetIndR(indicatorRight[nCurrentDisplayPatternIndex]);
			ledDriverSetIndL(indicatorLeft[nCurrentDisplayPatternIndex]);
		}
		displayAddI2CQueueEntry();
		I2CQueueAddEntry(&myI2CQueueEntry);
		startTimer(&patternTimer, displayPatterns[0].nPatternTimeMS);
	}
	return eRetVal;
}
static ePACKET_STATUS displayChange15LightFlashing(eDISPLAY_TYPES eDisplaySelection)
{
	ePACKET_STATUS eRetVal = ePACKET_STATUS_SUCCESS;
	switch(eDisplaySelection)
	{
		case eDISPLAY_TYPE_BLANK:
			nDisplayPatterns = 1;			

			displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_OFF);
			break;
		case eDISPLAY_TYPE_FOUR_CORNER:
			nDisplayPatterns = 2;
		
			displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_FourCorner_1_flashing_15_light);
			displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINCICATOR_PATTERN_BOTH);
			break;
		case eDISPLAY_TYPE_DOUBLE_ARROW:
			nDisplayPatterns = 2;
		
		  displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_DoubleArrow_1_flashing_15_light);
		  displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINCICATOR_PATTERN_BOTH);
			break;
		case eDISPLAY_TYPE_BAR:
			nDisplayPatterns = 2;
		
		  displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_Bar_1_flashing_15_light);
		  displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINCICATOR_PATTERN_BOTH);
			break;
		case eDISPLAY_TYPE_RIGHT_ARROW:
			nDisplayPatterns = 2;
		
		  displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_RightArrow_1_flashing_15_light);
		  displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_RIGHT);
			break;
		case eDISPLAY_TYPE_LEFT_ARROW:
			nDisplayPatterns = 2;
		
		  displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_LeftArrow_1_flashing_15_light);
		  displaySetPatternData(1, DISPLAY_FLASHING_TIME_MS, Pattern_Blank);
		
			displaySetIndicatorPatterns(eINDICATOR_PATTERN_LEFT);
			break;
		case eDISPLAY_TYPE_ALL_LIGHTS_ON:
			nDisplayPatterns = 1;

		  displaySetPatternData(0, DISPLAY_FLASHING_TIME_MS, Pattern_AllOn_flashing_15_light);
			displaySetIndicatorPatterns(eINCICATOR_PATTERN_BOTH);
			break;
		case eDISPLAY_TYPE_RIGHT_STEM_ARROW:
		case eDISPLAY_TYPE_LEFT_STEM_ARROW:
		case eDISPLAY_TYPE_RIGHT_WALKING_ARROW:
		case eDISPLAY_TYPE_LEFT_WALKING_ARROW:
		case eDISPLAY_TYPE_RIGHT_CHEVRON:
		case eDISPLAY_TYPE_LEFT_CHEVRON:
		case eDISPLAY_TYPE_DOUBLE_DIAMOND:
			eRetVal = ePACKET_STATUS_NOT_SUPPORTED;
			break;
	}
	if(ePACKET_STATUS_SUCCESS == eRetVal)
	{
		/////
		// disable the lamps
		/////
		pwmSetDriver(PWM_SUSPEND);
	
		/////
		// schedule the first pattern
		/////
		nCurrentDisplayPatternIndex = 0;
		nCurrentIndicatorPatternIndex = 0;
		ledDriverSetIndR(indicatorRight[nCurrentDisplayPatternIndex]);
		ledDriverSetIndL(indicatorLeft[nCurrentDisplayPatternIndex]);
		displayAddI2CQueueEntry();
		I2CQueueAddEntry(&myI2CQueueEntry);
		startTimer(&patternTimer, displayPatterns[0].nPatternTimeMS);
	}
	return eRetVal;
}


void displayInit(eDISPLAY_TYPES eInitialType)
{
	eInitialDisplaySelection = eInitialType;
	
	/////
	// remember the model number
	/////
	eModel = getModelNumber();
	
	//////////////////////////////////////
	//// Set the Lamp Rail According to
	//// the Model Number (Solar/Vehicle)
	//////////////////////////////////////
	switch(eModel)
	{
		///////////////////////////////////////
		//// Vehicle Mount Arrow Boards use
		//// 12 volt bulbs so set the lamp
		//// rail to 12 volts
		///////////////////////////////////////
		case eMODEL_VEHICLE_25_LIGHT_SEQUENTIAL:
		case eMODEL_VEHICLE_15_LIGHT_SEQUENTIAL:
		case eMODEL_VEHICLE_15_LIGHT_FLASHING:
			hwLampRailSelect(LAMP_RAIL_12V0);
			printf("displayInit: LAMP_RAIL_12V0 Chosen\n");
			break;
		
		///////////////////////////////////////
		//// Solar Trailer Arrow Boards use
		//// 8.4 volt bulbs so set the lamp
		//// rail to 8.4 volts
		///////////////////////////////////////
		case eMODEL_TRAILER_25_LIGHT_SEQUENTIAL:
		case eMODEL_TRAILER_15_LIGHT_SEQUENTIAL:
		case eMODEL_TRAILER_15_LIGHT_FLASHING:
			hwLampRailSelect(LAMP_RAIL_8V4);
			printf("displayInit: LAMP_RAIL_8V4 Chosen\n");
			break;	

		default:
			break;
	} 
	/////
	// initialize the pattern timer
	/////
	initTimer(&patternTimer);	
	
	/////
	// schedule I2C transaction to configure the driver
	// leave all drivers off for now
	/////
	pwmSetDriver(PWM_SUSPEND);
	displayAddConfigureI2CQueueEntry();
	I2CQueueAddEntry(&myI2CQueueEntry);
	
	nDisplayPatterns = 0;
	nIndicatorPatterns = 0;
}
ePACKET_STATUS displayChange(unsigned short nDisplaySelection)
{
	ePACKET_STATUS eRetVal = ePACKET_STATUS_SUCCESS;
	eDISPLAY_TYPES eDisplayType = (eDISPLAY_TYPES)nDisplaySelection;
	if((ADCGetLineVoltage() >= lineVoltageShutdownLimit) || ADCGetBatteryVoltage() >= batteryVoltageShutdownLimit)
	{
		printf("1-DisplayChange nDisplaySelection[%d] eModel[%d]\n", nDisplaySelection, eModel);
		switch(eModel)
		{
			case eMODEL_VEHICLE_25_LIGHT_SEQUENTIAL:
			case eMODEL_TRAILER_25_LIGHT_SEQUENTIAL:
				eRetVal = displayChange25Light(eDisplayType);
				break;
			case eMODEL_VEHICLE_15_LIGHT_SEQUENTIAL:
			case eMODEL_TRAILER_15_LIGHT_SEQUENTIAL:
				eRetVal = displayChange15LightSequential(eDisplayType);
				break;
			case eMODEL_VEHICLE_15_LIGHT_FLASHING:
			case eMODEL_TRAILER_15_LIGHT_FLASHING:
				eRetVal = displayChange15LightFlashing(eDisplayType);
				break;
			case eMODEL_NONE:
				break;
		}
	}
	else
	{
		/////
		// voltage too low to start display
		/////
		eRetVal = ePACKET_STATUS_GENERAL_ERROR;
	}
	if(ePACKET_STATUS_SUCCESS == eRetVal)
	{
		eCurrentDisplaySelection = eDisplayType;
		*((unsigned short*)pLastDisplayType) = eDisplayType;
	}
	else
	{
		eCurrentDisplaySelection = eDISPLAY_TYPE_BLANK;	
		*((unsigned short*)pLastDisplayType) = eDisplayType;
	}
	return eRetVal;
}

unsigned short displayGetCurrentPattern()
{

	return (unsigned short)eCurrentDisplaySelection;
}

unsigned short displayGetErrors()
{
	return errorInputGetDriveErrors();
}

void displayDoWork()
{
	BOOL bUpdateIndex = TRUE;
	switch(myI2CQueueEntry.eState)
	{
		case eI2C_TRANSFER_STATE_COMPLETE:
			if(TRUE == bInit)
			{
				/////
				// just booted up,
				// so set display to blank
				/////
				bInit = FALSE;
				displayChange(eInitialDisplaySelection);
			}
			else
			{
				//printf("1-\n\r");
				/////
				// transfer is complete
				// enable the lamps (if necessary)
				// and start the timer
				/////
				if((ADCGetLineVoltage() >= lineVoltageShutdownLimit) || ADCGetBatteryVoltage() >= batteryVoltageShutdownLimit)
				{
					if(isPatternBlank(nCurrentDisplayPatternIndex))
					{
						pwmSetDriver(PWM_SUSPEND);
					}
					else
					{
						pwmSetDriver(PWM_RESUME);
					}
				}
				else
				{
					/////
					// voltage too low
					// don't supply power to display
					/////
					pwmSetDriver(PWM_SUSPEND);
				}
				startTimer(&patternTimer, displayPatterns[nCurrentDisplayPatternIndex].nPatternTimeMS);
				myI2CQueueEntry.eState = eI2C_TRANSFER_STATE_IDLE;
			}
			break;
		case eI2C_TRANSFER_STATE_FAILED:
			//printf("2-\n\r");
			/////
			// transfer failed, so resend this one
			/////
			bUpdateIndex = FALSE;
		
			/////
			// setup the transfer state
			// and expire the timer
			// so we will start this pattern now
			/////
			myI2CQueueEntry.eState = eI2C_TRANSFER_STATE_IDLE;
			stopTimer(&patternTimer);
			break;
		case eI2C_TRANSFER_STATE_IDLE:
			//printf("3-\n\r");
			break;
		default:
			//printf("4-\n\r");
			break;
	}
	if((eI2C_TRANSFER_STATE_IDLE == myI2CQueueEntry.eState) && (isTimerExpired(&patternTimer)))
	{
		//printf("5-\n");
		/////
		// timer expired
		// disable lamps
		// and send the next pattern
		//////
		pwmSetDriver(PWM_SUSPEND);
		if(bUpdateIndex)
		{
			/////
			// update indicator pattern index
			/////
			nCurrentIndicatorPatternIndex++;
			if(nCurrentIndicatorPatternIndex >= nIndicatorPatterns)
			{
				nCurrentIndicatorPatternIndex = 0;
			}
			
			/////
			// update display pattern index
			/////		
			nCurrentDisplayPatternIndex++;
			if(nCurrentDisplayPatternIndex >= nDisplayPatterns)
			{
				nCurrentDisplayPatternIndex = 0;
			}
		}
		
		ledDriverSetIndR(indicatorRight[nCurrentDisplayPatternIndex]);
		ledDriverSetIndL(indicatorLeft[nCurrentDisplayPatternIndex]);

		/////
		// update the display
		/////

		displayAddI2CQueueEntry();
		I2CQueueAddEntry(&myI2CQueueEntry);
	}
}
