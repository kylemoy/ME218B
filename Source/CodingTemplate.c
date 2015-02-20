/****************************************************************************
Module: ModuleName.h
Description:
	Module Description
Author: Kyle Moy, 2/18/15
****************************************************************************/

#ifndef MODULE_H
#define MODULE_H

/*----------------------------- Include Files -----------------------------*/

/*----------------------------- Module Defines ----------------------------*/

/*----------------------- Public Function Prototypes ----------------------*/

#endif /* MODULE_H */






/****************************************************************************
Module: ModuleName.c
Description:
	Module Description
Author: Kyle Moy, 2/18/15
****************************************************************************/

//#define TEST	// For using the test harness

/*----------------------------- Include Files -----------------------------*/
// C Libraries

// Framework Libraries

// Hardware Libraries

// Module Libraries

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/

/*---------------------------- Module Variables ---------------------------*/

/*------------------------------ Module Code ------------------------------*/


/*------------------------- Private Function Code -------------------------*/

/*------------------------------ Test Harness -----------------------------*/
#ifdef TEST
/* Test Harness for Module */ 
int main(void) 
{ 
	_HW_Timer_Init(ES_Timer_RATE_1mS);
	TERMIO_Init();
	clrScrn();
	printf("In Test Harness for Module\n\r");
	return 0; 
} 
#endif 

/*------------------------------- Footnotes -------------------------------*/

/*------------------------------ End of file ------------------------------*/

