/****************************************************************************
Module: DRS.c
Description:
	Hardware module for the DRS SPI communication system.
	Includes functions to initialize the SPI, send a query, and 
	the End of Transmission (EOT) interrupt response.
Author: Kyle Moy, 2/18/15
****************************************************************************/

//#define TEST	// For using the test harness


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

// Hardware Libraries
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_ssi.h"
#include "inc/hw_nvic.h"
#include "driverlib/ssi.h"
#include "bitdefs.h"
#include "driverlib/gpio.h"

// Module Libraries
#include "DRS.h"
#include "SM_DRS.h"

/*----------------------------- Module Defines ----------------------------*/
#define BitsPerNibble 	4
#define TicksPerMS 		40000
#define ALL_BITS 		(0xFF<<2)


/*---------------------------- Module Functions ---------------------------*/


/*---------------------------- Module Variables ---------------------------*/
// An 8-byte array to store the SPI data
static uint8_t DRS_Data[8] =  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function: 		DRS_Initialize 
Parameters: 	void
Returns:			void
Description:	Initializes the DRS SSI on Tiva Pins A2-A5
				A2: SSI Module 0 Clock (SCK)
				A3: SSI Module 0 Frame Signal (SS)
				A4: SSI Module 0 Receive (SDI)
				A5: SSI Module 0 Transmit (SDO)
****************************************************************************/
void DRS_Initialize(void) {
		// Enable clock to GPIO Port A
    HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
    // Enable clock to SSI Module 0
    HWREG(SYSCTL_RCGCSSI) |= SYSCTL_RCGCSSI_R0;
    // Wait for the GPIO Port to be ready
    while((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R0) != SYSCTL_PRGPIO_R0)
        ;
    // Program the GPIO to use the alternate functions on the SSI pins (p650)
    HWREG(GPIO_PORTA_BASE+GPIO_O_AFSEL) |= (BIT2HI | BIT3HI | BIT4HI | BIT5HI);
    // Select the SSI alternate functions on those pins
    HWREG(GPIO_PORTA_BASE+GPIO_O_PCTL) = 
        (HWREG(GPIO_PORTA_BASE+GPIO_O_PCTL) & 0xff0000ff) + (2<<8) + (2<<12) + (2<<16) + (2<<20);
    // Program the port lines (Pins A2-A5) for digital I/O
    HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= (GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
    // Program the required data directions on the port lines 
    HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) |= (GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_5); // Output
    HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) &= ~(GPIO_PIN_4); // Input
    // Program the pull-up on the clock line (Pin A2)
    HWREG(GPIO_PORTA_BASE+GPIO_O_PUR) |= GPIO_PIN_2;
		// Program the output for SS (Pin A3) to be open-drain type
    HWREG(GPIO_PORTA_BASE+GPIO_O_ODR) |= GPIO_PIN_3;
    // Wait for the SSI0 to be ready
    while((HWREG(SYSCTL_PRSSI) & SYSCTL_PRSSI_R0) != SYSCTL_PRSSI_R0)
        ;
    // Make sure the SSI is disabled before programming mode bits
    HWREG(SSI0_BASE + SSI_O_CR1) &= ~(SSI_CR1_SSE);
    // Select master mode (MS) 
    HWREG(SSI0_BASE + SSI_O_CR1) &= ~(SSI_CR1_MS);
    // Select TXRIS indicating EOT
    HWREG(SSI0_BASE + SSI_O_CR1) |= SSI_CR1_EOT;
    // Configure the SSI clock source to the system clock
    HWREG(SSI0_BASE + SSI_O_CC) |= SSI_CC_CS_SYSPLL;
    // Configure the clock pre-scaler
    HWREG(SSI0_BASE + SSI_O_CPSR) |= 0x08;
    // Configure the clock rate (SCR) for 33 microsecond Bit Rate
		// Bit Rate = SYSCLK / (CPDVSR*(1+SCR)), SYSCLK = 40MHz, CPDVSR = 8 => SCR = 164
    HWREG(SSI0_BASE + SSI_O_CR0) |= 164<<8;
    // Configure the phase & polarity (SPH, SPO), mode (FRF), data size (DSS)
    HWREG(SSI0_BASE + SSI_O_CR0) |= (SSI_CR0_SPH | SSI_CR0_SPO | SSI_CR0_FRF_MOTO | SSI_CR0_DSS_8);
    // Locally enable interrupts on TXRIS
    HWREG(SSI0_BASE + SSI_O_IM) |= SSI_IM_EOTIM;
    // Make sure the SSI is enabled for operation
    HWREG(SSI0_BASE + SSI_O_CR1) |= SSI_CR1_SSE;
		// Enable the SSI0 interrupt in the NVIC
		// It is interrupt number 7 so appears in EN0 at bit 7
			HWREG(NVIC_EN0) |= BIT7HI;
		// make sure interrupts are enabled globally
			__enable_irq();
		// Print to console if successful initialization
		printf("DRS Initialized\n\r");
}


