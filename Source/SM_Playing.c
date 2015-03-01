/****************************************************************************
Module: SM_Playing.h
Description:
	The second level state machine for our robot in the PLAYING state.
	Contains three states: RACING, CROSSING_OBSTACLE, and BALL_LAUNCHING.
Author: Kyle Moy, 2/18/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"

// Module Libraries
#include "SM_Playing.h"
#include "SM_Racing.h"
#include "SM_BallLaunching.h"
#include "SM_ObstacleCrossing.h"
#include "Display.h"
#include "DriveMotors.h"

/*----------------------------- Module Defines ----------------------------*/
#define ENTRY_STATE RACING

/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringRacing(ES_Event Event);
static ES_Event DuringCrossingObstacle(ES_Event Event);
static ES_Event DuringBallLaunching(ES_Event Event);


/*---------------------------- Module Variables ---------------------------*/
static PlayingState_t CurrentState;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function:			RunPlayingSM
Parameters:		ES_Event CurrentEvent, the event to process
Returns:			ES_Event, an event to return
Description:	The run function for the top level state machine 
Notes:				Uses nested switch/case to implement the machine.
****************************************************************************/
ES_Event RunPlayingSM(ES_Event CurrentEvent) {
	bool MakeTransition = false;	// Assume not making a transition
	PlayingState_t NextState = CurrentState;
	ES_Event EntryEventKind = {ES_ENTRY, 0};// Default to normal entry to new state
	ES_Event ReturnEvent = {ES_NO_EVENT, 0}; // Assume no error

	switch(CurrentState) {
		case RACING:
			// Execute During function for RACING.
			// ES_ENTRY & ES_EXIT are processed here allow the lower
			// level state machines to re-map or consume the event
			CurrentEvent = DuringRacing(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					case E_OBSTACLE_CROSSING_ENTRY:
						NextState = CROSSING_OBSTACLE;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
					
					case E_BALL_LAUNCHING_ENTRY:
						NextState = BALL_LAUNCHING;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
					
				}
			}
			break;
			
		case CROSSING_OBSTACLE:
			// Execute During function for CROSSING_OBSTACLE.
			CurrentEvent = DuringCrossingObstacle(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					case E_OBSTACLE_CROSSING_EXIT:
						NextState = RACING;
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
					case E_BALL_LAUNCHING_EXIT:
						NextState = RACING;
					  EntryEventKind.EventType = ES_ENTRY_HISTORY;
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
		RunPlayingSM(CurrentEvent);
		CurrentState = NextState; //Modify state variable
		// Execute entry function for new state
		// this defaults to ES_ENTRY
		RunPlayingSM(EntryEventKind);
	}
	// In the absence of an error the top level state machine should
	// always return ES_NO_EVENT, which we initialized at the top of func
	return(ReturnEvent);
}


/****************************************************************************
Function:			StartPlayingSM
Parameters:		ES_Event CurrentEvent
Returns:			void
Description:	Does any required initialization for this state machine
****************************************************************************/
void StartPlayingSM(ES_Event CurrentEvent) {
  // Initialize the state variable
  CurrentState = RACING;
	// Let the Run function init the lower level state machines
  RunPlayingSM(CurrentEvent);
  return;
}


/****************************************************************************
Function:			QueryPlayingSM
Parameters:		None
Returns:			PlayingState_t, the current state of the Playing state machine
Description:	Returns the current state of the Playing state machine
****************************************************************************/
PlayingState_t QueryPlayingSM(void) {
	return(CurrentState);
}


/*------------------------- Private Function Code -------------------------*/

static ES_Event DuringRacing(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		// Implement any entry actions required for this state machine
		if(DisplayEntryStateTransitions && DisplaySM_Playing) printf("SM2_Playing: RACING\r\n");
		// after that start any lower level machines that run in this state
		StartRacingSM(Event);
		
		// repeat the StartxxxSM() functions for concurrent state machines
		// on the lower level
	} else if (Event.EventType == ES_EXIT) {
		// on exit, give the lower levels a chance to clean up first
		//RunLowerLevelSM(Event);
		// repeat for any concurrently running state machines
		// now do any local exit functionality
	} else {
    // do the 'during' function for this state
		// run any lower level state machine
		RunRacingSM(Event);
		// repeat for any concurrent lower level machines
		// do any activity that is repeated as long as we are in this state
	}
	return(ReturnEvent);
}


static ES_Event DuringCrossingObstacle(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Playing) printf("SM2_Playing: CROSSING_OBSTACLE\r\n");
		StartObstacleCrossingSM(Event);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunObstacleCrossingSM(Event);
		
	}
	return(ReturnEvent);
}

static ES_Event DuringBallLaunching(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Playing) printf("SM2_Playing: BALL_LAUNCHING\r\n");
		StartBallLaunchingSM(Event);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		RunBallLaunchingSM(Event);
	}
	return(ReturnEvent);
}
