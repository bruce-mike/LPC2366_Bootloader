///////////////////////////////////////////////////////
// stored configuration data
///////////////////////////////////////////////////////
#include "LPC23xx.H"                   /* LPC23xx definitions               */
#include "shareddefs.h"
#include <stdio.h>
#include <string.h>
#include "queue.h"
#include "sharedinterface.h"
#include "timer.h"
#include "storedconfig.h"
#include "spiflash.h"

/////
// 4k sector size
// both sectors are the same size
/////
#define SECTOR_SIZE 				0x1000
#define STORED_INFO_ADDRESS_SECTOR_1 0x000000
#define STORED_INFO_ADDRESS_SECTOR_2 0x001000

// sector flags
#define SECTOR_EMPTY        0xFFFFFFFF
#define SECTOR_INITIALIZING 0xAAFFFFFF
#define SECTOR_VALID        0xAAAAFFFF
#define SECTOR_INVALID      0xAAAAAAFF


// structure flags
#define STRUCT_EMPTY        0xFFFFFFFF
#define STRUCT_INITIALIZING 0xAAFFFFFF
#define STRUCT_VALID        0xAAAAFFFF
#define STRUCT_INVALID      0xAAAAAAFF

#define INVALID_DATA_OFFSET SECTOR_SIZE

static volatile DWORD mNextFreeOffset;
static unsigned int mMinimumDataSize;
// pointer to valid sector
static volatile DWORD mValidSector;
static STORED_INFO theStoredInfo;
static ePLATFORM_TYPE eOurPlatformType;

// maximum number of variables supported
// must be less than ( (SECTOR_SIZE - 48) / (MAX_VARIABLE_SIZE + 4) )
#define MAX_VARIABLES 100
static DWORD storedConfig_NVOL_InitSectors(void);
static BOOL storedConfig_NVOL_SetSectorFlags(DWORD nSector, DWORD Flags);
static BOOL storedConfig_NVOL_EraseSector(DWORD nSector);
static BOOL storedConfig_NVOL_SwapSectors(DWORD nSrcSector,DWORD nDstSector);
static DWORD storedConfig_NVOL_GetNextFreeOffset(DWORD nSector);
static DWORD storedConfig_NVOL_GetCurrentStructure(DWORD nSector, STORED_INFO* pStoredInfoStruct);


static void storedConfigWrite()
{
	STORED_INFO testStruct;
	DWORD nPreviousOffset = storedConfig_NVOL_GetCurrentStructure(mValidSector, &testStruct);
	DWORD nPreviousSector = mValidSector;
	DWORD nOffset = storedConfig_NVOL_GetNextFreeOffset(mValidSector);
//printf("1-storedConfigWrite nPreviousOffset[%lX] nPreviousSector[%lX] nOffset[%lX] mValidSector[%lX]\n", nPreviousOffset, nPreviousSector, nOffset, mValidSector);
	if(INVALID_DATA_OFFSET == nOffset)
	{
		//printf("2-storedConfigWrite\n");
		/////
		// this sector is full, so switch to the other one
		// mValidSector is set in storedConfig_NVOL_SwapSectors
		/////
		if(STORED_INFO_ADDRESS_SECTOR_1 == mValidSector)
		{
			storedConfig_NVOL_SwapSectors(STORED_INFO_ADDRESS_SECTOR_1, STORED_INFO_ADDRESS_SECTOR_2);
		}
		else
		{
			storedConfig_NVOL_SwapSectors(STORED_INFO_ADDRESS_SECTOR_2, STORED_INFO_ADDRESS_SECTOR_1);
		}
		nPreviousOffset = mMinimumDataSize;
		nPreviousSector = mValidSector;
		//printf("3-storedConfigWrite mValidSector[%lX]\n", mValidSector);
	}
	/////
	// get the next free offset
	// write the data
	// and update the flags
	/////
	mNextFreeOffset = storedConfig_NVOL_GetNextFreeOffset(mValidSector);
	//printf("4-storedConfigWrite mNextFreeOffset[%lX] mValidSector[%lX]\n", mNextFreeOffset, mValidSector);
	theStoredInfo.nStructFlags = STRUCT_VALID;
	spiFlashWriteData((BYTE *)&theStoredInfo, (mValidSector+mNextFreeOffset), sizeof(STORED_INFO), eOurPlatformType);
		
	if(INVALID_DATA_OFFSET != nPreviousOffset)
	{
		//printf("5-storedConfigWrite nPreviousSector[%lX] nPreviousOffset[%lX]\n", nPreviousSector, nPreviousOffset);
		/////
		// update the flags on the previous structure
		/////
		testStruct.nStructFlags = STRUCT_INVALID;
		spiFlashWriteData((BYTE *)&testStruct, (nPreviousSector+nPreviousOffset), sizeof(STORED_INFO), eOurPlatformType);
	}
}


