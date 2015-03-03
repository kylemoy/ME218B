/****************************************************************************
Module: SM_Racing.c
Description:
	The third level state machine for our robot in the RACING state.
	Contains two states: STRAIGHT, CORNER
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
#include "DRS.h"
#include "DriveMotorPID.h"


/*----------------------------- Module Defines ----------------------------*/
#define ENTRY_STATE STRAIGHT

/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringStraight(ES_Event Event);
static ES_Event DuringCorner(ES_Event Event);



/*---------------------------- Module Variables ---------------------------*/
static RacingState_t CurrentState;
static bool WillCrossObstacle = true;
static bool WillBallLaunch = true;
static uint8_t MotorTimeoutCase = 0;
static GamefieldPosition_t CurrentStraight;


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
		case STRAIGHT:
		default:
			// Execute During function for STRAIGHT.
			// ES_ENTRY & ES_EXIT are processed here allow the lower
			// level state machines to re-map or consume the event
			CurrentEvent = DuringStraight(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					
					case E_MOTOR_TIMEOUT:
						// This is for the case where we timeout for a encoder movement 
						// on Straight 3, moving towards the Obstacle Entry
						if (CurrentStraight == Straight3 && WillCrossObstacle) {
								StopMotors();
								printf("Entering the Obstacle\r\n");
								ES_Event Event = {E_OBSTACLE_CROSSING_ENTRY, 0};
								PostMasterSM(Event);
								
								
						} else if (CurrentStraight == Straight2 && WillBallLaunch) {
							StopMotors();
							printf("Entering the Obstacle\r\n");
							ES_Event Event = {E_BALL_LAUNCHING_ENTRY, 0};
							PostMasterSM(Event);
						
						// This is for the standard case where we timeout for a encoder movement 
						// on any any straight, to slow down before a corner
						} else {
							SetPIDgains(0.05, 0.02, 0);
							DriveForwardWithBias(105, 100, 0);
						}
						break;
						
					case E_BUMP_DETECTED:
						printf("Bump is detected\r\n");
						NextState = CORNER;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
					
					case E_DRS_UPDATED:
						if (CurrentStraight == Straight3) {
							PrintMyKartStatus();
						}
						// Test for entry into ball shooting using DRS
						//if (CurrentStraight == Straight2 && GetMyKart().KartY > BallLaunchingEntryYBound) {
						//	if (WillBallLaunch) {
						//		printf("Passed the Ball Shooting Entry Y-Bound = %d.\r\n", BallLaunchingEntryYBound);
						//		printf("Entering the Ball Launch\r\n");
						//		ES_Event Event = {E_BALL_LAUNCHING_ENTRY, 0};
						//		PostMasterSM(Event);
						//	}
						
						// Test for entry into obstacle crossing using DRS
						//}	else if (CurrentStraight == Straight3 && GetMyKart().KartX > ObstacleEntryXBound) {
						//	if (WillCrossObstacle) {
						//		printf("Passed the Obstacle Entry X-Bound = %d.\r\n", ObstacleEntryXBound);
						//		printf("Entering the Obstacle\r\n");
						//		ES_Event Event = {E_OBSTACLE_CROSSING_ENTRY, 0};
						//		PostMasterSM(Event);
						//	}
						//}						
						//break;
				}
			}
			break;
			
		case CORNER:
			// Execute During function for CORNER.
			CurrentEvent = DuringCorner(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				switch (CurrentEvent.EventType) {
					
					case E_MOTOR_TIMEOUT:
						if (MotorTimeoutCase == 0) {
							PivotCCWwithSetTicks(150, 18);
							MotorTimeoutCase = 1;
						} else if (MotorTimeoutCase == 1) {
							DriveBackwardsWithBias(100, 100, 150);
							ClearSumError();
							MotorTimeoutCase = 2;
						} else {
							StopMotors();
							MotorTimeoutCase = 0;
							// Update to the next straight
							switch (CurrentStraight) {
								case Straight1: CurrentStraight = Straight2; break;
								case Straight2: CurrentStraight = Straight3; break;
								case Straight3: CurrentStraight = Straight4; break;
								case Straight4: CurrentStraight = Straight1; break;
							}
							NextState = STRAIGHT;
							MakeTransition = true;
							ReturnEvent.EventType = ES_NO_EVENT;
							
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
		// Initialize the state variable based on current position
		switch (GetMyKart().GamefieldPosition) {
			case Undefined: case Straight1: case Corner4: default:
				CurrentStraight = Straight1; break;
			case Corner1: case Straight2:
				CurrentStraight = Straight2; break;
			case Corner2: case Straight3:
				CurrentStraight = Straight3; break;
			case Corner3: case Straight4:
				CurrentStraight = Straight4; break;
		}
		CurrentState = STRAIGHT;
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

static ES_Event DuringStraight(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Racing: STRAIGHT (%s)\r\n", GamefieldPositionString(CurrentStraight));
		
		// Make it so that we attempt a ball launch every lap
		if (CurrentStraight == Straight1) {
			WillBallLaunch = true;
		}
		
		// If we just exited out of the ball launch area,
		// then let's not trip the ball launch again
		if (WillBallLaunch && Event.EventType == ES_ENTRY_HISTORY && CurrentStraight == Straight2) {
			WillBallLaunch = false;
			//SetPIDgains(0.1, 0.5, 0);
			//DriveForwardWithSetDistance(500, 400);
			
			SetPIDgains(0.05, 0.02, 0);
			DriveForwardWithBias(105, 100, 0);
			
		// If we are triggering a encoder movement to ball launch
		} else if (CurrentStraight == Straight2 && WillBallLaunch) {
			SetPIDgains(0.05, 0.02, 0);
			DriveForwardWithSetDistance(200, 1000);
			
		// If we are triggering a encoder movement towards the obstacle
		} else if (CurrentStraight == Straight3 && WillCrossObstacle) {
			SetPIDgains(0.05, 0.02, 0);
			DriveForwardWithSetDistance(200, 1250);
			
		// Standard encoder movement for any other lap leg
		} else {
			SetPIDgains(0.1, 0.5, 0);
			DriveForwardWithSetDistance(500, 1000);
		}
	} else if ( Event.EventType == ES_EXIT ) {
	} else {
	}
	return(ReturnEvent);
}

static ES_Event DuringCorner(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM3_Racing: CORNER (%s)\r\n", GamefieldPositionString(CurrentStraight));
		// Drive backwards a little bit
		DriveBackwardsWithBias(100, 100, 25);
		MotorTimeoutCase = 0;
	} else if ( Event.EventType == ES_EXIT ) {
	} else {
	}
	return(ReturnEvent);
}
