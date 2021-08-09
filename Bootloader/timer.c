#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "shareddefs.h"
#include "sharedinterface.h"
#include "timer.h"
#include "lpc23xx.h"
#include "irq.h"
static ePLATFORM_TYPE ePlatformType;
static BOOL driverBoardToggleLED()
{
#define uP_SYS_LED 15
	static BOOL bSysLED = FALSE;
	if(bSysLED)
	{
		FIO1CLR |= (1<<uP_SYS_LED);
		bSysLED = FALSE;
	}
	else
	{
		FIO1SET |= (1<<uP_SYS_LED);
		bSysLED = TRUE;
	}
	return bSysLED;
}
static BOOL handHeldToggleLED()
{
#define uP_DBG_LED 6
	static BOOL bSysLED = FALSE;
	if(bSysLED)
	{
		FIO0CLR |= (1<<uP_DBG_LED);
		bSysLED = FALSE;
	}
	else
	{
		FIO0SET |= (1<<uP_DBG_LED);
		bSysLED = TRUE;
	}
	return bSysLED;
}

static BOOL powerControlUnitToggleLED()
{
#define uP_AUX_LED 6
	static BOOL bAuxLED = FALSE;
	if(bAuxLED)
	{
		FIO2CLR |= (1<<uP_AUX_LED);
		bAuxLED = FALSE;
	}
	else
	{
		FIO2SET |= (1<<uP_AUX_LED);
		bAuxLED = TRUE;
	}
	return bAuxLED;
}

BOOL timerToggleLED()
{
	BOOL bRetVal = FALSE;
	switch(ePlatformType)
	{
		case ePLATFORM_TYPE_HANDHELD:
			bRetVal = handHeldToggleLED();
			break;
		case ePLATFORM_TYPE_ARROW_BOARD:
			bRetVal = driverBoardToggleLED();
			break;
		case ePLATFORM_TYPE_PCU:
			bRetVal = powerControlUnitToggleLED();
			break;
		default:
			break;
	}
	return bRetVal;
}
static int nLEDCycles;
static unsigned int nCurrentMilliseconds;
static unsigned char nCurrentEpoch;
static BOOL toggle = TRUE;

void T0Handler(void)  __irq
{

	// acknowledge interrupt
	T0IR = (1 << T0IR_MR3);
	//	EXTINT = T0INT_CLEAR;

	// TOGGLE UNUSED I/O FOR TESTING
	// that the processor is alive.
	//FIO1PIN ^= (1 << uP_USB_LED);
	// 32b int msecs = 1,192h till rollover ~ 50days
	if(0 == ++nCurrentMilliseconds)
	{
		nCurrentEpoch++;
	}
	if(0 <= nLEDCycles)
	{
		if(0 >= --nLEDCycles)
		{
			timerToggleLED();
			nLEDCycles = 500;
		}
	}
	//printf("T0");
	
	 VICVectAddr = 0;		/* Acknowledge Interrupt */
}

void T1Handler (void) __irq 
{ 	
	T1IR 	=  (1 << T1IR_MR3); // Clear match 1 interrupt 
	
	if(toggle)
	{
		FIO2SET = (1 << uP_PWM_xACT);
		toggle = FALSE;
	}
	else
	{
		FIO2CLR = (1 << uP_PWM_xACT);
		toggle = TRUE;
	}
	
	VICVectAddr = 0x00000000; // Dummy write to signal end of interrupt 
} 

//*********
//*********
//*********
//*********
//*********
// configure int to fire every x millisecs
void T0_SETUP_PERIODIC_INT(int desired_msecs)
{
	
		// turn on periph power
		PCONP |= (1 << PCTIM0);
			
		// set pclock
		PCLKSEL0 |= (CCLK_OVER_4 << PCLK_TIMER0);
		// set to INT and reset on match
			
		T0MCR |= (1 << MR3I) | (1 << MR3R);

		// set match value to int 
		T0MR3 = (BASE_CLK_36MHZ)*(desired_msecs)/(4*1000);
		//printf("BASE_CLK[%d] desired_msecs[%d]\n", BASE_CLK, desired_msecs);

		// install handler
		// not sure why void() (void) __irq is necessary.  NXP demo code called for (void(*))?
		install_irq( TIMER0_INT, (void (*)(void)__irq) T0Handler, LOWEST_PRIORITY );

		// start counter
		T0TCR = (1 << T0_ENA);
	
}