//===================================
// initialization
//===================================
STORED_INFO* storedConfigInit(ePLATFORM_TYPE ePlatformType)
{
	DWORD nCurrentOffset;
	
	/////
	// minimum data size must be a factor of 256
	// so our writes do not cross page boundaries
	/////
	int nTest = sizeof(STORED_INFO)/16;
	mMinimumDataSize = sizeof(STORED_INFO);
	if((16*nTest) != sizeof(STORED_INFO))
	{
		mMinimumDataSize = (16*(nTest+1));
	}

	eOurPlatformType = ePlatformType;

	mValidSector = storedConfig_NVOL_InitSectors();
	nCurrentOffset = storedConfig_NVOL_GetCurrentStructure(mValidSector, &theStoredInfo);
	if((INVALID_DATA_OFFSET == nCurrentOffset) || (STRUCT_VALID != theStoredInfo.nStructFlags))
	{
		/////
		// the flash config data has not been initialized
		// so do it here
		/////
		memset((unsigned char*)&theStoredInfo, 0, sizeof(theStoredInfo));
		
		storedConfigWrite();
	}
	return &theStoredInfo;
}

//===================================
// get routines
//===================================
unsigned int storedConfigGetSendersESN()
{
	return theStoredInfo.nSendersESN;
}
unsigned int storedConfigGetDisallowedPatterns()
{
	return theStoredInfo.nDisallowedPatterns;
}
eACTUATOR_TYPES storedConfigGetActuatorType()
{
	return theStoredInfo.eActuatorType;
}
eBRIGHTNESS_CONTROL storedConfigGetArrowBoardBrightnessControl()
{
	return theStoredInfo.eBrightnessControl;
}
unsigned int storedConfigGetDefaultUI()
{
	return theStoredInfo.nDefaultUI;
}
eAUX_BATTERY_TYPES storedConfigGetAuxBatteryType()
{
	return theStoredInfo.eAuxBatteryType;
}
eACTUATOR_BUTTON_MODE storedConfigGetActuatorButtonMode()
{
	return theStoredInfo.eActuatorButtonMode;
}
int storedConfigGetActuatorUpLimit()
{
	return theStoredInfo.nActuatorUpLimit;
}
int storedConfigGetActuatorDownLimit()
{
	return theStoredInfo.nActuatorDownLimit;
}

void storedConfigGetTPCalCoefficients(TP_MATRIX *pTPCalCoefficients)
{
	memcpy((unsigned char*)pTPCalCoefficients, (unsigned char*)&theStoredInfo.tpCalCoefficients, sizeof(TP_MATRIX));	
}

unsigned long storedConfigGetPCUSerialNumber(void)
{
	return theStoredInfo.nPCUSerialNumber;
}

