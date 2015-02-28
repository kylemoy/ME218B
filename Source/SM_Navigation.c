/****************************************************************************
Module: SM_Navigation.c
Description:
	A sub-level state machine for our robot that controls the point-destination
	driving navigation system. Used in the RACING state machine to get from
	corner to corner.
	Contains eight states: ORIENTING, DRIVING
	
Author: Kyle Moy, 2/24/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
// C Libraries
#include <math.h>

// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"

// Module Libraries
#include "SM_Navigation.h"
#include "Display.h"
#include "DriveMotors.h"
#include "DRS.h"
#include "GamefieldPositions.h"


/*----------------------------- Module Defines ----------------------------*/
#define ENTRY_STATE DRIVING

/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringOrienting(ES_Event Event);
static ES_Event DuringDriving(ES_Event Event);
static ES_Event DuringWaiting(ES_Event Event);


/*---------------------------- Module Variables ---------------------------*/
static NavigationState_t CurrentState;
static uint8_t TargetX;
static uint8_t TargetY;
static uint16_t TargetTheta;
static uint8_t Xold = 0;
static uint8_t Yold = 0;
static double CalculatedTheta;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function:			RunNavigationSM
Parameters:		ES_Event CurrentEvent, the event to process
Returns:			ES_Event, an event to return
Description:	The run function for the racing state machine
Notes:				Uses nested switch/case to implement the machine.
****************************************************************************/
ES_Event RunNavigationSM(ES_Event CurrentEvent) {
	bool MakeTransition = false;	// Assume not making a transition
	NavigationState_t NextState = CurrentState;
	ES_Event EntryEventKind = {ES_ENTRY, 0};// Default to normal entry to new state
	ES_Event ReturnEvent = {ES_NO_EVENT, 0}; // Assume no error

	switch(CurrentState) {
		case ORIENTING:
		default:
			// Execute During function for ORIENTING.
			// ES_ENTRY & ES_EXIT are processed here allow the lower
			// level state machines to re-map or consume the event
			CurrentEvent = DuringOrienting(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				// Variables to store current values
				int16_t CurrentTheta;
				
				switch (CurrentEvent.EventType) {
					case E_DRS_UPDATED:
						CurrentTheta = GetMyKart().KartTheta;
						PrintMyKartStatus();
					
						if (abs(CurrentTheta - TargetTheta) < 15 || abs(CurrentTheta - TargetTheta - 360) < 15 || abs(CurrentTheta - TargetTheta + 360) < 15) {
							printf("CurrentTheta = %d, Target Theta = %d has been reached, transition to driving\r\n", CurrentTheta, TargetTheta);
							StopMotors();
							NextState = DRIVING;
							MakeTransition = true;
							ReturnEvent.EventType = ES_NO_EVENT;
						}
						break;
				}
			}
			break;
			
		case DRIVING:
			// Execute During function for DRIVING.
			CurrentEvent = DuringDriving(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
				// Variables to store current values
				uint8_t CurrentX;
				uint8_t CurrentY;
				//int16_t DeltaY;
				//int16_t DeltaRPM;
				//uint16_t RPMR;
				//uint16_t RPML;
				
				switch (CurrentEvent.EventType) {
					case E_MOTOR_TIMEOUT:
						NextState = ORIENTING;
						MakeTransition = true;
						ReturnEvent.EventType = ES_NO_EVENT;
						break;
					
					case E_DRS_UPDATED:
						CurrentX = GetMyKart().KartX;
						CurrentY = GetMyKart().KartY;
						//printf("CurrentX = %d and CurrentY = %d, TargetX = %d and TargetY = %d\r\n", CurrentX, CurrentY, TargetX, TargetY);
						//DeltaY = CurrentY- TargetY;
						//DeltaRPM = DeltaY * 1.5;
						//RPML = 100 + DeltaRPM;
						//RPMR = 100 - DeltaRPM;
						//DriveForwardWithBias(RPML, RPMR, 0);
						//printf("Y = %d, TargetY = %d, DeltaY = %d, Drive Forward with RPML = %d, RPMR = %d\r\n", CurrentY, TargetY, DeltaY, RPML, RPMR);
						//PrintMyKartStatus();
						
						if (sqrt(pow(CurrentX - TargetX, 2) + pow(CurrentY - TargetY, 2)) < 10) {
							printf("CurrentX = %d and CurrentY = %d, TargetX = %d and TargetY = %d have been reached, transition to waiting\r\n", CurrentX, CurrentY, TargetX, TargetY);
							
							StopMotors();
							NextState = WAITING;
							MakeTransition = true;
							ReturnEvent.EventType = ES_NO_EVENT;
						}
						break;
				}
			}
			break;
			
		case WAITING:
			// Execute During function for WAITING.
			CurrentEvent = DuringWaiting(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) { // If an event is active
			}
			break;
		}
	
	// If we are making a state transition
	if (MakeTransition == true) {
		// Execute exit function for current state
		CurrentEvent.EventType = ES_EXIT;
		RunNavigationSM(CurrentEvent);
		CurrentState = NextState; //Modify state variable
		// Execute entry function for new state
		// this defaults to ES_ENTRY
		RunNavigationSM(EntryEventKind);
	}
	// In the absence of an error the top level state machine should
	// always return ES_NO_EVENT, which we initialized at the top of func
	return(ReturnEvent);
}


