#ifndef IAP_H_
#define IAP_H_

#include <stdint.h>



// bootloader and application upgrade flag location
// must match
#define MAX_SECTOR_NUM  14
#define NUM_SECTORS     15

// user code sectors 4-10 (98k - only use 96k max upgrade code size)
#define USER_CODE_FIRST_SECTOR          4
#define USER_CODE_LAST_SECTOR			10
#define USER_CODE_BASE_ADDRESS          0x4000 // first byte in sector 4

#define MAX_CODE_SIZE                   114687

// upgrade code sectors 10-12 (96k)
#define UPGRADE_CODE_FIRST_SECTOR		11 // 96k max upgrade code size
#define UPGRADE_CODE_LAST_SECTOR		14
#define UPGRADE_CODE_BASE_ADDRESS       0x20000 // first byte in sector 11

// upgrade sector 
#define UPGRADE_FLAG_SECTOR				3
#define SOFTWARE_UPGRADE_FLAG_LOC       0x3000 // first byte in sector 3 block 0
#define TARGET_ID_LOCATION              0x3004
#define CODE_SIZE_LOCATION              0x3008
#define CODE_CRC32_LOCATION             0x300C


#define FLASH_BLOCK_SIZE                256


//=========================================================================
// SECTOR 3, block 0 contains the upgrade information
//=========================================================================

// flag locations within first UPGRADE_FLAG_SECTOR 3 memory block 0
#define UPGRADE_FLAG_STATUS_LOC         0  // bytes 0, 1, 2, 3

// TARGET_ID - do not want to upgrade firmware tagergeted for wrong product
#define UPGRADE_TARGET_ID_LOC           4  // bytes 4, 5, 6 ,7

// size and crc32 values within UPGRADE_FLAG_SECTOR 3 memory block 1
#define UPGRADE_CODE_SIZE_LOC           8   // bytes 8, 9, 10, 11
#define UPGRADE_CRC32_LOC               12  // bytes 12, 13, 14, 15

// flag values
#define SOFTWARE_UPGRADE_FLAG_READY             0xAA  // upgrade ready on next reboot
#define SOFTWARE_UPGRADE_STATUS_SUCCESS    	    0x55  // upgrade completed successfully
#define SOFTWARE_UPGRADE_STATUS_CSUM_ERROR      0x44  // upgrade crc32 verify error
#define SOFTWARE_UPGRADE_STATUS_ERASE_ERROR     0x33  // upgrade code sector erase error
#define SOFTWARE_UPGRADE_STATUS_WRITE_ERROR     0x22  // upgrade code write verification error
#define SOFTWARE_UPGRADE_STAUS_TARGET_ERROR     0x11  // file uploaded for wrong product


/* IAP-Commands  */
#define PREPARE_SECTOR_FOR_WRITE_OPERATION  (50)
#define COPY_RAM_TO_FLASH                   (51)
#define ERASE_SECTOR                        (52)
#define BLANK_CHECK_SECTOR                  (53)
#define READ_PART_ID                        (54)
#define READ_BOOT_CODE_VERSION              (55)
#define COMPARE                             (56)

/* IAP status codes */
#define CMD_SUCCESS                                 (0)
#define INVALID_COMMAND                             (1)
#define SRC_ADDR_ERROR                              (2)
#define DST_ADDR_ERROR                              (3)
#define SRC_ADDR_NOT_MAPPED                         (4)
#define DST_ADDR_NOT_MAPPED                         (5)
#define COUNT_ERROR                                 (6)
#define INVALID_SECTOR                              (7)
#define SECTOR_NOT_BLANK                            (8)
#define SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION     (9)
#define COMPARE_ERROR                               (10)
#define BUSY                                        (11)


/* IAP start location on flash */
#define IAP_LOCATION    (0x7FFFFFF1)

/* PLL */
#define PLLCON_PLLE     (0x01)      ///< PLL Enable
#define PLLCON_PLLD     (0x00)      ///< PLL Disable
#define PLLCON_PLLC     (0x03)      ///< PLL Connect
//#define PLLSTAT_PLOCK   (0x0400)    //</ PLL Lock Status


uint32_t getCrc32(void);
BYTE flashrom_erase(BYTE *addr);
DWORD blank_check_sector(DWORD startSector, DWORD endSector);
DWORD copy_ram_to_flash(DWORD tmp_adr_dst, DWORD tmp_adr_src, DWORD tmp_size);
DWORD erase_sectors(DWORD tmp_sect1, DWORD tmp_sect2);
DWORD compare(DWORD tmp_adr_dst, DWORD tmp_adr_src, DWORD tmp_size);
unsigned long getSectorNumber(unsigned long adr);
DWORD getSectorAddress(BYTE sectorNum);
BYTE upgradeCodePresent(void);
uint8_t writeUpgradeFlag(uint8_t targetId, uint32_t codeSize, uint32_t crc32);
void eraseUpgradeFlag(void);
BYTE eraseUpgradeSectors(void);
BYTE eraseCodeSectors(void);
void eraseUpgradeFlagSector(void);
BYTE eraseUpgradeSectors(void);
BYTE writeUpgradeCompleteFlag(uint8_t flag);
uint32_t getUpgradeStatusFlag(void);
uint8_t writeFlashData(DWORD blockAddress, uint8_t *data, uint32_t bytes);
void writeTestSector(void);
uint8_t iapTestWriteApplicationCode(void);
DWORD iapGetBlockAddress(uint16_t sector, uint16_t block);
uint8_t copySectorData(uint32_t src, uint32_t dest);
uint8_t readCodeDeviceId(void);
uint32_t readCodeCrc32(void);
uint32_t readCodeSize(void);
uint8_t readCodeUpgradeFlag(void);
void iapDebug(uint8_t doDebug);


#endif /*IAP_H_*/
