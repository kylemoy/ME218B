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
	BallLaunchingArea,
	ObstacleCrossingArea,
	Undefined
} GamefieldPosition_t;

// The bounding lines
#define Corner1XBound 101
#define Corner1YBound 33
#define Corner2XBound 101
#define Corner2YBound 145
#define Corner3XBound 217
#define Corner3YBound 149
#define Corner4XBound 220
#define Corner4YBound 37

#define Straight1YBound 29
#define Straight2XBound 97
#define Straight3YBound 154 
#define Straight4XBound 225

#define BallLaunchingObstacleBound  163

#define ObstacleEntryXBound 150 // X-value that initiates the turn into the obstacle
#define BallLaunchingEntryYBound 50 // Y-value that initiates the turn into the obstacle
 
#define CornerExit 0
#define Corner1Entry 50 // Not being used, using encoder trip
#define Corner2Entry 25
#define Corner3Entry 50 // Not being used, using encoder trip
#define Corner4Entry 40

// Target points (not being used right now)
#define Corner1X 101
#define Corner1Y 33 //22

#define Corner2X 101
#define Corner2Y 145

#define Corner3X 217 
#define Corner3Y 149

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
//#define East 75  // i.e. Direction to Corner1 (Green/Pink facing forward)
//#define North 165	// i.e. Direction to Corner2
//#define West 255	// i.e. Direction to Corner3
//#define South 345	// i.e. Direction to Corner4

/*----------------------- Public Function Prototypes ----------------------*/
GamefieldPosition_t GetGamefieldPosition(uint16_t Xcoord, uint16_t Ycoord);
const char * GamefieldPositionString(GamefieldPosition_t GamefieldPosition);
double GetAngle(uint8_t Xcoord, uint8_t Ycoord, uint8_t Xtarget, uint8_t Ytarget);

#endif /*GamefieldPositions_H */

