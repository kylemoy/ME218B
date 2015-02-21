/****************************************************************************
Module: SM_Master.h
Description:
	The top level state machine for our robot.
	Contains three states: WAITING_START, PLAYING, WAITING_FINISHED.
Author: Kyle Moy, 2/16/15
****************************************************************************/

#ifndef SM_MASTER_H
#define SM_MASTER_H

/*----------------------------- Module Defines ----------------------------*/
// State definitions for use with the query function
typedef enum {WAITING_START, PLAYING, WAITING_FINISHED} MasterState_t;


/*----------------------- Public Function Prototypes ----------------------*/
ES_Event RunMasterSM(ES_Event CurrentEvent);
void StartMasterSM(ES_Event CurrentEvent);
bool PostMasterSM(ES_Event ThisEvent);
bool InitMasterSM(uint8_t Priority);
MasterState_t QueryMasterSM(void);

#endif /*SM_MASTER_H */

