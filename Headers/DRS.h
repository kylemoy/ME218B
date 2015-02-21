/****************************************************************************
Module: DRS.h
Description:
	Hardware module for the DRS SPI communication system.
	Includes functions to initialize the SPI, send a query, and 
	the End of Transmission (EOT) interrupt response.
Author: Kyle Moy, 2/18/15
****************************************************************************/

#ifndef DRS_H
#define DRS_H

/*----------------------------- Include Files -----------------------------*/

/*----------------------------- Module Defines ----------------------------*/
// Command Queries Byte
#define GAME_STATUS_QUERY 	0x3F
#define KART1_QUERY 				0xC3
#define KART2_QUERY 				0x5A
#define KART3_QUERY 				0x7E

// Flag type definitions
typedef enum {
	Flag_Waiting,
	Flag_Dropped,
	Flag_Caution,
	Flag_Finished
} Flag_t;

// Data structure for holding Kart information
typedef struct {
	uint16_t 	KartX;
	uint16_t 	KartY;
	int16_t 	KartTheta;
	uint8_t		LapsRemaining;
	bool		ObstacleCompleted;
	bool		TargetSuccess;
	Flag_t	FlagStatus;
} Kart_t;


/*----------------------- Public Function Prototypes ----------------------*/
void DRS_Initialize(void);
void DRS_EOTIntHandler(void);
bool DRS_SendQuery(uint8_t Query);
bool DRS_StoreData(void);
Kart_t GetKartData(uint8_t KartNumber);
void PrintKartData(void);
void PrintKartDataTableFormat(void);

#endif /* DRS_H */