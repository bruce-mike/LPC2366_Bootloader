#ifndef SHARED_INTERFACE_H
#define SHARED_INTERFACE_H


#ifndef PACKED
#define PACKED __packed
#endif



/////////////////////////////////////////////////
// platform type
/////////////////////////////////////////////////
typedef enum ePlatformType
{
	ePLATFORM_TYPE_ARROW_BOARD,
	ePLATFORM_TYPE_HANDHELD,
	ePLATFORM_TYPE_PCU	
}ePLATFORM_TYPE;


/////////////////////////////////////////////////
// aux battery type
/////////////////////////////////////////////////
typedef enum eAuxBatteryTypes
{
	eAUX_BATTERY_NONE 							  = 0x00,
	eAUX_BATTERY_INSTALLED						    = 0x01,
}eAUX_BATTERY_TYPES;

/////////////////////////////////////////////////
// Alarm Level definitions
/////////////////////////////////////////////////
#define ALARM_LEVEL_NONE   0x0000
#define ALARM_LEVEL_MEDIUM 0x0001
#define ALARM_LEVEL_HIGH	 0x0002

/////////////////////////////////////////////////
// Alarm bitmap definitions
/////////////////////////////////////////////////
#define ALARM_NONE										0x0000
#define ALARM_BITMAP_LVD 							0x0001
#define ALARM_BITMAP_RVLV							0x0002
#define ALARM_BITMAP_RVBV							0x0004
#define ALARM_BITMAP_LOW_LINE_VOLTAGE 0x0008
#define ALARM_BITMAP_LOW_BATTERY			0x0010
#define ALARM_BITMAP_CHARGER_ON				0x0020
#define ALARM_BITMAP_OVER_TEMP				0x0040
#define ALARM_BITMAP_DISPLAY_ERRORS		0x0080
#define ALARM_BITMAP_INDICATOR_ERRORS 0x0100
#define ALARM_BITMAP_AUX_ERRORS				0x0200
#define ALARM_BITMAP_ACTUATOR_ERROR		0x0400
#define ALARM_BITMAP_PHOTOCELL_ERROR	0x0800

/////////////////////////////////////////////////
// Aux error bitmap definitions
/////////////////////////////////////////////////
#define AUX_ERROR_INDR	 							0x0001
#define AUX_ERROR_INDL	 							0x0002
#define AUX_ERROR_AUX 	 							0x0004
#define AUX_ERROR_LIMIT_SW						0x0010

/////////////////////////////////////////////////
// Aux status bitmap definitions
/////////////////////////////////////////////////
#define AUX_STATUS_COMPS								0x0001
#define AUX_STATUS_RDR								  0x0002
#define AUX_STATUS_AUX0								  0x0004
#define AUX_STATUS_AUX1								  0x0008

/////////////////////////////////////////////////
// display type definitions
/////////////////////////////////////////////////
#define SWITCH_BITMAP_ACT_UP					0x0001
#define SWITCH_BITMAP_ACT_DOWN				0x0002
#define SWITCH_BITMAP_MODE_1					0x0004
#define SWITCH_BITMAP_MODE_2					0x0008
#define SWITCH_BITMAP_MODE_4					0x0010
#define SWITCH_BITMAP_MODE_8					0x0020
#define SWITCH_BITMAP_TP_10						0x8000
#define SWITCH_BITMAP_TP_11						0x4000

/////////////////////////////////////////////////
// actuator types definitions
/////////////////////////////////////////////////
typedef enum eActuatorTypes
{
	eACTUATOR_TYPE_NONE 							    = 0x00,
	eACTUATOR_TYPE_90_DEGREE_POWER_TILT   = 0x01,
	eACTUATOR_TYPE_180_DEGREE_POWER_TILT  = 0x02
}eACTUATOR_TYPES;

/////////////////////////////////////////////////
// actuator command definitions
// up is the same as right for 180 actuator
// down is the same as left for 180 actuator
/////////////////////////////////////////////////
typedef enum actuatorCommands
{
	eACTUATOR_COMMAND_NOOP							= 0,
	eACTUATOR_COMMAND_STOP							= 1,
	eACTUATOR_COMMAND_MOVE_UP						= 2,
	eACTUATOR_COMMAND_MOVE_DOWN					= 3,
	
	eACTUATOR_COMMAND_MOVE_RIGHT				= 2,
	eACTUATOR_COMMAND_MOVE_LEFT					= 3,
}eACTUATOR_COMMANDS;

