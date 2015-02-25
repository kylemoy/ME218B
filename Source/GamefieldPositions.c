/****************************************************************************
Module: GamefieldPositions.c
Description: This module defines the ten zones of the gamefield:
						 Straight1, Corner1, Straight2, Corner2,
						 Straight3, Corner3, Straight4, Corner4,
						 BallShootingZone, and ObstacleZone
Author: Kyle Moy, 2/21/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
#include <math.h>
#include "GamefieldPositions.h"


/*----------------------------- Module Defines ----------------------------*/
#define PI 3.14159265

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
Function: 		GetGamefieldPosition
Parameters:		uint8_t Xcoord, the X coordinate
						  uint8_t Ycoord, the Y coordinate
Returns:			GamefieldPosition_t, the gamefield position the point is in
Description:	Return the gamefield position that a point is in
****************************************************************************/
GamefieldPosition_t GetGamefieldPosition(uint8_t Xcoord, uint8_t Ycoord) {
	if (Xcoord == 0 && Ycoord == 0)
		return Undefined;
	if (Xcoord < Xright) {
		if (Ycoord < Ybottom)
			return Corner1;
		else if (Ycoord < Ytop) {
			if (Ycoord < Ytop - CornerEntry)
				return Straight2;
			else
				return Corner2;
		} else
			return Corner2;
	} else if (Xcoord < Xleft) {
		if (Ycoord < Ybottom) {
			if (Xcoord < Xright + CornerEntry)
				return Corner1;
			else
				return Straight1;
		} else if (Ycoord < Ytop)
			if (Xcoord < Xcenter)
				return BallShootingArea;
			else
				return ObstacleCrossingArea;
		else {
			if (Xcoord < Xleft - CornerEntry)
				return Straight3;
			else
				return Corner3;
		}
	} else {
		if (Ycoord < Ybottom)
			return Corner4;
		else if (Ycoord < Ytop)
			if (Ycoord < Ybottom + CornerEntry)
				return Corner4;
			else
				return Straight4;
		else
			return Corner3;
	}
}

/****************************************************************************
Function: 		GetAngle
Parameters:		uint8_t Xcoord, the X coordinate
						  uint8_t Ycoord, the Y coordinate
							uint8_t Xtarget, the target X coordinate
							uint8_t Ytarget, the target Y coordinate
Returns:			double, the angle between the current point and the target point
Description:	Return the angle between the current point and the target point
****************************************************************************/
double GetAngle(uint8_t Xcoord, uint8_t Ycoord, uint8_t Xtarget, uint8_t Ytarget) {
	double deltaX = Xtarget - Xcoord;
	double deltaY = Ytarget - Ycoord;
	double angle = atan2(deltaY, deltaX) * 180 / PI;
	return angle + 180;
}

	

const char * GamefieldPositionString(GamefieldPosition_t GamefieldPosition) {
	switch (GamefieldPosition) {
		case Straight1: return "Straight1";
		case Corner1: return "Corner1";
		case Straight2: return "Straight2";
		case Corner2: return "Corner2";
		case Straight3: return "Straight3";
		case Corner3: return "Corner3";
		case Straight4: return "Straight4";
		case Corner4: return "Corner4";
		case BallShootingArea: return "BallShootingArea";
		case ObstacleCrossingArea: return "ObstacleCrossingArea";
		case Undefined: default: return "Undefined";
	}
}