//===================================
// set routines
//===================================
void storedConfigWriteNewInfo(STORED_INFO *pNewStoredInfo)
{
	memcpy((unsigned char*)&theStoredInfo, (unsigned char*)pNewStoredInfo, sizeof(STORED_INFO));
	storedConfigWrite();
}
void storedConfigSetSendersESN(unsigned int nESN)
{
	theStoredInfo.nSendersESN = nESN;
	storedConfigWrite();
}
void storedConfigSetDisallowedPatterns(unsigned int nPatterns)
{
	theStoredInfo.nDisallowedPatterns = nPatterns;
	storedConfigWrite();
}
void storedConfigSetActuatorType(eACTUATOR_TYPES eActuatorType)
{
	theStoredInfo.eActuatorType = eActuatorType;
	storedConfigWrite();
}
void storedConfigSetArrowBoardBrightnessControl(eBRIGHTNESS_CONTROL eBrightnessControl)
{
	theStoredInfo.eBrightnessControl = eBrightnessControl;
	storedConfigWrite();
}
void storedConfigSetDefaultUI(unsigned int nDefaultUI)
{
	theStoredInfo.nDefaultUI = nDefaultUI;
	storedConfigWrite();
}
void storedConfigSetAuxBatteryType(eAUX_BATTERY_TYPES eAuxBatteryType)
{
	theStoredInfo.eAuxBatteryType = eAuxBatteryType;
	storedConfigWrite();
}
void storedConfigSetActuatorButtonMode(eACTUATOR_BUTTON_MODE eActuatorButtonMode)
{
	theStoredInfo.eActuatorButtonMode = eActuatorButtonMode;
	storedConfigWrite();
}
void storedConfigSetActuatorUpLimit(int nActuatorUpLimit)
{
	theStoredInfo.nActuatorUpLimit = nActuatorUpLimit;
	storedConfigWrite();
}
void storedConfigSetActuatorDownLimit(int nActuatorDownLimit)
{
	theStoredInfo.nActuatorDownLimit = nActuatorDownLimit;
	storedConfigWrite();
}

void storedConfigSetTPCalCoefficients(TP_MATRIX *pTPCalCoefficients)
{
	memcpy((unsigned char*)&theStoredInfo.tpCalCoefficients, (unsigned char*)pTPCalCoefficients, sizeof(TP_MATRIX));
	
	storedConfigWrite();
}

void storedConfigSetPCUSerialNumber(unsigned long nPCUSerialNumber)
{
	theStoredInfo.nPCUSerialNumber = nPCUSerialNumber;
	
	storedConfigWrite();	
}


