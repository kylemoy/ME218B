/****************************************************************************
Module: DRS.c
Description:
	Hardware module for the DRS SPI communication system.
	Includes functions to initialize the SPI, send a query, and 
	the End of Transmission (EOT) interrupt response.
Author: Kyle Moy, 2/18/15
****************************************************************************/

//#define TEST
// For testing the DRS, it's better to run the main program with the 
// DRS_ConsoleDisplay on in Display.h. This will run the state machine
// and continually update the DRS, whereas this Test Harness is just
// for testing the transmission of a single command query.

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
#include "Display.h"
#include "SM_Master.h"
#include "KartSwitchAndLED.h"

/*----------------------------- Module Defines ----------------------------*/
#define BitsPerNibble 	4
#define TicksPerMS 		40000
#define ALL_BITS 		(0xFF<<2)

// Query Game Status - DRS Response Byte Masks
#define LAPS_REMAINING_MASK		0x07	// Bits 0-2
#define FLAG_STATUS_MASK			0x18	// Bits 3-4
#define OBSTACLE_STATUS_MASK	0x40	// Bit 6
#define TARGET_STATUS_MASK		0x80	// Bit 7

// Query Game Status - DRS Response Byte Values
#define WAITING_FOR_START 	0x00	// 0x00 on Bits 3-4
#define FLAG_DROPPED				0x08	// 0x01 on Bits 3-4
#define CAUTION_FLAG				0x10	// 0x02 on Bits 3-4
#define RACE_OVER						0x18	// 0x03 on Bits 3-4
#define OBSTACLE_COMPLETED	0x40	// 1 on Bit 6
#define TARGET_SUCCESSFUL		0x80	// 1 on Bit 7
#define INVALID_READ 				0xFF


/*---------------------------- Module Functions ---------------------------*/


/*---------------------------- Module Variables ---------------------------*/
// The current query being processed;
uint8_t CurrentQuery;

// An 8-byte array to store the SPI data
static uint8_t DRS_Data[8] =  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


// Initializes the data structures for the Kart data
//		uint16_t 						KartX;
//		uint16_t 						KartY;
//		int16_t 						KartTheta;
//		uint8_t							LapsRemaining;
//		bool								ObstacleCompleted;
//		bool								TargetSuccess;
//		Flag_t							FlagStatus;
//		GamefieldPosition_t GamefieldPosition
Kart_t Kart1 = {0, 0, 0, 0, false, false, Flag_Waiting, Undefined};
Kart_t Kart2 = {0, 0, 0, 0, false, false, Flag_Waiting, Undefined};
Kart_t Kart3 = {0, 0, 0, 0, false, false, Flag_Waiting, Undefined};

	// Our Kart, initialized in DRS_Initialize function
Kart_t *MyKart;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function: 		InitializeDRS 
Parameters: 	void
Returns:			void
Description:	Initializes the DRS SSI on Tiva Pins A2-A5
				A2: SSI Module 0 Clock (SCK), output
				A3: SSI Module 0 Frame Signal (SS), output
				A4: SSI Module 0 Transmit (SDO), input
				A5: SSI Module 0 Receive (SDI), output
****************************************************************************/
void InitializeDRS(void) {
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
    HWREG(SSI0_BASE + SSI_O_CPSR) |= 0x10;
    // Configure the clock rate (SCR) for 66us period (33us SCK high, 33us SCK low)
		// Bit Rate = SYSCLK / (CPDVSR*(1+SCR)), SYSCLK = 40MHz, CPDVSR = 16 => SCR = 164
    HWREG(SSI0_BASE + SSI_O_CR0) |=  164<<8;
    // Configure the phase & polarity (SPH, SPO), mode (FRF), data size (DSS)
    HWREG(SSI0_BASE + SSI_O_CR0) |= (SSI_CR0_SPH | SSI_CR0_SPO | SSI_CR0_FRF_MOTO | SSI_CR0_DSS_8);
    // Locally enable interrupts on TXRIS
    HWREG(SSI0_BASE + SSI_O_IM) |= SSI_RIS_TXRIS;
    // Make sure the SSI is enabled for operation
    HWREG(SSI0_BASE + SSI_O_CR1) |= SSI_CR1_SSE;
		// Enable the SSI0 interrupt in the NVIC
		// It is interrupt number 7 so appears in EN0 at bit 7
			HWREG(NVIC_EN0) |= BIT7HI;
		// make sure interrupts are enabled globally
			__enable_irq();
		// Print to console if successful initialization
		printf("DRS Initialized\n\r");
		
		// Read our Kart number from the switch hardware
		switch(ReadKartSwitch()) {
			case 1:
			default:
				printf("We are Kart1.\r\n");
				MyKart = &Kart1;
				break;
			case 2:
				printf("We are Kart2.\r\n");
				MyKart = &Kart2;
				break;
			case 3:
				printf("We are Kart3.\r\n");
				MyKart = &Kart3;
				break;
		}
		MyKart = &Kart1;
} 


