#ifndef BeaconSensor_H
#define BeaconSensor_H

#include <stdint.h>
#include <stdbool.h>

// Init function
void InitBeaconSensingCapture( void );

// Input Capture Response for use in E+S Framework
void BeaconSensedCaptureResponse( void );

// Returns period in ticks
uint32_t GetPeriod(void);

// Returns period in uS
uint32_t GetPerioduS(void);

// Gets the average period in uS over the last set number of ticks (10)
uint32_t GetAveragePerioduS(void);

// Checks the average period against set thresholds
bool IsBeaconSensed(void);

#endif /* BeaconSensor_H */
