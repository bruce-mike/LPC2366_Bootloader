
#include "LPC23xx.H"                   /* LPC23xx definitions               */
#include "shareddefs.h"
#include <stdio.h>
#include <string.h>

#include "queue.h"
#include "sharedinterface.h"
#include "serial.h"
#include "irq.h"

#include "pcuPowerControl.h"
#include "watchdog.h"
#include "serialUserCommands.h"



static void serialDebugSendData(unsigned char nData);


//===================================================
// Double buffering for Rx data
//===================================================

#define SIZE_OF_RX_BUFFER	150

typedef struct {
  int counter;
  BYTE buffer[SIZE_OF_RX_BUFFER];
} RX_BUFFER;

static int nTxDebugRemoveFromIndex;
static int nTxDebugAddToIndex;


#define FIFO_LENGTH	1500

static unsigned char txDebugFIFO[FIFO_LENGTH];




//===========================================================================
// UART0 commincations
// PCU debug port
//===========================================================================
static RX_BUFFER uart0RxBuffer[2];
static BYTE uart0DataIndex = 0;
static BYTE uart0IsrIndex = 1;

#define SIZE_OF_DEBUG_CMD   100
static BYTE debugCommand[SIZE_OF_DEBUG_CMD];
static BYTE debugCursor = 0;

//=========================================
void
switchUart0RxBuffers()
//=========================================
{
    U0IER &= ~(RX_DATA_AVAILABLE_INTERRUPT_ENABLE);
    
    // swap ISR and application buffers - always diffferent
    uart0IsrIndex = uart0DataIndex;
    
    if(1 == uart0DataIndex)
        uart0DataIndex = 0;
    else
        uart0DataIndex = 1;
	    
    U0IER |= RX_DATA_AVAILABLE_INTERRUPT_ENABLE;
}

//=============================================
void
serialDebugDoWork()
//=============================================
{
    int i;
    BYTE bVal;
    	
    switchUart0RxBuffers();

	
    for(i = 0; i < uart0RxBuffer[uart0DataIndex].counter; i++)
    {
        bVal = uart0RxBuffer[uart0DataIndex].buffer[i];
        
        if(bVal == 0x0D || bVal == 0x0A)
        {
            debugCommand[debugCursor] = '\0';
            serialProcessUserCommand((char *)&debugCommand[0]);
            debugCursor = 0;
        }
        else
        {
            if(debugCursor < SIZE_OF_DEBUG_CMD)
            {
                debugCommand[debugCursor++] = bVal;
            }
        }        
    }
    
	// all bytes read - set buffer input count to 0
	uart0RxBuffer[uart0DataIndex].counter = 0;
}


//===========================================
 __irq void UART0_HANDLER(void)
//===========================================
{
	int nAddIndex;

	// don't do this if receive interrupts are turned off
	if(U0IER & RX_DATA_AVAILABLE_INTERRUPT_ENABLE)
	{
        // Rx interrupt triggered read all waiting data
        nAddIndex = uart0RxBuffer[uart0IsrIndex].counter;
        
        while(U0LSR & RDR )
		{
            // prevent buffser overrun - throw away extra characters
            if(nAddIndex < SIZE_OF_RX_BUFFER)
            {
                uart0RxBuffer[uart0IsrIndex].buffer[nAddIndex] = U0RBR;
                nAddIndex++;
            }
		}

        uart0RxBuffer[uart0IsrIndex].counter = nAddIndex;
	}

	if(U0IER & THRE_INTERRUPT_ENABLE)
	{
        // only do this if this interrupt is enabled
        // so we don't step on foreground process        
		while(U0LSR & THRE)
		{
			if(nTxDebugRemoveFromIndex != nTxDebugAddToIndex)
			{
				U0THR = txDebugFIFO[nTxDebugRemoveFromIndex++];
				if(sizeof(txDebugFIFO) <= nTxDebugRemoveFromIndex)
				{

					nTxDebugRemoveFromIndex = 0;
				}
			
				// if this is the last character for awhile
				// so turn off thre interrupt 
				// it will be re-enabled when there is something to send
				if(nTxDebugRemoveFromIndex == nTxDebugAddToIndex)
				{
					U0IER &= (~THRE_INTERRUPT_ENABLE);
					break;
				}
			}
			else
			{
				U0IER &= (~THRE_INTERRUPT_ENABLE);
				break;
			}
		}
    }

    //clear int flag??
    VICVectAddr = 0x00;
}



/*----------------------------------------------------------------------------
 *       init_serial:  Initialize Serial Interface
 *---------------------------------------------------------------------------*/
