#ifndef PCU_ERRORS_H
#define PCU_ERRORS_H
//************************
//9555 defs
//************************
//************************
#define PCA9555_PCU_ERRS_CONFIG	0x42  	// lamp channel error inputs + write cmd 
#define PCA9555_PCU_ERRS_RD   	0x43		// read cmd

// CONTROL BYTES
#define INP0		0x00		// input port 0
#define INP1		0x01		//       port 1
#define OUTP0		0x02		// output port 0
#define OUTP1		0x03		//        port 1
#define PINV0		0x04		// polarity invert port 0
#define PINV1		0x05		//                 port 1
#define CONP0		0x06		// config port 0
#define CONP1		0x07		//        port 1

// configuration bytes
#define ALL_OUTPUT	0x00
#define ALL_INPUT   0xFF

// ****************
// ****************
// ****************
// PCA9555 CHIP 'B' MISC ERRORS AND INPUTS DEFS
// INDICATOR, BEACON, USER, ACTUATOR, ETC
// ****************
// ****************
// ****************
#define I2_xERR_ROW0 	 0
#define I2_xERR_ROW1 	 1
#define I2_xERR_ROW2 	 2
#define I2_xERR_ROW3 	 3
#define I2_xERR_FAN0 	 4
#define I2_xERR_FAN1 	 5
#define I2_xERR_GPS  	 6
#define I2_xERR_MODEM  7
#define I2_xERR_DCU  	 8
#define I2_xERR_RADAR  9
#define I2_xERR_AUX0   10
#define I2_xERR_AUX1   11
#define I2_IN_VOK_VL 	 12
#define I2_IN_VOK_VB   13

#define PCU_ERRORS_SAMPLE_TIME_MS 100

typedef enum ePcuAlarms
{
	ePCU_VLINE_ALARM 			= 0,
	ePCU_VBATT_ALARM 			= 1,
	ePCU_RVLV_ALARM  			= 2,
	ePCU_RVBV_ALARM	 			= 3,
	ePCU_TEMP_ALARM  			= 4,
	ePCU_LVD_ALARM   			= 5,
	ePCU_CHARGER_ALARM 		= 6,
	ePCU_PHOTOCELL_ALARM 	= 7,
}ePCU_ALARMS;

typedef enum ePcuErrors
{
	ePCU_ROW0_ERROR  	= 0,
	ePCU_ROW1_ERROR  	= 1,
	ePCU_ROW2_ERROR  	= 2,
	ePCU_ROW3_ERROR  	= 3,
	ePCU_FAN0_ERROR  	= 4,
	ePCU_FAN1_ERROR  	= 5,
	ePCU_GPS_ERROR   	= 6,
	ePCU_MODEM_ERROR 	= 7,
	ePCU_DCU_ERROR 		= 8,
	ePCU_RADAR_ERROR  = 9,
	ePCU_AUX0_ERROR  	= 10,
	ePCU_AUX1_ERROR  	= 11,
	ePCU_VOK_VL_ERROR	= 12,
	ePCU_VOK_VB_ERROR	= 13,	
}ePCU_ERRORS;

void pcuErrorsInit(void);
void pcuErrorsDoWork(void);
unsigned short pcuErrorsGetSwitchData(void);
BOOL pcuErrorsGetAlarm(ePCU_ALARMS ePcuAlarm);
BOOL pcuErrorsGetError(ePCU_ERRORS ePcuError);
#endif //PCU_ERRORS_H
