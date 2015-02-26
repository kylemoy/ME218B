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
#define CornerEntry 0 // Extends the corner entry beyond the corner box

// Target points
#define Corner1X 65
#define Corner1Y 180 //22

#define Corner2X 64
#define Corner2Y 187

#define Corner3X 236
#define Corner3Y 177

#define Corner4X 230
#define Corner4Y 17

#define ObstacleEntryX 67
#define ObstacleEntryY 67

#define ObstacleExitX 67
#define ObstacleExitY 67

#define BallLaunchingEntryExitX 67
#define BallLaunchingEntryExitY 67

#define BallLaunchingX 67
#define BallLaunchingY 67


// Target angles
#define East 75  // i.e. Direction to Corner1 (Green/Pink facing forward)
#define North 165	// i.e. Direction to Corner2
#define West 255	// i.e. Direction to Corner3
#define South 345	// i.e. Direction to Corner4

/*----------------------- Public Function Prototypes ----------------------*/
GamefieldPosition_t GetGamefieldPosition(uint8_t Xcoord, uint8_t Ycoord);
const char * GamefieldPositionString(GamefieldPosition_t GamefieldPosition);
double GetAngle(uint8_t Xcoord, uint8_t Ycoord, uint8_t Xtarget, uint8_t Ytarget);

#endif /*GamefieldPositions_H */