void T1_SETUP_PERIODIC_INT(int rate, int duty)
{	
		// turn on periph power
		PCONP |= (1 << PCTIM1);
		 	
		// set pclock
		PCLKSEL0 |= (CCLK_OVER_8 << PCLK_TIMER1);
	
		// set to INT and reset on match			
		T1MCR |= (1 << MR3R) | (1 << MR3I);
	
		//T1MR0 = (BASE_CLK)/(4*rate);
		T1MR0 = 0; //2*(12000000)/rate;
	
		// set match value to int 
		T1MR1 |= 50; //(int)T1MR0*(100-duty)/100;
	
		T1PR |= 50;
	
		//T1MR1 = (BASE_CLK)*(desired_msecs)/(4*10000);
		//printf("BASE_CLK[%d] desired_msecs[%d]\n", BASE_CLK, desired_msecs);

		// install handler
		// not sure why void() (void) __irq is necessary.  NXP demo code called for (void(*))?
		install_irq( TIMER1_INT, (void (*)(void)__irq) T1Handler, LOWEST_PRIORITY );

		// start counter
		T1TCR |= (1 << T1_ENA);	
}


void initTimer(TIMERCTL* pTimer)
{
	pTimer->nTimeoutEpoch = 0;
	pTimer->nTimeoutTime = 0;
}

void timerShowLEDHeartbeat(void)
{
	nLEDCycles = 500;
}
void timerStopLEDHeartbeat()
{
	nLEDCycles = -1;
}
int getTimerNow()
{
	unsigned char nEpoch = nCurrentEpoch;
	unsigned int nMilliseconds = nCurrentMilliseconds;
	if(nEpoch != nCurrentEpoch)
	{
		/////
		// timer rolled over, so grab it again
		/////
		nEpoch = nCurrentEpoch;
		nMilliseconds = nCurrentMilliseconds;
	}
	return nMilliseconds;
}

void startTimer(TIMERCTL *pTimer, unsigned int nTimeoutMS)
{
	unsigned char nEpoch = nCurrentEpoch;
	unsigned int nMilliseconds = nCurrentMilliseconds;
	if(nEpoch != nCurrentEpoch)
	{
		/////
		// timer rolled over, so grab it again
		/////
		nEpoch = nCurrentEpoch;
		nMilliseconds = nCurrentMilliseconds;
	}
	pTimer->nTimeoutEpoch = nEpoch;
	pTimer->nTimeoutTime = nMilliseconds;
	if(0 < nTimeoutMS)
	{

		pTimer->nTimeoutTime += nTimeoutMS;
		if(pTimer->nTimeoutTime < nMilliseconds)
		{
			/////
			// our timer rolled over
			// so increment to next epoch
			/////
			pTimer->nTimeoutEpoch++;
		}
	}
}
BOOL isTimerExpired(TIMERCTL* pTimer)
{
	BOOL bRetVal = FALSE;
	unsigned char nEpoch = nCurrentEpoch;
	unsigned int nMilliseconds = nCurrentMilliseconds;

	if(nEpoch != nCurrentEpoch)
	{
		nEpoch = nCurrentEpoch;
		nMilliseconds = nCurrentMilliseconds;
	}
	if(pTimer->nTimeoutEpoch < nEpoch)
	{
		bRetVal = TRUE;
	}
	else if((pTimer->nTimeoutEpoch == nEpoch) && (pTimer->nTimeoutTime < nMilliseconds))
	{
		bRetVal = TRUE;
	}
	return bRetVal;
}

void stopTimer(TIMERCTL* pTimer)
{
	pTimer->nTimeoutEpoch = 0;
	pTimer->nTimeoutTime = 0;
}
void timerInit(ePLATFORM_TYPE ePlatform)
{
	ePlatformType = ePlatform;
	nCurrentMilliseconds = 0;
	nCurrentEpoch = 0;
	T0_SETUP_PERIODIC_INT(1);
	//T1_SETUP_PERIODIC_INT(5000,70);
}
/*****************************************************************************
** Function name:		delayMs
**
** Descriptions:		Start the timer delay in milliseconds
**									until elapsed
**
** parameters:			timer number, Delay value in millisecond			 
** 						
** Returned value:		None
** 
*****************************************************************************/
void delayMs(DWORD delayInMs)
{
	TIMERCTL delayTimer;
	initTimer(&delayTimer);
	if(0 >= delayInMs)
	{
		delayInMs = 1;
	}
	startTimer(&delayTimer, delayInMs);
	while(!isTimerExpired(&delayTimer));
}