/////////////////////////////////////////////////
// actuator state definitions
// up is the same as right for 180 actuator
// down is the same as left for 180 actuator
/////////////////////////////////////////////////
typedef enum actuatorStates
{
	eACTUATOR_STATE_IDLE        				= 0,
  eACTUATOR_STATE_MOVING_UP						= 1,
  eACTUATOR_STATE_MOVING_DOWN         = 2,
  eACTUATOR_STATE_STALLED_MOVING_UP		= 3,
	eACTUATOR_STATE_STALLED_MOVING_DOWN	= 4,
	
  eACTUATOR_STATE_MOVING_RIGHT				= 1,
  eACTUATOR_STATE_MOVING_LEFT         = 2,
  eACTUATOR_STATE_STALLED_MOVING_RIGHT= 3,
	eACTUATOR_STATE_STALLED_MOVING_LEFT	= 4,
}eACTUATOR_STATES;

/////////////////////////////////////////////////
// actuator limit switch definitions
/////////////////////////////////////////////////
typedef enum actuatorLimits
{
	eACTUATOR_LIMIT_NONE								= 1,
	eACTUATOR_LIMIT_BOTTOM							= 2,
	eACTUATOR_LIMIT_TOP									= 4,
	eACTUATOR_LIMIT_ERROR								= 5,
}eACTUATOR_LIMITS;

/////////////////////////////////////////////////
// calibrate actuator limits
/////////////////////////////////////////////////
typedef enum actuatorLimitCalibrate
{
	eACTUATOR_LIMIT_CALIBRATE_CANCEL			= 1,
	eACTUATOR_LIMIT_CALIBRATE_BEGIN_UP		= 2,
	eACTUATOR_LIMIT_CALIBRATE_BEGIN_DOWN	= 4,
	eACTUATOR_LIMIT_CALIBRATE_GRAB_LIMIT	= 5,
}eACTUATOR_LIMIT_CALIBRATE;

/////////////////////////////////////////////////
// interface definitions
/////////////////////////////////////////////////
typedef enum eInterface
{
	eINTERFACE_DEBUG			= 0,
	eINTERFACE_WIRELESS		= 1,
	eINTERFACE_RS485			= 2,
	eINTERFACE_PCU_RS232	= 3	
}eINTERFACE;
#define INTERFACE_COUNT	((int)eINTERFACE_PCU_RS232)+1

/////////////////////////////////////////////////
// HDLC delimiter and escape characters
/////////////////////////////////////////////////
#define PACKET_DELIM 					0x7E
#define PACKET_ESCAPE 					0x7D
#define PACKET_ESCAPE_DELIM 			0x5E
#define PACKET_ESCAPE_ESCAPE 			0x5D

/////////////////////////////////////////////////
// model definitions
/////////////////////////////////////////////////
typedef enum eModel
{
	////////////////////////////////////////
	//// Vehicle Mount
	////////////////////////////////////////
	eMODEL_VEHICLE_25_LIGHT_SEQUENTIAL = 0,
	eMODEL_VEHICLE_15_LIGHT_SEQUENTIAL = 1,
	eMODEL_VEHICLE_15_LIGHT_FLASHING	 = 2,
	//eMODEL_VEHICLE_15_LIGHT_WIG_WAG		 = 3,
	
	//////////////////////////////////////////
	//// Solar Trailer
	//////////////////////////////////////////
	eMODEL_TRAILER_25_LIGHT_SEQUENTIAL = 4,
	eMODEL_TRAILER_15_LIGHT_SEQUENTIAL = 5,
	eMODEL_TRAILER_15_LIGHT_FLASHING	 = 6,
	//eMODEL_TRAILER_15_LIGHT_WIG_WAG		 = 3,
	
	eMODEL_NONE													= 10,
}eMODEL;


/////////////////////////////////////////////////
// display type definitions
/////////////////////////////////////////////////
typedef enum eDisplayTypes
{
	eDISPLAY_TYPE_BLANK								= 0x00,
	eDISPLAY_TYPE_FOUR_CORNER					= 0x01,
	eDISPLAY_TYPE_DOUBLE_ARROW				= 0x02,
	eDISPLAY_TYPE_BAR									= 0x03,
	eDISPLAY_TYPE_RIGHT_ARROW					= 0x04,
	eDISPLAY_TYPE_LEFT_ARROW					= 0x05,
	eDISPLAY_TYPE_RIGHT_STEM_ARROW		= 0x06,
	eDISPLAY_TYPE_LEFT_STEM_ARROW			= 0x07,
	eDISPLAY_TYPE_RIGHT_WALKING_ARROW	= 0x08,
	eDISPLAY_TYPE_LEFT_WALKING_ARROW	= 0x09,
	eDISPLAY_TYPE_RIGHT_CHEVRON				= 0x0A,
	eDISPLAY_TYPE_LEFT_CHEVRON				= 0x0B,
	eDISPLAY_TYPE_DOUBLE_DIAMOND			= 0x0C,
	eDISPLAY_TYPE_ALL_LIGHTS_ON				= 0x0D,
}eDISPLAY_TYPES;


