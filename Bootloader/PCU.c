#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "timer.h"
#include "serial.h"
#include "adc.h"
#include "ADS7828.h"
#include "lm73.h"
#include "PCU.h"
#include "pcuCommands.h"
#include "pcuPowerControl.h"
#include "pcuErrors.h"
#include "pwm.h"
#include "mischardware.h"
#include "softwareVersion.h"



BYTE bUnstuffedPacket[20];

static BOOL pcuRxPacketVerifyCheckSum(BYTE *pMessage, int *nBytes)
{
	BYTE bCheckSum = 0;
	int nInIndex;
	BOOL bCheckSumMatch = FALSE;

	for(nInIndex=0; nInIndex < *nBytes; nInIndex++)
	{
		// Check for stuffed bytes
		bCheckSum ^= pMessage[nInIndex];
	}
	
	if(bCheckSum == 0x00)
	{
		bCheckSumMatch = TRUE;
	}
	else
	{
		bCheckSum = 0;

		for(nInIndex=0; nInIndex < *nBytes; nInIndex++)
		{
			// Check for stuffed bytes
			bCheckSum ^= pMessage[nInIndex];
			printf("badCheckSum[%X] pMessage[%d][%X]\r\n",bCheckSum,nInIndex,pMessage[nInIndex]);		
		}
		printf("\r\n");
	}
	
	return bCheckSumMatch;
}

static void pcuTxPacketCalculateCheckSum(BYTE *pTxPacket, int nNumBytes)
{
	BYTE bCheckSum = 0;
	int nInIndex;
	
	for(nInIndex=0; nInIndex < nNumBytes; nInIndex++)
	{
		bCheckSum ^= pTxPacket[nInIndex];		
	}
	
	pTxPacket[nInIndex] = bCheckSum;
}

static void pcuTxPacket(eINTERFACE eInterface, PCU_RESPONSE_PACKET *pPCUResponsePacket)
{
	int nInIndex;
	int nOutIndex = 0;
	BYTE *pDataIn = (unsigned char*)pPCUResponsePacket;
	BYTE bOutGoingPacket[(sizeof(PCU_RESPONSE_PACKET)*2)];
	
	/////
	// Calculate Checksum before Byte Stuffing
	/////	
	pcuTxPacketCalculateCheckSum(pDataIn, sizeof(PCU_RESPONSE_PACKET)-1);
    
	
	/////
	// escape all packet delimeters
	/////
	for(nInIndex = 0; nInIndex < sizeof(PCU_RESPONSE_PACKET); nInIndex++)
	{
		if(PACKET_DELIM == pDataIn[nInIndex])
		{
			bOutGoingPacket[nOutIndex++] = PACKET_ESCAPE;
			bOutGoingPacket[nOutIndex++] = PACKET_ESCAPE_DELIM;
		}
		else if(PACKET_ESCAPE == pDataIn[nInIndex])
		{
			bOutGoingPacket[nOutIndex++] = PACKET_ESCAPE;
			bOutGoingPacket[nOutIndex++] = PACKET_ESCAPE_ESCAPE;		
		}
		else
		{
			bOutGoingPacket[nOutIndex++] = pDataIn[nInIndex];
		}
	}
	
	serialWrite(eInterface, bOutGoingPacket, nOutIndex);
}

void pcuSendResponsePacket(eINTERFACE eInterface, PCU_RESPONSE_PACKET* pPCUResponsePacket)
{
	 pcuTxPacket(eInterface, pPCUResponsePacket);
}

void pcuProcessor(eINTERFACE eInterface, BYTE *pRxPacket, int nPacketLength)
{
	PCU_COMMAND_PACKET pcuCommandPacket;
	
	if(pcuRxPacketVerifyCheckSum(pRxPacket, &nPacketLength))
	{
			do
			{
				memcpy((BYTE*)&pcuCommandPacket, pRxPacket, nPacketLength);
				
							
			}while(0);
	}
}

