/****************************************************************************
Module: SM_ObstacleCrossing.h
Description:
	The third level state machine for our robot in the OBSTACLE_CROSSING state.
	Contains three states: OBSTACLE_ENTRY, CROSSING, OBSTACLE_EXIT
Author: Kyle Moy, 2/25/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"

// Module Libraries
#include "SM_ObstacleCrossing.h"
#include "Display.h"
#include "DriveMotors.h"
#include "SM_Navigation.h"
#include "GamefieldPositions.h"
#include "SM_Master.h"
#include "DriveMotorPID.h"


/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringObstacleEntry(ES_Event Event);
static ES_Event DuringCrossing(ES_Event Event);
static ES_Event DuringObstacleExit(ES_Event Event);


/*---------------------------- Module Variables ---------------------------*/
static ObstacleCrossingState_t CurrentState;
static uint8_t MotorTimeoutCase = 0;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function:			RunObstacleCrossingSM
Parameters:		ES_Event CurrentEvent, the event to process
Returns:			ES_Event, an event to return
Description:	The run function for the racing state machine
Notes:				Uses nested switch/case to implement the machine.
****************************************************************************/
ES_Event RunObstacleCrossingSM(ES_Event CurrentEvent) {
	bool MakeTransition = false;	// Assume not making a transition
	ObstacleCrossingState_t NextState = CurrentState;
	ES_Event EntryEventKind = {ES_ENTRY, 0};// Default to normal entry to new state
	ES_Event ReturnEvent = {ES_NO_EVENT, 0}; // Assume no error

	switch(CurrentState) {
		case OBSTACLE_ENTRY:
		default:
			// Execute During function for OBSTACLE_ENTRY.
			// ES_ENTRY & ES_EXIT are processed here allow the lower
			// level state machines to re-map or consume the event
			CurrentEvent = DuringObstacleEntry(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					
					case E_MOTOR_TIMEOUT:
						if (MotorTimeoutCase == 0) {
							SetPIDgains(0.05, 0.02, 0);
							MotorTimeoutCase = 1;
							PivotCCWwithSetTicks(150, 18);
						} else if (MotorTimeoutCase == 1) {
							DriveBackwardsWithBias(100, 100, 150);
							MotorTimeoutCase = 2;
						} else {
							StopMotors();
							MotorTimeoutCase = 0;
							NextState = CROSSING;
							MakeTransition = true;
							ReturnEvent.EventType = ES_NO_EVENT;
						}
						break;
				}
			}
			break;
			
		case CROSSING:
			// Execute During function for CROSSING.
			CurrentEvent = DuringCrossing(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					
					case E_BUMP_DETECTED:
							StopMotors();
							NextState = OBSTACLE_EXIT;
							MakeTransition = true;
							ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;
			
    case OBSTACLE_EXIT:
			// Execute During function for OBSTACLE_EXIT.
			CurrentEvent = DuringObstacleExit(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					case E_MOTOR_TIMEOUT:
						if (MotorTimeoutCase == 0) {
							PivotCCWwithSetTicks(150, 12);
							MotorTimeoutCase = 1;
						} else {
							StopMotors();
							MotorTimeoutCase = 0;
							ES_Event Event = {E_OBSTACLE_CROSSING_EXIT, 0};
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
		RunObstacleCrossingSM(CurrentEvent);
		CurrentState = NextState;
		RunObstacleCrossingSM(EntryEventKind);
	}
	return(ReturnEvent);
}


/****************************************************************************
Function:			StartObstacleCrossingSM
Parameters:		ES_Event CurrentEvent
Returns:			void
Description:	Does any required initialization for this state machine
****************************************************************************/
void StartObstacleCrossingSM(ES_Event CurrentEvent) {
	if (ES_ENTRY_HISTORY != CurrentEvent.EventType) {
		// Initialize the state variable
		CurrentState = OBSTACLE_ENTRY;
	}
	// Let the Run function init the lower level state machines
  RunObstacleCrossingSM(CurrentEvent);
  return;
}


/****************************************************************************
Function:			QueryObstacleCrossingSM
Parameters:		None
Returns:			ObstacleCrossingState_t, the current state of the Obstacle Crossing state machine
Description:	Returns the current state of the Obstacle Crossing state machine
****************************************************************************/
ObstacleCrossingState_t QueryObstacleCrossingSM(void) {
	return(CurrentState);
}


/*------------------------- Private Function Code -------------------------*/

static ES_Event DuringObstacleEntry(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Obstacle_Crossing: OBSTACLE_ENTRY\r\n");StopMotors();
		// A timer to allow bot to come to a standstill
		ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, 100);
		MotorTimeoutCase = 0;
	} else if ( Event.EventType == ES_EXIT ) {
	} else {
		
	}
	return(ReturnEvent);
}

static ES_Event DuringCrossing(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Obstacle_Crossing: CROSSING\r\n");
		// Drive forward (until bump detected)
		DriveForward(100, 0);
	} else if ( Event.EventType == ES_EXIT ) {
	} else {
		DriveForward(100, 0);
	}
	return(ReturnEvent);
}

static ES_Event DuringObstacleExit(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Obstacle_Crossing: OBSTACLE_EXIT\r\n");
		// Drive backwards a little bit
		DriveBackwardsWithBias(100, 100, 25);
	} else if ( Event.EventType == ES_EXIT ) {
	} else {
	}
	return(ReturnEvent);
}
