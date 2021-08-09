#ifndef DISPLAY_H
#define DISPLAY_H
#include "commands.h"

#define PCA9634_G1_8_SLA						0xE4
#define PCA9634_G9_16_SLA						0xE6

// U24 SYSTEM LEDS AND BCN+INDICATOR PWM
#define DISPLAY_CONFIG_MODEREG1   0x00		//
#define DISPLAY_CONFIG_MODEREG2	  0x34		// configure this chip as
																						// ??? This is not the setting
																						//LEDn = 1 WHEN OUTDRV = 1 (bit0,1)
																						// ???
																						// outut = totem pole (bit2)
																						// output sense invert (bit4)
typedef enum eLampConfig
{
	eLAMP_OFF 				= 0x00,
	eLAMP_ON					= 0x01,
	eLAMP_PWM_BRIGHT	= 0x02,
	eLAMP_GRP_BLINK		= 0x03,
	eLAMP_ALL_BLINK		= 0xFF 	// related to above, set all ch to blink enable (bit pattern '11')
}eLAMP_CONFIG_BITS;

#define DISPLAY_GROUP1 	(eLAMP_ON<<0)
#define DISPLAY_GROUP2 	(eLAMP_ON<<2)
#define DISPLAY_GROUP3 	(eLAMP_ON<<4)
#define DISPLAY_GROUP4 	(eLAMP_ON<<6)
	
#define DISPLAY_GROUP5 	(eLAMP_ON<<0)
#define DISPLAY_GROUP6 	(eLAMP_ON<<2)
#define DISPLAY_GROUP7 	(eLAMP_ON<<4)
#define DISPLAY_GROUP8 	(eLAMP_ON<<6)
	
#define DISPLAY_GROUP9 	(eLAMP_ON<<0)
#define DISPLAY_GROUP10 (eLAMP_ON<<2)
#define DISPLAY_GROUP11 (eLAMP_ON<<4)
#define DISPLAY_GROUP12 (eLAMP_ON<<6)

#define DISPLAY_GROUP13 (eLAMP_ON<<0)
#define DISPLAY_GROUP14 (eLAMP_ON<<2)
#define DISPLAY_GROUP15 (eLAMP_ON<<4)
#define DISPLAY_GROUP16 (eLAMP_ON<<6)

#define DISPLAY_TIME_MS 1875
#define DISPLAY_FLASHING_TIME_MS DISPLAY_TIME_MS/2
#define DISPLAY_SEQUENCE_4_TIME_MS DISPLAY_TIME_MS/4




#define MAX_DISPLAY_PATTERNS 4

typedef struct displayPattern
{
	unsigned char nGroup1_4Bits;
	unsigned char nGroup5_8Bits;
	unsigned char nGroup9_12Bits;
	unsigned char nGroup13_16Bits;
	int	nPatternTimeMS;
}DISPLAY_PATTERN;

/////
// this points to the first position in USB static ram
// this ram is not initialized at startup
// so we can use it for watchdog timeout recovery
// to redisplay the current pattern
//////
#define pLastDisplayType 0x7FD00000

void displayInit(eDISPLAY_TYPES eInitialType);
void displayDoWork(void);
BOOL displayHasIndicators(void);
ePACKET_STATUS displayChange(unsigned short nDisplaySelection);
unsigned short displayGetCurrentPattern(void);
unsigned short displayGetErrors(void);
#endif		// DISPLAY_H
