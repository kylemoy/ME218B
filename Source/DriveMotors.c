/****************************************************************************
Module: DriveMotors.c
Description:
	Hardware module for the DC drive motors. Handles initialization and control.
Author: Kyle Moy, 2/21/15
****************************************************************************/

//#define TEST

/*----------------------------- Include Files -----------------------------*/
// C Libraries
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"

// Hardware Libraries
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "termio.h"
#include "ES_Port.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"

// Module Libraries
#include "DriveMotors.h"
#include "DriveMotorsService.h"
#include "DriveMotorPID.h"
#include "DriveMotorEncoder.h"
#include "Display.h"
#include "SM_Master.h"


/*----------------------------- Module Defines ----------------------------*/
#define ALL_BITS (0xff <<2)
#define BitsPerNibble 4

// PWM Defines
// 40 ticks per microS assumes a 40Mhz clock, we will use SysClk/4 for PWM
#define PWMTicksPerMicroSecond 40/4
// Set 10KHz frequency, so 100microS period
#define PeriodInMicroSeconds 100


/*---------------------------- Module Variables ---------------------------*/
static uint8_t RightMotorDutyCycle = 0;  // PWM0
static uint8_t LeftMotorDutyCycle = 0;	 // PWM1
static bool RightMotorDirection = FORWARD;
static bool LeftMotorDirection = FORWARD;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function: 		InitializeDriveMotors
Parameters: 	void
Returns: 			void
Description: 	Initializes the PWM pins for the DC drive motors on Port B
							Left Motor PWM is on PB4 (PWM1), Right Motor PWM is on PB6 (PWM0)
							Left Motor Direction is on PB1, Right Motor Direction is on PB5
****************************************************************************/
void InitializeDriveMotors(void) {
  // Initialize the PWM
  volatile uint32_t Dummy; // use volatile to avoid over-optimization
  // Start by enabling the clock to the PWM Module (PWM0)
  HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;
  // Enable the clock to Port B
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
  // Select the PWM clock as System Clock/4
  HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
    (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_4);
  // Make sure that the PWM module clock has gotten going
  while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0 )
    ;
  // Disable the PWM while initializing
  HWREG( PWM0_BASE+PWM_O_0_CTL ) = 0;
  HWREG( PWM0_BASE+PWM_O_1_CTL ) = 0;
  // Program generator A to go to 0 at rising compare A, 1 on falling compare A  
  HWREG( PWM0_BASE+PWM_O_0_GENA) = 
    (PWM_0_GENA_ACTCMPAU_ZERO | PWM_0_GENA_ACTCMPAD_ONE );
  HWREG( PWM0_BASE+PWM_O_1_GENA) = 
    (PWM_1_GENA_ACTCMPAU_ZERO | PWM_1_GENA_ACTCMPAD_ONE );
  // Set the PWM period. Since we are counting both up & down, we initialize
  // the load register to 1/2 the desired total period
  HWREG( PWM0_BASE+PWM_O_0_LOAD) = (PeriodInMicroSeconds * PWMTicksPerMicroSecond)>>1;
  HWREG( PWM0_BASE+PWM_O_1_LOAD) = (PeriodInMicroSeconds * PWMTicksPerMicroSecond)>>1;
  // Set the initial duty cycle on A to 50% by programming the compare value
  // to 1/2 the period to count up (or down) 
  HWREG( PWM0_BASE+PWM_O_0_CMPA) = ((PeriodInMicroSeconds * PWMTicksPerMicroSecond)-1)>>2;
  HWREG( PWM0_BASE+PWM_O_1_CMPA) = ((PeriodInMicroSeconds * PWMTicksPerMicroSecond)-1)>>2;
  // Set changes to the PWM output Enables to be locally syncronized to a 
  // zero count
  HWREG(PWM0_BASE+PWM_O_ENUPD) =  (HWREG(PWM0_BASE+PWM_O_ENUPD) & 
      ~(PWM_ENUPD_ENUPD0_M | PWM_ENUPD_ENUPD1_M)) |
      (PWM_ENUPD_ENUPD0_LSYNC | PWM_ENUPD_ENUPD1_LSYNC);
  // Enable the PWM outputs
  HWREG( PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM2EN | PWM_ENABLE_PWM0EN);
  // Now configure the Port B pins to be PWM outputs
  // Start by selecting the alternate function for DC_MOTOR1 (PB4) [LEFT] and DC_MOTOR2 (PB6) [RIGHT]
  HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (BIT4HI | BIT6HI);
	// Now choose to map PWM to those pins, this is a mux value of 4 that we
	// want to use for specifying the function on bit 4 and bit 6
  HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) = 
    (HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0x00fffff) + (4<<(4*BitsPerNibble));
  HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) = 
    (HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0x00fffff) + (4<<(6*BitsPerNibble));
	// Enable pins on Port B for digital I/O
  HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (BIT1HI | BIT4HI | BIT5HI | BIT6HI);
	// Make pins on Port B into outputs
  HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (BIT1HI | BIT4HI | BIT5HI | BIT6HI);
	// Set the up/down count mode and enable the PWM generator
  HWREG(PWM0_BASE+ PWM_O_0_CTL) |= (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE);
  HWREG(PWM0_BASE+ PWM_O_1_CTL) |= (PWM_1_CTL_MODE | PWM_1_CTL_ENABLE);
 
	// Initialize the motor PWMs to default to 0
	SetMotorPWM(LEFT_MOTOR, 0);
	SetMotorPWM(RIGHT_MOTOR, 0);
	
	// Initialize the motor directions to default to FORWARD
	SetMotorDirection(LEFT_MOTOR, FORWARD);
	SetMotorDirection(RIGHT_MOTOR, FORWARD);
	
	InitInputCapturePeriod();
	InitPeriodicInt();
  printf("DC Motors Initialized.\n\r");
}


