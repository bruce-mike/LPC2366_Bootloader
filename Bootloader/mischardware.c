// support functions particlar to the driver board
// GPIO config, control of periphs and status bits
// connected straight to uP.

// UART/I2C/ADC etc handled in respective .c and .h files

#include <lpc23xx.h>
#include <stdio.h>

// hardware specs
#include "mischardware.h"


typedef PACKED struct sHwPowerControlData
{
	DWORD dChargerOffVoltage;
	DWORD dChargerOnVoltage;
	DWORD dBatteryVoltageGoodLimit; 		
	DWORD dBatteryVoltageHighLimit; 		
	DWORD dBatteryVoltageWarnLimit; 		
	DWORD dBatteryVoltageLVDLimit;
}sHW_POWER_CONTROL_DATA;

static sHW_POWER_CONTROL_DATA sHwPowerControlData;

typedef enum eLVDControlStates
{
	eLVD_MONITOR_VB_VL,
	eLVD_VB_VL_AT_SHUTDOWN_VOLTAGE, 
}eLVD_CONTROL_STATES;


static BOOL solarChargerOn, lvdOn = FALSE;
static BOOL batteryEnabled, ovpOverrideEnabled = FALSE;


// GPIO CONFIG
// set up I/Os as in/out, set default states
// peripheral pins (UART/ADC/SPI/ETC) are *NOT* configured here.

// NB: LAST STEP IS TO DISABLE ETM IN SW
// it is configured on boot by sampling RTCK.
// The value written to the config reg is xRTCK.  
// As drawn, this enables ETM wand hoses some PORT2 ops

// *********************
// *********************
// *********************
// DISABLE ETM (embedded trace - dorks up PORT2 pins 9:0)
static void hwDisableETM(void) {
	
	//FIXME MAGIC NUMBER
	PINSEL10 = 0x00000000;

}

void hwGPIOConfig(void)
{

	// 1 = output, 0 = input


	// enable FIO
	SCS = 0x1;	

	// SET DIRECTIONS


	// should port inits be individualized?
	// PORT1_INIT()
	// PORT1_INIT(), etc?
	
	//**************
	//******* PORT 0

	// plain I/O setup
	// chip default is input w/pull-ups ENABLED
	
	// special functions						  
	// ADC, UART, I2C pins set up in respective
	// peripheral init routines
	// should they be included here?


	// explicitly configure port as GPIO
	PINSEL0 = 0x00000000; 
	PINSEL1 = 0x00000000;

	// set directions
	FIO0DIR =	(IN  << ISP_xRST    ) |
						(OUT << uP_SPI1_xCS ) |
						(OUT << uP_SPI1_SCK ) |	
						(IN  << uP_SPI1_MISO) |
						(OUT << uP_SPI1_MOSI);
						
	FIO0SET =	(1 << uP_SPI1_xCS ) |
						(1 << uP_SPI1_SCK ) |	
						(1 << uP_SPI1_MOSI) ;

	//**************
	//******* PORT 1
	// peripherals are set up in their respective
	// config routines (ADC, UART, SPI)
	// set as GPIO
	PINSEL2 = 0x00000000;
	PINSEL3 = 0x00000000;

	// set directions
	FIO1DIR =   (IN  << uP_MOD_X1  ) |
				(IN  << uP_MOD_X2  ) |
				(IN  << uP_MOD_X4  ) |
				(IN  << uP_MOD_X8  ) |					
				(OUT << uP_PWR_VL  ) |	// SOLAR CHARGE ENABLE
				(OUT << uP_WD_KICK );

					
	FIO1SET = (1 << uP_WD_KICK ); // Watchdog Kick Circuit toggle 
						
	FIO1CLR = (1 << uP_WD_KICK )| // Watchdog Kick Circuit toggle
						(1 << uP_PWR_VL); 	// ENABLE SOLAR CHARGE
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		case eBoardRev2:
			FIO1DIR |= 	(IN  << uP_POS_VL  ) |
									(IN  << uP_POS_VB  ) |
									(OUT << uP_OVP_VL  ) |//4363 overvoltage lockout override ("force on" by defeating auto-retry)
									(OUT << uP_PWR_VB  );  // Battery Enable;  
			FIO1SET |= 	(1 << uP_PWR_VB) |  // ENABLE LT4365/BATTERY POWER
									(1 << uP_OVP_VL);			// 4363 over-voltage shutdown override(manual control)
																				// pull low to force part out of OVLO/defeat auto-retry
		default:
			break;

		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			break;
	}
	
	//**************
	//******* PORT 2
	// set as GPIO except for interrupt pins
	PINSEL4 = 	(EXT_INT << EXTINT2) | 
				(EXT_INT << EXTINT3);

	FIO2DIR = (OUT << uP_AUX_LED  ) |
						(OUT << uP_LOAD_SW_EN)|
						(OUT << uP_PWR_xSHDN) |		// LVVD
                        (OUT << uP_SPI1_UART_CS) |
                        (IN << uP_SPI1_UART_CS) |
						(IN  << uP_xISP     ) |
						(IN  << uP_INT_xERR ) |
						(IN << REV2 )         |
						(IN << REV1 )         |
						(IN << REV0 );
				
					
	// SET INITIAL VALUES ON OUTPUTS
	FIO2SET = (1 << uP_PWR_xSHDN) |		// disable I2C driver for system LEDs
						(1 << uP_LOAD_SW_EN);
						
	FIO2CLR = (1 << uP_AUX_LED);


	//**************
	//******* PORT 3
	PINSEL6 = 0x00000000;		
	PINSEL7 = 0x00000000;


	//**************
	//******* PORT 4
	//**************
	// the only port4 pins are SPI1.
	PINSEL9 = 0x00000000;
	
	FIO4DIR = (OUT << uP_SPI1_HOLD) |
						(OUT << uP_SPI1_WP  );

	FIO4SET = (1 << uP_SPI1_HOLD) |
						(1 << uP_SPI1_WP  );
						
	// the last step - disable ETM
	hwDisableETM();

}


