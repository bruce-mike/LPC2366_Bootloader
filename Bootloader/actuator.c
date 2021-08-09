// actuator routines
// presumes pindefs elsewhere
// presumes GPIO set up elsewhere
#include <stdlib.h>
#include <stdio.h>
#include <lpc23xx.h>
#include "shareddefs.h"
#include "ArrowBoardDefs.h"		// hardware 
#include "sharedinterface.h"
#include "queue.h"
#include "timer.h"

#include "mischardware.h"
#include "commands.h"
#include "actuator.h"	// actuator specifics
#include "errorinput.h"
#include "ADS7828.h"
#include "leddriver.h"
#include "pwm.h"
#include "storedconfig.h"
#include "watchdog.h"
#include "adc.h"

static TIMERCTL commandTimer;
static TIMERCTL inrushTimer;
static TIMERCTL limitSwitchTimer;
static eACTUATOR_COMMANDS eReceivedCommand;
static eACTUATOR_INTERNAL_STATE eInternalState;
static eACTUATOR_INTERNAL_STATE ePreviousInternalState;

/////
// define current limits
// for actuator motion
/////
static int nActuatorUpStopCurrent = ACT_DEFAULT_STOP_CURRENT;
static int nActuatorDownStopCurrent = ACT_DEFAULT_STOP_CURRENT;
static BOOL bCalibratingLimitCurrent = FALSE;
static TIMERCTL calibratingLimitCurrentTimer;
static int actuatorRun(int direction)
{
	int i,j;
	
		switch(direction) {		

		case UP:
			printf("inside run_act(UP)\n\r");
			FIO2CLR = (1 << uP_ENA_xACT);						// Enable Actuator
			FIO2CLR = (1 << uP_PWR_xACTA);					// Turn ON actuator A input
			FIO2SET = (1 << uP_PWR_xACTB);					// Turn OFF actuator B input
			for(i=5; i<101; i++)
			{
				pwmActuatorDriverSetDutyCycle(i); 
				for(j=0; j<10000; j++)
				{
					watchdogFeed();
				}
			}
			break;
				

		case DOWN:
			printf("Inside run_act(DOWN)\n\r");
			FIO2CLR = (1 << uP_ENA_xACT);						// Enable Actuator
			FIO2SET = (1 << uP_PWR_xACTA);					// Turn ON actuator A input
			FIO2CLR = (1 << uP_PWR_xACTB);					// Turn OFF actuator B input
			for(i=5; i<101; i++)
			{
				pwmActuatorDriverSetDutyCycle(i); 
				for(j=0; j<10000; j++)
				{
					watchdogFeed();
				}
			}
			break;

		case OFF: 
		default:
			FIO2SET = (1 << uP_ENA_xACT);					// Enable Actuator
			FIO2CLR = (1 << uP_PWR_xACTA);				// Turn OFF actuator A input
			FIO2CLR = (1 << uP_PWR_xACTB);				// Turn OFF actuator B input
			FIO2SET = (1 << uP_PWM_xACT);					// Disable actuator - necessary?  
			break;
		
		}

		return TRUE;

}


