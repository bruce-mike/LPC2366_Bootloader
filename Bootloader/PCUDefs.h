#ifndef POWERBOARD_DEFS_H
#define POWERBOARD_DEFS_H


#define POWER_BOARD_CONTROLLER

/////
// nvdata ids
/////
#define FLASH_SENDERS_ESN_ID 1

//DATA DIRECTIONS/pin modes
#define IN			0
#define OUT			1


// **********************
// PORT 0 BIT DEFS
// **********************
#define uP_SYS_TXD		0 // UART3 TXD3
#define uP_SYS_RXD		1 // UART3 RXD3
#define uP_DBG_TXD 		2 
#define uP_DBG_RXD		3
#define uP_W85_DEN 		4
#define uP_W85_xRE		5
#define uP_SPI1_xCS 	6
#define uP_SPI1_SCK		7
#define uP_SPI1_MISO 	8
#define uP_SPI1_MOSI	9
#define uP_W85_TXD		10
#define uP_W85_RXD		11
// 12 UNAVAILABLE (P.104)
// 13 UNAVAILABLE (P.104)
// 14 UNAVAILABLE (P.104)
#define uP_RDR_TXD		15
#define uP_RDR_RXD		16
// 17 UNUSED/NC
// 18 UNUSED/NC
// 19 UNUSED/NC
// 20 UNUSED/NC
// 21 UNUSED/NC
// 22 UNUSED/NC
#define uP_ADC_VS			23
#define uP_ADC_VL			24
#define uP_ACD_VB			25
#define uP_ADC_VD			26
#define I2C_SDA				27
#define I2C_SCL				28
// 29 UNUSED/NC
// 30 UNUSED/NC
// 31 UNAVAILABLE (P.104)

// **********************
// PORT 1 BIT DEFS
// **********************
// 0 UNUSED/NC
// 1 UNUSED/NC
// 2 UNAVAILABLE (P.107)
// 3 UNAVAILABLE (P.107)
#define uP_WD_KICK		4		// Watch Dog Pet
// 5 UNAVAILABLE (P.107)
// 6 UNAVAILABLE (P.107)
// 7 UNAVAILABLE (P.107)
// 8 UNUSED/NC
// 9 UNUSED/NC
// 10 UNUSED/NC
// 11 UNAVAILABLE (P.107)
// 12 UNAVAILABLE (P.107)
// 13 UNAVAILABLE (P.107)
// 14 UNUSED/NC
// 15 UNUSED/NC
// 16 UNUSED/NC
// 17 UNUSED/NC
// 18 UNUSED/NC
#define uP_MOD_X1			19		// model select
#define uP_POS_VL			20		// solar polarity
#define uP_POS_VB			21		// battery polarity
#define uP_MOD_X2			22		// ditto
#define uP_OVP_VL			23		// force 4363 on out of OVLO
// 24 UNUSED/NC
#define uP_MOD_X4			25		// ditto
#define ISP_xRST			26
#define uP_MOD_X8			27		// ditto
#define uP_PWR_VL			28		// solar charger 4363 enable
#define uP_PWR_VB			29		// battery 4365 enable
#define uP_ADC_PR			30
#define uP_ADC_PF		  31		// 

