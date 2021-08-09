// serialUserCommands.c


#include "LPC23xx.H"    // LPC23xx definitions
#include "shareddefs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "queue.h"
#include "sharedinterface.h"

#include "watchdog.h"
#include "iap.h"

#include "ProductID.h"


#define MAX_CMD_ARGS    3
#define MAX_CMD_ARG_LEN 40


#define CMD_BYTE    0
#define CMD_ARG1    1
#define CMD_ARG2    2




static uint8_t blockVals[FLASH_BLOCK_SIZE]; 



char cmdArgs[MAX_CMD_ARGS][MAX_CMD_ARG_LEN] = {0, 0, 0};


static BYTE processUserCommands(char *command, char *arg1, char *arg2)
{
	BYTE retVal = TRUE;

    if(!strcasecmp(cmdArgs[CMD_BYTE],"reset") ||
            !strcasecmp(cmdArgs[CMD_BYTE],"reboot"))
    {
        printf("Rebooting System\r\n");
        watchdogReboot();	
        while(1);
    }
	else
	{
		retVal = FALSE;
	}

	return retVal;
}


BOOL processFlashCommands(char *cmd, char *cmdArg1, char *cmdArg2)
{
    DWORD flashAddress;
    UINT sector;
    UINT block;
    BYTE *bytePtr, byteVal;
    UINT i;
    
    
    if(!strcasecmp(cmd,"wf"))
    {
        eraseUpgradeFlagSector();
        printf("Flash Upgrade Flag Written\r\n");
        writeUpgradeFlag(PRODUCT_TARGET_ID, 98600, 0xA1B2C3D4);
    }
    else if(!strcasecmp(cmd,"rf"))
    {
        printf("upgradeStatusFlag[%02x]", getUpgradeStatusFlag());        
    }
		else if(!strcasecmp(cmd,"flag"))
    {
				if(upgradeCodePresent())
				{
					printf("Upgrade flag present\r\n");
				}
				else
				{
					printf("Upgrade flag NOT present\r\n");
				}        
    }
	else if(!strcasecmp(cmd,"rs"))
	{
        printf("code size[%d]\r\n", readCodeCrc32());
	}
	else if(!strcasecmp(cmd,"rc"))
	{
		printf("code size[%04x]\r\n", readCodeCrc32());
	}
    else if(!strcasecmp(cmd, "addr"))
    {
        if(cmdArg1 == NULL)
        {
            printf("command requires an argument\r\n");
        }
        else
        {      
            sector = atoi(cmdArg1);
            flashAddress = getSectorAddress(sector);
            printf("sectorAddress[%08lx]\r\n", flashAddress);
        } 
    }   
    else if(!strcasecmp(cmd, "rb"))
    {
        if(cmdArg1 == NULL || cmdArg2 == NULL)
        {
            printf("command requires 2 arguments\r\n");
        }
        else
        {      
            sector = atoi(cmdArg1);
            block  = atoi(cmdArg2);
            flashAddress = getSectorAddress(sector) + (block * FLASH_BLOCK_SIZE);
            printf("sector[%d] block[%d] addr[%08lx]\r\n", sector, block, flashAddress);
            
            i = 0;
            do {
                bytePtr = (BYTE*)(flashAddress+i++);
                byteVal = *bytePtr;
                printf("[%02x] ", byteVal);
                if(0 == i%16)  printf("\r\n");
            } while (i < FLASH_BLOCK_SIZE);
            printf("\r\n");
        } 
    }
    else if(!strcasecmp(cmd,"wb"))
    {     
        for(i=0; i<FLASH_BLOCK_SIZE; i++)
        {
            blockVals[i] = i;
        }
        
        sector = atoi(cmdArg1);
        block  = atoi(cmdArg2);
        flashAddress = getSectorAddress(sector) + (block * FLASH_BLOCK_SIZE);
        printf("sector[%d] block[%d] addr[%08lx]\r\n", sector, block, flashAddress);
    
        writeFlashData(flashAddress, &blockVals[0], FLASH_BLOCK_SIZE);
    }
    else if(!strcasecmp(cmd,"wb2"))
    {     
        for(i=0; i<FLASH_BLOCK_SIZE; i++)
        {
            blockVals[i] = 255 - i;
        }
        
        sector = atoi(cmdArg1);
        block  = atoi(cmdArg2);
        flashAddress = getSectorAddress(sector) + (block * FLASH_BLOCK_SIZE);
        printf("sector[%d] block[%d] addr[%08lx]\r\n", sector, block, flashAddress);
    
        writeFlashData(flashAddress, &blockVals[0], FLASH_BLOCK_SIZE);
    }
    else if(!strcasecmp(cmd,"wt"))
    {     
        for(i=0; i<FLASH_BLOCK_SIZE; i++)
        {
            blockVals[i] = 255 - i;
        }
        
        sector = atoi(cmdArg1);
        block  = atoi(cmdArg2);

        flashAddress = getSectorAddress(sector);
        
        for(i=0; i<block; i++)
        {
            blockVals[0] = i;
            writeFlashData(flashAddress, &blockVals[0], FLASH_BLOCK_SIZE);
            printf("[%08lx]\r\n", flashAddress);
            flashAddress += FLASH_BLOCK_SIZE;
        }
    }        
    else if(!strcasecmp(cmd,"er"))
    {
        byteVal = erase_sectors(atoi(cmdArg1), atoi(cmdArg2));
        printf("erase_sectors %d to %d  ... return value: %d\r\n", 
                    atoi(cmdArg1),
                    atoi(cmdArg2),
                    byteVal);
    }    
    else if(!strcasecmp(cmd, "bl"))
    {
        for(sector = 0; sector <= MAX_SECTOR_NUM; sector++)
        {
            printf("secNum[%d] isBlank[%d]\r\n", sector, (int)blank_check_sector(sector, sector));
        }
    }
    else if(!strcasecmp(cmd, "testf"))
    {
        iapTestWriteApplicationCode();        
    }
    else if(!strcasecmp(cmd, "cb"))
    {
        uint32_t readSectorAddress =  iapGetBlockAddress(11, 0);
        uint32_t writeSectorAddress = iapGetBlockAddress(12, 0);
        
        for(i = 0; i < 128; i++)
        {
            copySectorData(readSectorAddress, writeSectorAddress);
            readSectorAddress  += FLASH_BLOCK_SIZE;
            writeSectorAddress += FLASH_BLOCK_SIZE;
            printf("[%d]\r\n", i);
        }
        
        printf("Complete\r\n");
    }    
    else if(!strcasecmp(cmd, "ba"))
    {
        if(cmdArg1 == NULL || cmdArg2 == NULL)
        {
            printf("command requires 2 arguments\r\n");
        }
        else
        {      
            sector = atoi(cmdArg1);
            block  = atoi(cmdArg2);
            flashAddress = iapGetBlockAddress(sector, block);
            printf("[%04lx][%ld]\r\n", flashAddress, flashAddress);
            
        } 
    } 
    else if(!strcasecmp(cmd, "sec"))
    {
        printf("sec[%ld]\r\n", getSectorNumber(0x20300));
        printf("sizeof(unsigned long)[%d]\r\n",sizeof(unsigned long));
        printf("sizeof(DWORD)[%d]\r\n",sizeof(DWORD));         
    }
    else
    {
        return FALSE;
    }
    
    return TRUE;
}


