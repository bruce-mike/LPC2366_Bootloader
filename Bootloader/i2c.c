/********i*********************************************************************
 *   i2c.c:  I2C C file for NXP LPC23xx/24xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.07.19  ver 1.00    Prelimnary version, first Release
 *
*****************************************************************************/
#include "LPC23xx.h"                        /* LPC23xx/24xx definitions */
#include "queue.h"

#include "sharedinterface.h"
#include "shareddefs.h"
#include "irq.h"
#include "i2c.h" 
#include "timer.h"
#include <stdio.h>		// printf
#include <string.h>		// printf


#define DEBUG 1


static volatile unsigned char I2CReadLength;
static volatile unsigned char I2CWriteLength;

static volatile BYTE I2CStarted;
static volatile BYTE I2CComplete;
static volatile BYTE I2CFailed;

static volatile BYTE I2CWriteBuffer[BUFSIZE];
static volatile BYTE I2CReadBuffer[BUFSIZE];
static volatile BYTE I2CSLA_W;
static volatile BYTE I2CSLA_R;
static volatile BYTE lastState;


static volatile unsigned char RdIndex = 0;
static volatile unsigned char WrIndex = 0;

static TIMERCTL hangTimer;

/* 
From device to device, the I2C communication protocol may vary, 
in the example below, the protocol uses repeated start to read data from or 
write to the device:
For master read: the sequence is: STA,Addr(W),offset,RE-STA,Addr(r),data...STO 
for master write: the sequence is: STA,Addr(W),length,RE-STA,Addr(w),data...STO
Thus, in state 8, the address is always WRITE. in state 10, the address could 
be READ or WRITE depending on the I2CCmd.
*/   
/*****************************************************************************
** Function name:		I2C0MasterHandler
**
** Descriptions:		I2C0 interrupt handler, deal with master mode
**				only.
**
** parameters:			None
** Returned value:	None
** 
*****************************************************************************/
static void I2C0MasterHandler(void) __irq 
{
  BYTE StatValue;

  /* this handler deals with master read and master write only */
	
	////
	// the document referenced below is UM10211 
	// LPC2366-UsersManual
	// I2C section, pages526 & 527
	/////
  StatValue = I20STAT;
	lastState = StatValue;
  IENABLE;				/* handles nested interrupt */	
	/////
	// Enable these to get spew of state transition
	/////
	//U0THR='0'+((StatValue&0xF0)>>4);
	//U0THR='0'+(StatValue&0x0f);
  switch ( StatValue )
  {
		case 0x08:
			I2CStarted = TRUE;
			/////
			// A start condition has been issued
			// Load SLA+W
			// Clear STA
			/////
			I20DAT = I2CSLA_W;

			/////
			// clear STA (start condition)
			// clear SIC (interrupt)
			/////
			I20CONCLR = (I2CONCLR_SIC | I2CONCLR_STAC);

			break;
	
		case 0x10:			/* A repeated started is issued */
			/////
			// A repeated start has been issued
			// the document says that:
			// 	to remain in MST/XMIT Mode, load SLA+W
			// 	to change to MST/REC mode, load SLA+R
			//
			// We will switch to MST/REC mode if SLA+R is available
			// otherwise, we will stop the bus and complete the transfer
			/////
			if(0 != I2CSLA_R)
			{
				I20DAT = I2CSLA_R;
			}
			else
			{
				I2CStarted = FALSE;
				I2CComplete = TRUE;
				I2CFailed = TRUE;
				I20CONSET = I2CONSET_STO;      /* Set Stop flag */
			}
		
		
			/////
			// clear STA (start condition)
			// clear SIC (interrupt)
			/////
			I20CONCLR = (I2CONCLR_SIC | I2CONCLR_STAC);
			break;
	
		case 0x18:			/* Regardless, it's an ACK */
			/////
			// SLA+W has been transmitted
			// ACK has been received
			/////
			/////
			// the documentation says:
			// Load data byte
			//      or
			// Issue repeated start
			//      or
			// Issue stop condition
			//      or
			// Issue stop condition follwed by a START condition 
			/////
			if ( WrIndex < I2CWriteLength )
			{
				/////
				// more data to send, 
				// we will load a data byte
				/////
				I20DAT = I2CWriteBuffer[WrIndex++];
			}
			else
			{
				if(0 < I2CReadLength)
				{
					/////
					// we have data to read
					// so issue a repeated start
					////
					I20CONSET = I2CONSET_STA;	/* Set Repeated-start flag */
				}
				else
				{
					/////
					// no more data to read or write
					// so stop the bus
					/////
					I2CComplete = TRUE;
					I20CONSET = I2CONSET_STO;      /* Set Stop flag */
				}
			}

			/////
			// Clear SIC (intterupt)
			/////
			I20CONCLR = I2CONCLR_SIC;
			break;
	
		case 0x20:
			/////
			// SLA+W has been transmitted
			// NOT ACK has been received
			/////
			/////
			// the documentation says:
			// Load data byte (ACK bit will be received)
			//      or
			// Issue repeated start
			//      or
			// Issue stop condition
			//      or
			// Issue stop condition follwed by a START condition
			// 
			/////
			if ( WrIndex < I2CWriteLength )
			{
				/////
				// more data to send, 
				// we will load a data byte
				/////
				I20DAT = I2CWriteBuffer[WrIndex++];
			}
			else
			{
				if(0 < I2CReadLength)
				{
					/////
					// we have data to read
					// so issue a repeated start
					////
					I20CONSET = I2CONSET_STA;	/* Set Repeated-start flag */
				}
				else
				{
					/////
					// no more data to read or write
					// so stop the bus
					/////
					I2CComplete = TRUE;
					I20CONSET = I2CONSET_STO;      /* Set Stop flag */
				}	
			}
	
			/////
			// Clear SIC (intterupt)
			/////
			I20CONCLR = I2CONCLR_SIC;
			break;
		
		case 0x28:	/* Data byte has been transmitted, regardless ACK or NACK */
		case 0x30:
			/////
			// Data byte has been transmitted
			// Ack has been received (0x28)
			// Not Ack has been received (0x30)
			/////
			/////
			// the documentation says:
			// Load data byte
			//      or
			// Issue repeated start
			//      or
			// Issue stop condition
			//      or
			// Issue stop condition follwed by a START condition
			// 
			/////
			if ( WrIndex < I2CWriteLength )
			{
				/////
				// data available to send, so load data byte
				/////
				I20DAT = I2CWriteBuffer[WrIndex++];
			}
			else
			{
				/////
				// no more write data available
				/////
				if ( I2CReadLength != 0 )
				{
					/////
					// data to read, so issue repeated start
					/////
					I20CONSET = I2CONSET_STA;	/* Set Repeated-start flag */
				}
				else
				{
					/////
					// no data to read, 
					// so stop the bus and complete the transfer
					/////
					I2CComplete = TRUE;
					I20CONSET = I2CONSET_STO;      /* Set Stop flag */
				}
			}
			/////
			// clear the interrupt
			/////
			I20CONCLR = I2CONCLR_SIC;
			break;
	
		case 0x40:
			/////
			// SLA+R has been  transmitted
			// ACK has been received
			/////
			/////
			// the documentation says:
			// Data byte will be received, Ack bit will be returned (AA)
			//      or
			// Data byte will be received, Not Ack bit will be returned (~AA)
			//      or
			// Issue stop condition
			//      or
			// Issue stop condition follwed by a START condition
			/////	
			if(1 < I2CReadLength)
			{
				/////
				// more than 1 byte to receive
				// so issue ACK when the next byte is received
				/////
				I20CONSET = I2CONSET_AA;
			}
			else
			{
				/////
				// only 1 byte to receive
				// so issue NOT ACK when the next byte is received
				/////
				I20CONCLR = I2CONSET_AA;
			}
			/////
			// clear the interrupt
			/////
			I20CONCLR = I2CONCLR_SIC;
			break;
			
		case 0x48:
			/////
			// SLA+R has been transmitted
			// NOT ACK has been received
			/////
			/////
			// the documentation says:
			// Repeated start condition will be transmitted
			//      or
			// Issue stop condition
			//      or
			// Issue stop condition follwed by a START condition
			/////	
			I2CComplete = TRUE;
			I2CFailed = TRUE;
		
			/////
			// issue the stop flag
			/////
			I20CONSET = I2CONSET_STO;
	
			/////
			// clear the interrupt
			/////
			I20CONCLR = I2CONCLR_SIC;
			break;	
		case 0x50:
		case 0x58:
			/////
			// data byte has been transmitted
			// ACK has been received (0x50)
			// NOT ACK has been received (0x58)
			/////
			/////
			// the documentation says:
			// Data byte will be received, Ack bit will be returned (AA)
			//      or
			// Data byte will be received, Not Ack bit will be returned (~AA)
			//      or
			// Issue stop condition
			//      or
			// Issue stop condition follwed by a START condition
			/////	
			I2CReadBuffer[RdIndex++] = I20DAT;
			if(RdIndex >= I2CReadLength-1)
			{
				/////
				// only one more byte to recieve
				// so issue NOT ACK when we read the next byte
				/////
				I20CONCLR = I2CONSET_AA;
				if(RdIndex >= I2CReadLength)
				{
					/////
					// no more data to receive
					// set the stop flag
					/////
					I2CComplete = TRUE;
					I20CONSET = I2CONSET_STO;
				}
			}

			I20CONCLR = I2CONCLR_SIC;
			break;
	

	
		case 0x38:
			/////
			// Arbitration lost in NOT Ack bit
			/////
			/////
			// the documentation says:
			// I2C bus will be released; the I2C block will enter a slave mode
			//      or
			// A start condition will be transmitted when the bus becomes free
			/////	
			//U0THR='A';
			I2CComplete = TRUE;
			I2CFailed = TRUE;
			I20CONCLR = I2CONCLR_SIC;
			//#if DEBUG
			//printf("ARBITRATION LOST!!!!\\r\n");	
			//#endif
			break;
		case 0x00:
			/////
			// bus error
			// set the stop flag to reset the I2C bus
			// clear the interrupt
			/////
			//U0THR='B';
			I2CComplete = TRUE;
			I2CFailed = TRUE;
			I20CONSET = I2CONSET_STO;
			I20CONCLR = I2CONCLR_SIC;
			break;
		case 0xF8:
			/////
			// no relavent information
			// this occurs between other states
			// and when the I2C block is not involved
			// in a serial transfer
			/////
			/////
			// if we are here, the interrupt flag is set
			// so clear it anyway
			/////
			I20CONCLR = I2CONCLR_SIC;
			break;
		default:
			//U0THR='O';
			I2CComplete = TRUE;
			I2CFailed = TRUE;
			I20CONSET = I2CONSET_STO;
			/////
			// clear the interrupt
			/////
			I20CONCLR = I2CONCLR_SIC;	
			break;
  }
  IDISABLE;
  VICVectAddr = 0;		/* Acknowledge Interrupt */
}
/*****************************************************************************
** Function name:		I2CStop
**
** Descriptions:		Set the I2C stop condition, if the routine
**				never exit, it's a fatal bus error.
**
** parameters:			None
** Returned value:		true or never return
** 
*****************************************************************************/
static unsigned long I2CStop( void )
{
  I20CONSET = I2CONSET_STO;      /* Set Stop flag */ 
  I20CONCLR = I2CONCLR_SIC;      /* Clear SI flag */ 
          
  /*--- Wait for STOP detected ---*/
  while( I20CONSET & I2CONSET_STO ); 
  return TRUE;
}