// enable various periphs connected straight to uP

void hwSetSysLED()
{
	FIO2SET = (1<<uP_AUX_LED);
}

void hwClrSysLED()
{
	FIO2CLR = (1<<uP_AUX_LED);
}


// *********************
// *********************
// *********************
// turn on/off solar charge
void hwSetSolarCharge(int action)
{
	if (ON == action) 
	{
		FIO1SET = (1 << uP_PWR_VL);
		
		solarChargerOn = TRUE;
	}
	else if (OFF == action)
	{
		FIO1CLR = (1 << uP_PWR_VL);
		
		solarChargerOn = FALSE;
	}
}

BOOL hwGetSolarChargeStatus(void)
{
	return solarChargerOn;
}

// *********************
// *********************
// *********************
// turn on/off LVD
void hwSetLVD(int action)
{
	if (ON == action)
	{	
		FIO2CLR = (1 << uP_PWR_xSHDN);
		
		lvdOn = TRUE;
	}
	else if (OFF == action)
	{	
		FIO2SET = (1 << uP_PWR_xSHDN);
		
		lvdOn = FALSE;
	}	
}

BOOL hwGetLVDStatus(void)
{
	return lvdOn;
}

// *********************
// *********************
// *********************

// check solar voltage polarity
// return TRUE if positive/correct
// FALSE if neg/reverse pol
BOOL hwIsVLPosPolarity(void)
{
	BOOL bRetVal = TRUE;
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		case eBoardRev2:
			{
				// read port
				int port = FIO1PIN;

				// mask off POS_VL bit
				port = port & (1 << uP_POS_VL);

				if(0 == port) 
				{
					bRetVal = FALSE;
				} 
			}
			break;
		default:
			break;

		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			break;
	}

return bRetVal;
	
}

// *********************
// *********************
// *********************

