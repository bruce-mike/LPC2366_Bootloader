#ifndef ERROR_INPUT_H
#define ERROR_INPUT_H
//************************
//9555/9554 defs
//************************
//************************
#define PCA9555_DRV_ERRS				0x42  	// lamp channel error inputs + write cmd 
#define PCA9555_DRV_ERRS_RD   	0x43		// read cmd
#define PCA9554_MISC_FLT_N_IO		0x40		// indicator/aux/etc errors, actuator switch, vlok, vbok
#define PCA9554_MISC_FLT_N_IORD	0x41		// read that chip

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
#define I2_xERR_COMPS  8
#define I2_xERR_RDR    9
#define I2_xERR_AUX0   10
#define I2_xERR_AUX1   11
#define I2_IN_VOK_VL 	 12
#define I2_IN_VOK_VB   13

#define ERROR_INPUT_SAMPLE_TIME_MS 100

int errorInputGetErrorCount(void);
void errorInputInit(void);
void errorInputDoWork(void);
unsigned short errorInputGetSwitchData(void);
unsigned short errorInputGetDriveErrors(void);
unsigned short errorInputGetAlarms(void);
unsigned short errorInputGetPcuErrors(void);
unsigned short errorInputGetMiscErrors(void);
BOOL errorInputIsVbOK(void);
BOOL errorInputIsVlOK(void);
#endif
