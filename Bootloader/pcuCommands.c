#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "timer.h"
#include "PCU.h"
#include "pwm.h"
#include "pcuCommands.h"
#include "mischardware.h"
#include "display.h"
#include "actuator.h"
#include "lm73.h"
#include "adc.h"
#include "pcuErrors.h"
#include "pcuPowerControl.h"
#include "pcuStatusBoard.h"
#include "flash_nvol.h"
#include "spiFlash.h"
#include "watchdog.h"
#include "serial.h"


void setSolarManualMode(BOOL bMode);
void resetDCU(void);
void pcuSendRadarData(eINTERFACE eInterface);

void pcuCommandProcessor(eINTERFACE eInterface, PCU_COMMAND_PACKET *pPCUCommandPacket)
{
	PCU_RESPONSE_PACKET pcuResponsePacket;
	
  union
  {
      DWORD dPcuCommandData;
      BYTE bPcuCommandData[4];
  }uPCU_COMMAND_DATA;
	
	uPCU_COMMAND_DATA.bPcuCommandData[0] = pPCUCommandPacket->nPcuCommand[1];
	uPCU_COMMAND_DATA.bPcuCommandData[1] = pPCUCommandPacket->nPcuCommand[2];
	uPCU_COMMAND_DATA.bPcuCommandData[2] = pPCUCommandPacket->nPcuCommand[3];
	uPCU_COMMAND_DATA.bPcuCommandData[3] = pPCUCommandPacket->nPcuCommand[4];
	
  
	do
	{			
			/////
			// so process the command
			/////
			switch(pPCUCommandPacket->nPcuCommand[0])
			{
				  //=============================
					// Set Output Commands (OFF/ON)
					//=============================
					case ePCU_COMMAND_SET_ROW0_ON: 
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_ROW0_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_ROW1_ON:     	 			
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_ROW1_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_ROW2_ON:     	 			
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_ROW2_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_ROW3_ON:     	 			
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_ROW3_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_FAN0_ON:                                               
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_FAN0_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_FAN1_ON:                                                                        
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_FAN1_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_GPS_ON:
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_GPS_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_MODEM_ON: 
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_MODEM_OUTPUT);	
						break;						
					case ePCU_COMMAND_RESET_DCU: 
						resetDCU();	
						break;						
					case ePCU_COMMAND_SET_RADAR_ON: 
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_RADAR_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_AUX0_ON: 
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_AUX0_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_AUX1_ON:
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_AUX1_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_SOLAR_ON:
						hwSetSolarCharge(ON);	
						break;						
					case ePCU_COMMAND_SET_BATTERY_ON:
						hwSetLT4365BatteryEnable(ON);
						break;						
					case ePCU_COMMAND_SET_OV_ON:       	 			
						hwSetLT4363OvpOverride(ON);	
						break;						
					case ePCU_COMMAND_SET_ROW0_OFF:    	 			
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_ROW0_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_ROW1_OFF:    	 			
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_ROW1_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_ROW2_OFF:    	 			
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_ROW2_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_ROW3_OFF:    	 			
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_ROW3_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_FAN0_OFF:                       
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_FAN0_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_FAN1_OFF:                                    
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_FAN1_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_GPS_OFF: 
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_GPS_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_MODEM_OFF: 
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_MODEM_OUTPUT);	
						break;						
					case ePCU_COMMAND_NOOP_1: 
						break;						
					case ePCU_COMMAND_SET_RADAR_OFF: 
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_RADAR_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_AUX0_OFF:
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_AUX0_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_AUX1_OFF:
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_AUX1_OUTPUT);	
						break;						
					case ePCU_COMMAND_SET_SOLAR_OFF:
						hwSetSolarCharge(OFF);	
						break;						
					case ePCU_COMMAND_SET_BATTERY_OFF: 	 			
						hwSetLT4365BatteryEnable(OFF);
						break;						
					case ePCU_COMMAND_SET_OV_OFF:      	 			
						hwSetLT4363OvpOverride(OFF);	
						break;					
					//====================================
					// Get Status Commands (OFF/ON)
					//====================================
					case ePCU_COMMAND_GET_STATUS: 
						break;					
					case ePCU_COMMAND_NOOP_2:
						break;
					//=======================================
					// Set Commands (Used for Debug, etc...)
					//=======================================
					case ePCU_COMMAND_SET_RADAR_LED_TOGGLE:
						pcuStatusBoardToggleRadarLED();						
						break;
					case ePCU_COMMAND_SET_CHRGR_OFF_VOLTAGE:
						hwSetPowerControlData(ePCU_COMMAND_SET_CHRGR_OFF_VOLTAGE,uPCU_COMMAND_DATA.dPcuCommandData);
						break;
					case ePCU_COMMAND_SET_CHRGR_ON_VOLTAGE:
						hwSetPowerControlData(ePCU_COMMAND_SET_CHRGR_ON_VOLTAGE,uPCU_COMMAND_DATA.dPcuCommandData);
						break;
					case ePCU_COMMAND_SET_PWR_GOOD_VOLTAGE:
						hwSetPowerControlData(ePCU_COMMAND_SET_PWR_GOOD_VOLTAGE,uPCU_COMMAND_DATA.dPcuCommandData);
						break;
					case ePCU_COMMAND_SET_PWR_HIGH_VOLTAGE:
						hwSetPowerControlData(ePCU_COMMAND_SET_PWR_HIGH_VOLTAGE,uPCU_COMMAND_DATA.dPcuCommandData);
						break;
					case ePCU_COMMAND_SET_PWR_WARN_VOLTAGE:
						hwSetPowerControlData(ePCU_COMMAND_SET_PWR_WARN_VOLTAGE, uPCU_COMMAND_DATA.dPcuCommandData);
						break;
					case ePCU_COMMAND_SET_LVD_VOLTAGE: 
						hwSetPowerControlData(ePCU_COMMAND_SET_LVD_VOLTAGE, uPCU_COMMAND_DATA.dPcuCommandData);
						break;
					case ePCU_COMMAND_SET_PCU_SYS_LED_ON:
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_LED_SYST_OUTPUT);
						break;
					case ePCU_COMMAND_SET_PCU_CHARGER_LED_ON:							
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_LED_CHRGR_OUTPUT);
						break;
					case ePCU_COMMAND_SET_PCU_ALARM_LED_ON:								
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_LED_ALARM_OUTPUT);
						break;
					case ePCU_COMMAND_SET_PCU_LOW_BATT_LED_ON:							
						pcuPowerControlSetOutput(ePOWER_OUTPUT_ON, ePCU_SET_LED_VLOW_OUTPUT);
						break;
					case ePCU_COMMAND_SET_STATUS_RADAR_LED_ON:
						pcuStatusBoardSetStatus(ePCU_STATUS_BOARD_OUTPUT_ON, ePCU_STATUS_BOARD_SET_RADAR_STATUS);
						break;
					case ePCU_COMMAND_SET_STATUS_CHARGER_LED_ON:						
						pcuStatusBoardSetStatus(ePCU_STATUS_BOARD_OUTPUT_ON, ePCU_STATUS_BOARD_SET_CHARGER_STATUS);
						break;
					case ePCU_COMMAND_SET_STATUS_SCHEDULE_LED_ON:					
						pcuStatusBoardSetStatus(ePCU_STATUS_BOARD_OUTPUT_ON, ePCU_STATUS_BOARD_SET_SCHEDULE_STATUS);
						break;
					case ePCU_COMMAND_SET_PCU_SYS_LED_OFF:
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_LED_SYST_OUTPUT);
						break;
					case ePCU_COMMAND_SET_PCU_CHARGER_LED_OFF:							
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_LED_CHRGR_OUTPUT);
						break;
					case ePCU_COMMAND_SET_PCU_ALARM_LED_OFF:								
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_LED_ALARM_OUTPUT);
						break;
					case ePCU_COMMAND_SET_PCU_LOW_BATT_LED_OFF:							
						pcuPowerControlSetOutput(ePOWER_OUTPUT_OFF, ePCU_SET_LED_VLOW_OUTPUT);
						break;
					case ePCU_COMMAND_SET_STATUS_RADAR_LED_OFF:							
						pcuStatusBoardSetStatus(ePCU_STATUS_BOARD_OUTPUT_OFF, ePCU_STATUS_BOARD_SET_RADAR_STATUS);
						break;
					case ePCU_COMMAND_SET_STATUS_CHARGER_LED_OFF:						
						pcuStatusBoardSetStatus(ePCU_STATUS_BOARD_OUTPUT_OFF, ePCU_STATUS_BOARD_SET_CHARGER_STATUS);
						break;
					case ePCU_COMMAND_SET_STATUS_SCHEDULE_LED_OFF:					
						pcuStatusBoardSetStatus(ePCU_STATUS_BOARD_OUTPUT_OFF, ePCU_STATUS_BOARD_SET_SCHEDULE_STATUS);
						break;
					case ePCU_COMMAND_SET_PCU_SERIAL_NUMBER:
						//storedConfigSetPCUSerialNumber(uPCU_COMMAND_DATA.dPcuCommandData);
						break;
					case ePCU_COMMAND_SET_SOLAR_MANUAL_MODE:
						if(0 == uPCU_COMMAND_DATA.dPcuCommandData)
						{
							setSolarManualMode(FALSE);
						}
						else
						{
							setSolarManualMode(TRUE);
						}
						break;
                    case ePCU_COMMAND_GET_RADAR_SPEED:
                        pcuSendRadarData(eInterface);
                        break;
                    case ePCU_COMMAND_SET_UPGRADE_MODE:
                        serialSetUpgradeMode(TRUE);
