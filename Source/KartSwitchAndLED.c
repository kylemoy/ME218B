/****************************************************************************
Module: KartSwitchAndLED.c
Description:
	This module initializes the Kart Switch and LED hardware, and it contains
	functions to determine the Kart Select (Kart1, Kart2, or Kart3) and to turn
	on/off an LED to indicate that a race is in progress.
Author: Kyle Moy, 3/1/15
****************************************************************************/

//#define TEST

/*----------------------------- Include Files -----------------------------*/
// C Libraries
#include <stdio.h>
#include <ctype.h>

// Hardware Libraries
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "termio.h"
#include "ES_Port.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"

// Module Libraries
#include "KartSwitchAndLED.h"


/*----------------------------- Module Defines ----------------------------*/
#define ALL_BITS (0xff <<2)


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function: 		InitializeKartSwitchAndLED
Parameters:		void
Returns:			void
Description:	Initializes the hardware for the Kart switch and LED.
****************************************************************************/
void InitializeKartSwitchAndLED(void) { 
	// Initialization of the Kart switch (Pins E1, E2, and E3)
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4; // Port E
	HWREG(GPIO_PORTE_BASE+GPIO_O_DEN) |= (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_0); // Enable Pin E1, E2, E0 for Digital I/O
	HWREG(GPIO_PORTE_BASE+GPIO_O_DIR) &= ~(GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_0); // Enable Pin E1, E2, E0 as Input
	HWREG(GPIO_PORTE_BASE+GPIO_O_PUR) |=  (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_0); // Enable Pull Up Resistor on Pin E1, E2, E0
	
	// Initialization of the LED (Pin F2)
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R5; // Port F
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN) |= GPIO_PIN_2; // Enable Pin F2 for Digital I/O
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR) |= GPIO_PIN_2; // Enable Pin F2 as Output
}


/****************************************************************************
Function: 		ReadKartSwitch
Parameters:		void
Returns:			void
Description:	Returns an integer (1-3) for the Kart that is switched on
****************************************************************************/
uint8_t ReadKartSwitch(void) {
	if ((HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA + ALL_BITS)) & (GPIO_PIN_0)) \
		&& (HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA + ALL_BITS)) & (GPIO_PIN_1)))
		return 2;
	else if ((HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA + ALL_BITS)) & (GPIO_PIN_1)) \
		&& (HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA + ALL_BITS)) & (GPIO_PIN_2)))
		return 3;
	else
		return 1;
}

/****************************************************************************
Function: 		TurnOnRaceLED
Parameters:		void
Returns:			void
Description:	Turns the race LED (on pin F1) on
****************************************************************************/
void TurnOnRaceLED(void) {
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) |= GPIO_PIN_2;
}

/****************************************************************************
Function: 		TurnOffRaceLED
Parameters:		void
Returns:			void
Description:	Turns the race LED (on pin F1) off
****************************************************************************/
void TurnOffRaceLED(void) {
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~GPIO_PIN_2;
}


/*------------------------------ Test Harness -----------------------------*/
#ifdef TEST 
/* Test Harness for Kart Switch and LED module */ 
int main(void) 
{ 
  TERMIO_Init(); 
	printf("In Test Harness for the Kart Switch and LED Module\n\r");
  InitializeKartSwitchAndLED();
	
	while(true){		
		char input = getchar();
		switch (input) {
			case '1':
				printf("Active Kart is Kart #%d\r\n", ReadKartSwitch());
				//if ((HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA + ALL_BITS)) & GPIO_PIN_0) == GPIO_PIN_0)
				//	printf("Port Pin E0 is high.\r\n");
				//if ((HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA + ALL_BITS)) & GPIO_PIN_1) == GPIO_PIN_1)
				//	printf("Port Pin E1 is high.\r\n");
				//if ((HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA + ALL_BITS)) & GPIO_PIN_2) == GPIO_PIN_2)
				//	printf("Port Pin E2 is high.\r\n");
				break;
			
			case '2':
				printf("Turn the Race LED on\r\n");
				TurnOnRaceLED();
				break;
			
			case '3':
				printf("Turn the Race LED off\r\n");
				TurnOffRaceLED();
				break;
		}
	}
}
#endif
