/*****************************************************************************
 *   uarttest.c:  main C entry file for NXP LPC23xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.07.13  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include "LPC23xx.h"                        /* LPC23xx/24xx definitions */
#include "shareddefs.h"
#include "PCUDefs.h"
#include "sharedinterface.h"
#include "irq.h"
#include "serial.h"

extern volatile DWORD UART0Count;
extern volatile BYTE UART0Buffer[SERIAL_BUFSIZE];
extern volatile DWORD UART1Count;
extern volatile BYTE UART1Buffer[SERIAL_BUFSIZE];

/*****************************************************************************
**   Main Function  main()
*****************************************************************************/
int main (void)
{
  UARTInit(0, 115200);	/* baud rate setting */
  UARTInit(1, 115200);	/* baud rate setting */

  while (1) 
  {				/* Loop forever */
		if ( UART0Count != 0 )
		{
			U0IER = IER_THRE | IER_RLS;			/* Disable RBR */
			UARTSend( 0, (BYTE *)UART0Buffer, UART0Count );
			UART0Count = 0;
			U0IER = IER_THRE | IER_RLS | IER_RBR;	/* Re-enable RBR */
		}
		if ( UART1Count != 0 )
		{
			U1IER = IER_THRE | IER_RLS;			/* Disable RBR */
			UARTSend( 1, (BYTE *)UART1Buffer, UART1Count );
			UART1Count = 0;
			U1IER = IER_THRE | IER_RLS | IER_RBR;	/* Re-enable RBR */
		}
  }
  return 0;
}

/*****************************************************************************
**                            End Of File
*****************************************************************************/
