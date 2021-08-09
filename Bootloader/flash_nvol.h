**************************************************************************
MODULE:       FLASH_NVOL
CONTAINS:     Storage of non-volatile variables in Flash memory
DEVELOPED BY: Embedded Systems Academy, Inc. 2010
              www.esacademy.com
COPYRIGHT:    NXP Semiconductors, 2010. All rights reserved.
VERSION:      1.10
***************************************************************************/ 

#ifndef _FLASHNVOL_
#define _FLASHNVOL_
#include "shareddefs.h"
//#include "sharedinterface.h"

#define COUNTER 1

/**************************************************************************
DOES:    Initializes access to non-volatile memory
RETURNS: TRUE for success, FALSE for error
**************************************************************************/
BOOL NVOL_Init
  (
  void
  );

/**************************************************************************
DOES:    Sets the value of a variable
RETURNS: TRUE for success, FALSE for error
**************************************************************************/
BOOL NVOL_SetVariable
  (
  WORD Id,				                           // id for variable
  BYTE *Value,										   // variable data
  WORD Size										   // size of data in bytes
  );

/**************************************************************************
DOES:    Gets the value of a variable
RETURNS: TRUE for success, FALSE for error
**************************************************************************/
BOOL NVOL_GetVariable
  (
  WORD Id,						                   // id of variable
  BYTE *Value,										   // location to store variable data
  WORD Size										   // size of variable in bytes
  );

#endif
