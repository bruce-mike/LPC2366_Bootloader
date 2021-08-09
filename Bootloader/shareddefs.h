//========================================================================================
// defines shared between the driver board and handheld
//========================================================================================
#ifndef SHARED_DEFS_H
#define SHARED_DEFS_H

#define TRUE 1
#define FALSE 0

//DATA DIRECTIONS/pin modes
#define IN			0
#define OUT			1
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned char  BOOL;
typedef unsigned int   UINT;

/////
// nvdata ids
/////
#define FLASH_SENDERS_ESN_ID 1

//periph clocking
#define CCLK_OVER_4 00			// datasheet p. 63
#define CCLK_OVER_1 1
#define CCLK_OVER_8 11
#define BASE_CLK_96MHZ   (95232000)
#define BASE_CLK_72MHZ   (72000000)
#define BASE_CLK_36MHZ   (36000000)

// PERIPHERAL POWER CONTROL BITS
// 0 UNUSED
#define PCTIM0   1
#define PCTIM1   2
#define PCUART0  3
#define PCUART1  4
// 5 UNUSED
#define PCPWM1   6
#define PCI2C0   7
#define PCSPI    8
#define PCRTC    9
#define PCSSP1  10
#define PCEMC   11
#define PCAD    12
#define PCAN1   13
#define PCAN2   14
// 15 UNUSED
// 16 UNUSED
// 17 UNUSED
// 18 UNUSED
#define PCI2C1  19
// 20 UNUSED
#define PCSSP0  21
#define PCTIM2  22
#define PCTIM3  23
#define PCUART2 24
#define PCUART3 25
#define PCI2C2  26
#define PCI2S   27
#define PCSDC   28
#define PCGPDMA 28
#define PCENET  30
#define PCUSB   31// PERIPHERAL POWER CONTROL BITS

// PERIPH CLOCK BITS

#define PCLK_UART0  6
#define PCLK_UART1  8
#define PCLK_UART2 16
#define PCLK_UART3 18

// **********************
// Interrupts
// **********************
#define EXT_INT 01

// pin mode bit locations
#define EXTINT0  20
#define EXTINT1  22
#define EXTINT2  24
#define EXTINT3  26

#define EXTINT0_CLEAR 0x00000001
#define EXTINT1_CLEAR 0x00000002
#define EXTINT2_CLEAR 0x00000004
#define EXTINT3_CLEAR 0x0000000

//==========================
// PORT 0 defs
// shared between handheld and driverboard
//===========================
// 0 UNUSED
// 1 UNUSED
// 2 UNUSED
// 3 UNUSED
// 4 UNUSED
// 5 UNUSED
#define uP_SPI1_xCS		6   // SPI1 Flash Chip Select
#define uP_SPI1_SCK		7		// SPI1 Flash SClock
#define uP_SPI1_MISO 	8		// SPI1 Flash MISO
#define uP_SPI1_MOSI	9		// SPI1 Flash MOSI


// 7 UNUSED
// 8 UNUSED
// 9 UNUSED

// 10 UNUSED
// 11 UNUSED
#define uP_SPI0_xCS		16	// SPI0 Flash Chip Select

//==========================
// PORT 1 defs
// for test
//===========================
#define uP_SYS_LED				15


//==========================
// PORT 2 defs
//===========================
#define uP_SPI_UART_RDY     2

#define uP_SPI1_UART_CS     3

#define uP_PWM_xACT		4

#endif		// SHARED_DEFS_H
