/****************************************************************************
Module: SM_BallLaunching.c
Description:
	The third level state machine for our robot in the BALL_LAUNCHING state.
	Contains four states: BALL_LAUNCHING_ENTRY, BALL_LAUNCHING_IR_ALIGN, 
												 BALL_LAUNCHING_LAUNCH, BALL_LAUNCHING_EXIT
Author: Kyle Moy, 2/25/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"

// Module Libraries
#include "SM_BallLaunching.h"
#include "Display.h"
#include "DriveMotors.h"
#include "SM_Navigation.h"
#include "GamefieldPositions.h"
#include "SM_Master.h"
#include "BallLauncher.h"
#include "DriveMotorPID.h"

/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringBallLaunchingEntry(ES_Event Event);
static ES_Event DuringBallLaunchingIRAlign(ES_Event Event);
static ES_Event DuringBallLaunchingLaunch(ES_Event Event);
static ES_Event DuringBallLaunchingExit(ES_Event Event);


/*---------------------------- Module Variables ---------------------------*/
static BallLaunchingState_t CurrentState;
static uint8_t MotorTimeoutCase = 0;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function:			RunBallLaunchingSM
Parameters:		ES_Event CurrentEvent, the event to process
Returns:			ES_Event, an event to return
Description:	The run function for the racing state machine
Notes:				Uses nested switch/case to implement the machine.
****************************************************************************/
ES_Event RunBallLaunchingSM(ES_Event CurrentEvent) {
	bool MakeTransition = false;	// Assume not making a transition
	BallLaunchingState_t NextState = CurrentState;
	ES_Event EntryEventKind = {ES_ENTRY, 0};// Default to normal entry to new state
	ES_Event ReturnEvent = {ES_NO_EVENT, 0}; // Assume no error

	switch(CurrentState) {
		case BALL_LAUNCHING_ENTRY:
		default:
			// Execute During function for BALL_LAUNCHING_ENTRY.
			// ES_ENTRY & ES_EXIT are processed here allow the lower
			// level state machines to re-map or consume the event
			CurrentEvent = DuringBallLaunchingEntry(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					
					case E_MOTOR_TIMEOUT:
						if (MotorTimeoutCase == 0) {
							SetPIDgains(0.05, 0.02, 0);
							PivotCCWwithSetTicks(40, 18);
							MotorTimeoutCase = 1;
						} else if (MotorTimeoutCase == 1) {
							DriveBackwardsWithBias(100, 100, 150);
							MotorTimeoutCase = 2;
						} else if (MotorTimeoutCase == 2) {
							StopMotors();
							ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, 50);
							MotorTimeoutCase = 3;
						} else if (MotorTimeoutCase == 3) {
							StopMotors();
							DriveForwardWithBias(100, 100, 118);
							MotorTimeoutCase = 4;
						} else {
							StopMotors();
							MotorTimeoutCase = 0;
							NextState = BALL_LAUNCHING_IR_ALIGN;
							MakeTransition = true;
							ReturnEvent.EventType = ES_NO_EVENT;
						}
						break;
				}
			}
			break;
			
    case BALL_LAUNCHING_IR_ALIGN:
			// Execute During function for BALL_LAUNCHING_IR_ALIGN.
			CurrentEvent = DuringBallLaunchingIRAlign(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					case E_IR_BEACON_DETECTED:
						// Apply some delay to the IR Beacon detection
						ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, 10);
						break;
					
					case E_MOTOR_TIMEOUT:
						StopMotors();
						NextState = BALL_LAUNCHING_LAUNCH;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
						
				}
			}
			break;
			
    case BALL_LAUNCHING_LAUNCH:
			// Execute During function for BALL_LAUNCHING_LAUNCH.
			CurrentEvent = DuringBallLaunchingLaunch(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					
					case E_MOTOR_TIMEOUT:
						StopMotors();
						if (MotorTimeoutCase == 0) {
							TurnOnShooter();
							ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, 700);
							MotorTimeoutCase = 1;
						} else if (MotorTimeoutCase == 1) {
							ServoForward();
							ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, 50);
							MotorTimeoutCase = 2;
						} else if (MotorTimeoutCase == 2) {
							ServoReverse();
							ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, 50);
							MotorTimeoutCase = 3;
						} else if (MotorTimeoutCase == 3) {
							ServoForward();
							ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, 50);
							MotorTimeoutCase = 4;
						} else if (MotorTimeoutCase == 4) {
							ServoReverse();
							ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, 50);
							MotorTimeoutCase = 5;
						} else if (MotorTimeoutCase == 5) {
							TurnOffShooter();
							ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, 400);
							MotorTimeoutCase = 6;
						} else {
							MotorTimeoutCase = 0;
							NextState = BALL_LAUNCHING_EXIT;
							MakeTransition = true;
							ReturnEvent.EventType = ES_NO_EVENT;
						}
						break;
				}
			}
			break;
			
    case BALL_LAUNCHING_EXIT:
			// Execute During function for BALL_LAUNCHING_EXIT.
			CurrentEvent = DuringBallLaunchingExit(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					case E_MOTOR_TIMEOUT:
						if (MotorTimeoutCase == 0) {
							RotateCCW(40, 60);
							MotorTimeoutCase = 1;
						} else {
							StopMotors();
							MotorTimeoutCase = 0;
							ES_Event Event = {E_BALL_LAUNCHING_EXIT, 0};
							PostMasterSM(Event);
						}
						break;
				}
			}
			break;
	}

	// If we are making a state transition
	if (MakeTransition == true) {
		// Execute exit function for current state
		CurrentEvent.EventType = ES_EXIT;
		RunBallLaunchingSM(CurrentEvent);
		CurrentState = NextState;
		RunBallLaunchingSM(EntryEventKind);
	}
	return(ReturnEvent);
}