//============================================================================================
// copied from flash_nvol.c
// and modified to work with our structures and spiflash code
//============================================================================================
/**************************************************************************
MODULE:       FLASH_NVOL
CONTAINS:     Storage of non-volatile variables in Flash memory
DEVELOPED BY: Embedded Systems Academy, Inc. 2010
              www.esacademy.com
COPYRIGHT:    NXP Semiconductors, 2010. All rights reserved.
VERSION:      1.10
***************************************************************************/
/**************************************************************************
DOES:    Identifies which sector is the valid one and completes any
         partially completed operations that may have been taking place
		 before the last reset
RETURNS: The valid sector or null for error
**************************************************************************/
static DWORD storedConfig_NVOL_InitSectors(void)
{
	DWORD Sector1Flags;
	DWORD Sector2Flags;
	
	/////
	// get the sector flags
	/////
	spiFlashReadData((BYTE*)&Sector1Flags, (DWORD)STORED_INFO_ADDRESS_SECTOR_1, (UINT)sizeof(Sector1Flags), (BYTE)SPI_FLASH_READ_DATA_NORMAL, eOurPlatformType);
	spiFlashReadData((BYTE*)&Sector2Flags, (DWORD)STORED_INFO_ADDRESS_SECTOR_2, (UINT)sizeof(Sector2Flags), (BYTE)SPI_FLASH_READ_DATA_NORMAL, eOurPlatformType);
//printf("1-NVOL_Init 1Flags[%lX] 2Flags[%lX]\n", Sector1Flags, Sector2Flags);
  // if sector 1 has invalid flags then erase it
  if ((Sector1Flags != SECTOR_EMPTY)        &&
      (Sector1Flags != SECTOR_INITIALIZING) &&
			(Sector1Flags != SECTOR_VALID)        &&
			(Sector1Flags != SECTOR_INVALID))
  {
		storedConfig_NVOL_EraseSector(STORED_INFO_ADDRESS_SECTOR_1);
		Sector1Flags = SECTOR_EMPTY;
  }

  // if sector 2 has invalid flags then erase it
  if ((Sector2Flags != SECTOR_EMPTY)        &&
      (Sector2Flags != SECTOR_INITIALIZING) &&
			(Sector2Flags != SECTOR_VALID)        &&
			(Sector2Flags != SECTOR_INVALID))
  {
		storedConfig_NVOL_EraseSector(STORED_INFO_ADDRESS_SECTOR_2);

		Sector2Flags = SECTOR_EMPTY;
  }

  // what happens next depends on status of both sectors
  switch (Sector1Flags)
  {
    case SECTOR_EMPTY:
	  switch (Sector2Flags)
	  {
	    // sector 1 empty, sector 2 empty
	    case SECTOR_EMPTY:
	      // use sector 1
	      storedConfig_NVOL_SetSectorFlags(STORED_INFO_ADDRESS_SECTOR_1, SECTOR_VALID);
	      mNextFreeOffset = mMinimumDataSize;
	      return STORED_INFO_ADDRESS_SECTOR_1;
		// sector 1 empty, sector 2 initializing
		case SECTOR_INITIALIZING:
	      // use sector 2
	      storedConfig_NVOL_SetSectorFlags(STORED_INFO_ADDRESS_SECTOR_2, SECTOR_VALID);
	      mNextFreeOffset = mMinimumDataSize;
  	      return STORED_INFO_ADDRESS_SECTOR_2;
		// sector 1 empty, sector 2 valid
		case SECTOR_VALID:
          mNextFreeOffset = storedConfig_NVOL_GetNextFreeOffset(STORED_INFO_ADDRESS_SECTOR_2);
	      // sector 2 is already active
	      return STORED_INFO_ADDRESS_SECTOR_2;
		// sector 1 empty, sector 2 invalid
		case SECTOR_INVALID:
	      // swap sectors 2 -> 1
	      storedConfig_NVOL_SwapSectors(STORED_INFO_ADDRESS_SECTOR_2, STORED_INFO_ADDRESS_SECTOR_1);
        mNextFreeOffset = storedConfig_NVOL_GetNextFreeOffset(STORED_INFO_ADDRESS_SECTOR_1);
	      // use sector 1
	      return STORED_INFO_ADDRESS_SECTOR_1;
	  }
	  break;

	case SECTOR_INITIALIZING:
	  switch (Sector2Flags)
	  {
	    // sector 1 initializing, sector 2 empty
	    case SECTOR_EMPTY:
	      // use sector 1
	      storedConfig_NVOL_SetSectorFlags(STORED_INFO_ADDRESS_SECTOR_1, SECTOR_VALID);
	      mNextFreeOffset = mMinimumDataSize;
	      return STORED_INFO_ADDRESS_SECTOR_1;
		// sector 1 initializing, sector 2 initializing
		case SECTOR_INITIALIZING:
	      // erase sector 2
	      storedConfig_NVOL_EraseSector(STORED_INFO_ADDRESS_SECTOR_2);
	      // use sector 1
	      storedConfig_NVOL_SetSectorFlags(STORED_INFO_ADDRESS_SECTOR_1, SECTOR_VALID);
	      mNextFreeOffset = mMinimumDataSize;
	      return STORED_INFO_ADDRESS_SECTOR_1;
		// sector 1 initializing, sector 2 valid
		case SECTOR_VALID:
	      // erase sector 1
	      storedConfig_NVOL_EraseSector(STORED_INFO_ADDRESS_SECTOR_1);
        // swap sectors 2 -> 1
	      storedConfig_NVOL_SwapSectors(STORED_INFO_ADDRESS_SECTOR_2, STORED_INFO_ADDRESS_SECTOR_1);
        mNextFreeOffset = storedConfig_NVOL_GetNextFreeOffset(STORED_INFO_ADDRESS_SECTOR_1);
	      // use sector 1
	      return STORED_INFO_ADDRESS_SECTOR_1;
		// sector 1 initializing, sector 2 invalid
		case SECTOR_INVALID:
	      // erase sector 2
	      storedConfig_NVOL_EraseSector(STORED_INFO_ADDRESS_SECTOR_2);
	      // use sector 1
	      storedConfig_NVOL_SetSectorFlags(STORED_INFO_ADDRESS_SECTOR_1, SECTOR_VALID);
	      mNextFreeOffset = mMinimumDataSize;
	      return STORED_INFO_ADDRESS_SECTOR_1;
	  }
	  break;

	case SECTOR_VALID:
	  switch (Sector2Flags)
	  {
	    // sector 1 valid, sector 2 empty
	    case SECTOR_EMPTY:
          mNextFreeOffset = storedConfig_NVOL_GetNextFreeOffset(STORED_INFO_ADDRESS_SECTOR_1);
	      // sector 1 is active
	      return STORED_INFO_ADDRESS_SECTOR_1;
		// sector 1 valid, sector 2 initializing
		case SECTOR_INITIALIZING:
	      // erase sector 2
	      storedConfig_NVOL_EraseSector(STORED_INFO_ADDRESS_SECTOR_2);
        // swap sectors 1 -> 2	  
	      storedConfig_NVOL_SwapSectors(STORED_INFO_ADDRESS_SECTOR_1, STORED_INFO_ADDRESS_SECTOR_2);
        mNextFreeOffset = storedConfig_NVOL_GetNextFreeOffset(STORED_INFO_ADDRESS_SECTOR_2);
	      // use sector 2
	      return STORED_INFO_ADDRESS_SECTOR_2;
		// sector 1 valid, sector 2 valid
		case SECTOR_VALID:
	      // erase sector 2 and use sector 1
	      storedConfig_NVOL_EraseSector(STORED_INFO_ADDRESS_SECTOR_2);
        mNextFreeOffset = storedConfig_NVOL_GetNextFreeOffset(STORED_INFO_ADDRESS_SECTOR_1);
	      return STORED_INFO_ADDRESS_SECTOR_1;
		// sector 1 valid, sector 2 invalid
		case SECTOR_INVALID:
	      // erase sector 2 and use sector 1
	      storedConfig_NVOL_EraseSector(STORED_INFO_ADDRESS_SECTOR_2);
        mNextFreeOffset = storedConfig_NVOL_GetNextFreeOffset(STORED_INFO_ADDRESS_SECTOR_1);
	      return STORED_INFO_ADDRESS_SECTOR_1;
	  }
	  break;

	case SECTOR_INVALID:
	  switch (Sector2Flags)
	  {
	    // sector 1 invalid, sector 2 empty
	    case SECTOR_EMPTY:
	      // swap sectors 1 -> 2
	      storedConfig_NVOL_SwapSectors(STORED_INFO_ADDRESS_SECTOR_1, STORED_INFO_ADDRESS_SECTOR_2);
	      // use sector 2
        mNextFreeOffset = storedConfig_NVOL_GetNextFreeOffset(STORED_INFO_ADDRESS_SECTOR_2);
	      return STORED_INFO_ADDRESS_SECTOR_2;
		// sector 1 invalid, sector 2 initializing
		case SECTOR_INITIALIZING:
	      // erase sector 1
	      storedConfig_NVOL_EraseSector(STORED_INFO_ADDRESS_SECTOR_2);
	      // use sector 2
	      storedConfig_NVOL_SetSectorFlags(STORED_INFO_ADDRESS_SECTOR_2, SECTOR_VALID);
	      mNextFreeOffset = mMinimumDataSize;
	      return STORED_INFO_ADDRESS_SECTOR_2;
		// sector 1 invalid, sector 2 valid
		case SECTOR_VALID:
	      // erase sector 1
	      storedConfig_NVOL_EraseSector(STORED_INFO_ADDRESS_SECTOR_1);
        mNextFreeOffset = storedConfig_NVOL_GetNextFreeOffset(STORED_INFO_ADDRESS_SECTOR_2);
	      // use sector 2
	      return STORED_INFO_ADDRESS_SECTOR_2;
		// sector 1 invalid, sector 2 invalid
		case SECTOR_INVALID:
	      // both sectors invalid so erase both and use sector 1
	      storedConfig_NVOL_EraseSector(STORED_INFO_ADDRESS_SECTOR_1);
	      storedConfig_NVOL_EraseSector(STORED_INFO_ADDRESS_SECTOR_2);
	      storedConfig_NVOL_SetSectorFlags(STORED_INFO_ADDRESS_SECTOR_1, SECTOR_VALID);
	      mNextFreeOffset = mMinimumDataSize;
	      return STORED_INFO_ADDRESS_SECTOR_1;
	  }
	  break;
  }

  return 0;
}


