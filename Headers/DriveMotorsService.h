/****************************************************************************
Module: DriveMotorsService.c
Description:
	This module handles the motor timer responses and responds to events.
Author: Kyle Moy, 2/22/15
****************************************************************************/

#ifndef DriveMotorsService_H
#define DriveMotorsService_H

/*----------------------- Public Function Prototypes ----------------------*/
bool InitDriveMotorsService(uint8_t Priority);
bool PostDriveMotorsService(ES_Event ThisEvent);
ES_Event RunDriveMotorsService(ES_Event ThisEvent);

#endif /* DriveMotorsService_H */

