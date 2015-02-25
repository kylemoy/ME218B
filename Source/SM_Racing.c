/****************************************************************************
Module: SM_Racing.h
Description:
	The third level state machine for our robot in the RACING state.
	Contains eight states: STRAIGHT1, CORNER1, STRAIGHT2, CORNER2,
												 STRAIGHT3, CORNER3, STRAIGHT4, CORNER4
Author: Kyle Moy, 2/19/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"

// Module Libraries
#include "SM_Racing.h"
#include "Display.h"
#include "DriveMotors.h"
#include "SM_Navigation.h"
#include "GamefieldPositions.h"
#include "SM_Master.h"


/*----------------------------- Module Defines ----------------------------*/
#define ENTRY_STATE STRAIGHT1

/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringStraight1(ES_Event Event);
static ES_Event DuringCorner1(ES_Event Event);
static ES_Event DuringStraight2(ES_Event Event);
static ES_Event DuringCorner2(ES_Event Event);
static ES_Event DuringStraight3(ES_Event Event);
static ES_Event DuringCorner3(ES_Event Event);
static ES_Event DuringStraight4(ES_Event Event);
static ES_Event DuringCorner4(ES_Event Event);


/*---------------------------- Module Variables ---------------------------*/
static RacingState_t CurrentState;
static bool WillCrossObstacle = true;
static bool WillBallLaunch = true;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function:			RunRacingSM
Parameters:		ES_Event CurrentEvent, the event to process
Returns:			ES_Event, an event to return
Description:	The run function for the racing state machine
Notes:				Uses nested switch/case to implement the machine.
****************************************************************************/
ES_Event RunRacingSM(ES_Event CurrentEvent) {
	bool MakeTransition = false;	// Assume not making a transition
	RacingState_t NextState = CurrentState;
	ES_Event EntryEventKind = {ES_ENTRY, 0};// Default to normal entry to new state
	ES_Event ReturnEvent = {ES_NO_EVENT, 0}; // Assume no error

	switch(CurrentState) {
		case STRAIGHT1:
		default:
			// Execute During function for STRAIGHT1.
			// ES_ENTRY & ES_EXIT are processed here allow the lower
			// level state machines to re-map or consume the event
			CurrentEvent = DuringStraight1(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
//					case E_MOTOR_TIMEOUT:
					case E_CORNER1_ENTRY:
						NextState = CORNER1;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;
			
		case CORNER1:
			// Execute During function for CORNER1.
			CurrentEvent = DuringCorner1(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
//					// case E_MOTOR_TIMEOUT:
//						// After turning is complete
//						//DriveForward();
//					case E_MOTOR_TIMEOUT:
					case E_CORNER1_EXIT:
						NextState = STRAIGHT2;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;
			
    case STRAIGHT2:
			// Execute During function for STRAIGHT2.
			CurrentEvent = DuringStraight2(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					//case E_MOTOR_TIMEOUT:
					case E_CORNER2_ENTRY:
						NextState = CORNER2;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;

		case CORNER2:
			// Execute During function for CORNER2.
			CurrentEvent = DuringCorner2(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					//case E_MOTOR_TIMEOUT:
						// After turning is complete
						//DriveForward();
					//case E_MOTOR_TIMEOUT:
					case E_CORNER2_EXIT:
						NextState = STRAIGHT3;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;
			
		case STRAIGHT3:
			// Execute During function for STRAIGHT3.
			CurrentEvent = DuringStraight3(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					//case E_MOTOR_TIMEOUT:
					case E_CORNER3_ENTRY:
						NextState = CORNER3;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;
			
    case CORNER3:
			// Execute During function for CORNER3.
			CurrentEvent = DuringCorner3(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					//case E_MOTOR_TIMEOUT:
						// After turning is complete
						//DriveForward();
					//case E_MOTOR_TIMEOUT:
					case E_CORNER3_EXIT:
						NextState = STRAIGHT4;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;
			
		case STRAIGHT4:
			// Execute During function for STRAIGHT4.
			CurrentEvent = DuringStraight4(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					//case E_MOTOR_TIMEOUT:
					case E_CORNER4_ENTRY:
						NextState = CORNER4;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;
			
    case CORNER4:
			// Execute During function for CORNER4.
			CurrentEvent = DuringCorner4(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					//case E_MOTOR_TIMEOUT:
						// After turning is complete
						//DriveForward();
					//case E_MOTOR_TIMEOUT:
					case E_CORNER4_EXIT:
						NextState = STRAIGHT1;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;
	}
	
	// If we are making a state transition
	if (MakeTransition == true) {
		// Execute exit function for current state
		CurrentEvent.EventType = ES_EXIT;
		RunRacingSM(CurrentEvent);
		CurrentState = NextState; //Modify state variable
		// Execute entry function for new state
		// this defaults to ES_ENTRY
		RunRacingSM(EntryEventKind);
	}
	// In the absence of an error the top level state machine should
	// always return ES_NO_EVENT, which we initialized at the top of func
	return(ReturnEvent);
}


/****************************************************************************
Function:			StartRacingSM
Parameters:		ES_Event CurrentEvent
Returns:			void
Description:	Does any required initialization for this state machine
****************************************************************************/
void StartRacingSM(ES_Event CurrentEvent) {
	if (ES_ENTRY_HISTORY != CurrentEvent.EventType) {
		// Initialize the state variable
		CurrentState = STRAIGHT1;
	}
  
	// Let the Run function init the lower level state machines
  RunRacingSM(CurrentEvent);
  return;
}


/****************************************************************************
Function:			QueryRacingSM
Parameters:		None
Returns:			RacingState_t, the current state of the Racing state machine
Description:	Returns the current state of the Racing state machine
****************************************************************************/
RacingState_t QueryRacingSM(void) {
	return(CurrentState);
}


/*------------------------- Private Function Code -------------------------*/

static ES_Event DuringStraight1(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Racing: STRAIGHT1\r\n");
		SetTargetTheta(East);
		SetTargetPosition(Corner1X, Corner1Y);
		StartNavigationSM(Event);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunNavigationSM(Event);
	}
	return(ReturnEvent);
}

static ES_Event DuringCorner1(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Racing: CORNER1\r\n");
		SetTargetTheta(North);
		SetTargetPosition(Corner2X, Corner2Y);
		StartNavigationSM(Event);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunNavigationSM(Event);
	}
	return(ReturnEvent);
}

static ES_Event DuringStraight2(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Racing: STRAIGHT2\r\n");
		if (WillBallLaunch && Event.EventType != ES_ENTRY_HISTORY) {
			ES_Event Event = {E_BALL_LAUNCHING_START};
			PostMasterSM(Event);
		} else {
			SetTargetTheta(North);
			SetTargetPosition(Corner2X, Corner2Y);
			StartNavigationSM(Event);
		}
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunNavigationSM(Event);
	}
	return(ReturnEvent);
}

static ES_Event DuringCorner2(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Racing: CORNER2\r\n");
		SetTargetTheta(West);
		SetTargetPosition(Corner3X, Corner3Y);
		StartNavigationSM(Event);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunNavigationSM(Event);
	}
	return(ReturnEvent);
}

static ES_Event DuringStraight3(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Racing: STRAIGHT3\r\n");
		if (WillCrossObstacle) {
			ES_Event Event = {E_OBSTACLE_CROSSING_START};
			PostMasterSM(Event);
		} else {
			SetTargetTheta(West);
			SetTargetPosition(Corner3X, Corner3Y);
			StartNavigationSM(Event);
		}
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunNavigationSM(Event);
	}
	return(ReturnEvent);
}

static ES_Event DuringCorner3(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Racing: CORNER3\r\n");
		SetTargetTheta(South);
		SetTargetPosition(Corner4X, Corner4Y);
		StartNavigationSM(Event);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunNavigationSM(Event);
	}
	return(ReturnEvent);
}

static ES_Event DuringStraight4(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM_Racing3: STRAIGHT4\r\n");
		SetTargetTheta(South);
		SetTargetPosition(Corner4X, Corner4Y);
		StartNavigationSM(Event);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunNavigationSM(Event);
	}
	return(ReturnEvent);
}

static ES_Event DuringCorner4(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Racing: CORNER4\r\n");
		SetTargetTheta(East);
		SetTargetPosition(Corner1X, Corner1Y);
		StartNavigationSM(Event);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunNavigationSM(Event);
	}
	return(ReturnEvent);
}
