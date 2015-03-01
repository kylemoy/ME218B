/****************************************************************************
Module: SM_Obstacle_Crossing.h
Description:
	The third level state machine for our robot in the OBSTACLE_CROSSING state.
	Contains three states: OBSTACLE_ENTRY, CROSSING, OBSTACLE_EXIT
Author: Kyle Moy, 2/25/15
****************************************************************************/

#ifndef SM_OBSTACLE_CROSSING_H
#define SM_OBSTACLE_CROSSING_H

/*----------------------------- Module Defines ----------------------------*/
// States of the SM_Obstacle_Crossing state machine
typedef enum {OBSTACLE_ENTRY, CROSSING, \
						  OBSTACLE_EXIT} ObstacleCrossingState_t;


/*----------------------- Public Function Prototypes ----------------------*/
ES_Event RunObstacleCrossingSM(ES_Event CurrentEvent);
void StartObstacleCrossingSM(ES_Event CurrentEvent);
ObstacleCrossingState_t QueryObstacleCrossingSM(void);

#endif /* SM_OBSTACLE_CROSSING_H */

