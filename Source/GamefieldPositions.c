/****************************************************************************
Module: GamefieldPositions.c
Description: This module defines the ten zones of the gamefield:
						 Straight1, Corner1, Straight2, Corner2,
						 Straight3, Corner3, Straight4, Corner4,
						 BallShootingZone, and ObstacleZone
Author: Kyle Moy, 2/21/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
#include "GamefieldPositions.h"


/*---------------------------- Module Variables ---------------------------*/

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
Function: 		GetGamefieldPosition
Parameters:		uint8_t Xcoord, the X coordinate
						  uint8_t Ycoord, the Y coordinate
Returns:			GamefieldPosition_t, the gamefield position the point is in
Description:	Return the gamefield position that a point is in
****************************************************************************/
GamefieldPosition_t GetGamefieldPosition(uint8_t Xcoord, uint8_t Ycoord) {
	if (Xcoord < Xright) {
		if (Ycoord < Ybottom)
			return Corner1;
		else if (Ycoord < Ytop)
			return Straight2;
		else
			return Corner2;
	} else if (Xcoord < Xleft) {
		if (Ycoord < Ybottom)
			return Straight1;
		else if (Ycoord < Ytop)
			if (Xcoord < Xcenter)
				return BallShootingArea;
			else
				return ObstacleCrossingArea;
		else
			return Straight3;
	} else {
		if (Ycoord < Ybottom)
			return Corner4;
		else if (Ycoord < Ytop)
			return Straight4;
		else
			return Corner3;
	}
}
