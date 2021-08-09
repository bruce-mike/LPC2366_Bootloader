#include "shareddefs.h"
#include "spiUart.h"
#include "ssp.h"
#include "timer.h"
#include "mischardware.h"
#include <stdio.h>


#define REG_ADDRESS_BYTE    0
#define REG_DATA_BYTE       1
#define REG_NUM_BYTES       2

#define DATA_BUFFER_SIZE 40

static TIMERCTL spiUartTimer;
#define SPI_UART_SOLAR_MANUAL_MODE_TIME_MS  70


BYTE registerData[REG_NUM_BYTES];

void spiUartDoWork()
{
    int numFifoChars, i;
    BYTE radarByte;
   
static long readingNum = 0;
    
    if(isTimerExpired(&spiUartTimer))
    {   
        if(hwGetSpiUartDataReady())
        {
            numFifoChars = spiUartReadReg(RXLVL_REG, CHANNEL_A);
            printf("SpiUartReady[%d]  ", numFifoChars); 
            for(i=0; i<numFifoChars; i++)
            {
                radarByte = spiUartReadReg(RHR_REG, CHANNEL_A);
                printf("[%02x]", radarByte);
                if(TRUE == radarParseMessage(radarByte))
                {
                    printf(" MPH[%d] %ld\r\n", radarGetMPH(), ++readingNum);
                }

            }
            printf("\r\n");
        }
        
        startTimer(&spiUartTimer, SPI_UART_SOLAR_MANUAL_MODE_TIME_MS);	
    } 
}

BYTE spiUartReadReg(BYTE reg, BYTE channel)
{   
    BYTE regData;
    
    reg = (reg<<3) | 0x80;    // register address byte    
    if (channel == 02)   
        reg |= channel;          // channel address byte   

    registerData[REG_ADDRESS_BYTE] = reg;
    // second byte of read is dummy byte
    // sent to clock in register data
    registerData[REG_DATA_BYTE] = 0x00;  
    
    SSPChipSelect(TRUE);    
    
    regData = SSPSend(&registerData[0], 2);  
     
    SSPChipSelect(FALSE);
    
    return regData;
}


void spiUartWriteReg(BYTE reg, BYTE value)
{
    SSPChipSelect(TRUE);
    
    registerData[REG_ADDRESS_BYTE] = reg;
    registerData[REG_DATA_BYTE] = value;
    SSPSend(&registerData[0], 2);

    SSPChipSelect(FALSE);
}    
    

void spiUartInit(void)
{ 
    initTimer(&spiUartTimer);
    
    startTimer(&spiUartTimer, SPI_UART_SOLAR_MANUAL_MODE_TIME_MS);
    
    // set up baud rate 1200 with 3.684 MHz
    // 192 = 3.6864MHz/(16 x 1200 baud)
    spiUartWriteReg(LCR_REG	<< 3, 0x80);	// 0x80 when LCR[7] = 1 DLL and DLH are accessible
	spiUartWriteReg(DLL_REG	<< 3, 192);	    // 0x08 = 115,200 baud rate when XTal = 14.7456 MHz
	spiUartWriteReg(DLH_REG	<< 3, 0x00);	// 0x00 = 115,200 baud rate when XTal = 14.7456 MHz

	spiUartWriteReg(LCR_REG	<< 3, 0xBF);	// Access special features register
	spiUartWriteReg(EFR_REG	<< 3, 0x10);	// enable enhanced functions
	spiUartWriteReg(LCR_REG	<< 3, 0x03);	// 8 data bit, 1 stop bit, no parity

	spiUartWriteReg(IODIR_REG	<< 3, 0xFF);	// set GPIO [7:0] to output (input by default)
	spiUartWriteReg(IOSTATE_REG << 3, 0x00);	// set GPIO [7:0] to 0x00 (Turn LEDs on)
	spiUartWriteReg(IER_REG	<< 3, 0x01);	// enable Rx data ready interrupt
    
    spiUartWriteReg(EFR_REG	<< 3, 0x08);	// Access TLR register
    spiUartWriteReg(MCR_REG << 3, 0x04);    // Access TLR register
    spiUartWriteReg(TLR_REG << 3, 0x70);    // Set up trigger value
    
    spiUartWriteReg(FCR_REG << 3, 0x01);    // Enable FIFO
    
}



