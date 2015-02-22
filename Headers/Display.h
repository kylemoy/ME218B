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
#define DRS_ConsoleDisplay false
	
// Choose to what to display
#define DisplayEntryStateTransitions true
#define DisplayEvents true
#define DisplayEvents_DRS false

// Choose which state machines to display
#define DisplaySM_DRS false
#define DisplaySM_Master true
#define DisplaySM_Playing true
#define DisplaySM_Racing true
	
// Display the gamefield positional changes of the Karts
#define DisplayGamefieldPositions true	// All Karts
#define DisplayMyGamefieldPosition true	// Just Our Kart
	
	
/*----------------------------- Module Defines ----------------------------*/
#define clrScrn() 	puts("\x1b[2J")

// At the usual 32ms/tick this is about a 3Hz update rate
#define DISPLAY_UPDATE_TIME 10*5


/*----------------------- Public Function Prototypes ----------------------*/
bool InitDisplay(uint8_t Priority);
bool PostDisplay(ES_Event ThisEvent);
ES_Event RunDisplay(ES_Event ThisEvent);

#endif /*Display_H */

