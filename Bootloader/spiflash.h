#ifndef SPI_FLASH_H
#define SPI_FLASH_H
#include "shareddefs.h"
#include "sharedinterface.h"
#include "queue.h"
#include "timer.h"

/////////////
// SPI FLASH Commands
/////////////
#define SPI_FLASH_WRITE_STATUS        0x01
#define SPI_FLASH_PAGE_PROGRAM        0x02 // Program 256 Byte Page
#define SPI_FLASH_READ_DATA_NORMAL    0x03
#define SPI_FLASH_READ_DATA_FAST      0x0B
#define SPI_FLASH_READ_DATA_FAST_DUAL 0x3B
#define SPI_FLASH_WRITE_DISABLE       0x04
#define SPI_FLASH_READ_STATUS         0x05
#define SPI_FLASH_WRITE_ENABLE        0x06
#define SPI_FLASH_SECTOR_ERASE        0x20 // Sector (4KB) Erase
#define SPI_FLASH_BLOCK_ERASE					0xD8 // Block (64KB) Erase
#define SPI_FLASH_CHIP_ERASE					0xC7 // Entire Chip Erase
#define SPI_FLASH_POWER_DOWN					0xB9
#define SPI_FLASH_REALESE_POWER_DOWN	0xAB

/////////////
// SPI FLASH Status Register Bits
/////////////
#define SPI_FLASH_WRITE_IN_PROGRESS				0x01
#define SPI_FLASH_WRITE_ENABLE_LATCH			0x02
#define SPI_FLASH_BLOCK_PROTECT_BIT0  		0x04
#define SPI_FLASH_BLOCK_PROTECT_BIT1  		0x08
#define SPI_FLASH_BLOCK_PROTECT_BIT2  		0x10
#define SPI_FLASH_BLOCK_PROTECT_BIT3  		0x20
#define SPI_FLASH_STATUS_REGISTER_PROTECT 0x80

void spiFlashErase(BYTE eraseType, DWORD eraseAddress, ePLATFORM_TYPE ePlatformType);
void spiFlashWriteAddress(BYTE *address, ePLATFORM_TYPE ePlatformType);
void spiFlashWriteData(BYTE *data, DWORD startAddress, UINT imageSize, ePLATFORM_TYPE ePlatformType);
void spiFlashWriteCommand(BYTE command, BOOL resetCS, ePLATFORM_TYPE ePlatformType);
void spiFlashReadStatus(BYTE *status, BOOL resetCS, ePLATFORM_TYPE ePlatformType);
void spiFlashReadData(BYTE *data, DWORD spiReadAddress, UINT imageSize, BYTE readType, ePLATFORM_TYPE ePlatformType);
BOOL spiFlashVerifyData(BYTE *data, DWORD spiReadAddress, UINT imageSize, BYTE readType, ePLATFORM_TYPE ePlatformType);
DWORD spiFlashSwapDword( DWORD Data ); 
WORD spiFlashSwapWord(WORD Data);

#endif		// SPI_FLASH_H
