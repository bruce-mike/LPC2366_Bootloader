#ifndef PCU_POWER_CONTROL_H
#define PCU_POWER_CONTROL_H
//************************
//************************
//************************
//9634 defs
//************************
//************************
// BASE ADDRESSES
#define PCA9634_SYS_POWER_CONTROL_0 0xE4
#define PCA9634_SYS_POWER_CONTROL_1 0xE6

#define PCA9634_SYS_POWER_CONTROL_SWRST 0x0C
#define SWRST_BYTE_1	0xA5
#define SWRST_BYTE_2	0x5A

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

////////
// MODE 1 register bit offsets
//////
#define	ALLCALL_BIT	0x00
#define	SUB3_BIT		0x01
#define	SUB2_BIT		0x02
#define SUB1_BIT		0x03
#define	SLEEP_BIT		0x04
#define AI0_BIT			0x05
#define AI1_BIT			0x06
#define AI2_BIT			0x07

//////
// MODE2 register bit offsets
/////
#define OUTNE_BIT		0x00
#define OUTDRV_BIT	0x02
#define OCH_BIT			0x03
#define INVRT_BIT		0x04
#define DMBLNK_BIT	0x05

//////
// MODE2 DMBLNK bit defintions
//////
#define DMBLNK_DIMMING	0x00<<DMBLNK_BIT
#define DMBLNK_BLINKING	0x01<<DMBLNK_BIT

//////
// MODE2 INVRT bit definitions
/////
#define INVRT_NOT_INVERTED 0x00<<INVRT_BIT
#define INVRT_INVERTED 0x01<<INVRT_BIT
//////
// MODE2 OUTDRV bit definitions
/////
#define OUTDRV_OPEN_DRAIN	0x00<<OUTDRV_BIT
#define OUTDRV_TOTEM_POLE	0x01<<OUTDRV_BIT
//////
// MODE2 OUTNE bit definitions
/////
#define OUTNE_xOE_LED_0	0x00<<OUTNE_BIT
#define OUTNE_xOE_LED_1	0x01<<OUTNE_BIT							// when OUTDRV = 1
#define OUTNE_xOE_LED_H	0x10<<OUTNE_BIT


// configure this chip as
// LEDn = 1 WHEN OUTDRV = 1 (bit0,1)
// outut = totem pole (bit2)
// blinking (bit5)  (bloomin' bloody blinkin LEDs!)

// U24 SYSTEM LEDS AND BCN+INDICATOR PWM
#define SYS_POWER_CONTROL_MODEREG1    					0x00	// configure this chip as: 
																											// AI disabled, AI1bit=0,AI2bit=0, 
																											// Normal mode
																											// does not respond to any subaddress
																											// does not respond to LED All Call
#//define SYS_POWER_CONTROL_MODEREG2_NON_INVERTING 0x24	// configure this chip as:
																											// Blinking, 
																											// Not inverted output
																											// for REV E PCU and beyond

//#define SYS_POWER_CONTROL_MODEREG2						   0x35	// configure this chip as:
#define SYS_POWER_CONTROL_MODEREG2_REVC						DMBLNK_BLINKING|INVRT_INVERTED|OUTDRV_TOTEM_POLE|OUTNE_xOE_LED_0
																											// Blinking, 
																											// Inverted output
																											// Totem pole
																											// ~OE = outputs are low (instead of high Z)
																											// this means that the LEDs will come on when ~OE
																											// but the power outputs will be off
																											//
																											// inverted output explanation, 
																											// in normal operation, a 1 turns on the LED, LED output is 0
																											// inverted output means a 1 turns on our power supply, LED output is 1
																											// for the RevE PCU with the inverters, we switch the meaning of the bits we send to the PCA9634