// **********************
// PORT 2 BIT DEFS
// **********************
// 0 UNUSED/NC
#define uP_LOAD_SW_EN	1
// 2 UNUSED/NC
// 3 UNUSED/NC
// 4 UNUSED/NC
// 5 UNUSED/NC
#define REV2            5
#define uP_AUX_LED 		6
#define REV1            7
#define REV0            8			//
#define uP_PWR_xSHDN	9			// LVD 
#define uP_xISP				10		// unused (except for ISP)
#define uP_INT_xTMP		11		// temp sensor interrupt (if programmed)
// 12 UNUSED/NC
#define uP_INT_xERR		13		// output channel errors
// 14 UNAVAILABLE (P.109)
// 15 UNAVAILABLE (P.109)
// 16 UNAVAILABLE (P.109)
// 17 UNAVAILABLE (P.109)
// 18 UNAVAILABLE (P.109)
// 19 UNAVAILABLE (P.109)
// 20 UNAVAILABLE (P.109)
// 21 UNAVAILABLE (P.109)
// 22 UNAVAILABLE (P.109)
// 23 UNAVAILABLE (P.109)
// 24 UNAVAILABLE (P.109)
// 25 UNAVAILABLE (P.109)
// 26 UNAVAILABLE (P.109)
// 27 UNAVAILABLE (P.109)
// 28 UNAVAILABLE (P.109)
// 29 UNAVAILABLE (P.109)
// 30 UNAVAILABLE (P.109)
// 31 UNAVAILABLE (P.109)


// **********************
// PORT 3
// **********************

// 0  UNAVAILABLE (P.111)
// 1  UNAVAILABLE (P.111)
// 2  UNAVAILABLE (P.111)
// 3  UNAVAILABLE (P.111)
// 4  UNAVAILABLE (P.111)
// 5  UNAVAILABLE (P.111)
// 6  UNAVAILABLE (P.111)
// 7  UNAVAILABLE (P.111)
// 8  UNAVAILABLE (P.111)
// 9  UNAVAILABLE (P.111)
// 10 UNAVAILABLE (P.111)
// 11 UNAVAILABLE (P.111)
// 12 UNAVAILABLE (P.111)
// 13 UNAVAILABLE (P.111)
// 14 UNAVAILABLE (P.111)
// 15 UNAVAILABLE (P.111)
// 16 UNAVAILABLE (P.111)
// 17 UNAVAILABLE (P.111)
// 18 UNAVAILABLE (P.111)
// 19 UNAVAILABLE (P.111)
// 20 UNAVAILABLE (P.111)
// 21 UNAVAILABLE (P.111)
// 22 UNAVAILABLE (P.111)
// 23 UNAVAILABLE (P.111)
// 25 UNAVAILABLE (P.111)
// 25 UNUSED/NC
// 26 UNUSED/NC
// 27 UNAVAILABLE (P.111)
// 28 UNAVAILABLE (P.111)
// 29 UNAVAILABLE (P.111)
// 30 UNAVAILABLE (P.111)
// 31 UNAVAILABLE (P.111)

// **********************
// PORT 4
// **********************

// 0  UNAVAILABLE (P.111)
// 1  UNAVAILABLE (P.111)
// 2  UNAVAILABLE (P.111)
// 3  UNAVAILABLE (P.111)
// 4  UNAVAILABLE (P.111)
// 5  UNAVAILABLE (P.111)
// 6  UNAVAILABLE (P.111)
// 7  UNAVAILABLE (P.111)
// 8  UNAVAILABLE (P.111)
// 9  UNAVAILABLE (P.111)
// 10 UNAVAILABLE (P.111)
// 11 UNAVAILABLE (P.111)
// 12 UNAVAILABLE (P.111)
// 13 UNAVAILABLE (P.111)
// 14 UNAVAILABLE (P.111)
// 15 UNAVAILABLE (P.111)
// 16 UNAVAILABLE (P.111)
// 17 UNAVAILABLE (P.111)
// 18 UNAVAILABLE (P.111)
// 19 UNAVAILABLE (P.111)
// 20 UNAVAILABLE (P.111)
// 21 UNAVAILABLE (P.111)
// 22 UNAVAILABLE (P.111)
// 23 UNAVAILABLE (P.111)
// 25 UNAVAILABLE (P.111)
// 26 UNAVAILABLE (P.111)
// 27 UNAVAILABLE (P.111)
#define uP_SPI1_HOLD 28
#define uP_SPI1_WP   29
// 30 UNAVAILABLE (P.111)
// 31 UNAVAILABLE (P.111)
#endif	// POWERBOARD_DEFS_H