/*****************************************************************************
** Function name:		I2CStart
**
** Descriptions:		Create I2C start condition, a timeout
**				value is set if the I2C never gets started,
**				and timed out. It's a fatal error. 
**
** parameters:			None
** Returned value:		true or false, return false if timed out
** 
*****************************************************************************/
static unsigned long I2CStart( void )
{
  unsigned long timeout = 0;
  unsigned long retVal = FALSE;
	
	I2CStarted = FALSE;
	I2CComplete = FALSE;
	I2CFailed = FALSE;
	
  /*--- Issue a start condition ---*/

	
	/////
	// set the start flag
	/////
  I20CONSET = I2CONSET_STA;	/* Set Start flag */
    
  /*--- Wait until START transmitted ---*/

  while( 1 )
  {
		if (I2CStarted)
		{
			retVal = TRUE;
			break;	
		}
		if ( timeout >= MAX_TIMEOUT )
		{
			retVal = FALSE;
			break;
		}
		timeout++;
  }
  return( retVal );
}



/*****************************************************************************
** Function name:		I2CHWInit
**
** Descriptions:		Initialize I2C controller
**
** parameters:			I2c mode is either MASTER or SLAVE
** Returned value:		true or false, return false if the I2C
**				interrupt handler was not installed correctly
** 
*****************************************************************************/
static unsigned long I2CHWInit( unsigned long I2cMode ) 
{
  PCONP   |= (1 << 19);
  PINSEL1 &= ~0x03C00000;
  PINSEL1 |= 0x01400000;	/* set PIO0.27 and PIO0.28 to I2C0 SDA and SCK */
							            /* function to 01 on both SDA and SCK. */
	
	// set up clocking
	//PCLKSEL0 |= (CCLK_OVER_1 << PCLK_I2C0);
	//PCLKSEL0 |= (CCLK_OVER_4 << PCLK_I2C0);
	PCLKSEL0 |= (CCLK_OVER_8 << PCLK_I2C0);
	PCONP |= (1 << PCI2C0);   	// i2c

  
  /*--- Clear flags ---*/
  I20CONCLR = I2CONCLR_AAC | I2CONCLR_SIC | I2CONCLR_STAC | I2CONCLR_I2ENC;    

  /*--- Reset registers ---*/
  I20SCLL   = I2SCLL_SCLL;
  I20SCLH   = I2SCLH_SCLH;

  if ( I2cMode == I2CSLAVE )
  {
	I20ADR = 0x00;		//UC SHOULD NOT BE SLAVE PCA9532_ADDR;
  }    
	I2CStarted = FALSE;
	I2CComplete = FALSE;
	I2CFailed = FALSE;
  /* Install interrupt handler */
  //void I2C0MasterHandler(void) __irq 	
  if ( install_irq( I2C0_INT, (void (*) (void) __irq) I2C0MasterHandler, HIGHEST_PRIORITY ) == FALSE )
  {
	return( FALSE );
  }
  I20CONSET = I2CONSET_I2EN;
  return( TRUE );
}


