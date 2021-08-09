#ifndef SERIAL_H
#define SERIAL_H
#include <LPC23xx.H>                   /* LPC23xx definitions               */
#include <stdio.h>
#include "shareddefs.h"
#include "PCUDefs.h"

#define DEBUG_USE_INTERRUPTS 1

#define FIFO_ENABLE 0x01

#define THRE_INTERRUPT_ENABLE 0x02
#define RX_DATA_AVAILABLE_INTERRUPT_ENABLE 0x01
#define RDR  0x01
#define THRE 0x20
#define TEMT 0x40


//RS485 MODES
#define XMIT 			0
#define RECV			1
#define SHUTDOWN	2

#define SUCCESS 	 1
#define ERROR 		-1

#define EIGHTBITS  	0x03	// LCR select 8 bits
#define DATA_WIDTH  0			// LCR bits 1:0 = data witdth
#define DLAB	   		7    	// LCR bit 7
/////
// BAUD = PCLK/(16*((256*DLM)+DLL)*(1+(divAddValue/MultValue))
// ignoring divAddValue and multValue
// BAUD = PCLK/(16*((256*DLM)+DLL))
// PCLK/BAUD = (16*((256*DLM)+DLL))
// PCLK/(BAUD*16) = (256*DLM)+DLL;
// DLM = (PCLK/(BAUD*16))/256)
// DLL = PCLK/(BAUD*16)-(256*DLM)
/////
#define DLL_9600_96MHZ		 			0x71	//MAGIC #: 9600bps@96000000 CCLK = PCLK	  
#define DLM_9600_96MHZ					2

#define DLL_115200_96MHZ	 			0x34	//MAGIC #: 115200bps@96000000 CCLK = PCLK	  
#define DLM_115200_96MHZ				0

#define DLL_9600_72MHZ		 			0xD4	//MAGIC #: 9600bps@72000000 CCLK = PCLK	  
#define DLM_9600_72MHZ					1	

#define DLL_19200_72MHZ		 			0xEA	//MAGIC #: 19200bps@72000000 CCLK = PCLK	  
#define DLM_19200_72MHZ					0	

#define DLL_115200_72MHZ	 			0x27	//MAGIC #: 115200bps@72000000 CCLK = PCLK	  
#define DLM_115200_72MHZ				0

#define DLL_9600_36MHZ		 			0xE6	//MAGIC #: 9600bps@36000000 CCLK = PCLK	  
#define DLM_9600_36MHZ					0	

#define DLL_19200_36MHZ					0x75	//MAGIC #: 19200bps@36000000 CCLK = PCLK
#define DLM_19200_36MHZ					0

#define DLL_57600_36MHZ					0x27	//MAGIC #: 57600bps@36000000 CCLK = PCLK
#define DLM_57600_36MHZ					0

#define TXD0_SEL	4   		// pinsel0
#define RXD0_SEL	6				// pinsel0
#define UART0_SEL	0x01		// this value is shifted left by [T, R]xD0_SEL in pisel0 register

#define TXD1_SEL	30	 		// **PINSEL0
#define RXD1_SEL	0	 			// **PINSEL1
#define UART1_SEL 0x01		// ditto pinsel1

#define TXD2_SEL	20			//pinsel0
#define RXD2_SEL	22			//pinsel0
#define UART2_SEL	0x01		// ...and so on

#define TXD3_SEL	0 //24	// pinsel0
#define RXD3_SEL	2 //26	// pinsel0
#define UART3_SEL	0x02		// ...and so forth



// periph clock
#define PCLK_UART2 	16

// periph power
#define PCUART2 		24	

#define NO_PACKET_DATA 							-1
#define MAX_PACKET_LENGTH						MAX_COMMAND_LENGTH*2		// twice the actual packet size in case all bytes are escaped

#define SEND_QUEUE_MAX_ENTRIES		30

//#define SEND_QUEUE_IDLE_TIME			250
//#define SEND_QUEUE_IDLE_TIME			300
//#define SEND_QUEUE_IDLE_TIME				100
#define SEND_QUEUE_IDLE_TIME				50

typedef struct send_queue_entry
{
		QUEUE_ENTRY queueEntry;
		unsigned char packet[MAX_PACKET_LENGTH];
		int nPacketLength;
}SEND_QUEUE_ENTRY;

//********************
// **** PROTOTYPES
//int init_serial   (int);	
//void SET_485_MODE (int);
int sendchar      (int);
int sendcharU1    (int);
int sendcharU2    (int);
int sendcharU3    (int);
#if 0
int getkey        (void);
int getkey485     (void);
#endif
void serialInit(void);
void serialDoWork(void);
void serialDisableInterface(eINTERFACE eInterface);
void serialEnableInterface(eINTERFACE eInterface);
void serialRS485SendData(unsigned char nData);
void serialPcuSendData(unsigned char nData);
unsigned char serialReadWireless(unsigned char* pData);
unsigned char serialWriteWireless(unsigned char nData);
void serialSendPacket(eINTERFACE eInterface, unsigned char* pPacket, int nPacketLength);
void serialWrite(eINTERFACE eInterface, unsigned char* pPacket, int nPacketLength);
int pcuUnstuffRxPacket(unsigned char *pStuffedRxPacket, unsigned char *pUnstuffedRxPacket, int *nStuffedPacketLength);
void serialSendUpgradeCommand(eINTERFACE eInterface, BYTE txByte);
void serialSetUpgradeMode(BYTE active);
unsigned char serialGetUpgradeMode(void);


#endif		// SERIAL_H

