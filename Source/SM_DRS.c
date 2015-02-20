/****************************************************************************
Module: SM_DRS.c
Description:
	State machine to communicate with the DRS.
	Contains three states: WAITING_FOR_QUERY, QUERYING, and READING
Author: Kyle Moy, 2/16/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
// C Libraries
#include <stdint.h>
#include <stdbool.h>
#include <cmath>

// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"
#include "ES_Types.h"
#include "ES_Events.h"

// Module Libraries
#include "DRS.h"
#include "SM_DRS.h"


/*----------------------------- Module Defines ----------------------------*/
// Interval between two successive transfers, should be at least 2ms
// Keep it under 10ms since DRS updates at 100Hz
// But want it as fast as possible so data from the four queries is updated
#define COMMAND_INTERVAL 3	*	100 // For debugging, * 100

// If a command takes longer than this duration, then something is probably wrong
// We'll timeout and go back to the waiting state
#define COMMAND_TIMEOUT 50	*	100 // For debugging, * 100


/*---------------------------- Module Functions ---------------------------*/
static uint8_t GetNextQuery(void);
static bool StoreData(void);
static ES_Event DuringWaitingForQuery(ES_Event Event);
static ES_Event DuringQuerying(ES_Event Event);
static ES_Event DuringReading(ES_Event Event);


/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
static DRSState_t CurrentState;

static uint8_t CurrentQuery;	// Keep track of the current query
static uint8_t LastQuery;		// Save the last query in case of transfer failure

static uint8_t MyKart;	 // Our Kart number
static KART_t Kart1;		 // Data for Kart1
static KART_t Kart2;		 // Data for Kart2
static KART_t Kart3;		 // Data for Kart3


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function: 		InitDRS_SM
Parameters: 	uint8_t Priority, the priority of this service
Returns:			bool, false if error during initialization, true otherwise
Description:	Initializes the DRS state machine
****************************************************************************/
bool InitDRS_SM (uint8_t Priority) {
	ES_Event ThisEvent;
	MyPriority = Priority;
	// Initializes the DRS hardware
	DRS_Initialize();
	// Initialize the Kart Number switch hardware
	// For now, assume we are Kart1
	MyKart = 1;
	// Set the CurrentState to WaitingForQuery
	CurrentState = WAITING_FOR_QUERY;
	// Now let the Run function initialize the state machine
	ThisEvent.EventType = ES_ENTRY;
	RunDRS_SM(ThisEvent);
	return true;
}


/****************************************************************************
Function: 		PostDRS_SM
Parameters: 	ES_Event ThisEvent, the event to post to the queue
Returns:			bool, false if the enqueue operation failed, true otherwise
Description:	Posts an event to this state machine's queue
****************************************************************************/
bool PostDRS_SM(ES_Event ThisEvent) {
  return ES_PostToService(MyPriority, ThisEvent);
}


/****************************************************************************
Function:			RunDRS_SM
Parameters:		ES_Event CurrentEvent, the event to process
Returns:			ES_Event, ES_NO_EVENT if no error, ES_ERROR otherwise
Description:	Runs the DRS state machine
****************************************************************************/
ES_Event RunDRS_SM (ES_Event CurrentEvent) {
	bool MakeTransition = false; // Assume no state transition
	DRSState_t NextState = CurrentState;
  ES_Event EntryEventKind = {ES_ENTRY, 0}; // Default to normal entry to new state
	ES_Event ReturnEvent = {ES_NO_EVENT, 0}; // Assume no error
	
	switch(CurrentState) {
		case WAITING_FOR_QUERY: 
			// Execute During function for WAITING_FOR_QUERY state
			CurrentEvent = DuringWaitingForQuery(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				switch (CurrentEvent.EventType) {
					case ES_TIMEOUT: 
							// Command interval time out, time to query the next command
							NextState = QUERYING;
							MakeTransition = true;
						break;
				}
			}
			break;
			
		case QUERYING:
			// Execute During function of QUERYING state
			CurrentEvent = DuringQuerying(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				switch (CurrentEvent.EventType) {
					case E_NEW_DRS_QUERY: 
						// Get the next query to send
						CurrentQuery = GetNextQuery();
						printf("Current Query = %#02x\r\n", CurrentQuery);
						if(DRS_SendQuery(CurrentQuery)) {
							// Query was successful, transition to READING
							printf("DRS query successful\r\n");
							NextState = READING;
							MakeTransition = true;
						}
						else
						{
							// Query was unsuccessful, transition back to WAITING_FOR_QUERY
							printf("DRS query unsuccessful\r\n");
							CurrentQuery = LastQuery;
							NextState = WAITING_FOR_QUERY;
							MakeTransition = true;
						}
						break;
						
					case ES_TIMEOUT : 
						// No EOT interrupt was received and we timed out, so let's start over
						printf("ES_TIMEOUT during QUERYING\r\n");
						NextState = WAITING_FOR_QUERY;
						MakeTransition = true;
						// Reset current query to last successful query
						CurrentQuery = LastQuery;
						break;
				}
			}
			break;
			
		case READING: 
			// Execute During function for READING state
			CurrentEvent = DuringReading(CurrentEvent);
			// Process any events
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				switch (CurrentEvent.EventType) {
					// EOT interrupt occured
					case E_DRS_EOT:
						if(StoreData()) {
							// Data was successfully stored, transitioning to WAITING_FOR_QUERY
							NextState = WAITING_FOR_QUERY;
							MakeTransition = true;
						} else {
							// Data was invalid, transitioning to WAITING_FOR_QUERY and retrying the query
							NextState = WAITING_FOR_QUERY;
							MakeTransition = true;
							// Reset current query to last successful query
							CurrentQuery = LastQuery;
						}
						break;
						
					case ES_TIMEOUT : 
						// No EOT interrupt was received and we timed out, so let's start over
						printf("ES_TIMEOUT during READING\r\n");
						NextState = WAITING_FOR_QUERY;
						MakeTransition = true;
						// Reset current query to last successful query
						CurrentQuery = LastQuery;
						break;
				}
			}
			break;
	}
	
	// If we are making a state transition
	if (MakeTransition == true) {
		// Execute exit function for the current state
		CurrentEvent.EventType = ES_EXIT;
		RunDRS_SM(CurrentEvent);
		// Modify current state
		CurrentState = NextState;
		// Execute entry function for new state
		RunDRS_SM(EntryEventKind);
	}
	// In the absence of an error, the top level state machine always
  // returns ES_NO_EVENT, which we initialized at the top of this function
	return ReturnEvent;
}