/*****************************************************************************
** Function name:		I2CClearBuffer
**
** Descriptions:		added to this file WPN 22-SEP to make 	
**						buffer clear a function call 
**
** parameters:			None
** Returned value:		none
*****************************************************************************/
static void I2CBufferClear()
{
   int i = 0;

   for ( i = 0; i < BUFSIZE; i++ )	/* clear buffer */
   {
		I2CReadBuffer[i] = 0;
		I2CWriteBuffer[i] = 0;
   }
}
 
/////
// start the next transfer
/////
static BOOL I2CStartTransfer(BYTE *pWriteBuffer, int nWriteLength, int nReadLength, BYTE nSLA_W, BYTE nSLA_R)
{
	BOOL nRetVal = TRUE;
	int i;
	//printf("\n\r8-nSLA_W[%X] nSLA_R[%X]:", nSLA_W, nSLA_R);

	/////
	// stop the bus
	/////
	I2CStop();
	
	/////
	// clear the master buffer
	/////
	I2CBufferClear();
	
	/////
	// start the lost transfer timer
	/////
	startTimer(&hangTimer, I2C_HANGUP_TIMER_MS);	
	/////
	// copy write data to our buffer
	/////
	for(i=0; i<nWriteLength; i++)
	{
		I2CWriteBuffer[i] = pWriteBuffer[i];
	}
	
	/////
	// grab SLA_W and SLA_R
	// grab write and read lengths
	/////
	I2CWriteLength = nWriteLength;
	I2CReadLength = nReadLength;
	I2CSLA_W = nSLA_W;
	I2CSLA_R = nSLA_R;
	
	/////
	// reset the indexes
	// and start the transfer
	/////
	RdIndex = 0;
	WrIndex = 0;
	if (I2CStart() != TRUE)
	{
		I2CStop();
		I2CComplete = TRUE;
		I2CFailed = TRUE;
		nRetVal = FALSE;
	}

	return nRetVal;
}
static BOOL I2CIsTransferComplete(void)
{
	BOOL bRetVal = FALSE;
	if((TRUE == I2CComplete) || (FALSE == I2CStarted))
	{
		bRetVal = TRUE;
	}
	return bRetVal;
}

