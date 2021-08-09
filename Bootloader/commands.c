#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "timer.h"
#include "hdlc.h"
#include "pwm.h"
#include "commands.h"
#include "mischardware.h"
#include "display.h"
#include "actuator.h"
#include "lm73.h"
#include "adc.h"
#include "errorinput.h"
#include "leddriver.h"
#include "flash_nvol.h"
#include "spiFlash.h"
#include "storedconfig.h"
#include "watchdog.h"


void commandDoWork()
{
	HDLCPACKET hdlcPacket;
	eINTERFACE eInterface;
	if(hdlcReceivePacket(&hdlcPacket, &eInterface))
	{
		printf("commandProcessor: eInterface[%d]\n",eInterface);
		commandProcessor(eInterface, &hdlcPacket);
	}
}
void commandInit()
{
}
void commandProcessor(eINTERFACE eInterface, HDLCPACKET *pHDLCPacket)
{
	int nCommandEnum;
	
	COMMAND* pCommand = &pHDLCPacket->command;

	do
	{			
				/////
				// so process the command
				/////
				nCommandEnum = pCommand->nPacketType;
				pCommand->nPacketID_Status &= ~PACKET_STATUS_MASK;
				switch(nCommandEnum)
				{
					case eCOMMAND_GET_MODEL_TYPE:
						{
							pCommand->nData = getModelNumber();
							pCommand->nPacketID_Status |= ePACKET_STATUS_SUCCESS;
							hdlcSendResponse(eInterface, pHDLCPacket);
						}
						break;
					case eCOMMAND_STATUS_ALARMS:
            {
							pCommand->nData = errorInputGetAlarms();
							pCommand->nPacketID_Status |= ePACKET_STATUS_SUCCESS;
							hdlcSendResponse(eInterface, pHDLCPacket);
						}
						break;
					case eCOMMAND_STATUS_LINE_VOLTAGE:
						{
							pCommand->nData = ADCGetLineVoltage();
							pCommand->nPacketID_Status |= ePACKET_STATUS_SUCCESS;
							hdlcSendResponse(eInterface, pHDLCPacket);
						}
						break;
					case eCOMMAND_STATUS_BATTERY_VOLTAGE:
						{
							pCommand->nData = ADCGetBatteryVoltage();
							pCommand->nPacketID_Status |= ePACKET_STATUS_SUCCESS;
							hdlcSendResponse(eInterface, pHDLCPacket);
						}
						break;
					case eCOMMAND_STATUS_AUX:
						{
							pCommand->nData = ledDriverGetAuxStatus();
							pCommand->nPacketID_Status |= ePACKET_STATUS_SUCCESS;
							hdlcSendResponse(eInterface, pHDLCPacket);
						}
						break;
					case eCOMMAND_STATUS_AUX_ERRORS:
						{
							pCommand->nData = errorInputGetAuxErrors();
							pCommand->nPacketID_Status |= ePACKET_STATUS_SUCCESS;
							hdlcSendResponse(eInterface, pHDLCPacket);
						}
						break;
					case eCOMMAND_STATUS_SWITCHES:
						{
							pCommand->nData = errorInputGetSwitchData();
							pCommand->nPacketID_Status |= ePACKET_STATUS_SUCCESS;
							hdlcSendResponse(eInterface, pHDLCPacket);
						}
						break;
					case eCOMMAND_STATUS_TEMPERATURE:
						{
							pCommand->nData = lm73GetDegreesC();
							pCommand->nPacketID_Status |= ePACKET_STATUS_SUCCESS;
							hdlcSendResponse(eInterface, pHDLCPacket);
						}
						break;
					case eCOMMAND_DO_RESET:
						watchdogReboot();
						break;
					default:
						break;
				}

	}while(0);
}
