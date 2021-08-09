//===================================================================================
//   crc32.c
//===================================================================================


#include "shareddefs.h"


// 0xEDB88320L represents the coefficients of the CRC32 polynomial as 1's
// where there is an x^n term (except for the x^32 term, which is left out).
#define CRC32_POLY  0xEDB88320L



//=====================================
void
crc32Init(DWORD *pCRC)
//=====================================
{
	*pCRC = 0xffffffff;
}
 
//===========================================
void
crc32AddByte(DWORD *pCRC, BYTE val8)
//===========================================
{
    DWORD i, poly;
    DWORD entry;
		DWORD crcIn;
    DWORD crcOut;

    crcIn = *pCRC;
    poly = CRC32_POLY;

    entry = (crcIn ^ ((DWORD)val8)) & 0xFF;

    for (i=0; i<8; i++)
    {
	    if (entry & 1)
        {
	        entry = (entry >> 1) ^ poly;
        }
        else
		{
            entry >>= 1;
		}
   }

   crcOut = ((crcIn>>8) & 0x00FFFFFF) ^ entry;

   *pCRC = crcOut;
}


//======================================================
void
crc32AddBytes(DWORD *pCRC, BYTE *vals, DWORD numBytes)
//======================================================
{
	while(numBytes)
	{
		crc32AddByte(pCRC, *vals);
		vals++;
		numBytes--;
	}
}	
		
 
//==============================
void
crc32End(DWORD *pCRC)
//==============================
{
	*pCRC ^= 0xffffffff;
}
 
//===========================================
DWORD
crc32Calculate(void *pBfr, DWORD size)
//===========================================
{
	DWORD crc32;
    BYTE  *pu8;

    crc32Init(&crc32);
    pu8 = (BYTE *) pBfr;

    while (size-- != 0)
    {
		crc32AddByte(&crc32, *pu8);
        pu8++ ;
    }

    crc32End(&crc32);

    return(crc32);
}
 
