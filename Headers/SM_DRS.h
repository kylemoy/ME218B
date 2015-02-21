/****************************************************************************
Module: 	SM_DRS.h
Description:
	State machine that queries the DRS to keep the Kart data updated
	at all times during the gameplay.
	Contains three states: WAITING_FOR_QUERY, QUERYING, and READING
Author: 	Kyle Moy
History:	2/16/15 - Started working on the module, set up the states
					2/19/15	- DRS.c hardware module was completed
					2/20/15 - Finished store data function to Kart data structure
****************************************************************************/
#ifndef SM_DRS_H
#define SM_DRS_H

/*----------------------------- Include Files -----------------------------*/

/*----------------------------- Module Defines ----------------------------*/
// State definitions for use with the query function
typedef enum {WAITING_FOR_QUERY, QUERYING, READING} DRSState_t;

/*----------------------- Public Function Prototypes ----------------------*/
bool InitDRS_SM(uint8_t Priority);
bool PostDRS_SM(ES_Event ThisEvent);
ES_Event RunDRS_SM(ES_Event ThisEvent);
DRSState_t QueryDRS_SM(void);

#endif /* SM_DRS_H */
