#ifndef PCU_STATUS_BOARD_H
#define PCU_STATUS_BOARD_H
//************************
//************************
//************************
// PCA9531 defs
//************************
//************************
// PCA9531 BASE ADDRESS
#define PCA9531_STATUS_BOARD_CONTROL 0xC0

// PCA9531 REGISTER DEFS
#define PCA9531_INPUT				0x00		
#define PCA9531_PSC0		    0x01		
#define PCA9531_PWM0				0x02	
#define PCA9531_PSC1				0x03
#define PCA9531_PWM1				0x04
#define PCA9531_LEDSEL0			0x15 // Auto Increment Bit Set/ Select LED0-LED3 Outputs
#define PCA9531_LEDSEL1		  0x06 // Select LED4-LED7 Outputs 
#define PCA9531_AUTO_INC	  0x10

#define PCU_STATUS_BOARD_SAMPLE_TIME_MS 10

typedef enum ePcuStatusBoardControl
{
	ePCU_STATUS_BOARD_OUTPUT_OFF 				= 0x00,
	ePCU_STATUS_BOARD_OUTPUT_ON					= 0x01,
}ePCU_STATUS_BOARD_CONTROL;

typedef enum ePcuStatusBoardSetStatus
{
	ePCU_STATUS_BOARD_SET_SCHEDULE_STATUS = 0,	
	ePCU_STATUS_BOARD_SET_RADAR_STATUS,	
	ePCU_STATUS_BOARD_SET_CHARGER_STATUS,
}ePCU_STATUS_BOARD_SET_STATUS;

typedef enum ePcuStatusBoardGetStatus
{
	ePCU_STATUS_BOARD_GET_SCHEDULE_STATUS = 0,	
	ePCU_STATUS_BOARD_GET_RADAR_STATUS,	
	ePCU_STATUS_BOARD_GET_CHARGER_STATUS,
}ePCU_STATUS_BOARD_GET_STATUS;


void pcuStatusBoardInit(void);
void pcuStatusBoardDoWork(void);
void pcuStatusBoardSetStatus(ePCU_STATUS_BOARD_CONTROL ePcuStatusBoardControl, ePCU_STATUS_BOARD_SET_STATUS ePcuStatusBoardSetStatus);
unsigned char pcuStatusBoardGetStatus(ePCU_STATUS_BOARD_GET_STATUS ePcuStatusBoardGetStatus);
void pcuStatusBoardToggleRadarLED(void);

#endif // PCU_STATUS_BOARD_H

