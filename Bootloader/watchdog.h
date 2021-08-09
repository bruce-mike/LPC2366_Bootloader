#ifndef WATCHDOG_H
#define WATCHDOG_H


#define WATCHDOG_ENABLE_RESET				0x03
#define WATCHDOG_ENABLE_INTERRUPT		    0x02

#define WATCHDOG_CLOCK_INTERNAL_RC	        0x00
#define WATCHDOG_CLOCK_PCLK					0x01
#define WATCHDOG_CLOCK_RTC					0x03

#define WATCHDOG_CLOCK_SOURCE WATCHDOG_CLOCK_INTERNAL_RC

// multiply this by the 
// desired number of milliseconds
// 1 to 64

#define WATCHDOG_TIMER_SCALER 				((4000000/4))/1000

//#define WATCHDOG_TIMER_VALUE 				((BASE_CLK/4)*3)/1000

#define RESET_MASK_POR							0x01
#define RESET_MASK_EXTR							0x02
#define RESET_MASK_WDTR							0x04
#define RESET_MASK_BODR							0x08

unsigned char lastResetType(void);
void watchdogInit(int nMilliseconds);
void watchdogFeed(void);
void watchdogReboot(void);


#endif		// WATCHDOG_H