/****************************************************************************
Function: 		SetMotorPWM
Parameters: 	uint8_t Motor (#defines are LEFT_MOTOR or RIGHT_MOTOR)
							uint8_t DutyCycle
Returns: 			void
Description: 	Sets the PWM duty cycle for a DC drive motor
****************************************************************************/
void SetMotorPWM(uint8_t Motor, uint8_t DutyCycle) {
  switch (Motor) {
    case RIGHT_MOTOR:
      RightMotorDutyCycle = DutyCycle;
      if (RightMotorDutyCycle == 0) {
				// Set the Duty cycle on A to 0% by programming the compare value
				// to 0. However, since the CmpADn action (set to one) wins, we also
				// need to disable the output  
        HWREG(PWM0_BASE+PWM_O_0_CMPA) = 0;
        HWREG(PWM0_BASE+PWM_O_ENABLE) &= ~PWM_ENABLE_PWM0EN;
      } else if (RightMotorDutyCycle == 100) {
        // Set the Duty cycle on A to 100% by programming the compare value
        // to the load value. Since the CmpBDn action (set to one) wins, we get 100%
        HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM0EN;
        HWREG(PWM0_BASE+PWM_O_0_CMPA) = HWREG(PWM0_BASE+PWM_O_0_LOAD);
      } else {
        HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM0EN;
        uint32_t DutyCycleTicks = (PeriodInMicroSeconds * PWMTicksPerMicroSecond) / 2 * RightMotorDutyCycle / 100;
        HWREG(PWM0_BASE+PWM_O_0_CMPA) = DutyCycleTicks;
      }
      break;
    case LEFT_MOTOR:
      LeftMotorDutyCycle = DutyCycle;
      if (LeftMotorDutyCycle == 0) {
				// Set the Duty cycle on A to 0% by programming the compare value
				// to 0. However, since the CmpADn action (set to one) wins, we also
				// need to disable the output  
        HWREG(PWM0_BASE+PWM_O_1_CMPA) = 0;
        HWREG(PWM0_BASE+PWM_O_ENABLE) &= ~PWM_ENABLE_PWM2EN;
      } else if (LeftMotorDutyCycle == 100) {
        // Set the Duty cycle on A to 100% by programming the compare value
        // to the load value. Since the CmpBDn action (set to one) wins, we get 100%
        HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM2EN;
        HWREG(PWM0_BASE+PWM_O_1_CMPA) = HWREG(PWM0_BASE+PWM_O_1_LOAD);
      } else {
        HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM2EN;
        uint32_t DutyCycleTicks = (PeriodInMicroSeconds * PWMTicksPerMicroSecond) / 2 * LeftMotorDutyCycle / 100;
        HWREG(PWM0_BASE+PWM_O_1_CMPA) = DutyCycleTicks;
      }
      break;
  }
}


