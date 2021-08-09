//=================================================
// softwareUpgrade.c
//=================================================

#include "shareddefs.h"
#include "iap.h"
#include "crc32.h"
#include "timer.h"
#include <stdio.h>
#include <string.h>
#include "queue.h"
#include "serial.h"


#define LEN_BYTE                    0
#define LEN_BYTE_256                1   // this byte cannot ever be 0 - add offset of 1
#define CMD_BYTE                    2
#define PARAM_BYTE                  3
#define DATA_START_BYTE             4

#define COMMAND_GET                 1
#define COMMAND_SET                 2
#define COMMAND_HISTORY             3
#define COMMAND_SOFTWATE_UPDATE     4
#define COMMAND_STATUS              5
#define COMMAND_DATA                6

#define PARAM_COMPASS_BEARING       1
#define PARAM_COMPASS_ORIENTATION   2
#define PARAM_BATTERY_VOLTAGE       3
#define PARAM_BATTERY_CURRENT       4
#define PARAM_SOLAR_VOLTAGE         5
#define PARAM_SOLAR_CURRENT         6
#define PARAM_CHARGING_INDICATOR    7
#define PARAM_PATTERN_SELECTION     8
#define PARAM_PHOTO_CELL_READING    9
#define PARAM_ALARM_BITMASK         10
#define PARAM_HARDWARE_VERSION      11
#define PARAM_SOFTWARE_VERSION      12
#define PARAM_GET_MODEL_TYPE        13
#define PARAM_GET_DATE_TIME         14
#define PARAM_GET_TELEMETRY_DATA    15
#define PARAM_GET_MAINTENANCE_DATA  16
#define PARAM_START_UPGRADE         17
#define PARAM_ERASE_UPGRADE_SECTORS 18
#define PARAM_WRITE_BLOCK           19
#define PARAM_END_UPGRADE           20



#define STATUS_OKAY                     0
#define STATUS_ERROR_CHECKSUM           1
#define STATUS_ERROR_UNKNOWN_CMD        2
#define STATUS_ERROR_UNKOWN_PARAM       3
#define STATUS_ERROR_ILLEGAL_SETTING    4
#define STATUS_ERROR_SECTOR_NOT_BLANK   5
#define STATUS_ERROR_WRITE_BLOCK        6
#define STATUS_CODE_CS_ERROR            7
#define STATUS_ERROR_WRITE_FLAG         8


#define MAX_CMD_LEN  300

#define TDU_RX_IN_TIMER  30  // 30 milliseconds between characters means it must be start of a new command
static TIMERCTL tduRxInTimer;



static uint32_t codeSize;
static uint32_t codeCrc32;


//===================================================================
// CRTITCAL!!!
// this must be aligned on a 4 byte address or the IAP upgrade 
// will be broke !!!
//===================================================================
__align(4) static uint8_t blockCodeUpgrade[FLASH_BLOCK_SIZE];
//===================================================================




uint32_t convertBytesToUint32(uint8_t *bytes)
{
    uint32_t uint32Val;
    
    uint32Val = ((uint32_t)bytes[0] << 24) +
                ((uint32_t)bytes[1] << 16) +
                ((uint32_t)bytes[2] << 8) + bytes[3];

    return uint32Val;
}

int calculateCommandLength(BYTE *msg)
{
    int length = (int)msg[0] + (int)((msg[1] - 1) * 256); 
    return (length);
}


static void addCheckSum(BYTE *msg)
{
    int i;
    BYTE cSum = 0;
    
    for(i=0; i<msg[0]; i++)
    {
        cSum += msg[i];
    }
    
    msg[i] = cSum;
}


static void writeStatusCommand(BYTE cmd[])
{
    // add checksum and then send out to modem port
    int i;
      
    
    addCheckSum(cmd);
       
    for(i=0; i<=cmd[0] ;i++)
    {
        serialRS485SendData(cmd[i]);
    }  
}


