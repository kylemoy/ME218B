/****************************************************************************
Module: DriveMotorsService.c
Description:
	This module handles the motor timer responses and responds to events.
Author: Kyle Moy, 2/22/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
// C Libraries
#include <stdio.h>
#include <ctype.h>

// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"

// Module Libraries
#include "DriveMotors.h"
#include "DriveMotorsService.h"
#include "SM_Master.h"
#include "DriveMotorPID.h"


/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
bool R_Tick_TimedOut = false;
bool L_Tick_TimedOut = false;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function: 		InitDriveMotorsService
Parameters:		uint8_t Priority, the priority of this service
Returns:			bool, false if error in initialization, true otherwise
Description:	Saves away the priority, and does any other required
							initialization for this service
****************************************************************************/
bool InitDriveMotorsService(uint8_t Priority) {
  MyPriority = Priority;
  return true;
}

/****************************************************************************
Function: 		PostDriveMotorsService
Parameters:		EF_Event ThisEvent, the event to post to the queue
Returns:			bool, false if the Enqueue operation failed, true otherwise
Description:	Posts an event to this state machine's queue
****************************************************************************/
bool PostDriveMotorsService(ES_Event ThisEvent) {
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
Function:			RunDriveMotorsService
Parameters:		ES_Event ThisEvent, the event to process
Returns:			ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise
Description:	Runs the drive motors service, responds to timeout events
****************************************************************************/
ES_Event RunDriveMotorsService(ES_Event ThisEvent) {
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	
	// Event that might get posted to SM_Master
	ES_Event Event;
	
	switch (ThisEvent.EventType) {
		// Motor Timer expired, so stop the motors
		case ES_TIMEOUT:
			//printf("Motor timer expired, stopping motors\r\n");
			StopMotors();
			// Post this event to the master state machine
			Event.EventType = E_MOTOR_TIMEOUT;
			PostMasterSM(Event);
			break;
//		
//		case E_MOTOR_R_TICK_TIMEOUT:
//			if (!R_Tick_TimedOut)
//				printf("E_MOTOR_R_TICK_TIMEOUT\r\n");
//			R_Tick_TimedOut = true;
//			SetTargetRPMR(0);
//			if (L_Tick_TimedOut) {
//				Event.EventType = E_MOTOR_TICK_TIMEOUT;
//				printf("Both R and L Timed Out, so E_MOTOR_TICK_TIMEOUT\r\n");
//				PostMasterSM(Event);
//				R_Tick_TimedOut = false;
//				L_Tick_TimedOut = false;
//			}
//			break;
//			
//		case E_MOTOR_L_TICK_TIMEOUT:
//			if (!L_Tick_TimedOut)
//				printf("E_MOTOR_L_TICK_TIMEOUT\r\n");
//			StopMotors();
//			L_Tick_TimedOut = true;
//			SetTargetRPML(0);
//			if (R_Tick_TimedOut) {
//				printf("Both R and L Timed Out, so E_MOTOR_TICK_TIMEOUT\r\n");
//				Event.EventType = E_MOTOR_TICK_TIMEOUT;
//				PostMasterSM(Event);
//				R_Tick_TimedOut = false;
//				L_Tick_TimedOut = false;
//			}
//			break;
			
			
		case E_MOTOR_TICK_TIMEOUT:
			printf("E_MOTOR_TICK_TIMEOUT\r\n");
			StopMotors();
			Event.EventType = E_MOTOR_TIMEOUT;
			PostMasterSM(Event);
			break;
		
		default:
			break;
	}
  return ReturnEvent;
}


