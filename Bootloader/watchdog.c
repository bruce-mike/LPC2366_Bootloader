//============================================================================
// watchdog timer control
//============================================================================
#include "LPC23xx.h"                        /* LPC23xx/24xx definitions */
#include "stdio.h"
#include "shareddefs.h"
#include "watchdog.h"


unsigned char lastResetType()
{
	return RSIR;
}

void watchdogInit(int nMilliseconds)
{
	/////
	// setup the watchdog timer
	/////
	WDCLKSEL = WATCHDOG_CLOCK_SOURCE;
	WDTC = WATCHDOG_TIMER_SCALER*nMilliseconds;
	
	/////
	// tell watchdog to reset on timeout
	/////
	WDMOD |= WATCHDOG_ENABLE_RESET;
	
	/////
	// give it the first feeding
	// this actually starts the watchdog timer
	/////
	watchdogFeed();
}

void watchdogFeed()
{
	/////
	// the datasheet says to disable interrupts
	// this caused problems during operation
	// so we will not do that here,
	// but try to feed the watchdog more often than needed
	// thinkng that not all of these feeds will be interrupted in the middle
	/////

	/////
	// feed the watchdog
	/////
	WDFEED = 0xAA;
	WDFEED = 0x55;
;
}

void watchdogReboot()
{
    // PCU utilizes off board watchdog so 
    // initialize on board watchdog and force
    // a reset
    
    watchdogInit(200);
    while(1);
}
