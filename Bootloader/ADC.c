/*****************************************************************************
 *   adc.c:  ADC module file for NXP LPC23xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.08.15  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include "LPC23xx.h"                        /* LPC23xx definitions */
#include <stdio.h>
#include <string.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "timer.h"
#include "lm73.h"
#include "mischardware.h"

#include "adc.h"
#include "irq.h"



#define ADC_Fpclk 48000000


volatile WORD ADC0Value[ADC_NUM];

static volatile BYTE ADC0IntDone = 0;
static volatile BYTE ADC1IntDone = 0;
static volatile BYTE ADC2IntDone = 0;
static volatile BYTE ADC3IntDone = 0;
static volatile BYTE ADC4IntDone = 0;
static volatile BYTE ADC5IntDone = 0;
static volatile WORD nVbatt;

#define SIZE_OF_MEDIAN_FILTER   5
#define MEDIAN_VALUE_INDEX      2

#define SIZE_OF_AVG_FILTER      8
#define DIVIDE_BY_EIGHT_SHIFT   3

#define DEFAULT_VOLTAGE         1200

typedef struct {
  BYTE medianIndex;
  BYTE avgIndex;
  UINT avgValues[SIZE_OF_AVG_FILTER];
  UINT medianValues[SIZE_OF_MEDIAN_FILTER];
  UINT filteredValue;
} ADC_FILTER;


static ADC_FILTER lineVoltageFilter;
static ADC_FILTER batteryVoltageFilter;

WORD ADCGetVs(void);
WORD ADCGetIl(void);
WORD ADCGetVl(void);
WORD ADCGetVb(void);
WORD ADCGetIs(void);
WORD ADCGetPr(void);
WORD ADCGetPf(void);



static void initAdcFilter(ADC_FILTER *filter)
{
   int i;
    
   filter->medianIndex = 0;
   filter->avgIndex = 0;
    
   for(i=0; i<SIZE_OF_MEDIAN_FILTER; i++)
   {
       filter->medianValues[i] = DEFAULT_VOLTAGE;
   }
   
   for(i=0; i<SIZE_OF_AVG_FILTER; i++)
   {
       filter->avgValues[i] = DEFAULT_VOLTAGE;
   }   
}

static UINT getMedianValue(UINT medianValues[])
// sort array of values in ascending order
// return median value
{
	int i,j;
        
	for (i = 0; i < SIZE_OF_MEDIAN_FILTER; i++) //Loop for ascending ordering
	{
		for (j = 0; j < SIZE_OF_MEDIAN_FILTER; j++)  //Loop for comparing other values
		{
			if (medianValues[j] > medianValues[i])  //Comparing other array elements
			{
				int tmp = medianValues[i];         //Using temporary variable for storing last value
				medianValues[i] = medianValues[j]; //replacing value
				medianValues[j] = tmp;             //storing last value
			}  
		}
	}
 
    return medianValues[MEDIAN_VALUE_INDEX];   
}

static UINT adcFilter(ADC_FILTER *filter, UINT newValue)
// 1st stage - median filter (noise rejection)
// 2nd stage - avg values (smoothing)
{
    unsigned long sum = 0;
    int i;

    filter->medianValues[filter->medianIndex] = newValue;
    if(++filter->medianIndex > SIZE_OF_MEDIAN_FILTER)
    {
		filter->medianIndex = 0;
    }

    filter->avgValues[filter->avgIndex] = getMedianValue(&filter->medianValues[0]);
    
    if(++filter->avgIndex >= SIZE_OF_AVG_FILTER)
    { 
       filter->avgIndex = 0;
    }

    for(i=0; i<SIZE_OF_AVG_FILTER; i++)
    {
      sum += filter->avgValues[i];
    }
		
    sum = sum/SIZE_OF_AVG_FILTER;
    
 
    return ((UINT)sum);  
}