int pcuUnstuffRxPacket(unsigned char *pStuffedRxPacket, unsigned char *pUnstuffedRxPacket, int *nStuffedRxPacketLength)
{
	BOOL bRxPacketUnstuffed = FALSE;
	int nStuffedRxIndex,nUnstuffedRxIndex = 0;
	int nNumberStuffedBytes = 0;
	
	for(nStuffedRxIndex=0; nStuffedRxIndex < *nStuffedRxPacketLength; nStuffedRxIndex++)
	{
		// Check for stuffed bytes
		if(pStuffedRxPacket[nStuffedRxIndex] == PACKET_ESCAPE) // 7D
		{
			nStuffedRxIndex++;
			
			if(pStuffedRxPacket[nStuffedRxIndex] == PACKET_ESCAPE_ESCAPE) // 5D
			{				
				nNumberStuffedBytes++;
				
				pUnstuffedRxPacket[nUnstuffedRxIndex] = 0x7D;
				
				nUnstuffedRxIndex++;
				
				bRxPacketUnstuffed = TRUE;
				
				continue;
			}
			else if(pStuffedRxPacket[nStuffedRxIndex] == PACKET_ESCAPE_DELIM) // 5E
			{
				nNumberStuffedBytes++;
				
				pUnstuffedRxPacket[nUnstuffedRxIndex] = 0x7E;
				
				nUnstuffedRxIndex++;
				
				bRxPacketUnstuffed = TRUE;
				
				continue;
			}
		}
		else
		{
			pUnstuffedRxPacket[nUnstuffedRxIndex] = pStuffedRxPacket[nStuffedRxIndex];	
			
			nUnstuffedRxIndex++;
		}			
	}
		
	*nStuffedRxPacketLength -= nNumberStuffedBytes;
	
	/////
	// DEH 03/01/17
	// this is a Kludge,
	// a misunderstanding of payload length
	// the DCU calculated the checksum with the original length
	// then stuck the length of the stuffed packet here.
	// see H_MBPC_1.c, MBPC_Pckg_Xmit()
	//
	// now, we have to stick the packet length back here again.
	//
	// to make this backward compatable, we have to leave these as is
	/////
	pUnstuffedRxPacket[0] = *nStuffedRxPacketLength;
	
	return bRxPacketUnstuffed;
}

