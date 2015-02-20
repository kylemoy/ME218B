/****************************************************************************
Module: SM_Playing.h
Description:
	The second level state machine for our robot in the PLAYING state.
	Contains three states: RACING, CROSSING_OBSTACLE, and BALL_LAUNCHING.
Author: Kyle Moy, 2/18/15
****************************************************************************/

#ifndef SM_PLAYING_H
#define SM_PLAYING_H

/*----------------------------- Module Defines ----------------------------*/
// State definitions for use with the query function
typedef enum {RACING, CROSSING_OBSTACLE, BALL_LAUNCHING} PlayingState_t;


/*----------------------- Public Function Prototypes ----------------------*/
ES_Event RunPlayingSM(ES_Event CurrentEvent);
void StartPlayingSM(ES_Event CurrentEvent);
PlayingState_t QueryPlayingSM(void);

#endif /*SM_PLAYING_H */

