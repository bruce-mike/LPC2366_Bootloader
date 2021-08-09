#include "shareddefs.h"
#include "sharedinterface.h"
#include "iap.h"
#include "crc32.h"
#include "queue.h"
#include "serial.h"
#include <stdio.h>
#include <string.h>
#include "timer.h"





//=============================================================================
// command format
//  <length><seqNum><command><parameter><data byte(s)><checksum>
//=============================================================================

#define LEN_BYTE                    0
#define LEN_BYTE_256                1 // this byte cannot ever be 0 - add offset of 1
#define CMD_BYTE                    2
#define PARAM_BYTE                  3
#define DATA_START_BYTE             4

#define COMMAND_GET                 1
#define COMMAND_SET                 2
#define COMMAND_HISTORY             3
#define COMMAND_SOFTWATE_UPDATE     4
#define COMMAND_STATUS              5
#define COMMAND_DATA                6
#define COMMAND_TEST                7

#define PARAM_START_UPGRADE         17
#define PARAM_ERASE_UPGRADE_SECTORS 18
#define PARAM_WRITE_BLOCK           19
#define PARAM_END_UPGRADE           20
#define PARAM_ABORT_UPGRADE         21

#define STATUS_OKAY                     0
#define STATUS_ERROR_CHECKSUM           1
#define STATUS_ERROR_UNKNOWN_CMD        2
#define STATUS_ERROR_UNKOWN_PARAM       3
#define STATUS_ERROR_ILLEGAL_SETTING    4
#define STATUS_ERROR_SECTOR_NOT_BLANK   5
#define STATUS_ERROR_WRITE_BLOCK        6
#define STATUS_CODE_CS_ERROR            7
#define STATUS_ERROR_WRITE_FLAG         8


#define MAX_OUT_CMD_LENGTH  50 
#define MAX_IN_COMMAND_LENGTH  300
           
static BYTE statusOutCmd[MAX_OUT_CMD_LENGTH];
static BYTE upgradeCmd[MAX_IN_COMMAND_LENGTH];

static unsigned long codeSize;
static unsigned long codeCrc32;

static TIMERCTL upgradeCmdByteTimer;
#define NEW_CMD_BYTE_TIME_MS    100

static eINTERFACE upgradeComPort;

typedef unsigned short USHORT;


//===================================================================
// CRTITCAL!!!
// this must be aligned on a 4 byte address or the IAP upgrade 
// will be broke !!!
//===================================================================
__align(4) static BYTE blockCodeUpgrade[FLASH_BLOCK_SIZE];
//===================================================================


static unsigned long convertBytesToUint32(BYTE *bytes)
{
    unsigned long uint32Val;
    
    uint32Val = ((unsigned long)bytes[0] << 24) +
                ((unsigned long)bytes[1] << 16) +
                ((unsigned long)bytes[2] << 8) + bytes[3];

    return uint32Val;
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


static int calculateCommandLength(BYTE *msg)
{
    int length = (int)msg[0] + (int)((msg[1] - 1) * 256); 
    return (length);
}



static void writeModemCommand(BYTE cmd[])
{
    // add checksum and then send out to modem port
    int i;
      
    printf("\r\nwriteModemCommand()\r\n"); 
    addCheckSum(cmd);
   
    for(i=0; i<=cmd[0] ;i++)
    {
        serialSendUpgradeCommand(upgradeComPort, cmd[i]);
    }  

}

static void writeRawCommand(BYTE cmd[])
{
   int i;
            
    for(i=0; i<=cmd[0] ;i++)
    {
        serialSendUpgradeCommand(upgradeComPort, cmd[i]);
    }    
}


static void sendStatusCommand(BYTE status)
{
    statusOutCmd[LEN_BYTE] = 4;
    // Asset tracker cannot have 0 byte value - always add 1 as offset
    statusOutCmd[LEN_BYTE_256] = 1;
    statusOutCmd[CMD_BYTE] = COMMAND_STATUS;
    statusOutCmd[PARAM_BYTE] = status;
    
    writeModemCommand(statusOutCmd);
}


static void doSoftwareUpdate(BYTE cmd[])
{
    unsigned long writeAddress;
    unsigned int cmdLength;
    unsigned int blockNum;
    WORD codeBytes;
    unsigned int i;
    BYTE  status = STATUS_OKAY;
    
    switch(cmd[PARAM_BYTE])
    {
        case PARAM_START_UPGRADE:
            codeSize = convertBytesToUint32(&cmd[DATA_START_BYTE]);
            codeCrc32 = convertBytesToUint32(&cmd[DATA_START_BYTE+4]);
printf("codeSize[%lu]  codeCrc32[%08lx]\r\n", codeSize, codeCrc32);
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
printf("write block[%d]\r\n", blockNum);
                if(TRUE == writeFlashData(writeAddress, &blockCodeUpgrade[0], codeBytes))
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
            
        case PARAM_ABORT_UPGRADE:
            serialSetUpgradeMode(FALSE);
            break;
            
    }   

    sendStatusCommand(status);
}


static void interpretUpgradeCommand(BYTE cmd[])
{
printf("cmdByte[%02x]\r\n", cmd[CMD_BYTE]);
    
    switch(cmd[CMD_BYTE])
    {
        //case COMMAND_GET:     
        //case COMMAND_SET:      
        //case COMMAND_HISTORY:
        
        case COMMAND_SOFTWATE_UPDATE:
            doSoftwareUpdate(cmd);
        break;
        
        case COMMAND_STATUS:
printf("Status Command!! \r\n");
            
        break;
        
        case COMMAND_TEST:
printf("Command test\r\n");
            writeRawCommand(&cmd[0]);
        break;
            
        default:
            sendStatusCommand(STATUS_ERROR_UNKNOWN_CMD);
    }    
}


//========================================================
void
initUpgradeByteTimer()
//========================================================
{
    initTimer(&upgradeCmdByteTimer);
    startTimer(&upgradeCmdByteTimer, NEW_CMD_BYTE_TIME_MS);    
}    

//==================================================================
void
upgradeProcessBytes(eINTERFACE eInterface, uint8_t rxByte)
//==================================================================
{
    static int cmdCursor = 0;
    static int cmdLength;
    
    
    upgradeComPort = eInterface;
    
    if(isTimerExpired(&upgradeCmdByteTimer))
    {
printf("Timer reset\r\n");
        cmdCursor = 0;
    }
    
    startTimer(&upgradeCmdByteTimer, NEW_CMD_BYTE_TIME_MS); 
    
    upgradeCmd[cmdCursor] = rxByte;
    
printf("[%02x] ", rxByte);
    
    
    if(cmdCursor == 1)
	{         
        cmdLength = calculateCommandLength(upgradeCmd);
printf("cmdLength[%d]\r\n", cmdLength);
        if(cmdLength >= MAX_IN_COMMAND_LENGTH)
        {
            cmdCursor = 0;
        }
    }
    else if(cmdCursor > 1)
    {
        if(cmdCursor == cmdLength)
        {
printf(" Command received\r\n");
            interpretUpgradeCommand(&upgradeCmd[0]);
            cmdCursor = 0;
        }
	}
    
    if(++cmdCursor >= MAX_IN_COMMAND_LENGTH)
	{
        cmdCursor = 0;
	}
    
}



    
