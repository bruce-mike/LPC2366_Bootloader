#ifndef PWM_H
#define PWM_H
// PWM.H

//LPC2366 PWM setup and defs

#define PCPWM1	       6			// power control reg bit location
#define PCLK_PWM1     12			// clock control reg bit location (2-bits)
#define PWM1_2        01			// select pin function PWM
#define GPIO					00      // select pin function GPIO
#define P2_1_PIN_SEL   2			// pin function bit location in PINSEL4
#define P2_1_MODE_SEL  2			// pin mode function bit location in PINMODE4
#define PULL_UP				00		  // mode select bit(s) value in PINMODE
#define PWMMR0R        1		  // bit location in pWM1MCR
#define PWMENA2				10			// bit location in PWM1PCR
#define PWM_MATCH0_LATCH 0		// bit location in PWM1LER
#define PWM_MATCH2_LATCH 2		// bit location in PWM1LER
#define PWM_MATCH5_LATCH 5		// bit location in PWM1LER
#define PWMENA5				13
#define P2_4_PIN_SEL	8
#define PWM1_5 				1
#define COUNTER_ENA   0    		// PWM1TCR bit location
#define PWM_ENA			  3    		// ditto

/////////////////////////////////////////
//// PWM Lamp Brightness Duty Cycle%
/////////////////////////////////////////
#define PWM_FULL_BRIGHT_DUTY   100 // 100% Duty Cycle
#define PWM_MEDIUM_BRIGHT_DUTY 50  // 50% Duty Cycle
#define PWM_DIM_BRIGHT_DUTY		 10  // 10% Duty Cycle

// this should be def'd elsewhere? With CCLK and PLL dividers?
#define PWMCLK		BASE_CLK_36MHZ //36000000 = CCLK.  for MR0/rate configuration


// MISC DEFS
#define PWM_OFF 				0
#define PWM_ON 					1
#define PWM_TOGGLE 			2
#define PWM_FULL_BRIGHT 3
#define PWM_BLINK       4
#define PWM_SUSPEND			5
#define PWM_RESUME			6

typedef enum ePhotoCellData
{
	ePHOTOCELL_FORWARD_DATA = 0,
	ePHOTOCELL_REAR_DATA    = 1
}ePHOTOCELL_DATA;

///////////////////////
//// func defs
///////////////////////
void pwmInit(void);
void pwmActuatorDriverSetup(int rate, int duty); 
void pwmLampDriverSetup(int rate, int duty);
void pwmLampDriverSetDutyCycle(int duty);
void pwmActuatorDriverSetDutyCycle(int duty);
void pwmSetDriver(int action);
void pwmSetLampBrightnessControl(eBRIGHTNESS_CONTROL brightness);
void pwmDoWork(void);
int  pwmGetPhotoCellDataAvgCounts(ePHOTOCELL_DATA ePhotoCellData); 
int pwmGetPhotoCellDataPercent(ePHOTOCELL_DATA ePhotoCellData);
#endif		// PWM_H

