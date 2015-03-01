/****************************************************************************
Module: SM_BallLaunching.h
Description:
	The third level state machine for our robot in the BALL_LAUNCHING state.
	Contains four states: BALL_LAUNCHING_ENTRY, BALL_LAUNCHING_IR_ALIGN, 
												 BALL_LAUNCHING_LAUNCH, BALL_LAUNCHING_EXIT
Author: Kyle Moy, 2/25/15
****************************************************************************/

#ifndef SM_BALL_LAUNCHING_H
#define SM_BALL_LAUNCHING_H

/*----------------------------- Module Defines ----------------------------*/
// States of the SM_Ball_Launching state machine
typedef enum {BALL_LAUNCHING_ENTRY, BALL_LAUNCHING_IR_ALIGN, \
						  BALL_LAUNCHING_LAUNCH, BALL_LAUNCHING_EXIT} BallLaunchingState_t;


/*----------------------- Public Function Prototypes ----------------------*/
ES_Event RunBallLaunchingSM(ES_Event CurrentEvent);
void StartBallLaunchingSM(ES_Event CurrentEvent);
BallLaunchingState_t QueryBallLaunchingSM(void);

#endif /* SM_BALL_LAUNCHING_H */