/****************************************************************************
Function:		EOTIntHandler
Parameters:	none
Returns:		none
Description:	End of Transfer (EOT) interrupt response handler
							Posts a new DRS_Read event to the DRS state machine
****************************************************************************/
void EOTIntHandler(void) {
	// Clear the source of the interrupt
	HWREG(SSI0_BASE+SSI_O_ICR) = SSI_ICR_RORIC;
	
	// Check that SPI is not transmitting before reading the receive register
	if ((HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_BSY) != SSI_SR_BSY) {
		// Read the 8 bytes into SPI_Data
		for (int i = 0; i < 8; i++) {
			DRS_Data[i] = HWREG(SSI0_BASE + SSI_O_DR);
		}
	}
	if (DRS_ConsoleDisplay)
			printf("DRS_Data = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n", \
				DRS_Data[0], DRS_Data[1], DRS_Data[2], DRS_Data[3], DRS_Data[4], DRS_Data[5], DRS_Data[6], DRS_Data[7]);
	
	// We'll only process this information in the test harness
	// In the actual implentation, DRS_StoreData will be called in the SM
	#ifdef TEST
	printf("DRS_Data = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n", \
		DRS_Data[0], DRS_Data[1], DRS_Data[2], DRS_Data[3], DRS_Data[4], DRS_Data[5], DRS_Data[6], DRS_Data[7]);
	StoreData();
	PrintKartData();
	#endif
 	// Post NewRead event to DRS
 	ES_Event Event = {E_DRS_EOT};
 	PostDRS_SM(Event);
}


/****************************************************************************
Function: 		SendQuery
Parameters:		uint8_t Query, the query to be sent
Returns:			bool, true if query was successfully sent, false if not
Description:	Query the DRS with the CurrentQuery
****************************************************************************/
bool SendQuery(uint8_t Query) {	
	// Set the module variable
	CurrentQuery = Query;
	
	// Check if the data output FIFO queue is empty
	if((HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TFE) == SSI_SR_TFE) {
		// Unmask the SSI transmit interrupt (SSI_IM_TXIM) to enable EOT interrupt 
		HWREG(SSI0_BASE + SSI_O_IM) |= SSI_IM_TXIM;
		
		// Write query byte to the data output register
		HWREG(SSI0_BASE + SSI_O_DR) = Query;
		// Write 0x00 to the data output register 7 times
		for (int i = 1; i < 8; i++) {
			HWREG(SSI0_BASE + SSI_O_DR) = 0x00;
		}	
		return true;
	}
	return false;
}


