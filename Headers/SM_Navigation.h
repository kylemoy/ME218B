/****************************************************************************
Module: SM_Navigation.h
Description:
	A sub-level state machine for our robot that controls the point-destination
	driving navigation system. Used in the RACING state machine to get from
	corner to corner.
	Contains eight states: ORIENTING, DRIVING
	
Author: Kyle Moy, 2/24/15
****************************************************************************/

#ifndef SM_NAVIGATION_H
#define SM_NAVIGATION_H

/*----------------------------- Include Files -----------------------------*/
#include <stdint.h>

/*----------------------------- Module Defines ----------------------------*/
// States of the SM_Navigation state machine
typedef enum {ORIENTING, DRIVING, WAITING} NavigationState_t;


/*----------------------- Public Function Prototypes ----------------------*/
ES_Event RunNavigationSM(ES_Event CurrentEvent);
void StartNavigationSM(ES_Event CurrentEvent);
NavigationState_t QueryNavigationSM(void);
void SetTargetPosition(uint8_t X, uint8_t Y);
void SetTargetTheta(uint16_t Theta);

#endif /*SM_NAVIGATION_H */

