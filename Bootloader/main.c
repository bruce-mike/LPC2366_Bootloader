#include <lpc23xx.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shareddefs.h"
#include "pll.h"
#include "irq.h"
#include "watchdog.h"
#include "softwareVersion.h"
#include "iap.h"
#include "pll.h"


const uint32_t bootLoaderSoftwareVersion __attribute__((at(0x02ffc))) = SOFTWARE_VERSION;  



// Watch Dog Pet - Port 1.4
#define uP_WD_KICK		4		

#define WATCHDOG_FEEDING_TIME_MS    1000
#define LAMPS_ENABLE_TIME_MS        10000

#define ADC_SAMPLE_TIME_MS          100
#define ACD_SAMPLE_TICKS_PER_SECOND 10

#define SOLAR_MANUAL_MODE_TIME_MS   (1000*60*5)
#define DCU_RESET_TIME_MS           2000

#define USER_START_SECTOR 4
#define USER_START_SECTOR_ADDRESS 0x4000 


#define CODE_BLOCK_OKAY         1
#define CODE_UPGRADE_WRITING    2
#define CODE_BLOCK_ERROR        3
#define CODE_WRITE_COMPLETE     4


__align(4) static uint32_t upgradeBytesWritten = 0;
__align(4) static uint32_t upgradeCodeSize;

static uint32_t readSectorAddress;
static uint32_t writeSectorAddress;


static void doWork(void);


//=======================================================
static void
disableInterrupts(void)
//=======================================================
{      
    // disable all (32) interrupts by writing 1's to 
    // Interrupt Enable Clear Register
    VICIntEnClr = 0xFFFFFFFF;
}



//=====================================================
uint8_t
copySectors()
//=====================================================
{
    int16_t i; 

    
    for(i = 0; i < 10; i++)
    {
        copySectorData(readSectorAddress, writeSectorAddress);
        readSectorAddress  += FLASH_BLOCK_SIZE;
        writeSectorAddress += FLASH_BLOCK_SIZE;
        upgradeBytesWritten += FLASH_BLOCK_SIZE;
        if(upgradeBytesWritten >= upgradeCodeSize)
        {
             return CODE_WRITE_COMPLETE;
        }
    }
      
  
    return CODE_UPGRADE_WRITING;    
}    



//==========================================
void
execute_user_code(void)
//==========================================
{
    void (*user_code_entry)(void);
    user_code_entry = (void (*)(void))USER_START_SECTOR_ADDRESS;
    user_code_entry();
}


//================================================
static
void doWork()
//================================================
{   
    uint8_t doCodeUpgrade = CODE_UPGRADE_WRITING;
    
    
    while(doCodeUpgrade != CODE_WRITE_COMPLETE)
    {
        // feed the watchdog (ADM6320)
        FIO1SET = (1 << uP_WD_KICK );
        FIO1CLR = (1 << uP_WD_KICK );
        doCodeUpgrade = copySectors();
    }
    
}




//======================================================
int
main(void)
//======================================================
{
	int delay;
	
    // delay for JTAG	 to catch the processor and put it in reset	
	for(delay = 0; delay < 10000; delay++)
	{}

    //hwDisableETM();
        
    PLL_CONFIG();
	
    if(TRUE == upgradeCodePresent())
    {       
        upgradeCodeSize     =  readCodeSize();
        readSectorAddress   =  iapGetBlockAddress(11, 0);
        writeSectorAddress  =  iapGetBlockAddress(4, 0);
        upgradeBytesWritten = 0;

        eraseCodeSectors();        
        doWork();
        
        writeUpgradeCompleteFlag(SOFTWARE_UPGRADE_STATUS_SUCCESS);
    }

    disableInterrupts();    
    execute_user_code();
}