/****************************************************************************
Function:			StoreData
Parameters:		none
Returns:			bool true if successful
Description:	Stores the DRS_Data to the appropriate Kart variable
****************************************************************************/
bool StoreData(void) {
	// Pointer to the Kart struct that will be updated
	Kart_t *Kart;
	
	// Check if the current query is a GAME_STATUS_QUERY
	if (CurrentQuery == GAME_STATUS_QUERY) {
		// Process SS1 (match status for Kart1), Response Byte 3
		// Process SS2 (match status for Kart2), Response Byte 4
		// Process SS3 (match status for Kart3), Response Byte 5
		for (int byte = 3; byte <= 5; byte++) {
			// Set the Kart to update
			switch (byte) {
				case 3: Kart = &Kart1; break;
				case 4: Kart = &Kart2; break;
				case 5: Kart = &Kart3; break;
			}
			// Record the game status data to the Kart
			Kart->LapsRemaining = DRS_Data[byte] & LAPS_REMAINING_MASK;
			switch (DRS_Data[byte] & FLAG_STATUS_MASK) {
				case WAITING_FOR_START:
					Kart->FlagStatus = Flag_Waiting; break;
				case FLAG_DROPPED:
					// Post an E_RACE_STARTED event if the FlagStatus changes to Flag_Dropped
					// Only do this for one Kart, to avoid triggering three events.
					if (byte == 3 && Kart->FlagStatus != Flag_Dropped) {
						ES_Event Event = {E_RACE_STARTED};
						PostMasterSM(Event);
					}
					Kart->FlagStatus = Flag_Dropped; break;
				case CAUTION_FLAG:
					// Post an E_RACE_CAUTION event if the FlagStatus changes to Flag_Caution
					// Only do this for one Kart, to avoid triggering three events.
					if (byte == 3 && Kart->FlagStatus != Flag_Caution) {
						ES_Event Event = {E_RACE_CAUTION};
						PostMasterSM(Event);
					}
					Kart->FlagStatus = Flag_Caution; break;
				case RACE_OVER:
					// Post an E_RACE_FINISHED event if the FlagStatus changes to Flag_Finished
					// Only do this for one Kart, to avoid triggering three events.
					if (byte == 3 && Kart->FlagStatus != Flag_Finished) {
						ES_Event Event = {E_RACE_FINISHED};
						PostMasterSM(Event);
					}
					Kart->FlagStatus = Flag_Finished; break;
			}
			// Post an E_OBSTACLE_COMPLETED event if our ObstacleCompleted status changes
			// from false to true.
			if (Kart->ObstacleCompleted == false && 
				  (DRS_Data[byte] & OBSTACLE_STATUS_MASK) &&
					Kart == MyKart) {
						ES_Event Event = {E_OBSTACLE_COMPLETED};
						PostMasterSM(Event);
			}
			Kart->ObstacleCompleted = DRS_Data[byte] & OBSTACLE_STATUS_MASK;
			
			// Post an E_TARGET_SUCCESS event if our TargetSuccess status changes
			// from false to true.
			if (Kart->TargetSuccess == false && 
				  DRS_Data[byte] & TARGET_STATUS_MASK &&
					Kart == MyKart) {
						ES_Event Event = {E_TARGET_SUCCESS};
						PostMasterSM(Event);
			}
			Kart->TargetSuccess = DRS_Data[byte] & TARGET_STATUS_MASK;
		}
		
	// If not a GAME_STATUS_QUERY, then current query must be a KART_QUERY
	} else {
		// Set the Kart to update
		switch (CurrentQuery) {
			case KART1_QUERY: Kart = &Kart1; break;
			case KART2_QUERY: Kart = &Kart2; break;
			case KART3_QUERY: Kart = &Kart3; break;
		}
		// Record the Kart data
		Kart->KartX = DRS_Data[2]<<8 | DRS_Data[3]; // PXm (Byte 2) | PXl (Byte 3)
		Kart->KartY = DRS_Data[4]<<8 | DRS_Data[5]; // PYm (Byte 4) | PYl (Byte 5)
		Kart->KartTheta = (DRS_Data[6]<<8 | DRS_Data[7]) % 360; // Om (Byte 6) | Ol (Byte 7)
		if (Kart == MyKart) {
			ES_Event Event = {E_DRS_UPDATED};
			PostMasterSM(Event);
		};
		
		// Check if our gamefield position has changed
		GamefieldPosition_t NewGamefieldPosition = GetGamefieldPosition(Kart->KartX, Kart->KartY);
		if (Kart->GamefieldPosition != NewGamefieldPosition) {
			int KartNumber;
			if (Kart == &Kart1) KartNumber = 1;
			if (Kart == &Kart2) KartNumber = 2;
			if (Kart == &Kart3) KartNumber = 3;
//			
//			// Post an event if this is our Kart
//			if (Kart == MyKart) {
//				if (Kart->GamefieldPosition == Straight1 && NewGamefieldPosition == Corner1) {
//					ES_Event Event = {E_CORNER1_ENTRY};
//					PostMasterSM(Event);
//				} else if (Kart->GamefieldPosition == Corner1 && NewGamefieldPosition == Straight2) {
//					ES_Event Event = {E_CORNER1_EXIT};
//					PostMasterSM(Event);
//				} else if (Kart->GamefieldPosition == Straight2 && NewGamefieldPosition == Corner2) {
//					ES_Event Event = {E_CORNER2_ENTRY};
//					PostMasterSM(Event);
//				} else if (Kart->GamefieldPosition == Corner2 && NewGamefieldPosition == Straight3) {
//					ES_Event Event = {E_CORNER2_EXIT};
//					PostMasterSM(Event);
//				} else if (Kart->GamefieldPosition == Straight3 && NewGamefieldPosition == Corner3) {
//					ES_Event Event = {E_CORNER3_ENTRY};
//					PostMasterSM(Event);
//				} else if (Kart->GamefieldPosition == Corner3 && NewGamefieldPosition == Straight4) {
//					ES_Event Event = {E_CORNER3_EXIT};
//					PostMasterSM(Event);
//				} else if (Kart->GamefieldPosition == Straight4 && NewGamefieldPosition == Corner4) {
//					ES_Event Event = {E_CORNER4_ENTRY};
//					PostMasterSM(Event);
//				} else if (Kart->GamefieldPosition == Corner4 && NewGamefieldPosition == Straight1) {
//					ES_Event Event = {E_CORNER4_EXIT};
//					PostMasterSM(Event);
//				//} else if (Kart->GamefieldPosition == Straight2 && NewGamefieldPosition == BallLaunchingArea) {
//				//	ES_Event Event = {E_BALL_LAUNCHING_ENTRY};
//				//	PostMasterSM(Event);
//				//} else if (Kart->GamefieldPosition == BallLaunchingArea && NewGamefieldPosition == Straight2) {
//				//	ES_Event Event = {E_BALL_LAUNCHING_EXIT};
//				//	PostMasterSM(Event);
//				}
//			}
//			
			// Print statements to the display (don't print if transitioning from Undefined)
			if (Kart->GamefieldPosition != Undefined) {
				if (DisplayMyGamefieldPosition && Kart == MyKart)
					printf("My Kart (Kart %d): X = %d, Y = %d, Theta = %d, Laps Left = %d, Obstacle = %d, Target = %d, Gamefield Position = %s\r\n", \
					KartNumber, GetKartData(KartNumber).KartX, GetKartData(KartNumber).KartY, \
					GetKartData(KartNumber).KartTheta, GetKartData(KartNumber).LapsRemaining, \
					GetKartData(KartNumber).ObstacleCompleted, GetKartData(KartNumber).TargetSuccess, \
					GamefieldPositionString(NewGamefieldPosition));
				
				else if (DisplayGamefieldPositions)
					printf("Kart %d: X = %d, Y = %d, Theta = %d, Laps Left = %d, Obstacle = %d, Target = %d, Gamefield Position = %s\r\n", \
					KartNumber, GetKartData(KartNumber).KartX, GetKartData(KartNumber).KartY, \
					GetKartData(KartNumber).KartTheta, GetKartData(KartNumber).LapsRemaining, \
  				GetKartData(KartNumber).ObstacleCompleted, GetKartData(KartNumber).TargetSuccess, \
					GamefieldPositionString(NewGamefieldPosition));
			}
			
			// Update the gamefield position
			Kart->GamefieldPosition = NewGamefieldPosition;
		}
	}
	return true;
}