printf("ePCU_COMMAND_SET_UPGRADE_MODE\r\n");
                        break;
					default:
                        return;					
			}	
		   
            if(ePCU_COMMAND_GET_RADAR_SPEED == pPCUCommandPacket->nPcuCommand[0])
            {
                pcuSendRadarData(eInterface);               
            }
            else 
            {
                pcuBuildResponsePacket(&pcuResponsePacket,pPCUCommandPacket->nPcuCommand[0]);
                pcuSendResponsePacket(eInterface, &pcuResponsePacket);
            }

	}while(0);
}

void pcuSendRadarData(eINTERFACE eInterface)
{
	PCU_RADAR_PACKET radarPacket;
    
    
    // set up delimters - add extra on at beginning to gurantee sync w/ DCU
    radarPacket.preDelimiter = PACKET_DELIM;
    radarPacket.postDelimiter = PACKET_DELIM;
    
	radarPacket.nPayLoadLength = sizeof(radarPacket) - 2; // subtract delimiters from length

    radarPacket.nPcuCommand = ePCU_COMMAND_GET_RADAR_SPEED;  
    radarPacket.radarSpeed = radarGetMPH();    

    // add checksum
    // Payload Length Minus 2 BYTES, payload length byte(calculated above) and checksum 
    // byte. XOR 1 Command BYTE and 5 Data BYTES    
    radarPacket.nCheckSum = 0;
	radarPacket.nCheckSum ^= radarPacket.nPayLoadLength;
    radarPacket.nCheckSum ^= radarPacket.nPcuCommand;
    radarPacket.nCheckSum ^= radarPacket.radarSpeed;

#ifdef CONFIG_DEBUG_RADAR
for(i=0; i<sizeof(radarPacket); i++)
  printf("[%02x]", bytePtr[i]);
printf("%d\r\n", counter++);
#endif
    
    serialWrite(eInterface, (BYTE *)&radarPacket, sizeof(radarPacket));
}    

