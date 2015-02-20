/****************************************************************************
Module: MapKeys.c
Description: This module maps keystrokes to events.
Author: Kyle Moy, 2/18/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
#include <stdio.h>
#include <ctype.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MapKeys.h"
#include "SM_Master.h"


/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function: 		InitMapKeys
Parameters:		uint8_t Priority, the priority of this service
Returns:			bool, false if error in initialization, true otherwise
Description:	Saves away the priority, and does any other required
							initialization for this service
****************************************************************************/
bool InitMapKeys (uint8_t Priority) {
  MyPriority = Priority;
  return true;
}

/****************************************************************************
Function: 		PostMapKeys
Parameters:		EF_Event ThisEvent, the event to post to the queue
Returns:			bool, false if the Enqueue operation failed, true otherwise
Description:	Posts an event to this state machine's queue
****************************************************************************/
bool PostMapKeys(ES_Event ThisEvent) {
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
Function:			RunMapKeys
Parameters:		ES_Event ThisEvent, the event to process
Returns:			ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise
Description:	Maps keys to events
****************************************************************************/
ES_Event RunMapKeys(ES_Event ThisEvent) {
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  if (ThisEvent.EventType == ES_NEW_KEY) // there was a key pressed
  {
		switch (toupper(ThisEvent.EventParam)) {
			
			// (1) SM_Master Event Triggers
			case 'Q': ThisEvent.EventType = E_RACE_STARTED; printf("E_RACE_STARTED\r\n"); break;
			case 'W': ThisEvent.EventType = E_RACE_FINISHED; printf("E_RACE_FINISHED\r\n"); break;
			
			// (1)SM_Master->(2)SM_Playing Event Triggers
			case 'A': ThisEvent.EventType = E_OBSTACLE_COMPLETED; break;
			case 'S': ThisEvent.EventType = E_BALL_LAUNCHING_COMPLETED; break;
			
			// (1)SM_Master->(2)SM_Playing->(3)SM_Racing Event Triggers
			// Look at Keyboard Numpad for track layout
			case '3': ThisEvent.EventType = E_CORNER1_ENTRY; printf("E_CORNER1_ENTRY\r\n"); break;
			case '6': ThisEvent.EventType = E_CORNER1_EXIT; printf("E_CORNER1_EXIT\r\n"); break;
			case '9': ThisEvent.EventType = E_CORNER2_ENTRY; printf("E_CORNER2_ENTRY\r\n"); break;
			case '8': ThisEvent.EventType = E_CORNER2_EXIT; printf("E_CORNER2_EXIT\r\n"); break;
			case '7': ThisEvent.EventType = E_CORNER3_ENTRY; printf("E_CORNER3_ENTRY\r\n"); break;
			case '4': ThisEvent.EventType = E_CORNER3_EXIT; printf("E_CORNER3_EXIT\r\n"); break;
			case '1': ThisEvent.EventType = E_CORNER4_ENTRY; printf("E_CORNER4_ENTRY\r\n"); break;
			case '2': ThisEvent.EventType = E_CORNER4_EXIT; printf("E_CORNER4_EXIT\r\n"); break;
		}
		PostMasterSM(ThisEvent);
	}
  return ReturnEvent;
}


