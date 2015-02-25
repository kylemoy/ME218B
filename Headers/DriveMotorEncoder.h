#ifndef _MotorEncoder_H_
#define _MotorEncoder_H_

#include <stdint.h>

void InitInputCapturePeriod( void );
void RDriveCaptureResponse( void );
void LDriveCaptureResponse( void );
uint32_t GetPeriodR( void );
uint32_t GetPeriodL( void );
uint32_t GetRPMR(void);
uint32_t GetRPML(void);


#endif //_IntSample_H_
