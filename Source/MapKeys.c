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
#include "Display.h"
#include "DRS.h"


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
			case 'Q': ThisEvent.EventType = E_RACE_STARTED; break;
			case 'W': ThisEvent.EventType = E_RACE_FINISHED; break;
			case 'E': ThisEvent.EventType = E_RACE_CAUTION; break;
			
			// (1)SM_Master->(2)SM_Playing Event Triggers
			case 'A': ThisEvent.EventType = E_OBSTACLE_COMPLETED; break;
			case 'S': ThisEvent.EventType = E_TARGET_SUCCESS; break;
			
			// (1)SM_Master->(2)SM_Playing->(3)SM_Racing Event Triggers
			// Look at Keyboard Numpad for track layout
			case '3': ThisEvent.EventType = E_CORNER1_ENTRY; break;
			case '6': ThisEvent.EventType = E_CORNER1_EXIT; break;
			case '9': ThisEvent.EventType = E_CORNER2_ENTRY; break;
			case '8': ThisEvent.EventType = E_CORNER2_EXIT; break;
			case '7': ThisEvent.EventType = E_CORNER3_ENTRY; break;
			case '4': ThisEvent.EventType = E_CORNER3_EXIT; break;
			case '1': ThisEvent.EventType = E_CORNER4_ENTRY; break;
			case '2': ThisEvent.EventType = E_CORNER4_EXIT; break;
			
			// Information Queries
			case 'Z':
				printf("Kart1: X = %d, Y = %d, Laps Left = %d, Obstacle = %d, Target = %d, Gamefield Position = %s\r\n", \
					GetKartData(1).KartX, GetKartData(1).KartY, GetKartData(1).LapsRemaining, \
					GetKartData(1).ObstacleCompleted, GetKartData(1).TargetSuccess, \
					GamefieldPositionString(GetKartData(1).GamefieldPosition));
				break;
			case 'X':
				printf("Kart2: X = %d, Y = %d, Laps Left = %d, Obstacle = %d, Target = %d, Gamefield Position = %s\r\n", \
					GetKartData(2).KartX, GetKartData(2).KartY, GetKartData(2).LapsRemaining, \
					GetKartData(2).ObstacleCompleted, GetKartData(2).TargetSuccess, \
					GamefieldPositionString(GetKartData(2).GamefieldPosition));
				break;
			case 'C':
				printf("Kart3: X = %d, Y = %d, Laps Left = %d, Obstacle = %d, Target = %d, Gamefield Position = %s\r\n", \
					GetKartData(3).KartX, GetKartData(3).KartY, GetKartData(3).LapsRemaining, \
					GetKartData(3).ObstacleCompleted, GetKartData(3).TargetSuccess, \
					GamefieldPositionString(GetKartData(3).GamefieldPosition));
				break;
		}
		PostMasterSM(ThisEvent);
	}
	
  return ReturnEvent;
}


