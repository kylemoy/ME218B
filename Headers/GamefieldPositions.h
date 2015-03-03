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


/* THESE ARE THE IMPORTANT TRIGGERS */
#define Straight2CenterY 93	// Calibrate this value
#define BallLaunchingYOffset 60	// This offset should be constant
#define BallLaunchingEntryYBound (Straight2CenterY - BallLaunchingYOffset)

#define Straight3CenterX	156	// Calibrate this value
#define ObstacleEntryXOffset 40	// This offset should be constant
#define ObstacleEntryXBound (Straight3CenterX - ObstacleEntryXOffset)


// The bounding lines
#define Corner1XBound 105
#define Corner1YBound 29
#define Corner2XBound 105
#define Corner2YBound 140
#define Corner3XBound 220
#define Corner3YBound 148
#define Corner4XBound 220
#define Corner4YBound 33
#define Straight1YBound 32
#define Straight2XBound 104
#define Straight3YBound 140 
#define Straight4XBound 214
#define BallLaunchingObstacleBound  163


/* OUTDATED CODE */
// We no longer use these, since we're using bumpers to trigger corners
//#define CornerExit 0
//#define Corner1Entry 0
//#define Corner2Entry 0
//#define Corner3Entry 0
//#define Corner4Entry 0


/*----------------------- Public Function Prototypes ----------------------*/
GamefieldPosition_t GetGamefieldPosition(uint16_t Xcoord, uint16_t Ycoord);
const char * GamefieldPositionString(GamefieldPosition_t GamefieldPosition);
//double GetAngle(uint8_t Xcoord, uint8_t Ycoord, uint8_t Xtarget, uint8_t Ytarget);

#endif /*GamefieldPositions_H */

