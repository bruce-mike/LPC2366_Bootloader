// pwm.c setup and config routines for PWM peripheral

#include <LPC23xx.H>
#include <stdio.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "PCUDefs.h"
#include "pwm.h"
#include "actuator.h"
#include "adc.h"

#define PHOTOCELL_MAX_SAMPLES 100
#define MAX_PHOTOCELL_VALUE 1024

//static BOOL autoBrightnessModeOn = FALSE;
static DWORD nForwardPhotocellData[PHOTOCELL_MAX_SAMPLES];
static DWORD nRearPhotocellData[PHOTOCELL_MAX_SAMPLES];
static int nRearPhotocellSampleIndex, nForwardPhotocellSampleIndex, i;
static DWORD nAverageRear;
static DWORD nAverageForward;

/////
// DEH 030117
// these are not used at this time
//////
#if 0
//*******
//*******
//*******
// rate = Hz base freq.  Duty = %.  e.g SETUP...(100, 50) is 100Hz, 50% duty
void pwmLampDriverSetup(int rate, int duty) 
{

	// power-up device
	PCONP |= (1 << PCPWM1);

	// set up peripheral clock = CCLK (12MHz)
	PCLKSEL0 |= (1 << PCLK_PWM1);
		
	// set PWM pins and modes
	// we would like to use the PWM peripheral to generate 
	// the control signal for driver I2C chips.  This will be
	// a 50% duty, 100Hz square wave for dim, or steady-on for full-bright.
	// this signal is modulated by timer0 to produce blinking with
	// period of 1.875s	
		
	// set pin function for P2.1 (pwm1 channel 2) to pwm
	PINSEL4 |= (PWM1_2 << P2_1_PIN_SEL);	
	//printf("pwmLampDriverSetup: PINSEL4[%08X]\n",PINSEL4);
		
	// TURN ON PULL UP unnecessary - pin is output	

	PINMODE4 |= (PULL_UP << P2_1_MODE_SEL);
		
	// set up match register.  
	// mr0 sets repetition rate.  MR2 sets pulse width if MR1 = 0.  Otherwise
	// MR1 sets leading edge and MR2 sets falling edge
	PWM1MR0 = 2*PWMCLK/rate;
	PWM1MR1 = 0x00; 
	
	// sense of output is inverted, so correct with (100 - duty)
	PWM1MR2 = (int)PWM1MR0*(100-duty)/100;  // = MR0/2

	// set up match control reg to reset on match with MR0
	// mr0 sets repetition rate.  MR2 sets pulse width
	PWM1MCR = (1 << PWMMR0R);

	// set match value latching
	PWM1LER |= (1 << PWM_MATCH2_LATCH);

	// enable the output p2.1
	PWM1PCR |= (1 << PWMENA2);

}

void pwmActuatorDriverSetup(int rate, int duty) 
{

	// power-up device
	PCONP |= (1 << PCPWM1);

	// set up peripheral clock = CCLK (12MHz)
	PCLKSEL0 |= (1 << PCLK_PWM1);
		
	// set PWM pins and modes
	// we would like to use the PWM peripheral to generate 
	// the control signal for driver I2C chips.  This will be
	// a 50% duty, 100Hz square wave for dim, or steady-on for full-bright.
	// this signal is modulated by timer0 to produce blinking with
	// period of 1.875s	
		
	// set pin function for P2.4 (pwm1 channel 5) to pwm
	PINSEL4 |= (PWM1_5 << P2_4_PIN_SEL);	
	//printf("pwmActuatorDriverSetup: PINSEL4[%08X]\n",PINSEL4);
	
	// set up match register.  
	// mr0 sets repetition rate.  MR2 sets pulse width if MR1 = 0.  Otherwise
	// MR1 sets leading edge and MR2 sets falling edge
	PWM1MR0 = 2*PWMCLK/rate;
	PWM1MR1 = 0x00; 
	
	// sense of output is inverted, so correct with (100 - duty)
	PWM1MR5 = (int)PWM1MR0*(100-duty)/100;

	// set up match control reg to reset on match with MR0
	// mr0 sets repetition rate.  MR2 sets pulse width
	PWM1MCR = (1 << PWMMR0R);

	// set match value latching
	PWM1LER |= (1 << PWM_MATCH5_LATCH);

	// enable the output p2.4 (pwm1 channel 5)
	PWM1PCR |= (1 << PWMENA5);
}

