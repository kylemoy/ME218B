/****************************************************************************
Module: Display.h
Description: This module updates the console with the state of the robot
Author: Kyle Moy, 2/19/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
// C Libraries
#include <stdio.h>

// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"

// Module Libraries
#include "DRS.h"
#include "Display.h"

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;


/*---------------------------- Module Functions ---------------------------*/
/****************************************************************************
Function: 		UpdateDisplay
Parameters:		void
Returns:			void
Description:	Updates the console with the robot's state information
****************************************************************************/
void UpdateDisplay(void) {
	clrScrn();
	// Right now, not using this function yet
}

/****************************************************************************
Function: 		InitDisplay
Parameters:		uint8_t Priority, the priority of this service
Returns:			bool, false if error in initialization, true otherwise
Description:	Saves away the priority, and does any other required
							initialization for this service
****************************************************************************/
bool InitDisplay(uint8_t Priority) {
  MyPriority = Priority;
  // Start the display timer if we are using it
	//if (DisplayDRSInfo)
	//	ES_Timer_InitTimer(DISPLAY_TIMER, DISPLAY_UPDATE_TIME);
  return true;
}

/****************************************************************************
Function: 		PostDisplay
Parameters:		EF_Event ThisEvent, the event to post to the queue
Returns:			bool, false if the Enqueue operation failed, true otherwise
Description:	Posts an event to this state machine's queue
****************************************************************************/
bool PostDisplay(ES_Event ThisEvent) {
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
Function:			RunDisplay
Parameters:		ES_Event ThisEvent, the event to process
Returns:			ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise
Description:	Prints display to console
****************************************************************************/
ES_Event RunDisplay(ES_Event ThisEvent) {
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	// Display timer timeout, time to update the display
	if (ThisEvent.EventType == ES_TIMEOUT) {
     UpdateDisplay();
     ES_Timer_InitTimer(DISPLAY_TIMER, DISPLAY_UPDATE_TIME);
	}
	// An event occurred, print it to the console if DisplayEvents is enabled
	if (DisplayEvents) {
		switch (ThisEvent.EventType) {
			// SM_Playing Events
			case E_RACE_STARTED: printf("(EVENT) E_RACE_STARTED\r\n"); break;
			case E_RACE_FINISHED: printf("(EVENT) E_RACE_FINISHED\r\n"); break;
			
			// SM_Master Events
			case E_OBSTACLE_COMPLETED: printf("(EVENT) E_OBSTACLE_COMPLETED\r\n"); break;
			case E_BALL_LAUNCHING_COMPLETED: printf("(EVENT) E_BALL_LAUNCHING_COMPLETED\r\n"); break;
			
			// SM_Racing Events
			case E_CORNER1_ENTRY: printf("(EVENT) E_CORNER1_ENTRY\r\n"); break;
			case E_CORNER1_EXIT: printf("(EVENT) E_CORNER1_EXIT\r\n"); break;
			case E_CORNER2_ENTRY: printf("(EVENT) E_CORNER2_ENTRY\r\n"); break;
			case E_CORNER2_EXIT: printf("(EVENT) E_CORNER2_EXIT\r\n"); break;
			case E_CORNER3_ENTRY: printf("(EVENT) E_CORNER3_ENTRY\r\n"); break;
			case E_CORNER3_EXIT: printf("(EVENT) E_CORNER3_EXIT\r\n"); break;
			case E_CORNER4_ENTRY: printf("(EVENT) E_CORNER4_ENTRY\r\n"); break;
			case E_CORNER4_EXIT: printf("(EVENT) E_CORNER4_EXIT\r\n"); break;
										
			// SM_DRS Events
			case E_NEW_DRS_QUERY: printf("(EVENT) E_NEW_DRS_QUERY\r\n"); break;
			case E_DRS_EOT: printf("(EVENT) E_DRS_EOT\r\n"); break;
			
			default: break;
		}
	}
  return ReturnEvent;
}
