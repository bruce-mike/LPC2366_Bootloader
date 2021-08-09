// ssp.c

#include "LPC23xx.h"    // LPC23XX/24xx Peripheral Registers
#include <stdio.h>
#include "ssp.h"
#include "irq.h"

/* statistics of all the interrupts */
volatile DWORD interruptRxStat = 0;
volatile DWORD interruptOverRunStat = 0;
volatile DWORD interruptRxTimeoutStat = 0;


void SSPChipSelect(BOOL active)
{
    if(active)
        FIO2CLR |= (1 << uP_SPI1_UART_CS);
    else 
        FIO2SET |= (1 << uP_SPI1_UART_CS);
}    
            
        
void SSP0Handler (void) __irq 
{
    DWORD regValue;
  
    IENABLE;    // handles nested interrupt

    regValue = SSP0MIS;
	
    //printf("SSP0Handler\n");
	
    if(regValue & SSPMIS_RORMIS)    // Receive overrun interrupt
    {
        interruptOverRunStat++;
        SSP0ICR = SSPICR_RORIC;     // clear interrupt
    }
	
    if(regValue & SSPMIS_RTMIS)     // Receive timeout interrupt
    {
        interruptRxTimeoutStat++;
		SSP0ICR = SSPICR_RTIC;      // clear interrupt
  }

    // please be aware that, in main and ISR, CurrentRxIndex and CurrentTxIndex
    // are shared as global variables. It may create some race condition that main
    //and ISR manipulate these variables at the same time. SSPSR_BSY checking (polling)
    //in both main and ISR could prevent this kind of race condition */
    //if ( regValue & SSPMIS_RXMIS )	/* Rx at least half full */
    {
        interruptRxStat++;		/* receive until it's empty */		
    }
	
    IDISABLE;
    VICVectAddr = 0;		/* Acknowledge Interrupt */
}


void SSP1Handler (void) __irq 
{
    DWORD regValue;
  
    IENABLE;    // handles nested interrupt

    regValue = SSP1MIS;
	
    if(regValue & SSPMIS_RORMIS)    // Receive overrun interrupt
    {
        interruptOverRunStat++;
        SSP1ICR = SSPICR_RORIC;     // clear interrupt
    }
	
    if(regValue & SSPMIS_RTMIS)     // Receive timeout interrupt
    {
        interruptRxTimeoutStat++;
        SSP1ICR = SSPICR_RTIC;		/* clear interrupt */
    }

    // please be aware that, in main and ISR, CurrentRxIndex and CurrentTxIndex
    // are shared as global variables. It may create some race condition that main
    // and ISR manipulate these variables at the same time. SSPSR_BSY checking (polling)
    // in both main and ISR could prevent this kind of race condition */
    if(regValue & SSPMIS_RXMIS) // Rx at least half full
    {
        interruptRxStat++;  // receive until it's empty		
    }
    IDISABLE;
    VICVectAddr = 0;		/* Acknowledge Interrupt */
}

DWORD SSPInit()
{
    BYTE i, Dummy=Dummy;
	
    printf("SSPInit: ePLATFORM_TYPE_PCU\r\n");
    // Configure pins for SSP1 port SCK1, MISO1, MOSI1, and SSEL1 		
    // NOTE: ALWAYS SET SSEL1 OF PINSEL0 AS A GPIO PIN. THIS ALLOWS SOFTWARE CONTROL OF 
    // MOSI (P0.9), MISO (P0.8), SCK1 (P0.7) set to 0x10 (neither pullup or pulldown enabled)
    // SSEL1 (FLASH_CS) left to default 0x00 (pullup enabled)
    // Note: P2.3 (SPI_UART_CS) left to default 0x00 (pullup enabled) - chip selects are GPIO (manually controlled)
    PINSEL0 |= 0x000A8000;

    SSPChipSelect(TRUE);        
        	
    // Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15
    SSP1CR0 = 0x0707;
		
    // SSP1CPSR clock prescale register, master mode, minimum divisor is 0x02
    SSP1CPSR = 0x2;
		
    for ( i = 0; i < FIFOSIZE; i++ )
    {
        Dummy = SSP1DR;		/* clear the RxFIFO */
    }
		
    if ( install_irq( SSP1_INT,(void (*)(void) __irq)SSP1Handler, HIGHEST_PRIORITY ) == FALSE )
    {
        printf("Unable to Install SSP1_INT\r\n");
        return (FALSE);
    }
    else
    {
        printf("SSP1_INT Installed Successfully\r\n");		
    }
			
    // Device select as master, SSP Enabled
    SSP1CR1 = SSPCR1_SSE;
			
    // Set SSPINMS registers to enable interrupts
    // enable all error related interrupts
    SSP1IMSC = SSPIMSC_RORIM | SSPIMSC_RTIM;
	
    return( TRUE );
}


BYTE SSPSend(BYTE *buf, DWORD Length)
{
    DWORD i;
    BYTE regData;
		
    //Assert CS
    SSPChipSelect(TRUE);        
			
    for ( i = 0; i < Length; i++ )
    {
        // Move on only if NOT busy and TX FIFO not full
        while ( (SSP1SR & (SSPSR_TNF|SSPSR_BSY)) != SSPSR_TNF );
		
		SSP1DR = buf[i];

		while ( (SSP1SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE );
				
        // Whenever a byte is written, MISO FIFO counter increments, Clear FIFO 
        // on MISO. Otherwise, when SSP1Receive() is called, previous data byte
        // is left in the FIFO
        regData = SSP1DR;
    } 
    
    return regData;
}


void SSPSendByte(BYTE value)
{
    SSPSend(&value, 1);
}


void SSPReceive(BYTE *buf, WORD Length)
{
    WORD i;
	
	for ( i = 0; i < Length; i++ )
    {
        // As long as Receive FIFO is not empty, I can always receive. */
		// if it's a peer-to-peer communication, SSPDR needs to be written
		// before a read can take place
		SSP1DR = 0xFF;
				
		// Wait until the Busy bit is cleared */
		while((SSP1SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE);
				
		buf[i] = SSP1DR;
    }
			
}