/****************************************************************************
Function: 		SetMotorDirection
Parameters: 	uint8_t Motor (#defines are LEFT_MOTOR or RIGHT_MOTOR)
							uint8_t Direction (#defines are FORWARD or BACKWARD)
Returns: 			void
Description: 	Sets the direction for a DC drive motor
****************************************************************************/
void SetMotorDirection(uint8_t Motor, uint8_t Direction) {
  switch (Motor) {
    case RIGHT_MOTOR:
			RightMotorDirection = Direction;
			if (RightMotorDirection == FORWARD) {
				HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) |= BIT5HI;
			} else {
				HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) &= BIT5LO;
			}
      break;
    case LEFT_MOTOR:
			LeftMotorDirection = Direction;
      if (LeftMotorDirection == FORWARD) {
				HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) |= BIT1HI;
			} else {
				HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) &= BIT1LO;
      }
			break;
  }
}

/****************************************************************************
Function: 		RotateCW
Parameters: 	void
Returns: 			void
Description: 	Rotates the bot clockwise
****************************************************************************/
void RotateCW(uint16_t TargetRPM, uint16_t Duration) {
	if (DisplayMotorInfo) printf("Drive Motors: Rotating CW, TargetRPM = %d\r\n", TargetRPM);
	EnablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, FORWARD);
	SetMotorDirection(RIGHT_MOTOR, BACKWARD);
	SetTargetRPM(TargetRPM, TargetRPM);
	if (Duration != 0) {
		ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, Duration);
	}
}

/****************************************************************************
Function: 		RotateCCW
Parameters: 	void
Returns: 			void
Description: 	Rotates the bot counter-clockwise
****************************************************************************/
void RotateCCW(uint16_t TargetRPM, uint16_t Duration) {
	if (DisplayMotorInfo) printf("Drive Motors: Rotating CCW, TargetRPM = %d\r\n", TargetRPM);
	EnablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, BACKWARD);
	SetMotorDirection(RIGHT_MOTOR, FORWARD);
	SetTargetRPM(TargetRPM, TargetRPM);
	if (Duration != 0) {
		ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, Duration);
	}
}


/****************************************************************************
Function: 		StopMotors
Parameters: 	void
Returns: 			void
Description: 	Stops the bot
****************************************************************************/
void StopMotors(void) {
	if (DisplayMotorInfo) printf("Drive Motors: Stopping\r\n");
	DisablePIDcontrol();
	SetMotorPWM(LEFT_MOTOR, 0);
	SetMotorPWM(RIGHT_MOTOR, 0);
	SetTargetRPM(0, 0);
}

/****************************************************************************
Function: 		DriveForward
Parameters: 	void
Returns: 			void
Description: 	Drive the bot forward
****************************************************************************/
void DriveForward(uint16_t TargetRPM, uint16_t Duration) {
	if (DisplayMotorInfo) printf("Drive Motors: Driving Forward, TargetRPM = %d\r\n", TargetRPM);
	EnablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, FORWARD);
	SetMotorDirection(RIGHT_MOTOR, FORWARD);
	SetTargetRPM(TargetRPM, TargetRPM);
	if (Duration != 0) {
		ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, Duration);
	}
}


/****************************************************************************
Function: 		PivotCWwithSetTicks
Parameters: 	void
Returns: 			void
Description: 	Pivot the bot clockwise
****************************************************************************/
void PivotCWwithSetTicks(uint16_t TargetRPM, uint32_t Ticks) {
	EnablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, FORWARD);
	SetMotorDirection(RIGHT_MOTOR, BACKWARD);
	SetTargetRPM(0, TargetRPM);
	if (DisplayMotorInfo) printf("Drive Motors: Pivoting CW, TargetTicks = %d\r\n", Ticks);
	SetTargetTickCount(0, Ticks);
}

/****************************************************************************
Function: 		PivotCCWwithSetTicks
Parameters: 	void
Returns: 			void
Description: 	Pivot the bot counter-clockwise
****************************************************************************/
void PivotCCWwithSetTicks(uint16_t TargetRPM, uint32_t Ticks) {
	EnablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, BACKWARD);
	SetMotorDirection(RIGHT_MOTOR, FORWARD);
	SetTargetRPM(0, TargetRPM);
	if (DisplayMotorInfo) printf("Drive Motors: Pivoting CCW, TargetTicks = %d\r\n", Ticks);
	SetTargetTickCount(Ticks, 0);
}