/****************************************************************************
Function:		DRS_EOTIntHandler
Parameters:	none
Returns:		none
Description:	End of Transfer (EOT) interrupt response handler
							Posts a new DRS_Read event to the DRS state machine
****************************************************************************/
void DRS_EOTIntHandler(void) {
	// Check that SPI is not transmitting before reading the receive register
	if ((HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_BSY) != SSI_SR_BSY) {
		// Read the 8 bytes into SPI_Data
		for (int i = 0; i < 8; i++) {
			DRS_Data[i] = HWREG(SSI0_BASE + SSI_O_DR);
		}
	}
 	// Post NewRead event to DRS
 	ES_Event Event = {E_DRS_EOT};
 	PostDRS_SM(Event);
}




/****************************************************************************
Function: 		DRS_SendQuery
Parameters:		uint8_t Query, the query to be sent
Returns:			bool, true if query was successfully sent, false if not
Description:	Query the DRS with the CurrentQuery
****************************************************************************/
bool DRS_SendQuery(uint8_t Query) {	
	// Check if the data output FIFO queue is empty
	if((HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TFE) == SSI_SR_TFE) {
		// Write query byte to the data output register
		HWREG(SSI0_BASE + SSI_O_DR) = Query;
		// Write 0x00 to the data output register 7 times
		for (int i = 1; i < 8; i++) {
			HWREG(SSI0_BASE + SSI_O_DR) = 0x00;
		}	
		// Unmask the SSI transmit interrupt (SSI_IM_TXIM) to enable EOT interrupt 
		HWREG(SSI0_BASE + SSI_O_IM) |= SSI_IM_TXIM;
		return true;
	}
	return false;
}


/****************************************************************************
Function:		DRS_PrintData
Parameters:		none
Returns:		bool true if successful
Description:	Retrives the data from RS_Data based on the current query.
****************************************************************************/
void DRS_PrintData(void) {
	printf("DRS_Data: %#02x %#02x %#02x %#02x %#02x %#02x %#02x %#02x\r\n", \
		DRS_Data[0], DRS_Data[1], DRS_Data[2], DRS_Data[3], DRS_Data[4], DRS_Data[5], DRS_Data[6], DRS_Data[7]);
}


/*------------------------- Private Function Code -------------------------*/

/*------------------------------ Test Harness -----------------------------*/
#ifdef TEST 

/* Test Harness Includes / Defines */ 
#include "termio.h" 
#define clrScrn() 	printf("\x1b[2J")
#define goHome()	printf("\x1b[1,1H")
#define clrLine()	printf("\x1b[K")

/* Test Harness for the DRS SPI Module */ 
int main(void) 
{ 
	_HW_Timer_Init(ES_Timer_RATE_1mS);
	TERMIO_Init();
	clrScrn();
	printf("In Test Harness for the DRS SPI Module\n\r");
	DRS_Initialize();
	uint8_t Query = GAME_STATUS_QUERY;
	printf("Query: %#02x\n\r", Query);
	DRS_SendQuery(Query);
	
	return 0; 
} 
#endif 


/*------------------------------- Footnotes -------------------------------*/

/*------------------------------ End of file ------------------------------*/