#if ADC_INTERRUPT_FLAG
/******************************************************************************
** Function name:		ADC0Handler
**
** Descriptions:		ADC0 interrupt handler
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
static void ADC0Handler (void) __irq 
{
  DWORD regVal;
	
  regVal = AD0STAT;		/* Read ADC will clear the interrupt */
	
  if ( regVal & 0x0000FF00 )	/* check OVERRUN error first */
  {
		regVal = (regVal & 0x0000FF00) >> 0x08;
		
		/* if overrun, just read ADDR to clear */
		/* regVal variable has been reused. */
		switch ( regVal )
		{
			case 0x01:
				regVal = AD0DR0;
				break;
	
			case 0x02:
				regVal = AD0DR1;
				break;
	
			case 0x04:
				regVal = AD0DR2;
				break;
	
			case 0x08:
				regVal = AD0DR3;
				break;
	
			case 0x10:
				regVal = AD0DR4;
				break;
	
			case 0x20:
				regVal = AD0DR5;
				break;
	
			default:
				break;
		}
  }
    
  if ( regVal & ADC_ADINT )
  {
		switch ( regVal & 0xFF )	/* check DONE bit */
		{
			case 0x01:
				ADC0Value[0] = ( AD0DR0 >> 6 ) & 0x3FF;
				ADC0IntDone = 1;
				break;
	
			case 0x02:				
				ADC0Value[1] = ( AD0DR1 >> 6 ) & 0x3FF;
				ADC1IntDone = 1;
				break;
	
			case 0x04:
				ADC0Value[2] = ( AD0DR2 >> 6 ) & 0x3FF;
				ADC2IntDone = 1;
				break;
	
			case 0x08:
				ADC0Value[3] = ( AD0DR3 >> 6 ) & 0x3FF;
				ADC3IntDone = 1;
				break;
	
			case 0x10:
				ADC0Value[4] = ( AD0DR4 >> 6 ) & 0x3FF;
				ADC4IntDone = 1;
				break;
	
			case 0x20:
				ADC0Value[5] = ( AD0DR5 >> 6 ) & 0x3FF;
				ADC5IntDone = 1;
				break;
					
			default:
				break;
		}
  }
  VICVectAddr = 0;		/* Acknowledge Interrupt */
}
#endif

/*****************************************************************************
** Function name:		ADCHWInit
**
** Descriptions:		initialize ADC channel
**
** parameters:			ADC clock rate
** Returned value:		true or false
** 
*****************************************************************************/
static DWORD ADCHWInit( DWORD ADC_Clk )
{
  /* Enable CLOCK into ADC controller */
  PCONP |= (1 << PCAD);
	
	PCLKSEL0 |= (1 << PCLK_ADC_BIT_25);
	
  /* all the related pins are set to ADC inputs, AD0.0~5 (LPC2366 is a 6 channel ADC)*/
  PINSEL1 |= 0x00154000;	/* P0.23~26, A0.0~3, function 01 */
  PINSEL3 |= 0xF0000000;	/* P1.30~31, A0.4~5, function 11 */

  AD0CR = ( 1 << 0  ) | 	/* SEL=1,select channel 0~5 on ADC0 */
					( 7 << 8  ) | //( ( ADC_Fpclk / ADC_Clk - 1 ) << 8 ) |  /* CLKDIV = ADC_Fpclk / 1000000 - 1 */ 
					( 0 << 16 ) | 	/* BURST = 0, no BURST, software controlled */
					( 0 << 17 ) |  	/* CLKS = 0, 11 clocks/10 bits */
					( 1 << 21 ) |  	/* PDN = 1, normal operation */
					( 0 << 22 ) |  	/* TEST1:0 = 00 */
					( 0 << 24 ) |  	/* START = 0 A/D conversion stops */
					( 0 << 27 );		/* EDGE = 0 (CAP/MAT singal falling,trigger A/D conversion) */ 

	//printf("AD0CR[%X]\n",AD0CR);
	
  /* If POLLING, no need to do the following */
#if ADC_INTERRUPT_FLAG
	
  AD0INTEN = 0x03F;		/* Enable interrupts for channels 0 thru 5 (LPC2366 is a 6 channel ADC)*/
	
  if ( install_irq( ADC0_INT, ADC0Handler, HIGHEST_PRIORITY ) == FALSE )
  {
		return (FALSE);
  }
	
#endif
	
  return (TRUE);
}