/////////////////////////////////////////////////
// wireless configuration definitions
/////////////////////////////////////////////////
typedef enum eWirelessConfig
{
	eWIRELESS_CONFIG_BEGIN_BONDING,
	eWIRELESS_CONFIG_BONDING_COMPLETE,
	eWIRELESS_CONFIG_BONDING_FAILURE_ESN,
	eWIRELESS_CONFIG_BONDING_FAILURE_TIMEOUT
}eWIRELESS_CONFIG;

/////////////////////////////////////////////////
// brightness control defintions
/////////////////////////////////////////////////
typedef enum eBrightnessControl
{
	eBRIGHTNESS_CONTROL_NONE,
	eBRIGHTNESS_CONTROL_AUTO,
	eBRIGHTNESS_CONTROL_MANUAL_BRIGHT,
	eBRIGHTNESS_CONTROL_MANUAL_MEDIUM,
	eBRIGHTNESS_CONTROL_MANUAL_DIM,
}eBRIGHTNESS_CONTROL;



/////////////////////////////////////////////////
// packet definitions
/////////////////////////////////////////////////
#define MAX_COMMAND_LENGTH					12

#define PACKET_TYPE_MASK					0x3F
#define RESPONSE_INDICATOR				0x80
#define RETRY_INDICATOR						0x40

//////////////////////////////////////////////////
// HDLC 
//////////////////////////////////////////////////
#define NR_MASK										0xFE00
#define NR_SHIFT									9
#define NR_MASK2									0x7F
#define NS_MASK2									0x7F
#define PF_BIT										0x0100
#define NS_MASK										0x00FE
#define NS_SHIFT									1
#define SF_BIT										0x01
#define UF_BIT										0x02

#define SFLAG_TYPE_MASK						0x0C
#define SFLAG_TYPE_SHIFT					2

#define UFLAG_TYPE1_MASK					0x0C
#define UFLAG_TYPE2_MASK					0xE0
#define UFLAG_TYPE1_SHIFT					2
#define UFLAG_TYPE2_SHIFT					3

//#define HDLC_WINDOW_SIZE					8
#define HDLC_WINDOW_SIZE					128
#define SEND_WINDOW_SIZE					20
//#define SEND_WINDOW_SIZE					2

typedef enum sPacketTypes
{
	eSPACKET_TYPE_RR								=	0x00,
	eSPACKET_TYPE_RNR								=	0x01,
	eSPACKET_TYPE_REJ								=	0x02
}eSPACKET_TYPE;

typedef enum uPacketTypes
{
	eUPACKET_TYPE_SABM      = 0x07,
	eUPACKET_TYPE_DISC      = 0x08,
	eUPACKET_TYPE_DM        = 0x03,
	eUPACKET_TYPE_UA        = 0x0c
}eUPACKET_TYPE;

#define PACKET_STATUS_MASK  0x03
#define PACKET_ID_MASK      0xFC
#define PACKET_ID_SHIFT     2

//////////////////////////////////////////////////
// command packet definitions
//////////////////////////////////////////////////
typedef PACKED struct command
{
	unsigned char nPacketID_Status;
	unsigned char nPacketType;
	unsigned short nData;
	unsigned int nSendersESN;
}COMMAND;

////////////
// nHDLCControl
//
//   I Frames
// 7  6  5  4  3  2  1  0
// --------    -------
//  N(r)         N(s)
//          PF          SF
//
// N(R) = id of next packet we expect to receive
// PF   = Poll flag, 1 = request, 0 = response
// N(s) = id of the last packet we sent
// SF   = Supervisory flag, 0 = I frame, 1 = supervisory frame
//
//   S frames
// 7  6  5  4  3  2  1  0
// --------    ----
//  N(r)       Type
//          PF       U   1
//
// N(R) = id of next packet we expect to receive
// PF   = Poll flag, 1 = request, 0 = response
// N(s) = id of the last packet we sent
// SF   = 1
// U    = 0, supervisory packet, 1 = unnumbered packet
//
// S Frame types
// 00 = RR
// 01 = RNR
// 10 = REJ
//
//
//   U frames
// 7  6  5  4  3  2  1  0
// --------    ----
//  Type       Type
//          PF       1   1
//
// N(R) = id of next packet we expect to receive
// PF   = Poll flag, 1 = request, 0 = response
// N(s) = id of the last packet we sent
// SF   = 1
// U    = 1
//
// U Frame types
// SABM = 0x07
// DISC = 0x08
// DM   = 0x03
// UA   = 0x0c
/////

/////
// nPacketID_Status
// 7  6  5  4  3  2  1  0
// ________________  _____
// ID (0-32)         Status
/////
//typedef __packed struct command
typedef PACKED struct hdlcPacket
{
	unsigned short nHDLCControl;
	COMMAND	command;
	unsigned char nChecksum;
}HDLCPACKET;


