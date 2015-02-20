/****************************************************************************
Module: Main.c
Description:
	This contains the main function that starts the HSM framework for our robot.
Author: Kyle Moy, 2/18/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
// C Libraries
#include <stdint.h>
#include <stdbool.h>

// Hardware Libraries
#include "driverlib/sysctl.h"

// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Timers.h"


/*----------------------------- Module Defines ----------------------------*/
#define clrScrn() 	puts("\x1b[2J")


/*------------------------------ Module Code ------------------------------*/
int main (void)
{
  ES_Return_t ErrorType;
    
	// Hardware initialization functions can go here
	
	// Set the clock to run at 40MhZ using the PLL and 16MHz external crystal
  SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
			| SYSCTL_XTAL_16MHZ);

  // Initialize the terminal for puts/printf debugging
  TERMIO_Init();
	clrScrn();
   
	// Print start message
	printf("ME218B Team 18's Speed Racer Bot\r\n");
	printf("Kenji Bowers, Alex Lin, Kyle Moy, & John Schoech\r\n");
	printf("Go Speed Racer, Go!\r\n");
	printf("%s %s\n",__TIME__, __DATE__);
	printf("\n\r\n");

// Initialize the Events and Services Framework and Start Running It
  ErrorType = ES_Initialize(ES_Timer_RATE_10mS);
  if ( ErrorType == Success ) {
    ErrorType = ES_Run();
  }
	
// If we got to here, there was an error
  switch (ErrorType){
    case FailedPointer:
      puts("Failed on NULL pointer");
      break;
    case FailedInit:
      puts("Failed Initialization");
      break;
    default:
      puts("Other Failure");
      break;
  }
  for(;;)   // Hang after reporting error
    ;
}