/****************************************************************************
Function: 		GetKartData
Parameters:		uint8_t KartNumber, the Kart number to get
Returns:			Kart_t, the Kart_t struct of the KartNumber
Description:	Returns the struct data for a Kart:
					uint16_t 	KartX;
					uint16_t 	KartY;
					uint16_t 	KartTheta;
					uint8_t		LapsRemaining;
					bool		ObstacleCompleted;
					bool		TargetSuccess;
					Flag_t	FlagStatus;
****************************************************************************/
Kart_t GetKartData(uint8_t KartNumber) {
	switch (KartNumber) {
		case 1:
			return Kart1;
		case 2:
			return Kart2;
		case 3:
		default:
			return Kart3;
	}
}

/****************************************************************************
Function:			PrintKartData
Parameters:		void
Returns:			void
Description:	Prints the Kart data to console
****************************************************************************/
void PrintKartData(void) {
	Kart_t Kart;
	for (uint8_t KartNumber = 1; KartNumber <= 3; KartNumber++) {
		printf("\r\nKart %d Data\r\n", KartNumber);
		switch (KartNumber) {
			case 1:
				Kart = Kart1; break;
			case 2:
				Kart = Kart2; break;
			case 3:
			default:
				Kart = Kart3; break;
		}
		printf("X Position = %d\r\n", Kart.KartX);
		printf("Y Position = %d\r\n", Kart.KartY);
		printf("Orientation = %d\r\n", Kart.KartTheta);
		printf("Laps Remaining = %d\r\n", Kart.LapsRemaining);
		printf("Flag Status = ");
		switch (Kart.FlagStatus) {
			case Flag_Waiting: printf("Flag_Waiting\r\n"); break;
			case Flag_Dropped: printf("Flag_Dropped\r\n"); break;
			case Flag_Caution: printf("Flag_Caution\r\n"); break;
			case Flag_Finished: printf("Flag_Finished\r\n"); break;
		}
		printf("Obstacle Complete = %s\r\n", Kart.ObstacleCompleted ? "true" : "false");
		printf("Target Success = %s\r\n", Kart.TargetSuccess ? "true" : "false");
	}
}