//===============================================
//static unsigned char i2cTXBuffer[I2C_MAX_LENGTH];
//static unsigned char i2cRXBuffer[I2C_MAX_LENGTH];
static I2C_QUEUE I2CQueue;
static I2C_QUEUE_ENTRY* pCurrentEntry;
I2C_TRANSACTION* pCurrentTransaction;

void I2CInitialize()
{
	initTimer(&hangTimer);	
	startTimer(&hangTimer, I2C_HANGUP_TIMER_MS);
	/////
	// initialize the I2C queue
	/////
	queueInitialize((QUEUE*)&I2CQueue);
	pCurrentEntry = NULL;
	pCurrentTransaction = NULL;
	I2CReadLength = 0;
	I2CWriteLength = 0;
	printf("1-I2CInitalize\r\n");
	/////
	// initialize the I2C hardware
	/////
	I2CHWInit( I2CMASTER );
	
	/////
	// install the I2C interrupt handler
	/////
}
void I2CQueueDoWork()
{
	if(TRUE == I2CIsTransferComplete())
	{
		if(NULL != pCurrentTransaction)
		{
			if(!pCurrentTransaction->bComplete)
			{
				/////
				// transaction just completed
				/////
				if(I2CFailed)
				{
						/////
						// transfer failed
						// don't process any further transactions from this queue entry
						/////
						pCurrentEntry->eState = eI2C_TRANSFER_STATE_FAILED;
						pCurrentTransaction->bComplete = TRUE;
						pCurrentEntry = NULL;
						pCurrentTransaction = NULL;
				}
				else
				{
					/////
					// copy rxbuffer to current transaction entry
					/////
					memcpy(pCurrentTransaction->cIncomingData, (unsigned char*)I2CReadBuffer, I2CReadLength);
				
					/////
					// are there more transactions
					// in this queue entry?
					/////
					if(0 < pCurrentEntry->nTransactions--)
					{
						/////
						// start the next transaction
						//////
						pCurrentTransaction = (I2C_TRANSACTION*)queuePop((QUEUE*)&pCurrentEntry->transactionQueue);
						if(!I2CStartTransfer(pCurrentTransaction->cOutgoingData, pCurrentTransaction->nOutgoingDataLength, pCurrentTransaction->nIncomingDataLength, pCurrentTransaction->nSLA_W, pCurrentTransaction->nSLA_R))
						{
							/////
							// couldn't start the transfer
							/////
							pCurrentEntry->eState = eI2C_TRANSFER_STATE_FAILED;
							pCurrentTransaction->bComplete = TRUE;
							pCurrentEntry = NULL;
							pCurrentTransaction = NULL;
						}
					}
					else
					{
						/////
						// transfer complete, no more transactions
						/////
						pCurrentEntry->eState = eI2C_TRANSFER_STATE_COMPLETE;
						pCurrentEntry = NULL;
						pCurrentTransaction = NULL;
					}
				}
			}
		}
		else
		{
			/////
			// get a new entry
			// and start the first transfer
			/////
			pCurrentEntry = (I2C_QUEUE_ENTRY*)queuePop((QUEUE*)&I2CQueue);
			if(NULL != pCurrentEntry)
			{
			  //printf("I2CQueueDoWork: eI2C_TRANSFER_STATE_TRANSFERRING\n");
				
				pCurrentEntry->eState = eI2C_TRANSFER_STATE_TRANSFERRING;
				if(0 < pCurrentEntry->nTransactions--)
				{
					pCurrentTransaction = (I2C_TRANSACTION*)queuePop((QUEUE*)&pCurrentEntry->transactionQueue);
					if(!I2CStartTransfer(pCurrentTransaction->cOutgoingData, pCurrentTransaction->nOutgoingDataLength, pCurrentTransaction->nIncomingDataLength, pCurrentTransaction->nSLA_W, pCurrentTransaction->nSLA_R))						
					{
						printf("I2CQueueDoWork: eI2C_TRANSFER_STATE_FAILED\r\n");
						/////
						// couldn't start the transfer
						/////
						pCurrentEntry->eState = eI2C_TRANSFER_STATE_FAILED;
						pCurrentTransaction->bComplete = TRUE;
						pCurrentEntry = NULL;
						pCurrentTransaction = NULL;
					}						
				}
				else
				{
					printf("I2CQueueDoWork: eI2C_TRANSFER_STATE_COMPLETE2\r\n");
					/////
					// transfer complete, no more transactions
					/////
					pCurrentEntry->eState = eI2C_TRANSFER_STATE_COMPLETE;
					pCurrentEntry = NULL;
					pCurrentTransaction = NULL;
				}
			}
		}
	}
	if(isTimerExpired(&hangTimer))
	{
		if((TRUE == I2CStarted) && (FALSE == I2CComplete))
		{
			//U0THR = 'X';
			//I2CStarted = FALSE;
			I2CComplete = TRUE;
			I2CFailed = TRUE;
		}
		startTimer(&hangTimer, I2C_HANGUP_TIMER_MS);
	}
}
void I2CQueueEntryInitialize(I2C_QUEUE_ENTRY* pEntry)
{
	pEntry->eState = eI2C_TRANSFER_STATE_IDLE;
	pEntry->nTransactions = 0;
	queueInitialize(&pEntry->transactionQueue);
}
void I2CQueueInitializeTransaction(I2C_TRANSACTION* pTransaction)
{
	pTransaction->queueEntry.next = NULL;
	pTransaction->bComplete = FALSE;
	pTransaction->nIncomingDataLength = 0;
	pTransaction->nOutgoingDataLength = 0;
	memset(pTransaction->cOutgoingData, 0, sizeof(pTransaction->cOutgoingData));
	memset(pTransaction->cIncomingData, 0, sizeof(pTransaction->cIncomingData));
	
}

void I2CQueueAddTransaction(I2C_QUEUE_ENTRY* pEntry, I2C_TRANSACTION* pTransaction)
{
	queueAdd(&pEntry->transactionQueue, &pTransaction->queueEntry);
	pEntry->nTransactions++;
}

void I2CQueueAddEntry(I2C_QUEUE_ENTRY* pEntry)
{
	pEntry->eState = eI2C_TRANSFER_STATE_PENDING;
	queueAdd((QUEUE*)&I2CQueue, (QUEUE_ENTRY*)pEntry);
}
//================================================