/****************************************************************************
Function: 		DriveForwardWithSetDistance
Parameters: 	void
Returns: 			void
Description: 	Drive the bot forward
****************************************************************************/
void DriveForwardWithSetDistance(uint16_t TargetRPM, uint32_t DistanceInMM) {
	EnablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, FORWARD);
	SetMotorDirection(RIGHT_MOTOR, FORWARD);
	SetTargetRPM(TargetRPM, TargetRPM);
	uint32_t NumberOfTicks = DistanceInMM / (3.141592 * 94 / 28); // 94mm diameter, 28 pulse/rev
	SetTargetTickCount(NumberOfTicks, NumberOfTicks);
	if (DisplayMotorInfo) printf("Drive Motors: Driving Forward with Set Distance = %d, TargetTicks = %d, TargetRPM = %d\r\n", DistanceInMM, NumberOfTicks, TargetRPM);
	
}


/****************************************************************************
Function: 		DriveForwardWithSetDistance
Parameters: 	void
Returns: 			void
Description: 	Drive the bot forward
****************************************************************************/
void DriveForwardWithBiasAndSetDistance(uint16_t TargetRPML, uint16_t TargetRPMR, uint32_t DistanceInMM) {
	EnablePIDcontrol();
	uint32_t NumberOfTicks = DistanceInMM / (3.141592 * 94 / 28); // 94mm diameter, 28 pulse/rev
	SetTargetTickCount(NumberOfTicks, NumberOfTicks);
	SetMotorDirection(LEFT_MOTOR, FORWARD);
	SetMotorDirection(RIGHT_MOTOR, FORWARD);
	SetTargetRPM(TargetRPMR, TargetRPML);
	if (DisplayMotorInfo) printf("Drive Motors: Driving Forward with Set Distance = %d, TargetTicks = %d, TargetRPMR = %d, TargetRPML = %d\r\n", DistanceInMM, NumberOfTicks, TargetRPMR, TargetRPML);
}

/****************************************************************************
Function: 		DriveForward
Parameters: 	void
Returns: 			void
Description: 	Drive the bot forward
****************************************************************************/
void DriveForwardWithBias(uint16_t TargetRPML, uint16_t TargetRPMR, uint16_t Duration) {
	if (DisplayMotorInfo) printf("Drive Motors: Driving Forward with Bias, TargetRPML = %d, TargetRPMR = %d\r\n", TargetRPML, TargetRPMR);
	EnablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, FORWARD);
	SetMotorDirection(RIGHT_MOTOR, FORWARD);
	SetTargetRPM(TargetRPMR, TargetRPML);
	if (Duration != 0) {
		ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, Duration);
	}
}

/****************************************************************************
Function: 		DriveBackwardsWithBias
Parameters: 	void
Returns: 			void
Description: 	Drive the bot backwards
****************************************************************************/
void DriveBackwardsWithBias(uint16_t TargetRPML, uint16_t TargetRPMR, uint16_t Duration) {
	if (DisplayMotorInfo) printf("Drive Motors: Driving Backwards with Bias, TargetRPML = %d, TargetRPMR = %d\r\n", TargetRPML, TargetRPMR);
	EnablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, BACKWARD);
	SetMotorDirection(RIGHT_MOTOR, BACKWARD);
	SetTargetRPM(TargetRPMR, TargetRPML);
	if (Duration != 0) {
		ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, Duration);
	}
}


/****************************************************************************
Function: 		DriveBackward
Parameters: 	void
Returns: 			void
Description: 	Drive the bot backward
****************************************************************************/
void DriveBackward(uint16_t TargetRPM, uint16_t Duration) {
	if (DisplayMotorInfo) printf("Drive Motors: Driving Backward, TargetRPM = %d\r\n", TargetRPM);
	EnablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, BACKWARD);
	SetMotorDirection(RIGHT_MOTOR, BACKWARD);
	SetTargetRPM(TargetRPM, TargetRPM);
	if (Duration != 0) {
		ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, Duration);
	}
}