#define SYS_POWER_CONTROL_MODEREG2_REVE						DMBLNK_BLINKING|INVRT_INVERTED|OUTDRV_TOTEM_POLE|OUTNE_xOE_LED_1
																											// Blinking, 
																											// Inverted output
																											// Totem pole
																											// ~OE = outputs are high (instead of high Z)
																											// this means that the LEDs and power outputs will be off when ~OE
																											//
																											// inverted output explanation, 
																											// in normal operation, a 1 turns on the LED, LED output is 0
																											// inverted output means a 1 turns on our power supply, LED output is 1
																											// for the RevE PCU with the inverters, we switch the meaning of the bits we send to the PCA9634


// AUX/INDICATOR bit defs
#define AUX			4		// bit locations 5:4 are config bits for LED2 output
#define LED3OUT 6   // bit locs 7:6 are config for LED3 output
#define INDR   	0   // bits 1:0 config LED0
#define INDL    2   // bits 3:2 config LED1

#define POWER_CONTROL_SAMPLE_TIME_MS 10

// LED0/1 config bit behaviours
typedef enum ePcuPowerControl
{    
    ePOWER_OUTPUT_OFF   = 0x00,
    ePOWER_OUTPUT_ON  = 0x01,
    ePOWER_OUTPUT_INIT = 0x02
}ePCU_POWER_CONTROL;

typedef enum ePcuPowerControlSetOutput
{
	ePCU_SET_ROW0_OUTPUT = 0,	
	ePCU_SET_ROW1_OUTPUT,		
	ePCU_SET_ROW2_OUTPUT,		
	ePCU_SET_ROW3_OUTPUT,		
	ePCU_SET_FAN0_OUTPUT,		
	ePCU_SET_FAN1_OUTPUT,		
	ePCU_SET_GPS_OUTPUT,		
	ePCU_SET_MODEM_OUTPUT,		
	ePCU_SET_DCU_OUTPUT,	
	ePCU_SET_RADAR_OUTPUT,	
	ePCU_SET_AUX0_OUTPUT,	
	ePCU_SET_AUX1_OUTPUT,
	ePCU_SET_LED_VLOW_OUTPUT,
	ePCU_SET_LED_CHRGR_OUTPUT,
	ePCU_SET_LED_SYST_OUTPUT,
	ePCU_SET_LED_ALARM_OUTPUT,
}ePCU_POWER_CONTROL_SET_OUTPUT;

typedef enum ePcuPowerControlStatus
{
	ePCU_GET_ROW0_STATUS = 0,	
	ePCU_GET_ROW1_STATUS,		
	ePCU_GET_ROW2_STATUS,		
	ePCU_GET_ROW3_STATUS,		
	ePCU_GET_FAN0_STATUS,		
	ePCU_GET_FAN1_STATUS,		
	ePCU_GET_GPS_STATUS,		
	ePCU_GET_MODEM_STATUS,		
	ePCU_GET_DCU_STATUS,	
	ePCU_GET_RADAR_STATUS,	
	ePCU_GET_AUX0_STATUS,	
	ePCU_GET_AUX1_STATUS,
	ePCU_GET_LED_VLOW_STATUS,
	ePCU_GET_LED_CHRGR_STATUS,
	ePCU_GET_LED_SYST_STATUS,
	ePCU_GET_LED_ALARM_STATUS,
}ePCU_POWER_CONTROL_GET_STATUS;

#define INDL_OFF			0xF3
#define INDR_OFF			0xFC
#define LED3_OFF      0x3F
#define AUX_OFF				0xCF


																						
																						

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

void pcuPowerControlReset(void);
void pcuPowerControlInit(void);
void pcuPowerControlDoWork(void);
void pcuPowerControlSetOutput(ePCU_POWER_CONTROL epcuPowerControl, ePCU_POWER_CONTROL_SET_OUTPUT epcuPowerControlSetOutput);
unsigned char pcuPowerControlGetStatus(ePCU_POWER_CONTROL_GET_STATUS ePcuPowerControlGetStatus);

#endif		// PCU_POWER_CONTROL_H
