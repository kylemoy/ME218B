/****************************************************************************
Module: BumpSensor.c
Description:
	This module handles the bump sensor hardware initialization, and posts an
	event when a bump is sensed.
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
#include "BumpSensor.h"
#include "SM_Master.h"


/*----------------------------- Module Defines ----------------------------*/
#define ALL_BITS (0xff <<2)


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function: 		InitializeBumpSensors
Parameters:		void
Returns:			void
Description:	Initializes the hardware for the bump sensors
****************************************************************************/
void InitializeBumpSensors(void) {
	// Initialization of the bump sensor
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3; // Port D
	HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= GPIO_PIN_1; // Enable Pin D2 for Digital I/O
	HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) &= ~GPIO_PIN_1; // Enable Pin B2 as Input
	HWREG(GPIO_PORTD_BASE+GPIO_O_PUR) |=  GPIO_PIN_1; // Enable Pull Down Resistor on Pin B2
}


/****************************************************************************
Function: 		BumpSensorDetected
Parameters:		void
Returns:			bool, true if bumped, false if not bumped
Description:	Returns true if bump sensor is hit
****************************************************************************/
bool BumpSensorDetected(void) {
	return ~HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA + ALL_BITS)) & GPIO_PIN_1;
}


/*------------------------------ Test Harness -----------------------------*/
#ifdef TEST 
/* Test Harness for Bump Sensor module */ 
int main(void) 
{ 
	TERMIO_Init(); 
	printf("\n\rIn Test Harness for Bump Sensor\r\n");
	InitializeBumpSensors();
	while(true){
		if (BumpSensorDetected()) {
			printf("Bump is detected.\r\n");
		} else {
			printf("Bump is not detected.\r\n");
		}
	}
}
#endif
