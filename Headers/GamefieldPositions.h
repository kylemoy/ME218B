/****************************************************************************
Module: GamefieldPositions.h
Description: This module defines the ten zones of the gamefield:
						 Straight1, Corner1, Straight2, Corner2,
						 Straight3, Corner3, Straight4, Corner4,
						 BallShootingZone, and ObstacleCrossingZone
Author: Kyle Moy, 2/21/15
****************************************************************************/

#ifndef GamefieldPositions_H
#define GamefieldPositions_H

/*----------------------------- Include Files -----------------------------*/
#include <stdint.h>

/*----------------------------- Module Defines ----------------------------*/
// The gamefield positions
typedef enum {
	Straight1,
	Corner1,
	Straight2,
	Corner2,
	Straight3,
	Corner3,
	Straight4,
	Corner4,
	BallShootingArea,
	ObstacleCrossingArea,
	Undefined
} GamefieldPosition_t;

// The bounding lines (see Racing_Positions.xlsx for data)
#define Xleft 205	
#define Xcenter 147
#define Xright 89
#define Ybottom 40
#define Ycenter 100
#define Ytop 156
#define CornerEntry 30 // Extends the corner entry beyond the corner box

/*----------------------- Public Function Prototypes ----------------------*/
GamefieldPosition_t GetGamefieldPosition(uint8_t Xcoord, uint8_t Ycoord);
const char * GamefieldPositionString(GamefieldPosition_t GamefieldPosition);

#endif /*GamefieldPositions_H */