// check battery voltage polarity
// return TRUE if positive/correct polarity
// FALSE if neg/reverse pol
BOOL hwIsVBPosPolarity(void)
{
	BOOL bRetVal = TRUE;
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		case eBoardRev2:
			{
				// read port
				int port = FIO1PIN;

				// mask off POS_VB bit
				port = port & (1 << uP_POS_VB);

				if(0 == port) 
				{
					bRetVal = FALSE;
				} 
			}
			break;
		default:
			break;

		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			break;
	}

return bRetVal;
}


// Read model select
char hwReadModel(void)
{
	int port;
	char nModel = 0;

	// model select on port0
	port = FIO1PIN;	

	// mask off model bits
	port = port & ((1 << uP_MOD_X1) |
                 (1 << uP_MOD_X2) |
							   (1 << uP_MOD_X4) |
							   (1 << uP_MOD_X8) );

	if(port & (1 << uP_MOD_X1))
	{
		nModel |= 0x01;
	}
	
	if(port & (1 << uP_MOD_X2))
	{
		nModel |= 0x02;
	}
	
	if(port & (1 << uP_MOD_X4))
	{
		nModel |= 0x04;
	}
	
	if(port & (1 << uP_MOD_X8))
	{
		nModel |= 0x08;
	}

	return nModel;
}


//=======================================
eMODEL
getModelNumber()
//=======================================
// read the model switch
//=======================================
{
	eMODEL eModel = (eMODEL)(hwReadModel() & 0x07);
	printf("eModel[%d] hwReadModel()[%X]\r\n", eModel, hwReadModel());
	return eModel;
}

#if 1
WORD hwReadDriveErrors(void)
{
	WORD port;
	
	port = FIO2PIN;
	
	port &= (1 << uP_INT_xERR);
	
	return port;
}
#endif

/////
// read all switches
// TBD
/////
int hwReadSwitches(void)
{
	int port;

	// model select on port0
	port = FIO1PIN;	

	// mask off model bits
	port = port & ((1 << uP_MOD_X1) |
                 (1 << uP_MOD_X2) |
							   (1 << uP_MOD_X4) |
							   (1 << uP_MOD_X8) );
	
	// shift model nibble into LSB
	port = port >> (uP_MOD_X1);
	
	return port;
}
// *********************
// *********************
// *********************
// shutdown/enable 4365 OV/UV/Rev BATT controller
void hwSetLT4365BatteryEnable(int action) 
{
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		case eBoardRev2:
			{
				if(ON == action) 
				{
					FIO1SET = (1 << uP_PWR_VB);
		
					batteryEnabled = TRUE;
				}
				else if(OFF == action) 
				{
					FIO1CLR = (1 << uP_PWR_VB);
		
					batteryEnabled = FALSE;	
				}
			}
			break;
		default:
			break;

		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			batteryEnabled = TRUE;
			break;
	}	
}

BOOL hwGetBatteryEnabledStatus(void)
{
	return batteryEnabled;
}

// *********************
// *********************
// *********************
void hwSetLT4363OvpOverride(int action) 
{
	switch(hwGetBoardRevision())
	{
		case eBoardRev1:
		case eBoardRev2:
			{
				// shutdonw/enable 4363 OVP Manual Override
				if(action == ON)
				{		
					FIO1CLR = (1 << uP_OVP_VL);
		
					ovpOverrideEnabled = TRUE;
				}
				else if(OFF == action)
				{		
					FIO1SET = (1 << uP_OVP_VL);
		
					ovpOverrideEnabled = FALSE;	
				}	
			}
			break;
		default:
			break;

		case eBoardRev3:
		case eBoardRev4:
		case eBoardRev5:
		case eBoardRev6:
		case eBoardRev7:
			ovpOverrideEnabled = FALSE;
			break;
	}

}

BOOL hwGetOvpOverrideEnabledStatus(void)
{
	return ovpOverrideEnabled;
}

