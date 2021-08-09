#ifndef MISC_HARDWARE_H
#define MISC_HARDWARE_H

#include "shareddefs.h"


#define OFF 0
#define ON 1
#define TOGGLE 2

#define LAMP_RAIL_8V4					0			// lamp rail select
#define LAMP_RAIL_12V0					1 		// 

#define LAST_4_BITS 0x0000000F		// MASK FOR LAST 4 BITS
#define	MAKE_PRINTABLE 0x30       // 'OR' model select value with this to make printable char '0', etc

typedef enum 
{
	eBoardRev1 = 1,
	eBoardRev2 = 2,
	eBoardRev3 = 3,
	eBoardRev4 = 4,
	eBoardRev5 = 5,
	eBoardRev6 = 6,
	eBoardRev7 = 7
}eBOARD_REVISION;

void hwGPIOConfig(void);

void hwSetSolarCharge(int action);
void readActuatorLimitSwitch(BOOL *pbLimitSwitchA, BOOL *pbLimitSwitchB);
void hwSetLVD(int action);
BOOL hwIsVLPosPolarity(void);
BOOL hwIsVBPosPolarity(void);
void hwLampRailSelect(int rail);
char hwReadModel(void);
int  hwReadSwitches(void);
void hwSetLT4365BatteryEnable (int action) ;
void hwSetLT4363OvpOverride(int action) ;
void hwEnableSystemLedsAndIndicators(void);
void hwDisableSystemLedsAndIndicators(void);
void hwSetSysLED(void);
void hwClrSysLED(void);
BOOL hwGetSolarChargeStatus(void);
BOOL hwGetBatteryEnabledStatus(void);
BOOL hwGetOvpOverrideEnabledStatus(void);
BOOL hwGetLVDStatus(void);
WORD hwReadIndicatorErrors(void);
WORD hwReadDriveErrors(void);
void hwSolarChargeControl(void);
void hwLVDControl(void);
eBOARD_REVISION hwGetBoardRevision(void);
void hwInitPowerControlData(void);
BYTE hwGetSpiUartDataReady(void);

#endif
