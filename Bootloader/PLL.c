#include "lpc23xx.h"
#include "shareddefs.h"
#include "PLL.h"



// reconfigure the PLL once we come out of sleep mode(s)
// current usage is for CPU freq = 24MHz
// most PCLKs = 6MHz (cclk/4) (DEFAULT)


/////
// this results in PCLK=95846400
/////
void PLL_CONFIG(void)
{
	
// FROM DATASHEET:
// 1 DISCONNECT PLL WITH FEED
// 2 DISABLE PLL WITH FEED
// 3 CHANGE CPU CLK DIVIDER (W/O PLL)
// 4 WRITE CLK SOURCE SEL REG
// 5 WRITE PLLCFG AND FEED			
// 6 ENABLE PLL WITH FEED
// 7 CHANGE CPPU CLK DIVIDER (W/PLL)
// 8 WAIT FOR LOCK
// 9 CONNECT PLL WITH FEED
					
//************************************	
		
		//change cpu clk divider to 4: (N + 1) = 4 --> N = 3
		// OUT OF ORDER? STEP  3
		//CCLKCFG = CPU_DIV;
		CCLKCFG = CCLKSEL_36MHZ;
		
		// reconfigure clocking
		// select IRC step 4
		CLKSRCSEL = 0x00;
		
		// write PLLCFG.  (multiplier val) STEP 5
		//PLLCFG = (PLL_MULT << MSEL); // | (1 << NSEL);
		PLLCFG =  (PLL_MULT << MSEL) | (PLL_DIV_72MHZ << NSEL);			
			
		// feed
		PLLFEED = 0xAA;
		PLLFEED = 0x55;
			

		// enable PLL STEP 6
		PLLCON = (1 << PLLE) | (1 << PLLC);

		// feed
		PLLFEED = 0xAA;
		PLLFEED = 0x55;
			
		// STEP 7
		// CHANGE CPU CLK DIV FOR OPERATION W/PLL
	  //CCLKCFG = CPU_DIV;
	  CCLKCFG = CCLKSEL_36MHZ; 

		// STEP 8
		// wait for lock
		while(!(PLLSTAT & (1 << PLLSTAT_PLOCK))) {}
			

		// 	STEP 9
		// connect with feed
		PLLFEED = 0xAA;
		PLLFEED = 0x55;
			
		// done
		//************************************	
}