#if (1)
// *********************
// *********************
// *********************
// ENABLE PCA9634 connected to system LEDs 
// and aux/indicator
// (lower xOE on PCA9634)
void hwEnableSystemLedsAndIndicators(void)
{	
	//printf("HWENA\n");
	FIO2CLR = (1 << uP_LOAD_SW_EN);			// xOE = 0 ENABLES PCA9436.
}

void hwDisableSystemLedsAndIndicators(void)
{	
	//printf("HWDISA\n");
	FIO2SET = (1 << uP_LOAD_SW_EN);			// xOE = 1 DISABLES PCA9436.
}
#endif

//================================================
eBOARD_REVISION
hwGetBoardRevision(void)
//================================================
{
    int port;
	static int nBoardRevision = 0;
    static BYTE firstTime = TRUE;
      
    if(TRUE == firstTime)
    {
        firstTime = FALSE;
        
        // revision select on port2
        port = FIO2PIN;

		// mask off board revision bits
		// note, these are inverted, pulled low is 1
		
		if(0 == (port & (1<<REV2)))
		{
            nBoardRevision |= 4;
		}
		if(0 == (port & (1<<REV1)))
		{
            nBoardRevision |= 2;
		}
		if(0 == (port & (1<<REV0)))
		{
            nBoardRevision |= 1;
		}
        
    }

    return (eBOARD_REVISION)nBoardRevision;
}

BYTE hwGetSpiUartDataReady()
{
	int port = FIO2PIN;

    // SPI UART active LOW output
	if(0 == (port & (1<<uP_SPI_UART_RDY)))
	{
		return TRUE;
	}

	return FALSE;
}

void hwInitPowerControlData(void)
{
	sHwPowerControlData.dChargerOffVoltage 			 = defaultChargerOffVoltage;
	sHwPowerControlData.dChargerOnVoltage 			 = defaultChargerOnVoltage;			
	sHwPowerControlData.dBatteryVoltageGoodLimit = defaultBatteryVoltageGoodLimit;
	sHwPowerControlData.dBatteryVoltageHighLimit = defaultBatteryVoltageHighLimit;
	sHwPowerControlData.dBatteryVoltageWarnLimit = defaultBatteryVoltageWarnLimit;
	sHwPowerControlData.dBatteryVoltageLVDLimit  = defaultBatteryVoltageLVDLimit;
}

void hwSetPowerControlData(ePCU_COMMANDS ePowerControlCommands, DWORD dPowerControlData)
{
	switch(ePowerControlCommands)
	{
		case ePCU_COMMAND_SET_CHRGR_OFF_VOLTAGE:
#ifdef DEBUG_POWER_CTL
    printf("hwSetPowerControlData: ePCU_COMMAND_SET_CHRGR_OFF_VOLTAGE[%lu]\r\n",dPowerControlData);
#endif
        sHwPowerControlData.dChargerOffVoltage = dPowerControlData;
			break;
		case ePCU_COMMAND_SET_CHRGR_ON_VOLTAGE:
#ifdef DEBUG_POWER_CTL
        printf("hwSetPowerControlData: ePCU_COMMAND_SET_CHRGR_ON_VOLTAGE[%lu]\r\n",dPowerControlData);
#endif
        sHwPowerControlData.dChargerOnVoltage = dPowerControlData;			
			break;
		case ePCU_COMMAND_SET_PWR_GOOD_VOLTAGE: 
#ifdef DEBUG_POWER_CTL
        printf("hwSetPowerControlData: ePCU_COMMAND_SET_PWR_GOOD_VOLTAGE[%lu]\r\n",dPowerControlData);
#endif
        sHwPowerControlData.dBatteryVoltageGoodLimit = dPowerControlData;
			break;
		case ePCU_COMMAND_SET_PWR_HIGH_VOLTAGE: 
#ifdef DEBUG_POWER_CTL
        printf("hwSetPowerControlData: ePCU_COMMAND_SET_PWR_HIGH_VOLTAGE[%lu]\r\n",dPowerControlData);
#endif
        sHwPowerControlData.dBatteryVoltageHighLimit = dPowerControlData;
			break;
		case ePCU_COMMAND_SET_PWR_WARN_VOLTAGE: 
#ifdef DEBUG_POWER_CTL
        printf("hwSetPowerControlData: ePCU_COMMAND_SET_PWR_WARN_VOLTAGE[%lu]\r\n",dPowerControlData);
#endif
        sHwPowerControlData.dBatteryVoltageWarnLimit = dPowerControlData;
			break;
		case ePCU_COMMAND_SET_LVD_VOLTAGE:      
#ifdef DEBUG_POWER_CTL
        printf("hwSetPowerControlData: ePCU_COMMAND_SET_LVD_VOLTAGE[%lu]\r\n",dPowerControlData);
#endif
        sHwPowerControlData.dBatteryVoltageLVDLimit = dPowerControlData;
			break;
		default:
			break;
	}
}