/****************************************************************************
Function: 		RotateCWwithDuty
Parameters: 	void
Returns: 			void
Description: 	Rotates the bot clockwise
****************************************************************************/
void RotateCWwithDuty(uint16_t DutyCycle, uint16_t Duration) {
	if (DisplayMotorInfo) printf("Drive Motors: Rotating CW, Duty Cycle = %d\r\n", DutyCycle);
	DisablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, FORWARD);
	SetMotorDirection(RIGHT_MOTOR, BACKWARD);
	SetMotorPWM(LEFT_MOTOR, DutyCycle);
	SetMotorPWM(RIGHT_MOTOR, DutyCycle);
	if (Duration != 0) {
		ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, Duration);
	}
}

/****************************************************************************
Function: 		RotateCCWwithDuty
Parameters: 	void
Returns: 			void
Description: 	Rotates the bot counter-clockwise
****************************************************************************/
void RotateCCWwithDuty(uint16_t DutyCycle, uint16_t Duration) {
	if (DisplayMotorInfo) printf("Drive Motors: Rotating CCW, Duty Cycle = %d\r\n", DutyCycle);
	DisablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, BACKWARD);
	SetMotorDirection(RIGHT_MOTOR, FORWARD);
	SetMotorPWM(LEFT_MOTOR, DutyCycle);
	SetMotorPWM(RIGHT_MOTOR, DutyCycle);
	if (Duration != 0) {
		ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, Duration);
	}
}

/****************************************************************************
Function: 		DriveForward
Parameters: 	void
Returns: 			void
Description: 	Drive the bot forward
****************************************************************************/
void DriveForwardWithDuty(uint16_t Duty, uint16_t Duration) {
	if (DisplayMotorInfo) printf("Drive Motors: Driving Forward, Duty Cycle = %d\r\n", Duty);
	DisablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, FORWARD);
	SetMotorDirection(RIGHT_MOTOR, FORWARD);
	SetMotorPWM(LEFT_MOTOR, Duty);
	SetMotorPWM(RIGHT_MOTOR, Duty);
	if (Duration != 0) {
		ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, Duration);
	}
}


/****************************************************************************
Function: 		DriveForward
Parameters: 	void
Returns: 			void
Description: 	Drive the bot forward
****************************************************************************/
void DriveForwardWithBiasWithDuty(uint16_t DutyL, uint16_t DutyR, uint16_t Duration) {
	if (DisplayMotorInfo) printf("Drive Motors: Driving Forward with Bias, DutyL = %d, DutyR = %d\r\n", DutyL, DutyR);
	DisablePIDcontrol();
	SetMotorDirection(LEFT_MOTOR, FORWARD);
	SetMotorDirection(RIGHT_MOTOR, FORWARD);
	SetMotorPWM(LEFT_MOTOR, DutyL);
	SetMotorPWM(RIGHT_MOTOR, DutyR);
	if (Duration != 0) {
		ES_Timer_InitTimer(DRIVE_MOTOR_TIMER, Duration);
	}
}

		

/*------------------------------ Test Harness -----------------------------*/
#ifdef TEST 
/* Test Harness for the DC Drive Motors Module */ 
int main(void) 
{ 
  // Set the clock to run at 40MhZ using the PLL and 16MHz external crystal
  SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
      | SYSCTL_XTAL_16MHZ);
  TERMIO_Init(); 
	clrScrn();
	printf("In Test Harness for the DC Drive Motors Module\n\r");
  InitializeDriveMotors();
	_HW_Timer_Init(ES_Timer_RATE_1mS);
	StopMotors();
	
	static int16_t TargetRPM = 500;
	static uint8_t deltaRPM = 100;
	
	while(true){		
		// Use keyboard input to control motor
		char input = getchar();
		switch (input) {
			case 'w':
				DriveForward(500, 0);
				break;
			
			case 's':
				DriveBackward(150, 0);
				break;
			
			case 'a':
				RotateCCW(30, 0);
				break;
			
			case 'd':
				RotateCW(20, 0);
				break;
			
			case ' ':
				StopMotors();
				break;		
			
			
			case 'e':
				DriveForwardWithBias(TargetRPM + deltaRPM, TargetRPM - deltaRPM, 0);
				break;	
			
			
			case 'q':
				DriveForwardWithBias(TargetRPM - deltaRPM, TargetRPM + deltaRPM, 0);
				break;	
			
			case 'r':
				deltaRPM++;
				printf("DeltaRPM = %d\r\n", deltaRPM);
				break;	
			
			case 'f':
				deltaRPM--;
				printf("DeltaRPM = %d\r\n", deltaRPM);
				break;	
		}
	}
}
#endif

/*------------------------------ End of file ------------------------------*/
