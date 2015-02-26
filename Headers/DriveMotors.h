/****************************************************************************
Module: DriveMotors.h
Description:
	Hardware module for the DC drive motors. Handles initialization and control.
Author: Kyle Moy, 2/21/15
****************************************************************************/

#ifndef DriveMotors_H
#define DriveMotors_H


/*----------------------------- Module Defines ----------------------------*/
#define RIGHT_MOTOR 0
#define LEFT_MOTOR  1

#define FORWARD 0
#define BACKWARD 1


/*----------------------- Public Function Prototypes ----------------------*/
// Initializes the PWM pins for the drive motors on Port B
// Left Motor is on PB4, Right Motor is on PB6
void InitializeDriveMotors(void);

// Functions to control the motor
// Motor parameter should be RIGHT_MOTOR or LEFT_MOTOR
void SetMotorPWM(uint8_t Motor, uint8_t DutyCycle);
void SetMotorDirection(uint8_t Motor, uint8_t Direction);

// Functions for specific robot movements
void RotateCW(uint16_t TargetRPM, uint16_t Duration);
void RotateCCW(uint16_t TargetRPM, uint16_t Duration);
void StopMotors(void);
void DriveForward(uint16_t TargetRPM, uint16_t Duration);
void DriveForwardWithBias(uint16_t TargetRPML, uint16_t TargetRPMR, uint16_t Duration);
void DriveBackward(uint16_t TargetRPM, uint16_t Duration);

#endif /* DriveMotors_H */