/**************************************************************************
DOES:    Updates the flags for a sector
RETURNS: TRUE for success, FALSE for error
**************************************************************************/
static BOOL storedConfig_NVOL_SetSectorFlags(DWORD nSector,DWORD Flags)
{
		spiFlashWriteData((BYTE *)&Flags, nSector, (UINT)sizeof(Flags), eOurPlatformType);

  return TRUE;
}
/**************************************************************************
DOES:    Erases a sector
RETURNS: TRUE for success, FALSE for error
**************************************************************************/
static BOOL storedConfig_NVOL_EraseSector(DWORD nSector)
{
	spiFlashErase((BYTE)SPI_FLASH_SECTOR_ERASE, nSector, eOurPlatformType);
  return TRUE; 
}
/**************************************************************************
DOES:    Moves the data from one sector to another, removing old entries
RETURNS: TRUE for success, FALSE for error
**************************************************************************/
static BOOL storedConfig_NVOL_SwapSectors(DWORD nSrcSector,DWORD nDstSector)
{
  DWORD nSrcOffset;
  DWORD nDstOffset = mMinimumDataSize;
	STORED_INFO srcStruct;

  // make sure destination sector is erased
	storedConfig_NVOL_EraseSector(nDstSector);

  // mark destination sector as being initialized
  storedConfig_NVOL_SetSectorFlags(nDstSector, SECTOR_INITIALIZING);
  nDstOffset = mMinimumDataSize;
	
	/////
	// locate valid record in source sector
	/////
	nSrcOffset = storedConfig_NVOL_GetCurrentStructure(nSrcSector, &srcStruct);
	if(INVALID_DATA_OFFSET != nSrcOffset)
	{
		/////
		// copy this to dest sector
		/////
		spiFlashWriteData((BYTE *)&srcStruct, nDstSector+nDstOffset, (UINT)sizeof(srcStruct), eOurPlatformType);

		// mark destination sector as being valid
		storedConfig_NVOL_SetSectorFlags(nDstSector, SECTOR_VALID); 
		// erase source sector
		storedConfig_NVOL_EraseSector(nSrcSector);
		// now using destination sector
		mValidSector = nDstSector;
		// get next free location in destination sector
		mNextFreeOffset = storedConfig_NVOL_GetNextFreeOffset(nDstSector);
	}
	else
	{
		/////
		// there was no valid data in the source sector
		// so make the dest sector the current valid one
		/////
		storedConfig_NVOL_SetSectorFlags(nDstSector, SECTOR_VALID); 
		// erase source sector
		storedConfig_NVOL_EraseSector(nSrcSector);
		// now using destination sector
		mValidSector = nDstSector;
	}

  return TRUE;
}

