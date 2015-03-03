/****************************************************************************
Module: SM_Master.c
Description:
	The top level state machine for our robot.
	Contains four states: WAITING_START, PLAYING, PAUSED, WAITING_FINISHED.
Author: Kyle Moy, 2/18/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"

// Module Libraries
#include "SM_Master.h"
#include "SM_Playing.h"
#include "Display.h"
#include "DriveMotors.h"
#include "BallLauncher.h"
#include "KartSwitchAndLED.h"
#include "DriveMotorPID.h"

/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringWaitingStart(ES_Event Event);
static ES_Event DuringPlaying(ES_Event Event);
static ES_Event DuringPaused(ES_Event Event);
static ES_Event DuringWaitingFinish(ES_Event Event);


/*---------------------------- Module Variables ---------------------------*/
static MasterState_t CurrentState;
static uint8_t MyPriority;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function:			InitMasterSM
Parameters:		uint8_t Priority, the priority of this service
Returns:			boolean, false if error in initialization, true otherwise
Description:	Saves away the priority, and starts the top level state machine
****************************************************************************/
bool InitMasterSM (uint8_t Priority) {
  ES_Event ThisEvent;
  MyPriority = Priority;  // save our priority
  ThisEvent.EventType = ES_ENTRY;
  // Start the Master State machine
  StartMasterSM(ThisEvent);
  return true;
}


/****************************************************************************
Function:			PostMasterSM
Parameters:		ES_Event ThisEvent, the event to post to the queue
Returns:			boolean, false if the post operation failed, true otherwise
Description:	Posts an event to this state machine's queue
****************************************************************************/
bool PostMasterSM(ES_Event ThisEvent) {
  return ES_PostToService( MyPriority, ThisEvent);
}


/****************************************************************************
Function:			RunMasterSM
Parameters:		ES_Event CurrentEvent, the event to process
Returns:			ES_Event, an event to return
Description:	The run function for the top level state machine 
Notes:				Uses nested switch/case to implement the machine.
****************************************************************************/
ES_Event RunMasterSM(ES_Event CurrentEvent) {
	bool MakeTransition = false;	// Assume not making a transition
	MasterState_t NextState = CurrentState;
	ES_Event EntryEventKind = {ES_ENTRY, 0};// Default to normal entry to new state
	ES_Event ReturnEvent = {ES_NO_EVENT, 0}; // Assume no error

	// Run the Display service to display the event
	RunDisplay(CurrentEvent);
	
	switch(CurrentState) {
		case WAITING_START:
			// Execute During function for WAITING_START.
			// ES_ENTRY & ES_EXIT are processed here allow the lower
			// level state machines to re-map or consume the event
			CurrentEvent = DuringWaitingStart(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					case E_RACE_STARTED:
						NextState = PLAYING;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;
			
		case PLAYING:
			// Execute During function for PLAYING.
			CurrentEvent = DuringPlaying(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					//case E_MOTOR_TIMEOUT:
					//	StopMotors();
					//	break;
					
					case E_RACE_CAUTION:
						NextState = PAUSED;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				
					case E_RACE_FINISHED:
						NextState = WAITING_FINISHED;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;
			
		case PAUSED:
			// Execute During function for PAUSED.
			CurrentEvent = DuringPaused(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					case E_RACE_STARTED:
						NextState = PLAYING;
					  EntryEventKind.EventType = ES_ENTRY_HISTORY;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
				}
			}
			break;
						
			
    case WAITING_FINISHED:
			// Execute During function for WAITING_FINISHED.
			CurrentEvent = DuringWaitingFinish(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					case E_RACE_STARTED:
						NextState = PLAYING;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
					default:
						break;
				}
			}
			break;
	}
	
	// If we are making a state transition
	if (MakeTransition == true) {
		// Execute exit function for current state
		CurrentEvent.EventType = ES_EXIT;
		RunMasterSM(CurrentEvent);
		CurrentState = NextState; //Modify state variable
		// Execute entry function for new state
		// this defaults to ES_ENTRY
		RunMasterSM(EntryEventKind);
	}
	// In the absence of an error the top level state machine should
	// always return ES_NO_EVENT, which we initialized at the top of func
	return(ReturnEvent);
}


/****************************************************************************
Function:			StartMasterSM
Parameters:		ES_Event CurrentEvent
Returns:			void
Description:	Does any required initialization for this state machine
****************************************************************************/
void StartMasterSM(ES_Event CurrentEvent) {
  // Initialize the state variable
  CurrentState = WAITING_START;
	// Let the Run function init the lower level state machines
  RunMasterSM(CurrentEvent);
  return;
}


/****************************************************************************
Function: 		QueryMasterSM
Parameters: 	none
Returns: 			MasterState_t, the current state of the Master state machine
Description: 	Returns the current state of the Master state machine.
****************************************************************************/
MasterState_t QueryMasterSM(void) {
	return CurrentState;
}


/*------------------------- Private Function Code -------------------------*/

static ES_Event DuringWaitingStart(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		// Implement any entry actions required for this state machine
		if(DisplayEntryStateTransitions && DisplaySM_Master) printf("SM1_Master: WAITING_START\r\n");
		StopMotors();
		TurnOffShooter();
	} else if (Event.EventType == ES_EXIT) {
	} else {
	}
	return(ReturnEvent);
}


static ES_Event DuringPlaying(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Master) printf("SM1_Master: PLAYING\r\n");
		TurnOnRaceLED();
		StartPlayingSM(Event);
		
			//SetPIDgains(0.03, 0.03, 0);
		
							//DriveForwardWithBias(105, 100, 0);
			//DriveForwardWithSetDistance(100, 1000);
	} else if (Event.EventType == ES_EXIT) {
		TurnOffRaceLED();
	} else {
		RunPlayingSM(Event);
	}
	return(ReturnEvent);
}


static ES_Event DuringPaused(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Master) printf("SM1_Master: PAUSED\r\n");
		StopMotors();
	} else if (Event.EventType == ES_EXIT) {
	} else {
	}
	return(ReturnEvent);
}

static ES_Event DuringWaitingFinish(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Master) printf("SM1_Master: WAITING_FINISHED\r\n");
		StopMotors();
		TurnOffShooter();
	} else if (Event.EventType == ES_EXIT) {
	} else {
	}
	return(ReturnEvent);
}