/****************************************************************************
Function:			StartNavigationSM
Parameters:		ES_Event CurrentEvent
Returns:			void
Description:	Does any required initialization for this state machine
****************************************************************************/
void StartNavigationSM(ES_Event CurrentEvent) {
  // Initialize the state variable
  CurrentState = ORIENTING;
	// Let the Run function init the lower level state machines
  RunNavigationSM(CurrentEvent);
  return;
}


/****************************************************************************
Function:			QueryNavigationSM
Parameters:		None
Returns:			PlayingState_t, the current state of the Navigation state machine
Description:	Returns the current state of the Navigation state machine
****************************************************************************/
NavigationState_t QueryNavigationSM(void) {
	return(CurrentState);
}

	
/****************************************************************************
Function:			SetTargetPosition
Parameters:		uint8_t X, the target X coordinate
							uint8_t Y, the target Y coordinate
Returns:			void
Description:	Sets the target coordinate that this state machine will direct to
****************************************************************************/
void SetTargetPosition(uint8_t X, uint8_t Y) {
	TargetX = X;
	TargetY = Y;
}
void SetTargetTheta(uint16_t Theta) {
	TargetTheta = Theta;
}

/*------------------------- Private Function Code -------------------------*/

static ES_Event DuringOrienting(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM4_Navigation: ORIENTING\r\n");
		printf("Rotating towards TargetTheta = %d\r\n", TargetTheta); 
		//RotateCCW(40, 0);
		//DriveForwardWithBias(30, 70, 0);
		//ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, 250);
		if (Xold != 0 && Yold != 0) {
			CalculatedTheta = atan2(GetMyKart().KartY - Yold, GetMyKart().KartX - Xold);
			printf("X1 = %d, Y1 = %d, X2 = %d, Y2 = %d\r\n", Xold, Yold, GetMyKart().KartX, GetMyKart().KartY);
			printf("Calculated Theta = %f\r\n", CalculatedTheta);
		}
		Xold = GetMyKart().KartX;
		Yold = GetMyKart().KartY;
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
	}
	return(ReturnEvent);
}

static ES_Event DuringDriving(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM4_Navigation: DRIVING\r\n");
		printf("Driving towards TargetX = %d, TargetY = %d\r\n", TargetX, TargetY); 
		//DriveForward(100, 0);
		//DriveForward(100, 0);
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		
	}
	return(ReturnEvent);
}

static ES_Event DuringWaiting(ES_Event Event) {
	ES_Event ReturnEvent = Event; // Assume no re-mapping or consumption
	// Process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		if(DisplayEntryStateTransitions && DisplaySM_Racing) printf("SM4_Navigation: WAITING\r\n");
		StopMotors();
	} else if ( Event.EventType == ES_EXIT ) {
		
	} else {
		
	}
	return(ReturnEvent);
}