// *********************
// *********************
// *********************
static void actuatorConfigGPIO(void) 
{
	
//set up directions
FIO2DIR |= (OUT << uP_PWM_xACT ) |
					 (OUT << uP_PWR_xACTA) |
	         (OUT << uP_PWR_xACTB) |
	         (OUT << uP_ENA_xACT ) |
	         (IN  << uP_IOK_xACTA) |
	         (IN  << uP_IOK_xACTB) ;
	
	
// make sure actuator doesn't run
FIO2CLR =  (1 << uP_PWM_xACT ) |
					 (1 << uP_PWR_xACTA) |
	         (1 << uP_PWR_xACTB) |
	         (1 << uP_ENA_xACT ) ;
}
void actuatorInit()
{
	int nLimit;
	actuatorConfigGPIO();
	actuatorRun(OFF);
	eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
	ePreviousInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
	eReceivedCommand = eACTUATOR_COMMAND_STOP;
	initTimer(&commandTimer);
	initTimer(&inrushTimer);
	initTimer(&limitSwitchTimer);
	initTimer(&calibratingLimitCurrentTimer);
	/////
	// then turn limit switch power on
	/////
	ledDriverSetLimitSWPower(eLED_ON);
	startTimer(&limitSwitchTimer, LIMIT_SWITCH_POWER_WAIT_TIMEOUT);
	
	nLimit = storedConfigGetActuatorUpLimit();
	if(0 < nLimit)
	{	
		nActuatorUpStopCurrent = nLimit;
	}
	printf("nActuatorUpStopCurrent[%d]\n", nActuatorUpStopCurrent);
	nLimit = storedConfigGetActuatorDownLimit();
	if(0 < nLimit)
	{	
		nActuatorDownStopCurrent = nLimit;
	}
	printf("nActuatorDownStopCurrent[%d]\n", nActuatorDownStopCurrent);
}
void actuatorDoState(eACTUATOR_EVENTS eEvent)
{
	//printf("1-actuatorDoState eEvent[%d] eInternalState[%d]\n", eEvent, eInternalState);
	switch(eInternalState)
	{
		case eACTUATOR_INTERNAL_STATE_IDLE:
			switch(eEvent)
			{
				case eACTUATOR_EVENT_NOOP:
				case eACTUATOR_EVENT_COMMAND_STOP:
				case eACTUATOR_EVENT_COMMAND_TIMEOUT:
				case eACTUATOR_EVENT_INRUSH_TIMER_EXPIRY:
				case eACTUATOR_EVENT_SWITCH_OFF:
				case eACTUATOR_EVENT_OVERCURRENT:
				case eACTUATOR_EVENT_UP_LIMIT_SWITCH_CLOSURE:
				case eACTUATOR_EVENT_DOWN_LIMIT_SWITCH_CLOSURE:
					//printf("1-DoState\n\r");
					actuatorRun(OFF);
					break;
				
				case eACTUATOR_EVENT_COMMAND_UP:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_UP_BEFORE_CURRENT_CHECK;
					//printf("2-DoState\n\r");
					ads7828SetActuatorStopCurrent(nActuatorUpStopCurrent);
					actuatorRun(UP);
					startTimer(&inrushTimer, ACTUATOR_INRUSH_TIMEOUT);
					startTimer(&commandTimer, ACTUATOR_COMMAND_TIMEOUT);
					break;
				case eACTUATOR_EVENT_COMMAND_DOWN:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_DOWN_BEFORE_CURRENT_CHECK;
					//printf("3-DoState\n\r");
					ads7828SetActuatorStopCurrent(nActuatorDownStopCurrent);
					actuatorRun(DOWN);
					startTimer(&inrushTimer, ACTUATOR_INRUSH_TIMEOUT);
					startTimer(&commandTimer, ACTUATOR_COMMAND_TIMEOUT);
					break;

				case eACTUATOR_EVENT_SWITCH_UP:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_UP_BEFORE_CURRENT_CHECK;
					//printf("4-DoState\n\r");
					ads7828SetActuatorStopCurrent(nActuatorUpStopCurrent);
					actuatorRun(UP);
					startTimer(&inrushTimer, ACTUATOR_INRUSH_TIMEOUT);
					break;
				case eACTUATOR_EVENT_SWITCH_DOWN:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_DOWN_BEFORE_CURRENT_CHECK;
					//printf("5-DoState\n\r");
					ads7828SetActuatorStopCurrent(nActuatorDownStopCurrent);
					actuatorRun(DOWN);
					startTimer(&inrushTimer, ACTUATOR_INRUSH_TIMEOUT);
					break;
			}
			break;
		case eACTUATOR_INTERNAL_STATE_UP_BEFORE_CURRENT_CHECK:
				switch(eEvent)
			{
				case eACTUATOR_EVENT_NOOP:
					break;
				case eACTUATOR_EVENT_COMMAND_STOP:
				case eACTUATOR_EVENT_SWITCH_OFF:
				case eACTUATOR_EVENT_COMMAND_TIMEOUT:
				case eACTUATOR_EVENT_UP_LIMIT_SWITCH_CLOSURE:
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					actuatorRun(OFF);
					break;
				case eACTUATOR_EVENT_COMMAND_UP:
					startTimer(&commandTimer, ACTUATOR_COMMAND_TIMEOUT);
					break;
				case eACTUATOR_EVENT_COMMAND_DOWN:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					actuatorRun(OFF);
					break;
				case eACTUATOR_EVENT_SWITCH_UP:
					break;
				case eACTUATOR_EVENT_SWITCH_DOWN:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					actuatorRun(OFF);
					break;
				case eACTUATOR_EVENT_INRUSH_TIMER_EXPIRY:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_UP_WITH_CURRENT_CHECK;
					break;
				case eACTUATOR_EVENT_OVERCURRENT:
					//printf("eACTUATOR_INTERNAL_STATE_UP_BEFORE_CURRENT_CHECK: eACTUATOR_EVENT_OVERCURRENT\n");
					break;
				case eACTUATOR_EVENT_DOWN_LIMIT_SWITCH_CLOSURE:
					break;
			}
			break;
		case eACTUATOR_INTERNAL_STATE_DOWN_BEFORE_CURRENT_CHECK:
			switch(eEvent)
			{
				case eACTUATOR_EVENT_NOOP:
					break;
				case eACTUATOR_EVENT_COMMAND_STOP:
				case eACTUATOR_EVENT_SWITCH_OFF:
				case eACTUATOR_EVENT_COMMAND_TIMEOUT:
				case eACTUATOR_EVENT_DOWN_LIMIT_SWITCH_CLOSURE:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					actuatorRun(OFF);
					break;
				case eACTUATOR_EVENT_COMMAND_UP:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					actuatorRun(OFF);
					break;
				case eACTUATOR_EVENT_COMMAND_DOWN:
					startTimer(&commandTimer, ACTUATOR_COMMAND_TIMEOUT);
					break;
				case eACTUATOR_EVENT_SWITCH_UP:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					actuatorRun(OFF);
					break;
				case eACTUATOR_EVENT_SWITCH_DOWN:
					break;
				case eACTUATOR_EVENT_INRUSH_TIMER_EXPIRY:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_UP_WITH_CURRENT_CHECK;
					break;
				case eACTUATOR_EVENT_OVERCURRENT:
					//printf("eACTUATOR_INTERNAL_STATE_DOWN_BEFORE_CURRENT_CHECK: eACTUATOR_EVENT_OVERCURRENT\n");
					break;
				case eACTUATOR_EVENT_UP_LIMIT_SWITCH_CLOSURE:
					break;
			}
			break;
		case eACTUATOR_INTERNAL_STATE_UP_WITH_CURRENT_CHECK:
			switch(eEvent)
			{
				case eACTUATOR_EVENT_NOOP:
					break;
				case eACTUATOR_EVENT_COMMAND_STOP:
				case eACTUATOR_EVENT_SWITCH_OFF:
				case eACTUATOR_EVENT_COMMAND_TIMEOUT:
				case eACTUATOR_EVENT_UP_LIMIT_SWITCH_CLOSURE:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					actuatorRun(OFF);
					break;
				case eACTUATOR_EVENT_COMMAND_UP:
					startTimer(&commandTimer, ACTUATOR_COMMAND_TIMEOUT);
					break;
				case eACTUATOR_EVENT_COMMAND_DOWN:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					actuatorRun(OFF);
					break;
				case eACTUATOR_EVENT_SWITCH_UP:

					break;
				case eACTUATOR_EVENT_SWITCH_DOWN:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					actuatorRun(OFF);
					break;
				case eACTUATOR_EVENT_INRUSH_TIMER_EXPIRY:
					break;
				case eACTUATOR_EVENT_OVERCURRENT:
					ePreviousInternalState = eInternalState;
					//printf("A1\n");
					eInternalState = eACTUATOR_INTERNAL_STATE_ERROR;
					actuatorRun(OFF);
					//printf("eACTUATOR_INTERNAL_STATE_UP_WITH_CURRENT_CHECK: eACTUATOR_EVENT_OVERCURRENT->actuatorRun(OFF)\n");
					break;
				case eACTUATOR_EVENT_DOWN_LIMIT_SWITCH_CLOSURE:
					break;
			}
			break;
		case eACTUATOR_INTERNAL_STATE_DOWN_WITH_CURRENT_CHECK:
			switch(eEvent)
			{
				case eACTUATOR_EVENT_NOOP:
					break;
				case eACTUATOR_EVENT_COMMAND_STOP:
				case eACTUATOR_EVENT_SWITCH_OFF:
				case eACTUATOR_EVENT_COMMAND_TIMEOUT:
				case eACTUATOR_EVENT_DOWN_LIMIT_SWITCH_CLOSURE:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					actuatorRun(OFF);
					break;
				case eACTUATOR_EVENT_COMMAND_UP:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					actuatorRun(OFF);
					break;
				case eACTUATOR_EVENT_COMMAND_DOWN:
					startTimer(&commandTimer, ACTUATOR_COMMAND_TIMEOUT);
					break;
				case eACTUATOR_EVENT_SWITCH_UP:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					actuatorRun(OFF);
					break;
				case eACTUATOR_EVENT_SWITCH_DOWN:
					break;
				case eACTUATOR_EVENT_INRUSH_TIMER_EXPIRY:
					break;
				case eACTUATOR_EVENT_OVERCURRENT:
					ePreviousInternalState = eInternalState;
					//printf("A2\n");
					eInternalState = eACTUATOR_INTERNAL_STATE_ERROR;
					actuatorRun(OFF);
					//printf("eACTUATOR_INTERNAL_STATE_DOWN_WITH_CURRENT_CHECK: eACTUATOR_EVENT_OVERCURRENT->actuatorRun(OFF)\n");
					break;
				case eACTUATOR_EVENT_UP_LIMIT_SWITCH_CLOSURE:
					break;
			}
			break;
		case eACTUATOR_INTERNAL_STATE_ERROR:
			switch(eEvent)
			{
				case eACTUATOR_EVENT_COMMAND_STOP:
				case eACTUATOR_EVENT_COMMAND_TIMEOUT:
				case eACTUATOR_EVENT_SWITCH_OFF:
					ePreviousInternalState = eInternalState;
					eInternalState = eACTUATOR_INTERNAL_STATE_IDLE;
					break;
				
				case eACTUATOR_EVENT_COMMAND_UP:
				case eACTUATOR_EVENT_COMMAND_DOWN:
					startTimer(&commandTimer, ACTUATOR_COMMAND_TIMEOUT);
					break;
				
				case eACTUATOR_EVENT_NOOP:
				case eACTUATOR_EVENT_SWITCH_UP:
				case eACTUATOR_EVENT_SWITCH_DOWN:
				case eACTUATOR_EVENT_INRUSH_TIMER_EXPIRY:
				case eACTUATOR_EVENT_OVERCURRENT:
				case eACTUATOR_EVENT_UP_LIMIT_SWITCH_CLOSURE:
				case eACTUATOR_EVENT_DOWN_LIMIT_SWITCH_CLOSURE:
					//printf("eACTUATOR_INTERNAL_STATE_ERROR: eACTUATOR_EVENT_OVERCURRENT\n");
					break;
			}
			break;
	}
}