/****************************************************************************
Function:			PrintKartDataTableFormat
Parameters:		void
Returns:			void
Description:	Prints the Kart data to console
****************************************************************************/
void PrintKartDataTableFormat(void) {
	printf("\r\n| Kart # |  X  |  Y  | Theta | Laps Left | Flag Status | Obstacle | Target |\r\n");
	printf("|--------|-----|-----|-------|-----------|-------------|----------|--------|\r\n");
	Kart_t Kart;
	for (uint8_t KartNumber = 1; KartNumber <= 3; KartNumber++) {
		switch (KartNumber) {
		case 1:
			Kart = Kart1; break;
		case 2:
			Kart = Kart2; break;
		case 3:
		default:
			Kart = Kart3; break;
		}
		printf("| Kart %d |", KartNumber);
		printf(" %3d |", Kart.KartX);
		printf(" %3d |", Kart.KartY);
		printf(" %5d |", Kart.KartTheta);
		printf("     %d     |", Kart.LapsRemaining);
		switch (Kart.FlagStatus) {
			case Flag_Waiting: printf(" %10s  |", "Waiting"); break;
			case Flag_Dropped: printf(" %10s  |", "Dropped"); break;
			case Flag_Caution: printf(" %10s  |", "Caution"); break;
			case Flag_Finished: printf(" %10s  |", "Finished"); break;
		}
		printf("  %5s   |", Kart.ObstacleCompleted ? "True" : "False");
		printf(" %5s  |\r\n", Kart.TargetSuccess ? "True" : "False");
	}
}

/****************************************************************************
Function:			GetMyKart
Parameters:		void
Returns:			void
Description:	Returns our Kart struct
****************************************************************************/
Kart_t GetMyKart(void) {
	return *MyKart;
}


/*------------------------------ Test Harness -----------------------------*/
#ifdef TEST 
#include "termio.h" 
/* Test Harness for the DRS SPI Module */ 
int main(void) 
{ 
	_HW_Timer_Init(ES_Timer_RATE_1mS);
	TERMIO_Init();
	clrScrn();
	printf("In Test Harness for the DRS Module\n\r");
	InitializeDRS();
	uint8_t Query = GAME_STATUS_QUERY;
	printf("Query = 0x%02x ", Query);
	switch(Query) {
		case GAME_STATUS_QUERY: printf("(GAME_STATUS_QUERY)\r\n"); break;
		case KART1_QUERY: printf("(KART1_QUERY)\r\n"); break;
		case KART2_QUERY: printf("(KART2_QUERY)\r\n"); break;
		case KART3_QUERY: printf("(KART3_QUERY)\r\n"); break;
	}
	SendQuery(Query);
	return 0; 
} 
#endif 


/*------------------------------- Footnotes -------------------------------*/

/*------------------------------ End of file ------------------------------*/