/****************************************************************************
Function:			StartBallLaunchingSM
Parameters:		ES_Event CurrentEvent
Returns:			void
Description:	Does any required initialization for this state machine
****************************************************************************/
void StartBallLaunchingSM(ES_Event CurrentEvent) {
	// Initialize the state variable
	CurrentState = BALL_LAUNCHING_ENTRY;
	// Let the Run function init the lower level state machines
  RunBallLaunchingSM(CurrentEvent);
  return;
}


/****************************************************************************
Function:			QueryBallLaunchingSM
Parameters:		None
Returns:			BallLaunchingState_t, the current state of the Ball Launching state machine
Description:	Returns the current state of the Ball Launching state machine
****************************************************************************/
BallLaunchingState_t QueryBallLaunchingSM(void) {
	return(CurrentState);
}


/*------------------------- Private Function Code -------------------------*/

static ES_Event DuringBallLaunchingEntry(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Ball_Launching: BALL_LAUNCHING_ENTRY1\r\n");
		StopMotors();
		ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, 100);
		MotorTimeoutCase = 0;
	} else if ( Event.EventType == ES_EXIT ) {
	} else {
	}
	return(ReturnEvent);
}

static ES_Event DuringBallLaunchingIRAlign(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Ball_Launching: BALL_LAUNCHING_IR_ALIGN\r\n");
		RotateCW(30, 0);
	} else if ( Event.EventType == ES_EXIT ) {
	} else {
	}
	return(ReturnEvent);
}

static ES_Event DuringBallLaunchingLaunch(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Ball_Launching: BALL_LAUNCHING_LAUNCH\r\n");
		RotateCCW(30, 10);
	} else if ( Event.EventType == ES_EXIT ) {
	} else {
	}
	return(ReturnEvent);
}

static ES_Event DuringBallLaunchingExit(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Ball_Launching: BALL_LAUNCHING_EXIT\r\n");
		DriveForwardWithBias(100, 100, 140);
	} else if ( Event.EventType == ES_EXIT ) {
	} else {
	}
	return(ReturnEvent);
}
