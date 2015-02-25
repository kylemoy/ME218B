/****************************************************************************
Module: SM_Obstacle_Crossing.h
Description:
	The third level state machine for our robot in the OBSTACLE_CROSSING state.
	Contains three states: STRAIGHT3_OBSTACLE_ENTRY, CROSSING, STRAIGHT1_OBSTACLE_EXIT
Author: Kyle Moy, 2/25/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"

// Module Libraries
#include "SM_Obstacle_Crossing.h"
#include "Display.h"
#include "DriveMotors.h"
#include "SM_Navigation.h"
#include "GamefieldPositions.h"
#include "SM_Master.h"


/*----------------------------- Module Defines ----------------------------*/
#define ENTRY_STATE STRAIGHT3_OBSTACLE_ENTRY

/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringStraight3ObstacleEntry(ES_Event Event);
static ES_Event DuringCrossing(ES_Event Event);
static ES_Event DuringStraight1ObstacleExit(ES_Event Event);


/*---------------------------- Module Variables ---------------------------*/
static ObstacleCrossingState_t CurrentState;


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
		case STRAIGHT3_OBSTACLE_ENTRY:
		default:
			// Execute During function for STRAIGHT3_OBSTACLE_ENTRY.
			// ES_ENTRY & ES_EXIT are processed here allow the lower
			// level state machines to re-map or consume the event
			CurrentEvent = DuringStraight3ObstacleEntry(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					//case E_CORNER1_ENTRY:
					//	NextState = CORNER1;
					//	MakeTransition = true;
					//	ReturnEvent.EventType = ES_NO_EVENT;
					//	break;
				}
			}
			break;
			
		case CROSSING:
			// Execute During function for CROSSING.
			CurrentEvent = DuringCrossing(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
//					// case E_MOTOR_TIMEOUT:
//						// After turning is complete
//						//DriveForward();
//					case E_MOTOR_TIMEOUT:
					//case E_CORNER1_EXIT:
					//	NextState = STRAIGHT2;
					//	MakeTransition = true;
					//	ReturnEvent.EventType = ES_NO_EVENT;
					//	break;
				}
			}
			break;
			
    case STRAIGHT1_OBSTACLE_EXIT:
			// Execute During function for STRAIGHT1_OBSTACLE_EXIT.
			CurrentEvent = DuringStraight1ObstacleExit(CurrentEvent);
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
		RunObstacleCrossingSM(CurrentEvent);
		CurrentState = NextState; //Modify state variable
		// Execute entry function for new state
		// this defaults to ES_ENTRY
		RunObstacleCrossingSM(EntryEventKind);
	}
	// In the absence of an error the top level state machine should
	// always return ES_NO_EVENT, which we initialized at the top of func
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
		CurrentState = STRAIGHT3_OBSTACLE_ENTRY;
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

static ES_Event DuringStraight3ObstacleEntry(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Obstacle_Crossing: STRAIGHT3_OBSTACLE_ENTRY\r\n");
		//SetTargetTheta(East);
		//SetTargetPosition(Corner1X, Corner1Y);
		StartNavigationSM(Event);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunNavigationSM(Event);
	}
	return(ReturnEvent);
}

static ES_Event DuringCrossing(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Obstacle_Crossing: CROSSING\r\n");
		//SetTargetTheta(North);
		//SetTargetPosition(Corner2X, Corner2Y);
		StartNavigationSM(Event);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunNavigationSM(Event);
	}
	return(ReturnEvent);
}

static ES_Event DuringStraight1ObstacleExit(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Obstacle_Crossing: STRAIGHT1_OBSTACLE_EXIT\r\n");

	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
	}
	return(ReturnEvent);
}