void pwmLampDriverSetDutyCycle(int duty)
{
	if(duty < 1)
	{
		duty = 1;
	}
	PWM1MR2 = (int)PWM1MR0*(100-duty)/100;
	//printf("PWM1MR2[%08X] duty[%d]\n",PWM1MR2,duty);
	// After assigning the new duty cycle to PWM1MR2, you must set the PWM1LER bit (PWM_MATCH2_LATCH)
  // to latch the newvalue into the PWM counter register upon the next period elapsing.	
	PWM1LER |= (1 << PWM_MATCH2_LATCH);
}

void pwmActuatorDriverSetDutyCycle(int duty)
{
	if(duty < 1)
	{
		duty = 1;
	}
	PWM1MR5 = (int)PWM1MR0*(100-duty)/100;
	// After assigning the new duty cycle to PWM1MR5, you must set the PWM1LER bit (PWM_MATCH5_LATCH)
  // to latch the newvalue into the PWM counter register upon the next period elapsing.	
	PWM1LER |= (1 << PWM_MATCH5_LATCH);
}
#endif
void pwmInit()
{
	////////////////////////////////////////////////////////////////
	//// Config PWM Peripheral for 'rate' (Hz) and 'duty' cycle (%) 
	////////////////////////////////////////////////////////////////
#if (0)
	pwmLampDriverSetup(5000, 50);
	pwmActuatorDriverSetup(5000,50);
#endif
	
	////////////////////////////////////////
	//// Set Master PWM Counter and Enable
	////////////////////////////////////////
	PWM1TCR = (1 << COUNTER_ENA) | (1 << PWM_ENA);
	//pwmSetDriver(PWM_ON);
	
	/////////////////////////////////////////
	//// Initialize the Photocell Data 
	/////////////////////////////////////////
	for(i=0; i<PHOTOCELL_MAX_SAMPLES; i++)
	{
		nForwardPhotocellData[i] = MAX_PHOTOCELL_VALUE;
		nRearPhotocellData[i]    = MAX_PHOTOCELL_VALUE;
	}
	nRearPhotocellSampleIndex = 0;
	nForwardPhotocellSampleIndex = 0;
}

void pwmDoWork(void)
{
	int i;
	nAverageRear = 0;
	nAverageForward = 0;
	
	
	/////
	// process photocell data
	/////
	nRearPhotocellData[nRearPhotocellSampleIndex++]       = ADCGetPr();
	nForwardPhotocellData[nForwardPhotocellSampleIndex++] = ADCGetPf();
		
	if(PHOTOCELL_MAX_SAMPLES <= nRearPhotocellSampleIndex)
	{
		nRearPhotocellSampleIndex = 0;
	}
		
	if(PHOTOCELL_MAX_SAMPLES <= nForwardPhotocellSampleIndex)
	{
		nForwardPhotocellSampleIndex = 0;
	}
		
	for(i=0; i<PHOTOCELL_MAX_SAMPLES; i++)
	{
		nAverageRear    += nRearPhotocellData[i];
		nAverageForward += nForwardPhotocellData[i];
	}
		
	nAverageRear    /= PHOTOCELL_MAX_SAMPLES;
	nAverageForward /= PHOTOCELL_MAX_SAMPLES;
}

int pwmGetPhotoCellDataAvgCounts(ePHOTOCELL_DATA ePhotoCellData)
{
	int photoCellData = 0;
	
	switch(ePhotoCellData)
	{
		case ePHOTOCELL_FORWARD_DATA:
			photoCellData = nAverageForward;
			break;
		
		case ePHOTOCELL_REAR_DATA:
			photoCellData = nAverageRear;
			break;

		default:
			break; 		
	}
	
	return photoCellData;
}

int pwmGetPhotoCellDataPercent(ePHOTOCELL_DATA ePhotoCellData)
{
	int photoCellData = 0;
	
	switch(ePhotoCellData)
	{
		case ePHOTOCELL_FORWARD_DATA:
			photoCellData = (100-((100*(MAX_PHOTOCELL_VALUE-nAverageForward))/MAX_PHOTOCELL_VALUE));
			break;
		
		case ePHOTOCELL_REAR_DATA:
			photoCellData = (100-((100*(MAX_PHOTOCELL_VALUE-nAverageRear))/MAX_PHOTOCELL_VALUE));
			break;

		default:
			break; 		
	}
	
	return photoCellData;
}