/**************************************************************************
DOES:    Gets the offset of the next free location in a sector
RETURNS: Returns offset (offset = INVALID_DATA_OFFSET when sector is full)
**************************************************************************/
static DWORD storedConfig_NVOL_GetNextFreeOffset(DWORD nSector)
{
	DWORD nOffset = mMinimumDataSize;
	STORED_INFO testStruct;
	while(1)
	{
		spiFlashReadData((BYTE*)&testStruct, nSector+nOffset, (UINT)sizeof(STORED_INFO), (BYTE)SPI_FLASH_READ_DATA_NORMAL, eOurPlatformType);
//printf("f[%X] o[%lX]\n", testStruct.nStructFlags, nOffset);
		if(STRUCT_EMPTY == testStruct.nStructFlags)
		{
			break;
		}
		nOffset += mMinimumDataSize;
		if(SECTOR_SIZE <= (nOffset+mMinimumDataSize))
		{
			nOffset = INVALID_DATA_OFFSET;
			break;
		}
	}
	return nOffset;
}
/**************************************************************************
DOES:    Locate the valid structure in the specified sector
RETURNS: offset into the current sector or INVALID_DATA_OFFSET if not found
         Also, the data is returned to the structure referenced by pStoredInfoStruct
**************************************************************************/
static DWORD storedConfig_NVOL_GetCurrentStructure(DWORD nSector, STORED_INFO* pStoredInfoStruct)
{
  DWORD nOffset = mMinimumDataSize;
  while(1)
	{
		spiFlashReadData((BYTE*)pStoredInfoStruct, nSector+nOffset, (UINT)sizeof(STORED_INFO), (BYTE)SPI_FLASH_READ_DATA_NORMAL, eOurPlatformType);	
		if(STRUCT_VALID == pStoredInfoStruct->nStructFlags)
		{
			break;
		}
		nOffset += mMinimumDataSize;
		if(SECTOR_SIZE <= (nOffset+mMinimumDataSize))
		{
			nOffset = INVALID_DATA_OFFSET;
			break;
		}
	}

  return nOffset;
}


//===============================================================================================
//===============================================================================================