void actuatorDoWork()
{
	static int nPrevSwitchBitmap = 0;
	BOOL bComplete = FALSE;
	
	/////
	// have we reached the current limit?
	/////
	if(bCalibratingLimitCurrent)
	{
		/////
		// we don't want to do this forever
		// if we lose communication
		/////
		if(isTimerExpired(&calibratingLimitCurrentTimer))
		{
			bCalibratingLimitCurrent = FALSE;
			actuatorCalibrate(eACTUATOR_LIMIT_CALIBRATE_CANCEL);
		}
	}
	else
	{
		if(ads7827IsActuatorCurrentLimitReached())
		{
			/////
			// issue an overcurrent event
			/////
			actuatorDoState(eACTUATOR_EVENT_OVERCURRENT);
		}
	}

	do
	{		
		/////
		// switches override commands
		// so handle them first
		/////
		int nSwitchBitmap = errorInputGetActSwitch();

		switch(nSwitchBitmap)
		{
			case SWITCH_BITMAP_ACT_UP:
				//printf("Act Switch up\n");
				actuatorDoState(eACTUATOR_EVENT_SWITCH_UP);
			
				/////
				// reset received command
				// so we don't act on it again
				/////
				eReceivedCommand = eACTUATOR_COMMAND_NOOP;
			
				/////
				// show that we acted on this information
				/////
				bComplete = TRUE;
				break;
			
			case SWITCH_BITMAP_ACT_DOWN:
				//printf("Act Switch down\n");
				actuatorDoState(eACTUATOR_EVENT_SWITCH_DOWN);
			
				/////
				// reset received command
				// so we don't act on it again
				/////
				eReceivedCommand = eACTUATOR_COMMAND_NOOP;
			
				/////
				// show that we acted on this information
				/////
				bComplete = TRUE;
				break;
			
			case 0:
				if(0 != nPrevSwitchBitmap)
				{
					/////
					// switch just went off
					/////
					//printf("Act Switch off\n");
					actuatorDoState(eACTUATOR_EVENT_SWITCH_OFF);
			
					/////
					// reset received command
					// so we don't act on it again
					/////
					eReceivedCommand = eACTUATOR_COMMAND_NOOP;
			
					/////
					// show that we acted on this information
					/////
					bComplete = TRUE;
				}
				break;	
				
			default:
				break;
		}
		nPrevSwitchBitmap = nSwitchBitmap;
		if(bComplete)
		{
			break;
		}

		/////
		// now handle any commands
		/////
		switch(eReceivedCommand)
		{
			case eACTUATOR_COMMAND_STOP:
				actuatorDoState(eACTUATOR_EVENT_COMMAND_STOP);
				break;
			
			case eACTUATOR_COMMAND_MOVE_UP:
				actuatorDoState(eACTUATOR_EVENT_COMMAND_UP);
				break;
			
			case eACTUATOR_COMMAND_MOVE_DOWN:
				actuatorDoState(eACTUATOR_EVENT_COMMAND_DOWN);
				break;
			
			default:
				if(isTimerExpired(&commandTimer))
				{
					actuatorDoState(eACTUATOR_EVENT_COMMAND_TIMEOUT);
					/////
					// DEH DEBUG 060614
					/////
					startTimer(&commandTimer, ACTUATOR_COMMAND_TIMEOUT);
				}
				if(isTimerExpired(&inrushTimer))
				{
					switch(eInternalState)
					{
						case eACTUATOR_INTERNAL_STATE_UP_BEFORE_CURRENT_CHECK:
							eInternalState = eACTUATOR_INTERNAL_STATE_UP_WITH_CURRENT_CHECK;
							ads7828BeginActuatorLimitCalculations();
							break;
						case eACTUATOR_INTERNAL_STATE_DOWN_BEFORE_CURRENT_CHECK:
							eInternalState = eACTUATOR_INTERNAL_STATE_DOWN_WITH_CURRENT_CHECK;
							ads7828BeginActuatorLimitCalculations();
							break;
						default:
							break;
					}					
				}
				break;
		}
		
		
		/////
		// reset received command
		// so we don't act on it again
		/////
		eReceivedCommand = eACTUATOR_COMMAND_NOOP;
	}while(0);
	if(isTimerExpired(&limitSwitchTimer))
	{
		switch(eInternalState)
		{

			case eACTUATOR_INTERNAL_STATE_UP_BEFORE_CURRENT_CHECK:
			case eACTUATOR_INTERNAL_STATE_DOWN_BEFORE_CURRENT_CHECK:
			case eACTUATOR_INTERNAL_STATE_UP_WITH_CURRENT_CHECK:
			case eACTUATOR_INTERNAL_STATE_DOWN_WITH_CURRENT_CHECK:
				break;
			case eACTUATOR_INTERNAL_STATE_IDLE:
			case eACTUATOR_INTERNAL_STATE_ERROR:
				{
					/////
					// read the limit switches and record the state
					/////
					eACTUATOR_LIMITS eLimitState = actuatorGetLimitState();
					switch(eLimitState)
					{
						case eACTUATOR_LIMIT_ERROR:
						case eACTUATOR_LIMIT_NONE:
							break;
						case eACTUATOR_LIMIT_TOP:
							actuatorDoState(eACTUATOR_EVENT_UP_LIMIT_SWITCH_CLOSURE);
							break;
						case eACTUATOR_LIMIT_BOTTOM:
							actuatorDoState(eACTUATOR_EVENT_DOWN_LIMIT_SWITCH_CLOSURE);
							break;
					}
					/////
					// wait for the next cycle
					/////
					startTimer(&limitSwitchTimer, LIMIT_SWITCH_POWER_IDLE_TIMEOUT);
				}
				break;
		}
	}
}
// need a helper routine to configure PWM for actuator

