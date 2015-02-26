
#include <stdint.h>
#include <stdio.h>


#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "bitdefs.h"
#include "DriveMotorEncoder.h"
#include "DriveMotors.h"
#include "DriveMotorPID.h"
//#include "ADService.h"
//#include "PWMDemo.h"

//#include "IntSample.h"

// 40,000 ticks per mS assumes a 40Mhz clock
#define TicksPerMicroSecond 40
#define TicksPerMilliSecond 40000

//#define TEST

//650 target, p 0.03, i 0.15, 1ms loop time
//500 target, p 0.03, i 0.15, 1ms loop time

static float TargetRPMR = 100.0;
static float TargetRPML = 100.0;

// we will use Timer B in Wide Timer 1 to generate the interrupt
void InitPeriodicInt( void ){
  volatile uint32_t Dummy; // use volatile to avoid over-optimization
  // start by enabling the clock to the timer (Wide Timer 1)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R1;
	// kill a few cycles to let the clock get going
	Dummy = HWREG(SYSCTL_RCGCGPIO);
  
  // make sure that timer (Timer B) is disabled before configuring
  HWREG(WTIMER1_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TBEN;
  
	// set it up in 32bit wide (individual, not concatenated) mode
	// the constant name derives from the 16/32 bit timer, but this is a 32/64
	// bit timer so we are setting the 32bit mode
  HWREG(WTIMER1_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
  
	// set up timer B in periodic mode so that it repeats the time-outs
  HWREG(WTIMER1_BASE+TIMER_O_TBMR) = 
     (HWREG(WTIMER1_BASE+TIMER_O_TBMR)& ~TIMER_TBMR_TBMR_M)| 
     TIMER_TBMR_TBMR_PERIOD;

	// set timeout to 1mS
  HWREG(WTIMER1_BASE+TIMER_O_TBILR) = TicksPerMilliSecond * 1; //1ms

	// enable a local timeout interrupt
  HWREG(WTIMER1_BASE+TIMER_O_IMR) |= TIMER_IMR_TBTOIM;

	// enable the Timer B in Wide Timer 1 interrupt in the NVIC
	// it is interrupt number 94 so apppears in EN2 at bit 30
	HWREG(NVIC_EN3) |= BIT1HI;

	// make sure interrupts are enabled globally
  
	// now kick the timer off by enabling it and enabling the timer to
	// stall while stopped by the debugger
  HWREG(WTIMER1_BASE+TIMER_O_CTL) |= (TIMER_CTL_TBEN | TIMER_CTL_TBSTALL);
}

void SetRPMResponse( void ){
	static float SumErrorR=0.0; /* error integrator */
	static float RPMErrorR; /* make static for speed */
	static float LastErrorR; /* for Derivative Control */

	static float SumErrorL=0.0; /* error integrator */
	static float RPMErrorL; /* make static for speed */
	static float LastErrorL; /* for Derivative Control */

	//static float pGain = 1.26;
	static float pGain = 0.05;//1.92;
	static float iGain = 0.03;
	static float dGain = 0.03;

	// start by clearing the source of the interrupt
  HWREG(WTIMER1_BASE+TIMER_O_ICR) = TIMER_ICR_TBTOCINT;
		
	RPMErrorR = TargetRPMR - GetRPMR();
	SumErrorR += RPMErrorR;
	
	RPMErrorL = TargetRPML - GetRPML();
	SumErrorL += RPMErrorL;
	
	int32_t RequestedDutyR = 
		(pGain * ((RPMErrorR) + (iGain * SumErrorR) + (dGain * (RPMErrorR - LastErrorR))));
		
	int32_t RequestedDutyL = 
		(pGain * ((RPMErrorL) + (iGain * SumErrorL) + (dGain * (RPMErrorL - LastErrorL))));
		
	if (RequestedDutyR > 100) {
		RequestedDutyR = 100;
		SumErrorR-=RPMErrorR; /* anti-windup */
	}
	else if (RequestedDutyR < 0){
		RequestedDutyR = 0;
		SumErrorR-=RPMErrorR; /* anti-windup */
	}
	
	if (RequestedDutyL > 100) {
		RequestedDutyL = 100;
		SumErrorL-=RPMErrorL; /* anti-windup */
	}
	else if (RequestedDutyL < 0){
		RequestedDutyL = 0;
		SumErrorL-=RPMErrorL; /* anti-windup */
	}
	LastErrorR = RPMErrorR;
	LastErrorL = RPMErrorL;
	
	//SetMotorDirection(1,1); //take this away
	SetMotorPWM(RIGHT_MOTOR, RequestedDutyR);
	SetMotorPWM(LEFT_MOTOR, RequestedDutyL);
}

void SetTargetRPM(float SetRPMR, float SetRPML){
	TargetRPMR = SetRPMR;
	TargetRPML = SetRPML;
}
/*
uint32_t GetTimeoutCount( void ){
  return TimeoutCount;
}
*/
/*------------------------------- Footnotes -------------------------------*/
#ifdef TEST
#include "termio.h"
int main(void){
	TERMIO_Init();
	InitializeDriveMotors();
	InitInputCapturePeriod();
	InitPeriodicInt();
	for(;;)
	;
}
#endif
/*------------------------------ End of file ------------------------------*/
