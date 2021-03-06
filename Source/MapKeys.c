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
#include "DriveMotors.h"
#include "BallLauncher.h"


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
			
			// Drive Command Triggers
			case 'W': DriveForwardWithSetDistance(100, 1000); break;
			case 'S': DriveBackward(100, 100); break;
			case 'D': PivotCWwithSetTicks(150, 18); break; // RotateCWwithDuty(40, 5); break; //DriveForwardWithBias(70, 30, 50); break; //RotateCW(150, 5); break;
			case 'A': PivotCCWwithSetTicks(150, 18); break;// RotateCCWwithDuty(40, 5); break; //DriveForwardWithBias(30, 70, 50); break; //RotateCCW(150, 5); break;
			case ' ': StopMotors(); break;
			
			// Shooter Motor Command Triggers
			case 'U': TurnOnShooter(); break; //SetShooterPWM(100); break;
			case 'I': TurnOffShooter(); break; // SetShooterPWM(0); break;
			case 'O': ServoForward(); break;
			case 'P': ServoReverse(); break;
			
			// (1) SM_Master Event Triggers
			case 'Z': ThisEvent.EventType = E_RACE_STARTED; break;
			case 'X': ThisEvent.EventType = E_RACE_FINISHED; break;
			case 'C': ThisEvent.EventType = E_RACE_CAUTION; break;
			
			// (1)SM_Master->(2)SM_Playing Event Triggers
			case 'V': ThisEvent.EventType = E_BALL_LAUNCHING_ENTRY; break;
			case 'B': ThisEvent.EventType = E_BALL_LAUNCHING_EXIT; break;
			case 'N': ThisEvent.EventType = E_BALL_LAUNCHING_COMPLETE; break;
			case 'F': ThisEvent.EventType = E_OBSTACLE_CROSSING_ENTRY; break;
			case 'G': ThisEvent.EventType = E_OBSTACLE_CROSSING_EXIT; break;
			
			// (1)SM_Master->(2)SM_Playing->(3)SM_Racing Event Triggers
			case '1': ThisEvent.EventType = E_BUMP_DETECTED; break;
			case '2': ThisEvent.EventType = E_IR_BEACON_DETECTED; break;
			
			// Information Queries
			case ',':
				printf("Kart1: X = %d, Y = %d, Theta = %d, Laps Left = %d, Obstacle = %d, Target = %d, Gamefield Position = %s\r\n", \
					GetKartData(1).KartX, GetKartData(1).KartY, GetKartData(1).KartTheta, GetKartData(1).LapsRemaining, \
					GetKartData(1).ObstacleCompleted, GetKartData(1).TargetSuccess, \
					GamefieldPositionString(GetKartData(1).GamefieldPosition));
				break;
			case '.':
				printf("Kart2: X = %d, Y = %d, Theta = %d, Laps Left = %d, Obstacle = %d, Target = %d, Gamefield Position = %s\r\n", \
					GetKartData(2).KartX, GetKartData(2).KartY, GetKartData(2).KartTheta, GetKartData(2).LapsRemaining, \
					GetKartData(2).ObstacleCompleted, GetKartData(2).TargetSuccess, \
					GamefieldPositionString(GetKartData(2).GamefieldPosition));
				break;
			case '/':
				printf("Kart3: X = %d, Y = %d, Theta = %d,  Left = %d, Obstacle = %d, Target = %d, Gamefield Position = %s\r\n", \
					GetKartData(3).KartX, GetKartData(3).KartY, GetKartData(3).KartTheta, GetKartData(3).LapsRemaining, \
					GetKartData(3).ObstacleCompleted, GetKartData(3).TargetSuccess, \
					GamefieldPositionString(GetKartData(3).GamefieldPosition));
				break;
		}
		PostMasterSM(ThisEvent);
	}
	
  return ReturnEvent;
}


