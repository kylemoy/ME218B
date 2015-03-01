/****************************************************************************
Module: BumpSensor.h
Description:
	This module handles the bump sensor hardware initialization, and posts an
	event when a bump is sensed.
Author: Kyle Moy, 3/1/15
****************************************************************************/

#ifndef BumpSensor_H
#define BumpSensor_H

/*----------------------- Public Function Prototypes ----------------------*/
void InitializeBumpSensors(void);
bool BumpSensorDetected(void);

#endif /* BumpSensor_H */


