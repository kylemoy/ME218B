#ifndef _MotorPID_H_
#define _MotorPID_H_

#include <stdint.h>
#include <stdbool.h>

#define FORWARD 0
#define REVERSE 1

void InitPeriodicInt( void );
void SetRPMResponse( void );
void SetTargetRPM (float SetRPMR, float SetRPML);
void EnablePIDcontrol(void);
void DisablePIDcontrol(void);
void ClearSumError(void);
void SetPIDgains(double p, double i, double d);

#endif //_IntSample_H_
