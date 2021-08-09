#ifndef LM73_H
#define LM73_H

#define LM73_SAMPLE_TIME_MS     2000

#define DEG_F           0
#define DEG_C           1


//#define LM75_ADDR          	0x90 //1001 a2 a1 a0 r/w
//#define READ_WRITE0x01
//#define RD_BIT        0x01
//#define LM75_RD       0x91

//LM73 DEFINES
#define LM73_WRITE_ADDR			 0x94
#define LM73_READ_ADDR			 0x95

// LM73 REGISTERS
#define TEMP_REG        0x00
#define CONFIG_REG      0x01
#define T_HIGH          0x02
#define T_LOW           0x03
#define CONTROL         0x04
#define SQUAWK_IDENT    0x07

// LM73 Control bits
#define LM73_TIME_OUT_DISABLE       0x80
#define LM73_RESOLUTION_11_BITS     0x00
#define LM73_RESOLUTION_12_BITS     0x20
#define LM73_RESOLUTION_13_BITS     0x40
#define LM73_RESOLUTION_14_BITS     0x60
#define LM73_ALERT_ON_HIGH          0x08 // read, 0 when the ~ALERT output pin is low
#define LM73_TEMPERATURE_HIGH       0x04 // read, 1 when measured temperature exceeds tHigh
#define LM73_TEMPERATURE_LOW        0x02 // read, 1 when measured temperature falls below tLow
#define LM73_DATA_AVAILABLE         0x01 // read, 1 when data is available

void lm73Init(void);
void lm73DoWork(void);
int lm73GetTempReading(BYTE tempUnit);

#endif		// LM73_H
