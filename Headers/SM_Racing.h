/****************************************************************************
Module: SM_Racing.h
Description:
	The third level state machine for our robot in the RACING state.
	Contains two states: STRAIGHT, CORNER
Author: Kyle Moy, 2/19/15
****************************************************************************/

#ifndef SM_RACING_H
#define SM_RACING_H

/*----------------------------- Module Defines ----------------------------*/
// States of the SM_Racing state machine
typedef enum {STRAIGHT, CORNER} RacingState_t;


/*----------------------- Public Function Prototypes ----------------------*/
ES_Event RunRacingSM(ES_Event CurrentEvent);
void StartRacingSM(ES_Event CurrentEvent);
RacingState_t QueryRacingSM(void);

#endif /*SM_RACING_H */