DWORD hwGetPowerControlData(ePCU_COMMANDS ePowerControlCommands)
{
	DWORD wPowerControlData = 0;
    
#ifdef DEBUG_POWER_CTL
static int delay = 0;
    
if(++delay >= 100000)
{
    delay = 0;
        
    printf("*****************************************************\r\n");
    printf("ChargerOffVoltage[%lu]\r\n", sHwPowerControlData.dChargerOffVoltage);
    printf("ChargerOnVoltage[%lu]\r\n", sHwPowerControlData.dChargerOnVoltage);      
    printf("BatteryGoodVoltage[%lu]\r\n", sHwPowerControlData.dBatteryVoltageGoodLimit);       
    printf("BatteryHighVoltage[%lu]\r\n", sHwPowerControlData.dBatteryVoltageHighLimit);
    printf("BatteryWarnVoltage[%lu]\r\n", sHwPowerControlData.dBatteryVoltageWarnLimit);
    printf("BatteryLVDVoltage[%lu]\r\n", sHwPowerControlData.dBatteryVoltageLVDLimit);       
    printf("\r\n\r\n");
}
#endif
    
	switch(ePowerControlCommands)
	{
		case ePCU_COMMAND_GET_CHRGR_OFF_VOLTAGE:
            wPowerControlData = sHwPowerControlData.dChargerOffVoltage;
			break;
		case ePCU_COMMAND_GET_CHRGR_ON_VOLTAGE:
			wPowerControlData = sHwPowerControlData.dChargerOnVoltage;			
			//printf("hwGetPowerControlData: ePCU_COMMAND_GET_CHRGR_ON_VOLTAGE[%lu]\n",wPowerControlData);
			break;
		case ePCU_COMMAND_GET_PWR_GOOD_VOLTAGE: 
			wPowerControlData = sHwPowerControlData.dBatteryVoltageGoodLimit;
			//printf("hwGetPowerControlData: ePCU_COMMAND_GET_PWR_GOOD_VOLTAGE[%lu]\n",wPowerControlData);
			break;
		case ePCU_COMMAND_GET_PWR_HIGH_VOLTAGE: 
			wPowerControlData = sHwPowerControlData.dBatteryVoltageHighLimit;
			//printf("hwGetPowerControlData: ePCU_COMMAND_GET_PWR_HIGH_VOLTAGE[%lu]\n",wPowerControlData);
			break;
		case ePCU_COMMAND_GET_PWR_WARN_VOLTAGE: 
			wPowerControlData = sHwPowerControlData.dBatteryVoltageWarnLimit;
			//printf("hwGetPowerControlData: ePCU_COMMAND_GET_PWR_WARN_VOLTAGE[%lu]\n",wPowerControlData);
			break;
		case ePCU_COMMAND_GET_LVD_VOLTAGE:      
			wPowerControlData = sHwPowerControlData.dBatteryVoltageLVDLimit;
            //printf("hwGetPowerControlData: ePCU_COMMAND_GET_LVD_VOLTAGE[%lu]\n",wPowerControlData);
			break;
		default:
			break;
	}
	
	return(wPowerControlData);
}


