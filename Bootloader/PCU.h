#ifndef PCU_H
#define PCU_H
#include "queue.h"

#ifndef PACKED
#define PACKED __packed
#endif

#define SHOW_PCU_TRAFFIC


/////////////////////////////////////////////////
// packet definitions
/////////////////////////////////////////////////
#define MAX_COMMAND_LENGTH					12

#define NO_TRAFFIC_TIMEOUT 1000
#define NO_TRAFFIC_RETRIES 5

#define PACKET_TYPE_MASK					0x3F
#define RESPONSE_INDICATOR				0x80
#define RETRY_INDICATOR						0x40

#define PCU_WINDOW_SIZE					128
#define SEND_WINDOW_SIZE				20
#define PCU_MAX_RECEIVE_PACKETS 20
//===================================
// PCU Commands RJH: Make into Enums
//===================================
#define PCU_STATUS_COMMAND			0x03
#define PCU_CONTROL_COMMAND     0x02


//////////////////////////////////////////////////
// PCU  Data Definition
//////////////////////////////////////////////////
typedef PACKED struct pcuData
{
	unsigned short pcuLineVoltage;
	unsigned short pcuLineCurrent;
	unsigned short pcuBatteryVoltage;
	unsigned short pcuSystemVoltage;	
	unsigned short pcuSystemCurrent;	
	unsigned short pcuSignTemperature;
	unsigned short pcuSwitchSettings;
	unsigned short pcuForwardPhotoCell;
	unsigned short pcuRearPhotoCell;
	unsigned long  pcuHwIdRadarSpeed;
	unsigned long  pcuSoftwareVersion;
}PCU_DATA;

//////////////////////////////////////////////////
// PCU  Status Definition
//////////////////////////////////////////////////
typedef PACKED struct pcuStatus
{
	// PCU Status Bits (OFF/ON)
	unsigned char pcuRow0Status  								: 1; 
	unsigned char pcuRow1Status  								: 1; 
	unsigned char pcuRow2Status  								: 1;
	unsigned char pcuRow3Status  								: 1;
	unsigned char pcuFan0Status  								: 1;
	unsigned char pcuFan1Status  								: 1;
	unsigned char pcuGPSStatus   								: 1;
	unsigned char pcuModemStatus 								: 1;
	unsigned char pcuDCUStatus 									: 1;
	unsigned char pcuRadarStatus   							: 1;
	unsigned char pcuAux0Status  								: 1;
	unsigned char pcuAux1Status  								: 1;
	unsigned char pcuLEDVlowStatus							: 1;
	unsigned char pcuLEDChgrStatus	            : 1;
	unsigned char pcuLEDSysStatus	              : 1;
	unsigned char pcuLEDAlarmStatus             : 1;
	// PCU Error Bits (1 is OK, 0 BAD!)
	unsigned char pcuRow0Error  								: 1;
	unsigned char pcuRow1Error  								: 1;
	unsigned char pcuRow2Error  								: 1;
	unsigned char pcuRow3Error  								: 1;
	unsigned char pcuFan0Error  								: 1;
	unsigned char pcuFan1Error  								: 1;
	unsigned char pcuGPSError   								: 1;
	unsigned char pcuModemError 								: 1;
	unsigned char pcuDCUError 									: 1;
	unsigned char pcuRadarError   							: 1;
	unsigned char pcuAux0Error  								: 1;
	unsigned char pcuAux1Error  								: 1;
	unsigned char pcuSolarFault  								: 1;
	unsigned char pcuBatteryFault								: 1;
	// PCU Alarms
	unsigned char pcuLowVoltageDisconnectAlarm	: 1; 							
	unsigned char pcuSolarReversedAlarm					: 1;							
	unsigned char pcuBatteryReversedAlarm				: 1;							
	unsigned char pcuLowLineVoltageAlarm      	: 1; 
	unsigned char pcuLowBatteryAlarm						: 1;			
	unsigned char pcuChargerOnAlarm							: 1;				
	unsigned char pcuTemperatureAlarm						: 1;				
	unsigned char pcuPhotoCellErrorAlarm				: 1;
	// PCU Enabled/Disabled
	unsigned char pcuOVDisable									: 1;
	unsigned char pcuSolarEnabled 							: 1;
	unsigned char pcuBatteryEnabled 						: 1;
	unsigned char pcuExtraBits									: 1;
}PCU_STATUS;

//////////////////////////////////////////////////
// PCU  Response Packet Definition
//////////////////////////////////////////////////
typedef PACKED struct pcuResponsePacket
{
	unsigned char nPayLoadLength;
	unsigned char nPcuCommand;
	PCU_STATUS pcuStatus;
	PCU_DATA pcuData;
	unsigned char nCheckSum;
}PCU_RESPONSE_PACKET;

//////////////////////////////////////////////////
// PCU Command Packet Definitions
//////////////////////////////////////////////////
typedef PACKED struct pcuCommandPacket
{
	unsigned char nPayLoadLength;
	unsigned char nPcuCommand[5];
	unsigned char nCheckSum;
}PCU_COMMAND_PACKET;

typedef PACKED struct pcuRadarPacket
{
    unsigned char preDelimiter; 
    unsigned char nPayLoadLength;
	unsigned char nPcuCommand;
    unsigned char radarSpeed;
	unsigned char nCheckSum;
    unsigned char postDelimiter;
}PCU_RADAR_PACKET;    



// control byte bits
#define xPCU_CTRL_MBP			0x01	// enable message board power
#define xPCU_CTRL_CHG_S		0x02	// connect solar panels to battery
#define xPCU_CTRL_CHG_A		0x04	// connect AC charger to battery
#define xPCU_CTRL_SWITCH	0x08	// enable console light switch
#define xPCU_CTRL_LIGHT		0x10	// force console light on
#define xPCU_CTRL_FAN_1		0x20	// enable fan 1
#define xPCU_CTRL_FAN_2		0x40	// enable fan 2
#define xPCU_CTRL_UNUSED	0x80	// reserved

// status byte bits
#define xPCU_STAT_CHG_S		0x01	// solar panels connected
#define xPCU_STAT_CHG_A		0x02	// AC charger connected
#define xPCU_STAT_FAN_1		0x04	// fan 1 power on
#define xPCU_STAT_FAN_2		0x08	// fan 2 power on
#define xPCU_STAT_DOOR 		0x10	// console door open
#define xPCU_STAT_KEY			0x20	// console key in "ON" position
#define xPCU_STAT_MBP			0x40	// message board power on
#define xPCU_STAT_UNUSED	0x80	// reserved

#define xPCU_CTRL_FAN_B                xMBPC_CTRL_FAN_1
#define xPCU_CTRL_BEACON               xMBPC_CTRL_FAN_2
#define xPCU_STAT_FAN_B                xMBPC_STAT_FAN_1
#define xPCU_STAT_BEACON               xMBPC_STAT_FAN_2

int pcuUnstuffRxPacket(unsigned char *pStuffedRxPacket, unsigned char *pUnstuffedRxPacket, int *nStuffedRxPacketLength);
void pcuProcessor(eINTERFACE eInterface, unsigned char* pPacket, int nPacketLength);
void pcuSendResponsePacket(eINTERFACE eInterface, PCU_RESPONSE_PACKET* pPcuPacket);
BOOL pcuReceiveCommandPacket(PCU_COMMAND_PACKET* pReceivedPacket, eINTERFACE* peInterface);
void pcuBuildResponsePacket(PCU_RESPONSE_PACKET* pPCUResponsePacket, unsigned char nPcuCommand);

#endif		// PCU_H