/*****************************************************************************
** Function name:		ADC0Read
**
** Descriptions:		Read ADC0 channel
**
** parameters:			Channel number
** Returned value:		Value read, if interrupt driven, return channel #
** 
*****************************************************************************/
static DWORD ADC0Read( BYTE channelNum )
{
#if !ADC_INTERRUPT_FLAG
  DWORD regVal, ADC_Data;
#endif

  /* channel number is 0 through 5 */
  if ( channelNum >= ADC_NUM )
  {
		channelNum = 0;		/* reset channel number to 0 */
  }
	
	AD0CR &= 0x0020FF00;
	
	AD0CR |= (1 << 24) |        // Start Conversion Now
					 (1 << channelNum);	// on the selected channel number
	
	//printf("ADC0Read: AD0CR[%X]\n",AD0CR);
		/* switch channel,start A/D convert */
#if !ADC_INTERRUPT_FLAG
  while ( 1 )			/* wait until end of A/D convert */
  {
		regVal = *(volatile unsigned long *)(AD0_BASE_ADDR 
						 + ADC_OFFSET + ADC_INDEX * channelNum);
		/* read result of A/D conversion */
		if ( regVal & ADC_DONE )
		{
			break;
		}
  }	
        
  AD0CR &= 0x08FFFFFF;	/* stop ADC now */
	
  if ( regVal & ADC_OVERRUN )	/* save data when it's not overrun, otherwise, return zero */
  {
		return ( 0 );
  }

  ADC_Data = ( regVal >> 6 ) & 0x3FF;
	
  return ( ADC_Data );    	/* return A/D conversion value */
	
#else
	
  return ( channelNum );	/* if it's interrupt driven, the ADC reading is 
							done inside the handler. so, return channel number */
#endif
}



// ******************
// WPN ADDED UTILITY FUNCTIONS RELATED TO ADC
// ************************************************
// CONVERT AD VAL TO SYSTEM/BATTERY VOLTAGE
// NOTE THIS IS NOT THE SAME AS VBAT = STRAIGHT TO BARBIE BATTERY/VOLTAGE DIVIDER
float CONVERT_TO_VS(int adreturn)
{	
	float v;
	
	// from Vbat monitor circuit: input to A/D = Vadc
	// Vbat = 1.847*Vadc + 2.983*Vref (See sch - diff amp vref, not ADC)
	//      =    K1*Vadc +    K2*Vref.
	float K1 = 8.01087;	 //FIXME: Magic #'s.  Belong in .h?
	float K2 = 2.98913;
	
	v = adreturn*K1*ADC_SCALE_FACTOR + K2*VS_REF;
	
	return v;
}

// CONVERT AD VAL TO Line Current
float CONVERT_TO_IL(int adreturn)
{	
	//////
	//  REVE PCU VB full scale current is 27.2
	// reference voltage is 3.0v
	// K1 = 27.2/3 = 9.066666667
	/////
	float K1 = 9.0667;
	
	/////
	//  REVE PCU VB full scale current is 27.2
	// reference voltage is 3.0v
	// K1 = 27.2/3 = 9.066666667
	/////
	float fAmps = adreturn*K1*ADC_SCALE_FACTOR;
	return fAmps;
}

// CONVERT TO BATTERY VOLTAGE
// NOTE THIS IS NOT THE SAME AS VS = SYSTEM VOLTAGE FROM DIFF AMP
// system voltage = [barbie, solar, alternator] --> LTC3824
float CONVERT_TO_VB(int adreturn)
{	
	float fVolts;
	/////
	// reference voltage is 3.0v
	// 10 bit a/d, 1024 steps
	//  REVE PCU VB full scale voltage is 16,58
	// reference voltage is 3.0v
	// K1 = 16.58/3 = 5.52666667
	/////

	float K1;
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		default:	
			//Vb is fed to 4.02k/40.2k voltage divider = div by 11
			K1 = 11.2;
			break;
		case eBoardRev2:
			K1 = 5.52666667;
			break;
		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			K1 = 5.52666667;
			break;
	}
	
	fVolts = adreturn*K1*ADC_SCALE_FACTOR;
	return fVolts;
}

