///////////////////////////////////////////////////////
// stored configuration data
///////////////////////////////////////////////////////
#ifndef STOREDCONFIG_H
#define STOREDCONFIG_H

#define MAX_TP_CALIBRATION_COEFFICIENTS 7

typedef PACKED struct storedInfo
{
	unsigned int nStructFlags;
	unsigned int nSendersESN;
	unsigned int nDisallowedPatterns;
	eACTUATOR_TYPES eActuatorType;
	eACTUATOR_BUTTON_MODE eActuatorButtonMode;
	int nActuatorUpLimit;
	int nActuatorDownLimit;
	eBRIGHTNESS_CONTROL eBrightnessControl;
	unsigned int nDefaultUI;
	eAUX_BATTERY_TYPES eAuxBatteryType;
	TP_MATRIX tpCalCoefficients;
	unsigned long nPCUSerialNumber;
}STORED_INFO;

//===================================
// initialization
//===================================
STORED_INFO* storedConfigInit(ePLATFORM_TYPE ePlatformType);

//===================================
// get routines
//===================================
unsigned int storedConfigGetSendersESN(void);
unsigned int storedConfigGetDisallowedPatterns(void);
eACTUATOR_TYPES storedConfigGetActuatorType(void);
eBRIGHTNESS_CONTROL storedConfigGetArrowBoardBrightnessControl(void);
unsigned int storedConfigGetDefaultUI(void);
eAUX_BATTERY_TYPES storedConfigGetAuxBatteryType(void);
eACTUATOR_BUTTON_MODE storedConfigGetActuatorButtonMode(void);
int storedConfigGetActuatorUpLimit(void);
int storedConfigGetActuatorDownLimit(void);
void storedConfigGetTPCalCoefficients(TP_MATRIX *pTPCalCoefficients);
eACTUATOR_BUTTON_MODE storedConfigGetActuatorButtonMode(void);
unsigned long storedConfigGetPCUSerialNumber(void);

//===================================
// set routines
//===================================
void storedConfigWriteNewInfo(STORED_INFO *pNewStoredInfo);
void storedConfigSetSendersESN(unsigned int nESN);
void storedConfigSetDisallowedPatterns(unsigned int nPatterns);
void storedConfigSetActuatorType(eACTUATOR_TYPES eActuatorType);
void storedConfigSetArrowBoardBrightnessControl(eBRIGHTNESS_CONTROL eBrightnessControl);
void storedConfigSetDefaultUI(unsigned int nDefaultUI);
void storedConfigSetAuxBatteryType(eAUX_BATTERY_TYPES eAuxBatteryType);
void storedConfigSetActuatorButtonMode(eACTUATOR_BUTTON_MODE eActuatorButtonMode);
void storedConfigSetActuatorUpLimit(int nActuatorUpLimit);
void storedConfigSetActuatorDownLimit(int nActuatorDownLimit);
void storedConfigSetTPCalCoefficients(TP_MATRIX *pTPCalCoefficients);
void storedConfigSetPCUSerialNumber(unsigned long nPCUSerialNumber);

#endif		// STOREDCONFIG_H
