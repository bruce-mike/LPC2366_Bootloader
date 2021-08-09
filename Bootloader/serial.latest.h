/*****************************************************************************
 *   uart.h:  Header file for NXP LPC23xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.09.01  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __UART_H 
#define __UART_H
#define DLL_9600		 			0xd4	 //MAGIC #: 9600bps@72000000 CCLK = PCLK	  
#define DLM_9600					1	
#define DLL_115200	 			0x27	//MAGIC #: 115200bps@72000000 CCLK = PCLK	  
#define DLM_115200				0

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

#define SERIAL_BUFSIZE		0x40

DWORD UARTInit( DWORD portNum, DWORD Baudrate );
void UART0Handler( void ) __irq;
void UART1Handler( void ) __irq;
void UARTSend( DWORD portNum, BYTE *BufferPtr, DWORD Length );

#endif /* end __UART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