// CONVERT TO LINE VOLTAGE
// NOTE THIS IS NOT THE SAME AS VS = SYSTEM VOLTAGE FROM DIFF AMP
// system voltage = [barbie, solar, alternator] --> LTC3824
float CONVERT_TO_VL(int adreturn)
{
	float v;
	
	//Vb is fed to 4.02k/40.2k voltage divider = div by 11
	float K1 = 11.2;	 //FIXME: Magic #'s.  Belong in .h?
		
	v = adreturn*K1*ADC_SCALE_FACTOR;
	
	return v;
}

// CONVERT TO LAMP VOLTAGE
float CONVERT_TO_VD(int adreturn)
{
	float v;
	/////
	//  REVE PCU VD full scale voltage is 17.1
	// adc reference voltage is 3.0v
	// K1 = 17.1/3 = 5.7
	//////
	float K1;
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		default:
			K1 = 4.4;
			break;
		case eBoardRev2:
			K1 = 5.7;
			break;
		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			K1 = 5.7;
			break;
	}
	//Vd is fed to 10k/34K voltage divider = div by 4.4
	//float K1 = 4.4;	 //FIXME: Magic #'s.  Belong in .h?
	
	//printf("Vd adc return: %d\n\r", adreturn);
	v = adreturn*K1*ADC_SCALE_FACTOR;
	
	return v;
}

// CONVERT TO sign current
float CONVERT_TO_IS(int adreturn)
{
	//////
	//  REVE PCU VB full scale current is 27.2
	// reference voltage is 3.0v
	// K1 = 27.2/3 = 9.066666667
	/////
	float K1 = 9.0667;
	
	/////
	//  REVE PCU VB full scale current is 27.2
	// reference voltage is 3.0v
	// K1 = 27.2/3 = 9.066666667
	/////
	float fAmps = adreturn*K1*ADC_SCALE_FACTOR;
	return fAmps;
}
//============================================
void ADCInit()
{
    memset((unsigned char*)ADC0Value, 0, sizeof(ADC0Value));
	
    initAdcFilter(&lineVoltageFilter);
    initAdcFilter(&batteryVoltageFilter);

	ADCHWInit( ADC_CLK );
}

void ADCDoWork()
{
	static BYTE nChannelNum = 0;
    float fValue;
    unsigned int nValue;

//#define DEBUG_MEDIAN_FILTER 1    

#ifdef DEBUG_MEDIAN_FILTER
int i;
#endif    
	
	DWORD regVal = *(volatile unsigned long *)(AD0_BASE_ADDR 
									+ ADC_OFFSET + ADC_INDEX * nChannelNum);
	
	/* read result of A/D conversion */
	if ( !(regVal & ADC_DONE) )
	{   
        if(1 == ADC1IntDone)
        {
            ADC1IntDone = 0;
            fValue = CONVERT_TO_VL(ADCGetVl());
            nValue = fValue*100;
            lineVoltageFilter.filteredValue = adcFilter(&lineVoltageFilter, nValue);
        }
        
        if(1 == ADC2IntDone)
        {
            ADC2IntDone = 0;
            fValue = CONVERT_TO_VB(ADCGetVb());
            nValue = fValue*100;
            batteryVoltageFilter.filteredValue = adcFilter(&batteryVoltageFilter, nValue);

#ifdef DEBUG_MEDIAN_FILTER
for(i=0; i<SIZE_OF_MEDIAN_FILTER; i++)
{
    printf("[%d] ", batteryVoltageFilter.medianValues[i]);
}
printf("*********************\r\n");
#endif
        }
        
		/////
		// previous conversion is complete
		// data was placed in ADC0Value[nChannelNum]
		/////
		
		/////
		// kick off the next conversion
		/////
		switch(nChannelNum)
		{
			case 0:
			case 1:
			case 2:
				nChannelNum++;
				break;
			case 3:
				nChannelNum = 4;
				break;
			case 4:
				nChannelNum = 5;
				break;
			case 5:
			default:
				nChannelNum = 0;
				break;
		}
		
		/////
		// start the next conversion
		/////
		ADC0Read(nChannelNum);
	}
}

WORD ADCGetVs()
{
	return ADC0Value[0];
}

WORD ADCGetIl()
{
	return ADC0Value[0];
}

WORD ADCGetVl()
{
	return ADC0Value[1];
}

WORD ADCGetVb()
{
	return ADC0Value[2];
}


