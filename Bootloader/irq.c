/*****************************************************************************
 *   irq.c: Interrupt handler C file for NXP LPC23xx/24xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.07.13  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include "LPC23xx.h"			/* LPC23XX/24xx Peripheral Registers */
#include "shareddefs.h"
#include <stdio.h>
#include "irq.h"

void dumpstack(char* pString, unsigned int *pData)
{
	unsigned int *pStackUp = pData;
	int i;
	printf("Exception [%s]\r\n", pString);

	printf("counting up\r\n");
	for(i=0;i<40;i++)
	{
		printf("[%X]\r\n", *pStackUp);
		pStackUp++;
	}
}

void UNDEF_Routine (void) __irq
{
	unsigned int nData;
	nData = 1234567;
	dumpstack("PAbt", &nData);

        while (1) ;
}
void PAbt_Routine (void) __irq
{

	unsigned int nData;
	nData = 1234567;
	dumpstack("PAbt", &nData);
        while (1) ;
}
void DAbt_Routine (void) __irq
{
	unsigned int nData;
	nData = 1234567;
	dumpstack("DAbt", &nData);
        while (1) ;
}

/******************************************************************************
** Function name:		FIQ_Handler
**
** Descriptions:		FIQ interrupt handler called in startup
** parameters:			 
**					
** Returned value:		
** 
******************************************************************************/
void FIQ_Handler( void ) __irq
{
#if FIQ
  if ( VICFIQStatus & (0x1<<4) && VICIntEnable & (0x1<<4) )
  {
	Timer0FIQHandler();	
  }
  if ( VICFIQStatus & (0x1<<5) && VICIntEnable & (0x1<<5) )
  {
	Timer1FIQHandler();	
  }
  return;
#endif 
}

/* Initialize the interrupt controller */
/******************************************************************************
** Function name:		init_VIC
**
** Descriptions:		Initialize VIC interrupt controller.
** parameters:			None
** Returned value:	None
** 
******************************************************************************/
void init_VIC(void) 
{
    DWORD i = 0;
    DWORD *vect_addr, *vect_prio;
   	
    /* initialize VIC*/
    VICIntEnClr = 0xffffffff;
    VICVectAddr = 0;
    VICIntSelect = 0;

    /* set all the vector and vector control register to 0 */
    for ( i = 0; i < VIC_SIZE; i++ )
    {
		vect_addr  = (DWORD *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + i*4);
		vect_prio  = (DWORD *)(VIC_BASE_ADDR + VECT_PRIO_INDEX + i*4);
		*vect_addr = 0x0;	
		*vect_prio = 0xF;
    }
    return;
}

/******************************************************************************
** Function name:		install_irq
**
** Descriptions:		Install interrupt handler
** parameters:			Interrupt number, interrupt handler address, 
**						interrupt priority
** Returned value:		true or false, return false if IntNum is out of range
** 
******************************************************************************/
DWORD install_irq( DWORD IntNumber, void (*HandlerAddr)(void)__irq, DWORD Priority )
{
    DWORD *vect_addr;
    DWORD *vect_prio;
      
    VICIntEnClr = 1 << IntNumber;	/* Disable Interrupt */
    if ( IntNumber >= VIC_SIZE )
    {
		return ( FALSE );
    }
    else
    {
		/* find first un-assigned VIC address for the handler */
		vect_addr    = (DWORD *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + IntNumber*4);
		vect_prio    = (DWORD *)(VIC_BASE_ADDR + VECT_PRIO_INDEX + IntNumber*4);
		*vect_addr   = (DWORD)HandlerAddr;	/* set interrupt vector */
		*vect_prio   = Priority;
		VICIntEnable = 1 << IntNumber;	/* Enable Interrupt */
			
		//printf("VICIntEnable[%lu]\n",VICIntEnable);
		return( TRUE );
    }
}


//===================================================================================
// DEBUG cause fault condition
// to test IRQ handlers for UNDEF, PAbt, DAbt interrupts
//===================================================================================
void causeCrash(int nType)
{
	switch(nType)
	{
		case 0:
			/////
			// DAbt
			/////
			printf("cause DAbt crash\r\n");
			{
			unsigned int* pData = (unsigned int*)12345678;
			unsigned int nData = *pData;
			printf("nData[%d]\r\n", nData);
			}
			break;
		case 1:
			/////
			// PAbt
			/////
			printf("cause PAbt crash (not yet)\r\n");
			break;
		case 2:
			/////
			// UNDEF
			/////
			printf("cause UNDEF crash (not yet)\r\n");
			break;
		default:
				break;
	}
}
/******************************************************************************
**                            End Of File
******************************************************************************/

