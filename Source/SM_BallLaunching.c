/****************************************************************************
Module: SM_Ball_Launching.c
Description:
	The third level state machine for our robot in the BALL_LAUNCHING state.
	Contains four states: BALL_LAUNCHING_ENTRY1, BALL_LAUNCHING_ENTRY2, 
												 BALL_LAUNCHING, BALL_LAUNCHING_EXIT
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


/*----------------------------- Module Defines ----------------------------*/
#define ENTRY_STATE BALL_LAUNCHING_ENTRY

/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringBallLaunchingEntry(ES_Event Event);
static ES_Event DuringBallLaunching(ES_Event Event);
static ES_Event DuringBallLaunchingExit(ES_Event Event);


/*---------------------------- Module Variables ---------------------------*/
static BallLaunchingState_t CurrentState;


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
					//case E_CORNER1_ENTRY:
					//	NextState = CORNER1;
					//	MakeTransition = true;
					//	ReturnEvent.EventType = ES_NO_EVENT;
					//	break;
					case E_BALL_LAUNCHING_ENTRY:
						NextState = BALL_LAUNCHING;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;
			
    case BALL_LAUNCHING:
			// Execute During function for BALL_LAUNCHING.
			CurrentEvent = DuringBallLaunching(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					//case E_MOTOR_TIMEOUT:
					//case E_CORNER2_ENTRY:
					//	NextState = CORNER2;
					//	MakeTransition = true;
					//	ReturnEvent.EventType = ES_NO_EVENT;
					//	break;
				}
			}
			break;
			
    case BALL_LAUNCHING_EXIT:
			// Execute During function for BALL_LAUNCHING_EXIT.
			CurrentEvent = DuringBallLaunchingExit(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					//case E_MOTOR_TIMEOUT:
					//case E_CORNER2_ENTRY:
					//	NextState = CORNER2;
					//	MakeTransition = true;
					//	ReturnEvent.EventType = ES_NO_EVENT;
					//	break;
				}
			}
			break;
	}

	// If we are making a state transition
	if (MakeTransition == true) {
		// Execute exit function for current state
		CurrentEvent.EventType = ES_EXIT;
		RunBallLaunchingSM(CurrentEvent);
		CurrentState = NextState; //Modify state variable
		// Execute entry function for new state
		// this defaults to ES_ENTRY
		RunBallLaunchingSM(EntryEventKind);
	}
	// In the absence of an error the top level state machine should
	// always return ES_NO_EVENT, which we initialized at the top of func
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
		//SetTargetTheta(East);
		//SetTargetPosition(Corner1X, Corner1Y);
		//StartNavigationSM(Event);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		//RunNavigationSM(Event);
	}
	return(ReturnEvent);
}

static ES_Event DuringBallLaunching(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Obstacle_Crossing: BALL_LAUNCHING\r\n");
		//SetTargetTheta(North);
		//SetTargetPosition(Corner2X, Corner2Y);
		//StartNavigationSM(Event);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		//RunNavigationSM(Event);
	}
	return(ReturnEvent);
}

static ES_Event DuringBallLaunchingExit(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Obstacle_Crossing: BALL_LAUNCHING_EXIT\r\n");
		//SetTargetTheta(North);
		//SetTargetPosition(Corner2X, Corner2Y);
		StartNavigationSM(Event);

	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunNavigationSM(Event);
	}
	return(ReturnEvent);
}