ePACKET_STATUS actuatorCommand(unsigned short nsActuatorControl)
{
	ePACKET_STATUS eRetVal = ePACKET_STATUS_SUCCESS;
	//printf("1-actuatorCommand[%d]\n", nsActuatorControl);
	if(ADCGetLineVoltage() >= lineVoltageShutdownLimit)
	{
		switch(nsActuatorControl)
		{
			case eACTUATOR_COMMAND_STOP:
		printf("4-actuatorCommand[%d]\n", nsActuatorControl);
				eReceivedCommand = eACTUATOR_COMMAND_STOP;
				break;
		
			case eACTUATOR_COMMAND_MOVE_UP:
				if(eACTUATOR_LIMIT_TOP == actuatorGetLimitState())
				{
						eRetVal = ePACKET_STATUS_OUT_OF_RANGE;
						printf("2B-actuatorCommand[%d] At Limit\n", nsActuatorControl);
				}
				else
				{
					printf("2-actuatorCommand[%d]\n", nsActuatorControl);
					eReceivedCommand = eACTUATOR_COMMAND_MOVE_UP;
					startTimer(&commandTimer, ACTUATOR_COMMAND_TIMEOUT);
				}
				break;
		
			case eACTUATOR_COMMAND_MOVE_DOWN:
				if(eACTUATOR_LIMIT_BOTTOM == actuatorGetLimitState())
				{
					eRetVal = ePACKET_STATUS_OUT_OF_RANGE;
					printf("3B-actuatorCommand[%d] At Limit\n", nsActuatorControl);
				}
				else
				{
		printf("3-actuatorCommand[%d]\n", nsActuatorControl);
					eReceivedCommand = eACTUATOR_COMMAND_MOVE_DOWN;
					startTimer(&commandTimer, ACTUATOR_COMMAND_TIMEOUT);
				}
				break;
		
			default:
				eRetVal = ePACKET_STATUS_NOT_SUPPORTED;
				break;
		}
	}
	else
	{
		/////
		// voltage too low
		// don't start actuator
		/////
		eRetVal = ePACKET_STATUS_GENERAL_ERROR;
	}

	return eRetVal;
}

