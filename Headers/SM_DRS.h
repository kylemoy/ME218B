/****************************************************************************
Module: SM_DRS.h
Description:
	State machine to communicate with the DRS.
	Contains three states: WAITING_FOR_QUERY, QUERYING, and READING
Author: Kyle Moy, 2/16/15
****************************************************************************/
#ifndef SM_DRS_H
#define SM_DRS_H

/*----------------------------- Include Files -----------------------------*/


/*----------------------------- Module Defines ----------------------------*/
// State definitions for use with the query function
typedef enum {WAITING_FOR_QUERY, QUERYING, READING} DRSState_t;

// Type definition for the game flag status
typedef enum {
 			Flag_Waiting,
 			Flag_Start,
 			Flag_Caution,
 			Flag_Finished
} Flag_t;

// Data structure that contains information for a Kart
typedef struct {
	uint16_t 	KartX;
	uint16_t 	KartY;
	uint16_t 	KartTheta;
	uint8_t		LapsRemaining;
	bool		ObstacleCompleted;
	bool		TargetSuccess;
	Flag_t	FlagStatus;
} KART_t;


/*----------------------- Public Function Prototypes ----------------------*/
bool InitDRS_SM(uint8_t Priority);
bool PostDRS_SM(ES_Event ThisEvent);
ES_Event RunDRS_SM(ES_Event ThisEvent);
DRSState_t QueryDRS_SM(void);
KART_t GetMyKartData(void);

#endif /* SM_DRS_H */
