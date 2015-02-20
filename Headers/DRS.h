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

// Query Game Status - DRS Response Byte Masks
#define LAPS_REMAINING_MASK		0x07	// Bits 0-2
#define FLAG_STATUS_MASK			0x18	// Bits 3-4
#define OBSTACLE_STATUS_MASK	0x40	// Bit 6
#define TARGET_STATUS_MASK		0x80	// Bit 7

// Query Game Status - DRS Response Byte Values
#define WAITING_FOR_START 	0x00	// 0x00 on Bits 3-4
#define FLAG_DROPPED				0x04	// 0x01 on Bits 3-4
#define CAUTION_FLAG				0x08	// 0x02 on Bits 3-4
#define RACE_OVER						0x09	// 0x03 on Bits 3-4
#define OBSTACLE_COMPLETED	0x40	// 1 on Bit 6
#define TARGET_SUCCESSFUL		0x80	// 1 on Bit 7
#define INVALID_READ 				0xFF

/*----------------------- Public Function Prototypes ----------------------*/
void DRS_Initialize(void);
void DRS_EOTIntHandler(void);
bool DRS_SendQuery(uint8_t Query);
void DRS_PrintData(void);

#endif /* DRS_H */
