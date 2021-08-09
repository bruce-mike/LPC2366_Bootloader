#ifndef _SPI_UART
#define _SPI_UART


// DUAL CHANNEL SPI UART
#define CHANNEL_A   1
#define CHANNEL_B   2

// General registers

//Read only
#define RHR_REG         0x00		//Receive Holding Register
#define IIR_REG         0x02		//Interrupt identification register
#define LSR_REG         0x05		//Line status register
#define MSR_REG         0x06		//Modem StatusRegister
#define TXLVL_REG       0x08		//Transmitter FIFO Level register
#define RXLVL_REG       0x09		//Receiver FIFO level register
#define IOSTATE_REG     0x0B		//Pins State Register

//Write
#define THR_REG         0x00		//Transmit Holding Register, where the data is store
#define IER_REG         0x01		//Interrupt enable register
#define FCR_REG         0x02		//Fifo control register
#define LCR_REG         0x03		//Line control register											//before being shifted out
#define MCR_REG         0x04		//Modem control register
#define SPR_REG         0x07		//Scratchpad Register
#define IODIR_REG       0x0A		//Pin Direction Register
#define IOINTENA_REG    0x0C		//Input/Output interrupt enable
#define IOCONTROL_REG   0x0E		//Input/Output control
#define EFCR_REG        0x0F		//Extra features control register


// Registers used for baud rate generation
// This register are only accessible when LCR[7] = 1
#define DLL_REG     0x00    //Division Register Low
#define DLH_REG     0x01    //Division Register High



// Enhanced registers
// Only accessible when LCR is set to 0xBF

#define EFR_REG     0x02    // Enhanced Features Register


#define XON1        0x04
#define XON2        0x05
#define XOFF1       0x06
#define XOFF2       0x07

// These registers are only accessible when EFR[4] = 1 and MCR[2] = 1
#define TCR_REG     0x06    //Transmission control register
#define TLR_REG     0x07    //Trigger Level Register



void spiUartInit(void);

BYTE spiUartReadReg(BYTE reg, BYTE channel);

void spiUartDoWork(void);



#endif