eACTUATOR_LIMITS actuatorGetLimitState()
{
	eACTUATOR_LIMITS nLimitState = eACTUATOR_LIMIT_NONE;
	/////
	// read the limit switches and record the state
	/////
	BOOL bActuatorLimitSwitchA = FALSE;
	BOOL bActuatorLimitSwitchB = FALSE;
	readActuatorLimitSwitch(&bActuatorLimitSwitchA, &bActuatorLimitSwitchB);
	if(bActuatorLimitSwitchA && bActuatorLimitSwitchB)
	{
		nLimitState = eACTUATOR_LIMIT_ERROR;
	}
	else if(bActuatorLimitSwitchA)
	{
		nLimitState = eACTUATOR_LIMIT_TOP;
	}
	else if(bActuatorLimitSwitchB)
	{
		nLimitState = eACTUATOR_LIMIT_BOTTOM;
	}
	return nLimitState;
}

unsigned short actuatorGetStatus()
{
	unsigned short nStatus = 0;
	switch(eInternalState)
	{
		case eACTUATOR_INTERNAL_STATE_IDLE:
			nStatus = eACTUATOR_STATE_IDLE;
			break;
		case eACTUATOR_INTERNAL_STATE_UP_BEFORE_CURRENT_CHECK:
		case eACTUATOR_INTERNAL_STATE_UP_WITH_CURRENT_CHECK:
			nStatus = eACTUATOR_STATE_MOVING_UP;		
			break;
		case	eACTUATOR_INTERNAL_STATE_DOWN_BEFORE_CURRENT_CHECK:
		case eACTUATOR_INTERNAL_STATE_DOWN_WITH_CURRENT_CHECK:
			nStatus = eACTUATOR_STATE_MOVING_DOWN;		
			break;
		case eACTUATOR_INTERNAL_STATE_ERROR:
			//printf("1-[%d] [%d]\n", eInternalState, ePreviousInternalState);
			nStatus = eACTUATOR_STATE_STALLED_MOVING_UP;	
			switch(ePreviousInternalState)
			{
				case eACTUATOR_INTERNAL_STATE_UP_BEFORE_CURRENT_CHECK:
				case eACTUATOR_INTERNAL_STATE_UP_WITH_CURRENT_CHECK:
					nStatus = eACTUATOR_STATE_STALLED_MOVING_UP;		
					break;
				case	eACTUATOR_INTERNAL_STATE_DOWN_BEFORE_CURRENT_CHECK:
				case eACTUATOR_INTERNAL_STATE_DOWN_WITH_CURRENT_CHECK:
					nStatus = eACTUATOR_STATE_STALLED_MOVING_DOWN;		
					break;
				default:
					break;
			}
			break;
	}
	return nStatus;
}

