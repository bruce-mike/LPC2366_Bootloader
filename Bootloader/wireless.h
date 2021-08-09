#ifndef WIRELESS_H
#define WIRELESS_H
void wirelessSetUARTMode(void);
void wirelessSetESNTeachMode(void);
unsigned long wirelessGetOurESN(void);
unsigned short wirelessGetConfig(void);
void wirelessInit(ePLATFORM_TYPE ePlatformType);
void wirelessDoWork(void);
BOOL wirelessDoBondLearn(void);
DWORD wirelessSwapDword( DWORD Data ); 

/////
// wireless port definitions
// arrow board controller use port 0
// handheld used port 1
/////
#define ARROW_BOARD_RF_M0											17
#define ARROW_BOARD_RF_M1											18
#define ARROW_BOARD_RF_M2											19
#define ARROW_BOARD_RF_IO											20
#define ARROW_BOARD_RF_LRN										21
#define ARROW_BOARD_RF_xRST										22
//////
// PINMODE Register 0, port 0 pins 16-26
//  0  0  0  0  0  1  1  1  1  1  2  2  2  2  2  3
//  1  3  5  7  9  1  3  5  7  9  1  3  5  7  9  1
//  1  1  1  1  2  2  2  2  2  2  2
//  6  7  8  9  0  1  2  3  4  5  6
// 00 00 00 00 II LL 00 00 00 00 00 rr rr rr rr rr
// 0000 0000 IILL 0000 0000 00rr rrrr rrrr
/////
#define ARROW_BOARD_RF_LRN_PINMODE_MASK				0x00300000
#define ARROW_BOARD_RF_LRN_PULLUP_ENABLE			0x00000000
#define ARROW_BOARD_RF_LRN_PULLDOWN_ENABLE		0x00300000
#define ARROW_BOARD_RF_LRN_PULLUP_OFF					0x00100000

#define ARROW_BOARD_RF_IO_PINMODE_MASK				0x00C00000
#define ARROW_BOARD_RF_IO_PULLUP_ENABLE				0x00000000
#define ARROW_BOARD_RF_IO_PULLDOWN_ENABLE			0x00C00000
#define ARROW_BOARD_RF_IO_PULLUP_OFF					0x00400000

#define HANDHELD_RF_M0					19
#define HANDHELD_RF_M1					22
#define HANDHELD_RF_M2					25
#define HANDHELD_RF_IO					27
#define HANDHELD_RF_LRN					28
#define HANDHELD_RF_xRST				29
//////
// PINMODE Register 3, port 1 pins 16-31
//  0  0  0  0  0  1  1  1  1  1  2  2  2  2  2  3
//  1  3  5  7  9  1  3  5  7  9  1  3  5  7  9  1
//  1  1  1  1  2  2  2  2  2  2  2  2  2  2  3  3
//  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1
// 00 00 00 00 00 00 00 00 00 00 00 II LL 00 00 00
// 0000 0000 0000 0000 0000 00II LL00 0000
/////
#define HANDHELD_RF_LRN_PINMODE_MASK				0x000000c0
#define HANDHELD_RF_LRN_PULLUP_ENABLE				0x00000000
#define HANDHELD_RF_LRN_PULLDOWN_ENABLE			0x000000c0
#define HANDHELD_RF_LRN_PULLUP_OFF					0x00000080

#define HANDHELD_RF_IO_PINMODE_MASK					0x00000300
#define HANDHELD_RF_IO_PULLUP_ENABLE				0x00000000
#define HANDHELD_RF_IO_PULLDOWN_ENABLE			0x00000300
#define HANDHELD_RF_IO_PULLUP_OFF						0x00000200

#endif		// WIRELESS_H
