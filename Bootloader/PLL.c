#include "lpc23xx.h"
#include "shareddefs.h"
#include "sharedinterface.h"
#include "PLL.h"
#include "queue.h"



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

void PLL_CLOCK_SETUP(ePLATFORM_TYPE ePlatformType)
{
// CPU DIVIDER = 4
// PCLK = CCLK/4
// PLL MULT = 12
// PLL_CONFIG();
// PLL_CONFIG() NOT CALLED --> PLL NOT SET UP, NOT CONFIGURED.  DEFAULT CLK SOURCE = IRC (4MHz)
// DEFAULT CLOCK SOURCE FOR PERIPHS IS CCLK/4
// set periph clk = cclk
	switch(ePlatformType)
	{
		case ePLATFORM_TYPE_ARROW_BOARD:
			PCLKSEL0 = (CCLK_OVER_1 << PCLK_UART0) | // DEBUG
								 (CCLK_OVER_1 << PCLK_UART1) ; // RFD

			PCLKSEL1 = (CCLK_OVER_1 << PCLK_UART2); // 485

			break;
		
		case ePLATFORM_TYPE_HANDHELD:
			PCLKSEL0 = (CCLK_OVER_1 << PCLK_UART0); // DEBUG

			PCLKSEL1 = (CCLK_OVER_1 << PCLK_UART2) | // 485
								 (CCLK_OVER_1 << PCLK_UART3) ; // RFD
			break;
		
		case ePLATFORM_TYPE_PCU:
			PCLKSEL0 = (CCLK_OVER_1 << PCLK_UART0) | // DEBUG
								 (CCLK_OVER_1 << PCLK_UART1) ; // RFD

			PCLKSEL1 = (CCLK_OVER_1 << PCLK_UART2)| // 485
								 (CCLK_OVER_1 << PCLK_UART3); // PCU
			break;
		
		default:
			break;
	}

	/////
	// configure the PLL
	/////
	PLL_CONFIG();
	
	/////
	// ENERGIZE PERIPHERALS
	/////
	switch(ePlatformType)
	{
		case ePLATFORM_TYPE_ARROW_BOARD:
			PCONP = (1 << PCUART0) |   	// debug uart
							(1 << PCUART2) |		// 485
							(1 << PCUART1) |		// rfd
							(1 << PCAD   ) |		// ADC
							(1 << PCTIM0 ) |		// timer
							(1 << PCI2C0 ) |		// timer
							(1 << PCSSP1 ) ;		// SSP1
			break;
		
		case ePLATFORM_TYPE_HANDHELD:
			PCONP = (1 << PCUART0) |   	// debug uart
							(1 << PCUART2) |		// 485
							(1 << PCUART3) |		// rfd
							(1 << PCAD   ) |		// ADC
							(1 << PCTIM0 ) |		// timer
							(1 << PCPWM1 ) |		// pwm
							(1 << PCSSP0 ) ;		// SSP0			
			break;
		
		case ePLATFORM_TYPE_PCU:
			PCONP = (1 << PCUART0) |   	// debug uart
							(1 << PCUART2) |		// 485
							(1 << PCUART1) |		// rfd
							(1 << PCUART3) |		// rfd
							(1 << PCAD   ) |		// ADC
							(1 << PCTIM0 ) |		// timer
							(1 << PCI2C0 ) |		// timer
							(1 << PCSSP1 ) ;		// SSP1
			break;
		
		default:
			break;
	}




}