WORD ADCGetIs()
{
	return ADC0Value[3];
}
//=======================
// Rear Facing PhotoCell
//=======================
WORD ADCGetPr()
{
	return ADC0Value[4];
}

//==========================
// Forward Facing PhotoCell
//==========================
WORD ADCGetPf()
{
	return ADC0Value[5];
}

unsigned short ADCGetLineVoltage()
{
    return lineVoltageFilter.filteredValue;
}

unsigned short ADCGetSystemVoltage()
{
	unsigned short nVolts = 0;
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		case eBoardRev2:
		default:
			{
				float fVolts;
	
				if(ADC0IntDone)
				{	
					fVolts = CONVERT_TO_VS(ADCGetVs());
		
					nVolts = fVolts*100;
		
					ADC0IntDone = 0;
				}
			}
			break;
		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			break;
	}
	return nVolts;
}
unsigned short ADCGetLineCurrent()
{
	unsigned short nAmps = 0;
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		case eBoardRev2:
		default:
			{
				/////
				// DEH 092017
				// this doesn't belong here
				// it will never be used for this board revision
				/////
				float fAmps;
	
				if(ADC0IntDone)
				{	
					fAmps = CONVERT_TO_IL(ADCGetIl());
		
					/////
					// convert to milliamps
					/////					
					nAmps = fAmps*1000;
					ADC0IntDone = 0;
				}
			}
			break;
		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			{
				float fAmps;
	
				if(ADC0IntDone)
				{	
					fAmps = CONVERT_TO_IL(ADCGetIl());

		
					/////
					// convert to milliamps
					/////					
					nAmps = fAmps*1000;
					ADC0IntDone = 0;
				}
			}
			break;
	}
	return nAmps;
}

unsigned short ADCGetSignCurrent()
{
	unsigned short nAmps = 0;
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		case eBoardRev2:
		default:
			{
				/////
				// DEH 092017
				// this doesn't belong here
				// it will never be used for this board revision
				/////
				float fAmps;
	
				if(ADC3IntDone)
				{	
					fAmps = CONVERT_TO_IS(ADCGetIs());
					
					/////
					// convert to milliamps
					/////		
					nAmps = fAmps*1000;
//printf("IS [%d] [%d]\n", ADCGetIs(), nAmps);
					ADC3IntDone = 0;
				}
			}
			break;
		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			{
				float fAmps;
	
				if(ADC3IntDone)
				{	
					int nSamples = ADCGetIs();
					fAmps = CONVERT_TO_IS(nSamples);
					//fAmps = CONVERT_TO_IS(ADCGetIs());
		
					/////
					// convert to milliamps
					/////
					nAmps = fAmps*1000;
//printf("IS [%d] [%d]\n", nSamples, nAmps);
					ADC3IntDone = 0;
				}
			}
			break;
	}
	return nAmps;
}

unsigned short ADCGetBatteryVoltage()
{	
    return batteryVoltageFilter.filteredValue;
}

#if (0)
// Temperature Compensated Charging Voltage Algorithm
// Vtemp = 14.92 volts - ((0.0056v/degree F * temperature(degrees F))
unsigned short ADCGetTempCompChargingVoltage(void)
{	
	unsigned short nVolts = 0;
	
	float fVolts = 14.92 - ( 0.0056 * lm73GetDegreesC(FALSE) );

	nVolts = fVolts*100;
	
	if (nVolts > tempCompVoltageHighLimit)
	{
		printf("nVolts[%d] > tempCompVoltageHighLimit[%d]\n",nVolts,tempCompVoltageHighLimit);
		
		nVolts = tempCompVoltageHighLimit;
		
		printf("nVolts[%d] Now Set To: tempCompVoltageHighLimit[%d]\n",nVolts,tempCompVoltageHighLimit);
	}	
	else if (nVolts < tempCompVoltageLowLimit)
	{
		printf("nVolts[%d] < tempCompVoltageLowLimit[%d]\n",nVolts,tempCompVoltageLowLimit);

		nVolts = tempCompVoltageLowLimit;
		
		printf("nVolts[%d] Now Set To: tempCompVoltageLowLimit[%d]\n",nVolts,tempCompVoltageLowLimit);
	}

	return nVolts;
}
#endif

//============================================
/*********************************************************************************
**                            End Of File
*********************************************************************************/