void actuatorCalibrate(eACTUATOR_LIMIT_CALIBRATE eStage)
{
	static eACTUATOR_LIMIT_CALIBRATE ePreviousStage;
	switch(eStage)
	{
		case eACTUATOR_LIMIT_CALIBRATE_CANCEL:
			bCalibratingLimitCurrent = FALSE;
			actuatorCommand(eACTUATOR_COMMAND_STOP);
			break;
		case eACTUATOR_LIMIT_CALIBRATE_BEGIN_UP:
			bCalibratingLimitCurrent = TRUE;
			startTimer(&calibratingLimitCurrentTimer, CALIBRATING_LIMIT_MAX_TIMEOUT);
			actuatorCommand(eACTUATOR_COMMAND_MOVE_UP);
			break;
		case eACTUATOR_LIMIT_CALIBRATE_BEGIN_DOWN:
			bCalibratingLimitCurrent = TRUE;
			startTimer(&calibratingLimitCurrentTimer, CALIBRATING_LIMIT_MAX_TIMEOUT);
			actuatorCommand(eACTUATOR_COMMAND_MOVE_DOWN);
			break;
		case eACTUATOR_LIMIT_CALIBRATE_GRAB_LIMIT:
			bCalibratingLimitCurrent = FALSE;
			switch(ePreviousStage)
			{
				case eACTUATOR_LIMIT_CALIBRATE_BEGIN_UP:
					nActuatorUpStopCurrent = nADS7828GetIA();
					storedConfigSetActuatorUpLimit(nActuatorUpStopCurrent);
					break;
				case eACTUATOR_LIMIT_CALIBRATE_BEGIN_DOWN:
					nActuatorDownStopCurrent = nADS7828GetIA();
					storedConfigSetActuatorDownLimit(nActuatorDownStopCurrent);
					break;
				case eACTUATOR_LIMIT_CALIBRATE_CANCEL:
				case eACTUATOR_LIMIT_CALIBRATE_GRAB_LIMIT:
					break;
			}
			actuatorCommand(eACTUATOR_COMMAND_STOP);
			break;
	}
	ePreviousStage = eStage;
}

