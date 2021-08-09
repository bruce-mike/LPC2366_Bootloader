#ifndef CRC32_H
#define CRC32_H


DWORD crc32Calculate(void *pBfr, DWORD size);
void crc32Init(DWORD *pCRC);
void crc32AddByte(DWORD *pCRC, BYTE val8);
void crc32End(DWORD *pCRC);
void crc32AddBytes(DWORD *pCRC, BYTE *vals, DWORD numBytes);


#endif