static void serialInitPort( )
{
    // SET UP REGS
	// SET UP UART for 19200 baud @36000000 CCLK
	U0LCR |= (1 << DLAB) | (EIGHTBITS << DATA_WIDTH);
	U0DLM = DLM_19200_36MHZ;
	U0DLL = DLL_19200_36MHZ;
	U0LCR &= ~(1 << DLAB);	
	// config IO pins
	PINSEL0 |= (UART0_SEL << TXD0_SEL) | (UART0_SEL << RXD0_SEL); 			
	//U0FCR |= FIFO_ENABLE;
}



static void serialDebugSendData(unsigned char nData)
{
	int nAddIndex;
	BOOL bInterruptsOn = FALSE;
	if((U0IER & THRE_INTERRUPT_ENABLE))
	{
		bInterruptsOn = TRUE;
	}
	
	// turn interrupts off 
	// while we are messing 
	// with the FIFO
	U0IER &= ~THRE_INTERRUPT_ENABLE;
	
	// add the character to the fifo
	// don't let the add to index
	// roll past the remove from index
	// discard the data if it would
	
	nAddIndex = nTxDebugAddToIndex+1;
	if(sizeof(txDebugFIFO) <= nAddIndex)
	{
			nAddIndex = 0;
	}
	if(nAddIndex != nTxDebugRemoveFromIndex)
	{
			txDebugFIFO[nTxDebugAddToIndex] = nData;
			nTxDebugAddToIndex = nAddIndex;
	}


	// if fifo was empty
	// then send the first character on the fifo
	// and enable THRE interrupts
	// note: this should be the only place that
	// we enable these interrupts for this UART
	if(!bInterruptsOn)
	{
		/////
		// note, we can modify the RemoveFrom index
		// here because we know that the interrupt
		// will not interfere
		/////
		int ch2 = txDebugFIFO[nTxDebugRemoveFromIndex++];
		if(nTxDebugRemoveFromIndex >= sizeof(txDebugFIFO))
		{
			nTxDebugRemoveFromIndex = 0;
		}

		U0THR = ch2;
		U0IER |= THRE_INTERRUPT_ENABLE;
	}
	else
	{
		U0IER |= THRE_INTERRUPT_ENABLE;
	}
}

/*----------------------------------------------------------------------------
 *       sendchar:  Write a character to Serial Port attached to UART0
 *                  called by printf through retarget.c
 *---------------------------------------------------------------------------*/
int sendchar (int ch)
{	
	serialDebugSendData(ch);
	return 0;
}


/*----------------------------------------------------------------------------
 *       RxDataReady:  if U0RBR contains valid data reurn 1, otherwise 0
 *---------------------------------------------------------------------------*/
int RxDataReady( void ) {
	return ( U0LSR & 0x01 );	//Receive data ready

}
void serialDisableInterface(eINTERFACE eInterface)
{
		switch(eInterface)
		{
			case eINTERFACE_DEBUG:
				U0IER &= ~(RX_DATA_AVAILABLE_INTERRUPT_ENABLE);
				break;
			
			default:
                break;
		}
}

void serialEnableInterface(eINTERFACE eInterface)
{
	/////
	// clear the FIFOs
	// reset the indicies
	// and enable receive interrupts
	// for the specified interface
	/////
	switch(eInterface)
	{
		case eINTERFACE_DEBUG:
			nTxDebugRemoveFromIndex = 0;
			nTxDebugAddToIndex = 0;
			memset(txDebugFIFO, 0, sizeof(txDebugFIFO));
			U0IER |= (RX_DATA_AVAILABLE_INTERRUPT_ENABLE);
			break;
		
		        
        default: 
            break;
	}
}

void serialInit()
{
	// setup debug port
	serialInitPort();
	U0IER &= (~THRE_INTERRUPT_ENABLE);
    install_irq( UART0_INT, (void (*) (void) __irq) UART0_HANDLER, HIGHEST_PRIORITY );
	serialEnableInterface(eINTERFACE_DEBUG);    
}




void serialDoWork()
{   
    // PCU serial debug interface
    serialDebugDoWork();
}


void serialWrite(eINTERFACE eInterface, unsigned char* pPacket, int nPacketLength)
{

	switch(eInterface)
	{
		case eINTERFACE_DEBUG:

#ifdef DEBUG_USE_INTERRUPTS
#if 0
			serialDebugSendData(PACKET_DELIM);
		
			for(nIndex=0; nIndex<nPacketLength; nIndex++)
			{
				serialDebugSendData(pPacket[nIndex]);
			}
			
			serialDebugSendData(PACKET_DELIM);
#endif
#endif
			
			break;
						            
        default:
            
            break;
	}
}






