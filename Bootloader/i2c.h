/*****************************************************************************
 *   i2c.h:  Header file for NXP LPC23xx/24xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.07.19  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/

#ifndef __I2C_H 
#define __I2C_H

// WARNING: MAY COLLIDE WITH COMMAND CHAR ARRAY DATA!!!
#define END 0xFE  		// WARNING: MAY COLLIDE WITH COMMAND CHAR ARRAY DATA!!!
// WARNING: MAY COLLIDE WITH COMMAND CHAR ARRAY DATA!!! 



#define BUFSIZE			0xA
#define MAX_TIMEOUT	0x0FFF //how many cycles??

#define I2CMASTER		0x01
#define I2CSLAVE		0x02

//************************
//************************
//************************
//9634 defs
//************************
//************************
// BASE ADDRESSES
#define PCA9634_SYS_LEDS_AND_PWM    0xE4
#define PCA9634_SYS_LEDS_AND_PWM_RD 0xE6
#define PCA9634_CH1_CH8							0xE4
#define PCA9634_CH9_CH16						0xE6

// REGISTER DEFS
#define MODE1				0x00		
#define MODE1_INC		0x80		// auto-inc bit set
#define AUTO_INC	  0x80
#define MODE2				0x01
#define PWM0				0x02	
#define PWM1				0x03
#define PWM2				0x04
#define PWM3				0x05
#define PWM4				0x06
#define PWM5				0x07
#define PWM6				0x08
#define PWM7				0x09
#define GRPPWM			0x0A
#define GRPFREQ			0x0B
#define LEDOUT0			0x8C		// auto-inc bit set
#define LEDOUT1			0x0D
#define SUBADR1			0x0E
#define SUBADR2			0x0F
#define SUBADR3			0x10
#define ALLCALLADR	0x11



//************************
//************************
//************************
//MISC I2C DEFS
//************************
//************************

#define I2CONSET_I2EN		0x00000040  /* I2C Control Set Register */
#define I2CONSET_AA			0x00000004
#define I2CONSET_SI			0x00000008
#define I2CONSET_STO		0x00000010
#define I2CONSET_STA		0x00000020

#define I2CONCLR_AAC		0x00000004  /* I2C Control clear Register */
#define I2CONCLR_SIC		0x00000008
#define I2CONCLR_STAC		0x00000020
#define I2CONCLR_I2ENC	0x00000040

#define I2DAT_I2C				0x00000000  /* I2C Data Reg */
#define I2ADR_I2C				0x00000000  /* I2C Slave Address Reg */
#define I2SCLH_SCLH			0x00000080  /* I2C SCL Duty Cycle High Reg */
#define I2SCLL_SCLL			0x00000080  /* I2C SCL Duty Cycle Low Reg */

#define PCLK_I2C0       14					/* peripheral clock select bit location WPN */



//extern unsigned long I2CInit          (unsigned long I2cMode );

#define I2C_HANGUP_TIMER_MS 100

//======================================================



typedef struct i2c_queue
{
	QUEUE head;
}I2C_QUEUE;


//=============================================================================
typedef enum eI2CTransferState
{
	eI2C_TRANSFER_STATE_IDLE,
	eI2C_TRANSFER_STATE_PENDING,
	eI2C_TRANSFER_STATE_TRANSFERRING,
	eI2C_TRANSFER_STATE_COMPLETE,
	eI2C_TRANSFER_STATE_FAILED
}eI2C_TRANSFER_STATE;

typedef struct i2c_queue_entry
{
		QUEUE_ENTRY queueEntry;
		int nTransactions;
		eI2C_TRANSFER_STATE eState;
		QUEUE transactionQueue;
}I2C_QUEUE_ENTRY;

#define I2C_MAX_LENGTH 10
typedef struct i2c_transaction
{
		QUEUE_ENTRY queueEntry;
		BOOL bComplete;
		unsigned char nSLA_W;
		unsigned char nOutgoingDataLength;
		unsigned char cOutgoingData[I2C_MAX_LENGTH];
		unsigned char nSLA_R;
		unsigned char nIncomingDataLength;
		unsigned char cIncomingData[I2C_MAX_LENGTH];
}I2C_TRANSACTION;


void I2CInitialize(void);
void I2CQueueDoWork(void);
void I2CQueueEntryInitialize(I2C_QUEUE_ENTRY* pEntry);
void I2CQueueInitializeTransaction(I2C_TRANSACTION* pTransaction);
void I2CQueueAddTransaction(I2C_QUEUE_ENTRY* pEntry, I2C_TRANSACTION* pTransaction);
void I2CQueueAddEntry(I2C_QUEUE_ENTRY* pEntry);
//=======================================================
#endif /* end __I2C_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/