/****************************************************************************
Function: 		QueryDRS_SM
Parameters: 	none
Returns: 			DRSState_t, the current state of the DRS state machine
Description: 	Returns the current state of the DRS state machine.
   				Possible states are: {DRS_Ready, DRS_Transferring, DRS_Waiting}
****************************************************************************/
DRSState_t QueryDRS_SM(void) {
	return CurrentState;
}

/****************************************************************************
Function: 		GetMyKartData
Parameters:		none
Returns:			KART_t, the data for our Kart
Description:	Returns the struct data for our Kart:
					uint16_t 	KartX;
					uint16_t 	KartY;
					uint16_t 	KartTheta;
					uint8_t		LapsRemaining;
					bool		ObstacleCompleted;
					bool		TargetSuccess;
					Flag_t	FlagStatus;
****************************************************************************/
KART_t GetMyKartData(void) {
	switch (MyKart) {
		default:
		case 1:
			return Kart1;
		case 2:
			return Kart2;
		case 3:
			return Kart3;
	}
}

/*------------------------- Private Function Code -------------------------*/
/****************************************************************************
Function:			GetNextQuery
Parameters:		none
Returns:			none
Description:	Gets the next query by cycling through the four queries:
							GAME_STATUS_QUERY, KART1_QUERY, KART2_QUERY, KART3_QUERY
****************************************************************************/
static uint8_t GetNextQuery(void) {
	uint8_t NextQuery;
	LastQuery = CurrentQuery;
	switch(LastQuery) {
		case GAME_STATUS_QUERY : 
			NextQuery = KART1_QUERY;
			break;
		case KART1_QUERY : 	
			NextQuery = GAME_STATUS_QUERY;
			break;
		case KART2_QUERY :				
			NextQuery = KART2_QUERY;
			break;
		case KART3_QUERY : 	
			NextQuery = KART3_QUERY;
			break;
		default : NextQuery = GAME_STATUS_QUERY;
	}
	return NextQuery;
}


/****************************************************************************
Function:			StoreData
Parameters:		none
Returns:			bool, true if the data was valid, false if data was invalid (0xFF)
Description:	Stores the data from the DRS read into the appropriate data struct.
****************************************************************************/
static bool StoreData(void) {
	DRS_PrintData();
	bool ReturnVal = true;
	return ReturnVal;
}


/****************************************************************************
Function:			DuringWaitingForQuery
Parameters:		ES_Event Event
Returns:			ES_Event
Description:	Processes the events for this state, assume no consumption
****************************************************************************/
static ES_Event DuringWaitingForQuery(ES_Event Event) {
	// process ES_ENTRY & ES_EXIT events
	if (Event.EventType == ES_ENTRY) {
		printf("\r\n(1) SM_DRS: WAITING_FOR_QUERY\r\n");
		// Start a timer to create a time interval between commands
		ES_Timer_InitTimer(DRS_TIMER, COMMAND_INTERVAL);
	} else if (Event.EventType == ES_EXIT) {
		// On exit, create a E_NEW_DRS_QUERY event
		ES_Event NewEvent = {E_NEW_DRS_QUERY, 0};
		PostDRS_SM(NewEvent);
	} else {
		// Do the 'during' function for this state
	}
	return Event;
}

/****************************************************************************
Function:			DuringQuerying
Parameters:		ES_Event Event
Returns:			ES_Event
Description:	Processes the events for this state, assume no consumption
****************************************************************************/
static ES_Event DuringQuerying(ES_Event Event) {
	// process ES_ENTRY & ES_EXIT events
	if (Event.EventType == ES_ENTRY) {
		printf("\r\n(1) SM_DRS: QUERYING\r\n");
		// Start a command timeout timer in case something goes wrong
		ES_Timer_InitTimer(DRS_TIMER, COMMAND_TIMEOUT);
	} else if (Event.EventType == ES_EXIT) {
		ES_Timer_StopTimer(DRS_TIMER);
	} else {
		// Do the 'during' function for this state
	}
	return Event;
}

/****************************************************************************
Function:			DuringReading
Parameters:		ES_Event Event
Returns:			ES_Event
Description:	Processes the events for this state, assume no consumption
****************************************************************************/
static ES_Event DuringReading(ES_Event Event) {
	// process ES_ENTRY & ES_EXIT events
	if (Event.EventType == ES_ENTRY) {
		printf("\r\n(1) SM_DRS: READING\r\n");
		// Start a command timeout timer in case something goes wrong
		ES_Timer_InitTimer(DRS_TIMER, COMMAND_TIMEOUT);
	} else if (Event.EventType == ES_EXIT) {
		ES_Timer_StopTimer(DRS_TIMER);
	} else {
		// Do the 'during' function for this state
	}
	return Event;
}

/*------------------------------- Footnotes -------------------------------*/

/*------------------------------ End of file ------------------------------*/

