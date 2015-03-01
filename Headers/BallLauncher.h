/****************************************************************************
Module: BallLauncher.h
Description:
	Hardware module for the Ball Shooter system. Handles the initialization and
	contains functions for launching the ball.
Author: Alex Lin, 2/25/15
****************************************************************************/

#ifndef BallLauncher_H
#define BallLauncher_H

/*----------------------------- Include Files -----------------------------*/

/*----------------------------- Module Defines ----------------------------*/

/*----------------------- Public Function Prototypes ----------------------*/
void InitializeBallLauncher(void);
void SetShooterPWM(uint8_t DutyCycle);
void ServoForward(void);
void ServoReverse(void);

#endif 

/* BallLauncher_H */