typedef enum eCommands
{
	eCOMMAND_GET_MODEL_TYPE             = 0x01,
	eCOMMAND_DISPLAY_CHANGE             = 0x02,
	eCOMMAND_ACTUATOR_SWITCH            = 0x03,
	eCOMMAND_STATUS_SIGN_DISPLAY        = 0x04,
	eCOMMAND_STATUS_AUX                 = 0x05,
	eCOMMAND_STATUS_SWITCHES            = 0x06,
	eCOMMAND_STATUS_ACTUATOR            = 0x07,
	eCOMMAND_STATUS_ACTUATOR_LIMIT      = 0x08,
	eCOMMAND_WIRELESS_CONFIG            = 0x09,	
	eCOMMAND_STATUS_ALARMS              = 0x0A,
	eCOMMAND_STATUS_INDICATOR_ERRORS    = 0x0B,
	eCOMMAND_STATUS_LINE_VOLTAGE        = 0x0C,
	eCOMMAND_STATUS_BATTERY_VOLTAGE     = 0x0D,
	eCOMMAND_STATUS_DISPLAY_ERRORS      = 0x0E,
	eCOMMAND_STATUS_AUX_ERRORS          = 0x0F,
	eCOMMAND_STATUS_TEMPERATURE         = 0x10,
	eCOMMAND_STATUS_PHOTOCELL_ERRORS    = 0x11,
	eCOMMAND_ACTUATOR_STOP              = 0x12,
	eCOMMAND_ACTUATOR_UP                = 0x13,
	eCOMMAND_ACTUATOR_DOWN              = 0x14,
	eCOMMAND_SET_ACTUATOR_TYPE          = 0x15,
	eCOMMAND_GET_ACTUATOR_TYPE          = 0x16,
	eCOMMAND_SET_DISALLOWED_PATTERNS    = 0x17,
	eCOMMAND_GET_DISALLOWED_PATTERNS    = 0x18,
	eCOMMAND_SET_BRIGHTNESS_CONTROL     = 0x19,
	eCOMMAND_GET_BRIGHTNESS_CONTROL     = 0x1A,
	eCOMMAND_SET_AUX_BATTERY_TYPE       = 0x1B,
	eCOMMAND_GET_AUX_BATTERY_TYPE       = 0x1C,
	eCOMMAND_SET_ACTUATOR_BUTTON_MODE   = 0x1D,
	eCOMMAND_GET_ACTUATOR_BUTTON_MODE   = 0x1E,
	eCOMMAND_DO_ACTUATOR_CALIBRATION    = 0x1F,
	eCOMMAND_DO_RESET                   = 0x20,
}eCOMMANDS;

typedef enum ePacketStatus
{
	ePACKET_STATUS_SUCCESS              = 0x00,
	ePACKET_STATUS_DELAYED_SUCCESS      = 0x01,
	ePACKET_STATUS_NOT_SUPPORTED        = 0x02,
	ePACKET_STATUS_OUT_OF_RANGE         = 0x03,
	ePACKET_STATUS_GENERAL_ERROR        = 0x04,
}ePACKET_STATUS;

//////////////////////////////////////////////
// LCD Actuator Button Modes (Auto/Manual)
//////////////////////////////////////////////
typedef enum eActuatorButtonMode
{
	eACTUATOR_BUTTON_AUTO_MODE   = 0x00,
	eACTUATOR_BUTTON_MANUAL_MODE = 0x01
}eACTUATOR_BUTTON_MODE;

#define lineVoltageHighLimit            3880
#define lineVoltageLowLimit             1250 
#define lineVoltageShutdownLimit        1150

#define defaultChargerOffVoltage        1440
#define defaultChargerOnVoltage         1350
#define defaultBatteryVoltageGoodLimit  1190
#define defaultBatteryVoltageHighLimit  1500
#define defaultBatteryVoltageWarnLimit  1150
#define defaultBatteryVoltageLVDLimit   1110

#define temperatureHighLimitC           165
#define temperatureLowLimitC            -40

#define temperatureHighLimitF           329
#define temperatureLowLimitF            -40

////////////////////////////////////////
// Touch Panel Calibration Structures
////////////////////////////////////////
typedef struct TPPoint 
{
    int nX;
    int nY;
}TP_POINT;


typedef PACKED struct TPMatrix
{
    int nA;
    int nB;
    int nC;
    int nD;
    int nE;
    int nF;
    int nDivider;
}TP_MATRIX;

///////////////////////////////////////
// LCD Sleep Mode States
///////////////////////////////////////
typedef enum eSleepModeStates
{
	eSLEEP_MODE_DISABLED = 0x00,
	eSLEEP_MODE_ENABLED  = 0x01,
}eSLEEP_MODE_STATES;

//=============================================================================
// prototypes
//=============================================================================


#endif		// SHARED_INTERFACE_H