void serialProcessUserCommand(char *usrCommand)
{
    int i;
    char *token;

     
    
    memset(cmdArgs, 0, sizeof(cmdArgs));
    
	if(strlen(usrCommand) == 0)
	{
		printf("strlen == 0\n\r");
	}
	else
	{
        token = strtok(usrCommand, " ");
            
        for(i=0; i<MAX_CMD_ARGS; i++)
        {   
            if(strlen(token) < MAX_CMD_ARG_LEN)
            {
                strcpy(cmdArgs[i], token);           
            }
            else
            {
                printf("Serial command length error\r\n");
                return;
            }
           
            token = strtok(NULL, " ");
            if(NULL == token)
            {
                break;
            }
            
        }
    }

// MJB    
printf("%s %s %s\r\n", &cmdArgs[CMD_BYTE][0], &cmdArgs[CMD_ARG1][0], &cmdArgs[CMD_ARG2][0]);
    
    if(processUserCommands(&cmdArgs[CMD_BYTE][0], &cmdArgs[CMD_ARG1][0], &cmdArgs[CMD_ARG2][0]))
    {
        return;
    }

    if(processFlashCommands(&cmdArgs[CMD_BYTE][0], &cmdArgs[CMD_ARG1][0], &cmdArgs[CMD_ARG2][0]))
    {
        return;
    }
    
    printf("unknown command\r\n");
    
}

