/****************************************************************************
Module: Display.h
Description: This module updates the console with the state of the robot
Author: Kyle Moy, 2/19/15
****************************************************************************/

#ifndef Display_H
#define Display_H


/*----------------------------- Display Defines ---------------------------*/
// Displays the DRS information on the console, uses up the entire screen
// so make sure to disable all other displays when using this one.
#define DRS_ConsoleDisplay true

// Choose to what to display
#define DisplayEntryStateTransitions false
#define DisplayEvents false

// Choose which state machines to display
#define DisplaySM_DRS false
#define DisplaySM_Master false
#define DisplaySM_Playing false
#define DisplaySM_Racing false
	
	
/*----------------------------- Module Defines ----------------------------*/
#define clrScrn() 	puts("\x1b[2J")

// At the usual 32ms/tick this is about a 3Hz update rate
#define DISPLAY_UPDATE_TIME 10*5


/*----------------------- Public Function Prototypes ----------------------*/
void UpdateDisplay(void);
bool InitDisplay(uint8_t Priority);
bool PostDisplay(ES_Event ThisEvent);
ES_Event RunDisplay(ES_Event ThisEvent);

#endif /*Display_H */

