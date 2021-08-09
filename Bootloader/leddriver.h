#ifndef LED_DRIVER_H
#define LED_DRIVER_H
//************************
//************************
//************************
//9634 defs
//************************
//************************
// BASE ADDRESSES
#define PCA9634_SYS_LEDS_AND_PWM    0xE4
#define PCA9634_SYS_LEDS_AND_PWM_RD 0xE6
#define PCA9634_CH1_CH8							0xE4
#define PCA9634_CH9_CH16						0xE6

// REGISTER DEFS
#define MODE1				0x00		
#define MODE1_INC		0x80		// auto-inc bit set
#define AUTO_INC	  0x80
#define MODE2				0x01
#define PWM0				0x02	
#define PWM1				0x03
#define PWM2				0x04
#define PWM3				0x05
#define PWM4				0x06
#define PWM5				0x07
#define PWM6				0x08
#define PWM7				0x09
#define GRPPWM			0x0A
#define GRPFREQ			0x0B
#define LEDOUT0			0x8C		// auto-inc bit set
#define LEDOUT1			0x0D
#define SUBADR1			0x0E
#define SUBADR2			0x0F
#define SUBADR3			0x10
#define ALLCALLADR	0x11

// AUX/INDICATOR bit defs
#define AUX			4		// bit locations 5:4 are config bits for LED2 output
#define LED3OUT 6   // bit locs 7:6 are config for LED3 output
#define INDR   	0   // bits 1:0 config LED0
#define INDL    2   // bits 3:2 config LED1

#define LEDDRIVER_SAMPLE_TIME_MS 10

// LED0/1 config bit behaviours
typedef enum eLedConfig
{
	eLED_OFF 				= 0x00,
	eLED_ON					= 0x01,
	eLED_PWM_BRIGHT	= 0x02,
	eLED_GRP_BLINK	= 0x03,
	eLED_ALL_BLINK	= 0xFF 	// related to above, set all ch to blink enable (bit pattern '11')
}eLED_CONFIG_BITS;



#define AUX_OFF				0xCF
#define LED3_OFF      0x3F
#define INDR_OFF			0xFC
#define INDL_OFF			0xF3


// configure this chip as
// LEDn = 1 WHEN OUTDRV = 1 (bit0,1)
// outut = totem pole (bit2)
// blinking (bit5)  (bloomin' bloody blinkin LEDs!)

// U24 SYSTEM LEDS AND BCN+INDICATOR PWM
#define SYS_LEDS_CONFIG_MODEREG1    0x00		//
#define SYS_LEDS_CONFIG_MODEREG2	  0x34		// configure this chip as
																						// ??? This is not the setting
																						//LEDn = 1 WHEN OUTDRV = 1 (bit0,1)
																						// ???
																						// outut = totem pole (bit2)
																						// output sense invert (bit4)
																						// blinking (bit5)  (bloomin' bloody blinkin LEDs!)
																						
																						

#define VLOW_LED				  	0x00 		// LEDOUT1 bit location
#define VLOW_LED_OFF				0xFC
#define VLOW_LED_BLINK      0x03		

#define CHG_LED   					0x02		// LEDOUT1 bit location
#define CHG_LED_OFF					0xF3    // bitmask
#define CHG_LED_BLINK 			0x0C
  
#define SYS_LED					    0x04		// LEDOUT bit location
#define SYS_LED_OFF         0xCF
#define SYS_LED_BLINK       0x30

#define ALRM_LED   			   	0x06  		// LEDOUT1 bit location	
#define ALRM_LED_OFF				0x3F			// bitmask
#define ALRM_LED_BLINK	    0xC0

#define SYS_LEDS_ALL_OFF		0x00
#define SYS_LEDS_ALL_ON			0x55

#define SYS_LEDS_BLINK_ENA	0x30
#define SYS_PWM_FULL_BRIGHT	0xff		// set PWM duty cycle D = value/256
#define BLINK_32FPM					0x2b		// set blink rate T = (val + 1)/24 seconds = 1.875s or 32fpm.  25 < MUTCD < 40FPM. 32FPM = geometric mean
#define GRP_DUTY_50PCT			0x80		// group blinking duty cycle 0x80 = 50%

#define DRIVER_MODE1_CFG		0x80		// auto increment enabled so write to
																		// LEDOUT0 is succeeded by write to LEDOUT1
#define DRIVER_MODE2_CFG 		0x34  	// LEDn = 1 when OUTDRV = 1
																		// totem pole output
																		// output sense invert
																		// blinking
																	
																					
#define DRIVER_ALL_OFF			0x00
#define DRIVER_ALL_ON				0x55


void ledDriverInit(void);
void ledDriverDoWork(void);
void ledDriverSetComps(eLED_CONFIG_BITS eNew);
void ledDriverSetRDR(eLED_CONFIG_BITS eNew);
void ledDriverSetAux0(eLED_CONFIG_BITS eNew);
void ledDriverSetAux1(eLED_CONFIG_BITS eNew);
void ledDriverSetLedVlow(eLED_CONFIG_BITS eNew);
void ledDriverSetLedChrgr(eLED_CONFIG_BITS eNew);
void ledDriverSetLedSyst(eLED_CONFIG_BITS eNew);
void ledDriverSetLedAlarm(eLED_CONFIG_BITS eNew);
void ledDriverSetRow0(eLED_CONFIG_BITS eNew);
void ledDriverSetRow1(eLED_CONFIG_BITS eNew);
void ledDriverSetRow2(eLED_CONFIG_BITS eNew);
void ledDriverSetRow3(eLED_CONFIG_BITS eNew);
void ledDriverSetFan0(eLED_CONFIG_BITS eNew);
void ledDriverSetFan1(eLED_CONFIG_BITS eNew);
void ledDriverSetGps(eLED_CONFIG_BITS eNew);
void ledDriverSetModem(eLED_CONFIG_BITS eNew);
unsigned short ledDriverGetAuxStatus(void);
//unsigned short ledDriverGetAux1Status(void);
#endif		// LED_DRIVER_H