static void sendStatusCommand(BYTE status)
{
    // format of status command:
    // byte0 length byte 1 (length / 256)
    // byte1 length byte 2 (length % 256)
    // byte2 command byte: COMMAND_STATUS
    // byte3 parameter byte status
    // byte4 checksum byte   
    BYTE statusCmd[5];
    
    statusCmd[LEN_BYTE] = 4;
    statusCmd[LEN_BYTE_256] = 1;
    statusCmd[CMD_BYTE] = COMMAND_STATUS;
    statusCmd[PARAM_BYTE] = status;
    
    writeStatusCommand(&statusCmd[0]);
}


void doSoftwareUpdate(BYTE cmd[])
{
    uint32_t writeAddress;
    uint16_t cmdLength;
    uint16_t blockNum;
    uint16_t codeBytes;
    uint16_t i;
    uint8_t  status = STATUS_OKAY;
    
    switch(cmd[PARAM_BYTE])
    {
        case PARAM_START_UPGRADE:
            codeSize = convertBytesToUint32(&cmd[DATA_START_BYTE]);
            codeCrc32 = convertBytesToUint32(&cmd[DATA_START_BYTE+4]);
printf("codeSize[%d]  codeCrc32[%08x]\r\n", codeSize, codeCrc32);
            if(!eraseUpgradeSectors())
            {
                status = STATUS_ERROR_SECTOR_NOT_BLANK;
            }
            break;
        
        case PARAM_WRITE_BLOCK:
            status = STATUS_ERROR_WRITE_BLOCK;
        
            cmdLength = calculateCommandLength(cmd);
            // first 2 data bytes is block number
            // remaining data bytes is the data to write        
            codeBytes = cmdLength - DATA_START_BYTE - 3;
printf("cmdLength[%d]  codeBytes[%d]\r\n", cmdLength, codeBytes);
            if(codeBytes <= FLASH_BLOCK_SIZE)
            {
                memset(&blockCodeUpgrade[0], 0xFF, FLASH_BLOCK_SIZE);
                for(i=0; i<codeBytes; i++)
                {
                    blockCodeUpgrade[i] = cmd[DATA_START_BYTE+2+i];
                }
                // note: block numbers start at 0
                blockNum = cmd[DATA_START_BYTE] + (cmd[DATA_START_BYTE+1] * 256);
                writeAddress = UPGRADE_CODE_BASE_ADDRESS + (blockNum * FLASH_BLOCK_SIZE);
printf("write block address [%08x]  codebytes[%d]\r\n", writeAddress, codeBytes);
                if(TRUE == flashWriteBlock((DWORD*)writeAddress, &blockCodeUpgrade[0], codeBytes))
                {
                    status = STATUS_OKAY;
                }
            }
            break;

        case PARAM_END_UPGRADE:
            if(codeCrc32 == crc32Calculate((BYTE*)UPGRADE_CODE_BASE_ADDRESS, codeSize))
            {   
                if(!writeUpgradeFlag(codeSize, codeCrc32))
                {
                    status = STATUS_ERROR_WRITE_FLAG;
                }
            }
            else
            {
                status = STATUS_CODE_CS_ERROR;
            }
            break;
            
    }   

    sendStatusCommand(status);
}


static uint32_t tduRxCursor = 0;
static uint32_t tduCmdLen;
static uint8_t tduRxBuffer[MAX_CMD_LEN];

//==================================================================
void
processSoftwareUpgradeBytes(uint8_t rxByte)
//==================================================================
{
	if(TRUE == isTimerExpired(&tduRxInTimer))
	{
        // if timer elapsed then this must be the start of a new modem command
		tduRxCursor = 0;
	}
    
    tduRxBuffer[tduRxCursor] = rxByte;
    
    if(tduRxCursor  == 1)
	{         
        tduCmdLen = calculateCommandLength(tduRxBuffer);
        if(tduCmdLen >= MAX_CMD_LEN)
        {
            tduRxCursor = 0;
        }
    }
    else if(tduRxCursor > 1)
    {
        if(tduRxCursor == tduCmdLen)
        {
            doSoftwareUpdate(&tduRxBuffer[0]);
            tduRxCursor = 0;
        }
	}
    
    if(++tduRxCursor >= MAX_CMD_LEN)
	{
        tduRxCursor = 0;
	}
    
    startTimer(&tduRxInTimer, TDU_RX_IN_TIMER);
}
