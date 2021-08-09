#ifndef ADS7828_H
#define ADS7828_H
//************************
//************************
//************************
//ADS 7828 defs (I2C ADC)
//************************
//************************
// driver ch (1-8) and (9-16)
#define AMPS_PER_BIT_DRIVERS (0.000814)

// solar/alt + battery 
#define AMPS_PER_BIT_ALT_AND_BATT (0.006726)

// 12V subsystem (beacons, indicators, aux, user...)
#define AMPS_PER_BIT_12V (0.002024) 

#define ADS7828_BASE 	0x90
#define ADS7828_RD   	0x91

// channel 'enums'
#define ID1					0
#define ID2         1
#define IB					2
#define IX          3
#define GND         4
#define IA          5
#define IL          6
#define IS          7


#define ADC_ID1			0x8C 	// single-ended, internal ref ON, ADC ON. Drivers 1-8
#define ADC_ID2 		0xCC	// ditto, drivers 9-16
#define ADC_IB 			0x9C	// beacons n indicators
#define ADC_IX 			0xDC	// hour, strobe, lbar, user
#define ADC_GND 		0xAC	
#define ADC_IA 			0xEC	// alternator/solar
#define ADC_IL 			0xBC	// battery
#define ADC_IS 			0xFC	// 12v subsystem (lamp drivers, beacon, indicator, user, hour, lbar, strobe

#define ADS7828_SAMPLE_TIME_MS  250
void ADS7828Init(void);
void ADS7828DoWork(void);

unsigned short nADS7828GetIL(void);
unsigned short nADS7828GetIS(void);

#endif		// ADS7828_H
  