//=======================================================================================
// Public interface
//=======================================================================================
void pcuBuildResponsePacket(PCU_RESPONSE_PACKET* pPCUResponsePacket,unsigned char nPcuCommand)
{
	pPCUResponsePacket->nPayLoadLength = sizeof(PCU_RESPONSE_PACKET);
	pPCUResponsePacket->nPcuCommand = nPcuCommand;
	
	pPCUResponsePacket->pcuStatus.pcuRow0Status 		= pcuPowerControlGetStatus(ePCU_GET_ROW0_STATUS);
	pPCUResponsePacket->pcuStatus.pcuRow1Status 		= pcuPowerControlGetStatus(ePCU_GET_ROW1_STATUS);
	pPCUResponsePacket->pcuStatus.pcuRow2Status 		= pcuPowerControlGetStatus(ePCU_GET_ROW2_STATUS);
	pPCUResponsePacket->pcuStatus.pcuRow3Status 		= pcuPowerControlGetStatus(ePCU_GET_ROW3_STATUS);
	pPCUResponsePacket->pcuStatus.pcuFan0Status 		= pcuPowerControlGetStatus(ePCU_GET_FAN0_STATUS);
	pPCUResponsePacket->pcuStatus.pcuFan1Status 		= pcuPowerControlGetStatus(ePCU_GET_FAN1_STATUS);
	pPCUResponsePacket->pcuStatus.pcuGPSStatus  		= pcuPowerControlGetStatus(ePCU_GET_GPS_STATUS);
	pPCUResponsePacket->pcuStatus.pcuModemStatus		= pcuPowerControlGetStatus(ePCU_GET_MODEM_STATUS);
	                                                  
	pPCUResponsePacket->pcuStatus.pcuDCUStatus			= pcuPowerControlGetStatus(ePCU_GET_DCU_STATUS);
	pPCUResponsePacket->pcuStatus.pcuRadarStatus  	= pcuPowerControlGetStatus(ePCU_GET_RADAR_STATUS);
	pPCUResponsePacket->pcuStatus.pcuAux0Status 		= pcuPowerControlGetStatus(ePCU_GET_AUX0_STATUS);
	pPCUResponsePacket->pcuStatus.pcuAux1Status 		= pcuPowerControlGetStatus(ePCU_GET_AUX1_STATUS);
	pPCUResponsePacket->pcuStatus.pcuLEDVlowStatus	=	pcuPowerControlGetStatus(ePCU_GET_LED_VLOW_STATUS);
	pPCUResponsePacket->pcuStatus.pcuLEDChgrStatus	=	pcuPowerControlGetStatus(ePCU_GET_LED_CHRGR_STATUS);
	pPCUResponsePacket->pcuStatus.pcuLEDSysStatus	 	=	pcuPowerControlGetStatus(ePCU_GET_LED_SYST_STATUS);
	pPCUResponsePacket->pcuStatus.pcuLEDAlarmStatus	=	pcuPowerControlGetStatus(ePCU_GET_LED_ALARM_STATUS);
	
	pPCUResponsePacket->pcuStatus.pcuRow0Error 		= 0x00; 		
	pPCUResponsePacket->pcuStatus.pcuRow1Error 		= 0x00; 		
	pPCUResponsePacket->pcuStatus.pcuRow2Error 		= 0x00; 		
	pPCUResponsePacket->pcuStatus.pcuRow3Error 		= 0x00; 		
	pPCUResponsePacket->pcuStatus.pcuFan0Error 		= 0x00; 		
	pPCUResponsePacket->pcuStatus.pcuFan1Error 		= 0x00; 		
	pPCUResponsePacket->pcuStatus.pcuGPSError  		= 0x00; 	
	pPCUResponsePacket->pcuStatus.pcuModemError		= 0x00; 	
	pPCUResponsePacket->pcuStatus.pcuDCUError			= 0x00;
	pPCUResponsePacket->pcuStatus.pcuRadarError  	= 0x00;	
	pPCUResponsePacket->pcuStatus.pcuAux0Error 	 	= 0x00;
	pPCUResponsePacket->pcuStatus.pcuAux1Error   	= 0x00;
	pPCUResponsePacket->pcuStatus.pcuSolarFault		= 0x00;  
	pPCUResponsePacket->pcuStatus.pcuBatteryFault = 0x00;

	pPCUResponsePacket->pcuStatus.pcuLowVoltageDisconnectAlarm = pcuErrorsGetAlarm(ePCU_LVD_ALARM);
	pPCUResponsePacket->pcuStatus.pcuSolarReversedAlarm				 = pcuErrorsGetAlarm(ePCU_RVLV_ALARM);
	pPCUResponsePacket->pcuStatus.pcuBatteryReversedAlarm			 = pcuErrorsGetAlarm(ePCU_RVBV_ALARM);
	pPCUResponsePacket->pcuStatus.pcuLowLineVoltageAlarm       = pcuErrorsGetAlarm(ePCU_VLINE_ALARM);
	pPCUResponsePacket->pcuStatus.pcuLowBatteryAlarm					 = pcuErrorsGetAlarm(ePCU_VBATT_ALARM);
	pPCUResponsePacket->pcuStatus.pcuChargerOnAlarm						 = pcuErrorsGetAlarm(ePCU_CHARGER_ALARM);
	pPCUResponsePacket->pcuStatus.pcuTemperatureAlarm			 		 = pcuErrorsGetAlarm(ePCU_TEMP_ALARM);
	pPCUResponsePacket->pcuStatus.pcuPhotoCellErrorAlarm			 = pcuErrorsGetAlarm(ePCU_PHOTOCELL_ALARM);

	pPCUResponsePacket->pcuStatus.pcuOVDisable			 = hwGetOvpOverrideEnabledStatus();
	pPCUResponsePacket->pcuStatus.pcuSolarEnabled 	 = hwGetSolarChargeStatus();
	pPCUResponsePacket->pcuStatus.pcuBatteryEnabled  = hwGetBatteryEnabledStatus();
	pPCUResponsePacket->pcuStatus.pcuExtraBits			 = 0x00;
																									
	pPCUResponsePacket->pcuData.pcuLineVoltage			 = ADCGetLineVoltage();

	pPCUResponsePacket->pcuData.pcuBatteryVoltage    = ADCGetBatteryVoltage();
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		case eBoardRev2:
		default:
			pPCUResponsePacket->pcuData.pcuSystemVoltage     = ADCGetSystemVoltage();
			break;

		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			pPCUResponsePacket->pcuData.pcuSystemVoltage     = ADCGetBatteryVoltage();
			break;
	}
	
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		case eBoardRev2:
		default:
			pPCUResponsePacket->pcuData.pcuLineCurrent			 = nADS7828GetIL();
			pPCUResponsePacket->pcuData.pcuSystemCurrent     = nADS7828GetIS();
			break;

		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			pPCUResponsePacket->pcuData.pcuLineCurrent			 = ADCGetLineCurrent();
			pPCUResponsePacket->pcuData.pcuSystemCurrent     = ADCGetSignCurrent();
			break;
	}

	pPCUResponsePacket->pcuData.pcuSignTemperature	    = lm73GetTempReading(DEG_F);
	pPCUResponsePacket->pcuData.pcuSwitchSettings	    = pcuErrorsGetSwitchData();
	pPCUResponsePacket->pcuData.pcuForwardPhotoCell     = pwmGetPhotoCellDataAvgCounts(ePHOTOCELL_FORWARD_DATA);
	pPCUResponsePacket->pcuData.pcuRearPhotoCell        = pwmGetPhotoCellDataAvgCounts(ePHOTOCELL_REAR_DATA);
    pPCUResponsePacket->pcuData.pcuHwIdRadarSpeed  =  (pPCUResponsePacket->pcuData.pcuHwIdRadarSpeed << 24) | hwGetBoardRevision();
	pPCUResponsePacket->pcuData.pcuSoftwareVersion 	 = SOFTWARE_VERSION;

	
	pPCUResponsePacket->nCheckSum = 0;
}
