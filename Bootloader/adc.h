/*****************************************************************************
 *   adc.h:  Header file for NXP LPC23xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.09.20  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __ADC_H 
#define __ADC_H

#define ADC_INTERRUPT_FLAG	1	/* 1 is interrupt driven, 0 is polling */

#define ADC_OFFSET		0x10
#define ADC_INDEX			4

#define ADC_DONE			0x80000000
#define ADC_OVERRUN		0x40000000
#define ADC_ADINT			0x00010000

#define PCLK_ADC_BIT_24		24
#define PCLK_ADC_BIT_25		25

#define ADC_NUM				8					/* for LPC23xx */
#define ADC_CLK				1000000		/* set to 1Mhz */

#define ADVREF				(3.0)
#define VS_REF        (3.3) 	// REFERENCE FOR 'VBAT MONITOR' DIFF AMP 
#define NUMBITS				(10)	 	// conversion 'width' N bits (aka 10 bits /8 bits)

#define SOLAR_FACTOR	(11) 		// Multiplier to account for R divider on solar

#define SOLAR_CHARGE_MAX_THRESHOLD (1350) // MAX Solar Charge On Battery
#define SOLAR_CHARGE_MIN_THRESHOLD (1300) // MIN Solar Charge On Battery

static const float ADC_SCALE_FACTOR = (ADVREF/(1 << NUMBITS));

//extern void ADC0Handler(void) __irq;
//extern DWORD ADCInit   (DWORD ADC_Clk);
//extern DWORD ADC0Read  (BYTE channelNum);
float  CONVERT_TO_VS   (int);							// system voltage from diff amp
float  CONVERT_TO_VB   (int);							// barbie battery/aux bank
float  CONVERT_TO_VD   (int);							// lamp voltage 8V4/12V0
int    CONVERT_PCELL   (int);							// photocell reading

void  ADCInit(void);
void  ADCDoWork(void);
WORD ADCGetVs(void);
WORD ADCGetVl(void);
WORD DCGetVb(void);
WORD ADCGetVd(void);
WORD ADCGetPr(void);
WORD ADCGetPf(void);

unsigned short ADCGetLineVoltage(void);
unsigned short ADCGetSystemVoltage(void);
unsigned short ADCGetBatteryVoltage(void);
unsigned short ADCGetLineCurrent(void);
unsigned short ADCGetSignCurrent(void);


#endif /* end __ADC_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
