/****************************************************************************
Module: BallLauncher.c
Description:
	Hardware module for the Ball Shooter system. Handles the initialization and
	contains functions for launching the ball.
Author: Alex Lin, 2/25/15
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
#include "BallLauncher.h"
#include "BeaconSensor.h"
#include "DriveMotors.h"

/*----------------------------- Module Defines ----------------------------*/
#define ALL_BITS (0xff <<2)
#define BitsPerNibble 4

// PWM Defines
// 40 ticks per microS assumes a 40Mhz clock.  SysClk/32 gives a 10KHz frequency.
#define PWMTicksPerMicroSecond 40
// Set 10KHz frequency, so 100microS period
// Scale frequency for easy control of pulse width.  20 mS.
//#define PeriodInMicroSeconds 100000
#define PeriodInMicroSeconds 40000
#define SERVO_FORWARD_PULSE_WIDTH 30
#define SERVO_REVERSE_PULSE_WIDTH 48



/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function: 		InitializeBallLauncher
Parameters: 	void
Returns:			void
Description:	Initializes the ball launcher hardware (Motor on E4, Servo on C4)
***************************************************************************/
void InitializeBallLauncher(void) {
// Initialize PWM 2 for shooter motor (M4) and PWM 3 for shooter servo control (M6)
  volatile uint32_t Dummy; // use volatile to avoid over-optimization
// start by enabling the clock to the PWM Module (PWM0)
  HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;
// enable the clock to Port C and E 
  HWREG(SYSCTL_RCGCGPIO) |= (SYSCTL_RCGCGPIO_R2 | SYSCTL_RCGCGPIO_R4);
// Select the PWM clock as System Clock/32
  HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
    (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
// make sure that the PWM module clock has gotten going
	while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0)
    ;
// disable the PWM while initializing
  HWREG( PWM0_BASE+PWM_O_2_CTL ) = 0;
	HWREG( PWM0_BASE+PWM_O_3_CTL ) = 0;
  // Program generator A to go to 0 at rising compare A, 1 on falling compare A  
  HWREG( PWM0_BASE+PWM_O_2_GENA) = 
    (PWM_2_GENA_ACTCMPAU_ZERO | PWM_2_GENA_ACTCMPAD_ONE );
  HWREG( PWM0_BASE+PWM_O_3_GENA) = 
    (PWM_3_GENA_ACTCMPAU_ZERO | PWM_3_GENA_ACTCMPAD_ONE );
  // Set the PWM period. Since we are counting both up & down, we initialize
  // the load register to 1/2 the desired total period
  HWREG( PWM0_BASE+PWM_O_2_LOAD) = (PeriodInMicroSeconds * PWMTicksPerMicroSecond)>>1;
  HWREG( PWM0_BASE+PWM_O_3_LOAD) = (PeriodInMicroSeconds * PWMTicksPerMicroSecond)>>1;
  // Set the initial duty cycle on A to 50% by programming the compare value
  // to 1/2 the period to count up (or down) 
  HWREG( PWM0_BASE+PWM_O_2_CMPA) = ((PeriodInMicroSeconds * PWMTicksPerMicroSecond)-1)>>2;
  HWREG( PWM0_BASE+PWM_O_3_CMPA) = ((PeriodInMicroSeconds * PWMTicksPerMicroSecond)-1)>>2;
  // Set changes to the PWM output Enables to be locally syncronized to a 
  // zero count
  HWREG(PWM0_BASE+PWM_O_ENUPD) |=  (HWREG(PWM0_BASE+PWM_O_ENUPD) & 
      ~(PWM_ENUPD_ENUPD2_M | PWM_ENUPD_ENUPD3_M)) |
      (PWM_ENUPD_ENUPD2_LSYNC | PWM_ENUPD_ENUPD3_LSYNC);
  // Enable the PWM outputs
  HWREG( PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM4EN | PWM_ENABLE_PWM6EN);
  // Now configure the Port E4 (Motor) and C4 pins (Servo) to be PWM outputs
  // Start by selecting the alternate function
  HWREG(GPIO_PORTE_BASE+GPIO_O_AFSEL) |= (BIT4HI);
	HWREG(GPIO_PORTC_BASE+GPIO_O_AFSEL) |= (BIT4HI);
	// Now choose to map PWM to those pins, this is a mux value of 4 that we
	// want to use for specifying the function on bit 4 and bit 4
  HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) |= 
    (HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) & 0x00fffff) + (4<<(4*BitsPerNibble));
  HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) |= 
    (HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) & 0x00fffff) + (4<<(4*BitsPerNibble));
	// Enable pins on Port C and E for digital I/O
  HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= (BIT4HI);
	HWREG(GPIO_PORTE_BASE+GPIO_O_DEN) |= (BIT4HI);
	// Make pins on Port C and E into outputs
  HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) |= (BIT4HI);
	HWREG(GPIO_PORTE_BASE+GPIO_O_DIR) |= (BIT4HI);
	// Enable a pull down on Port E4
	//HWREG(GPIO_PORTE_BASE+GPIO_O_PDR) |=  GPIO_PIN_4;
	// Set the up/down count mode and enable the PWM generator
  HWREG(PWM0_BASE+ PWM_O_2_CTL) |= (PWM_2_CTL_MODE | PWM_2_CTL_ENABLE);
  HWREG(PWM0_BASE+ PWM_O_3_CTL) |= (PWM_3_CTL_MODE | PWM_3_CTL_ENABLE);
	ServoReverse();
	//SetShooterPWM(0);
	TurnOffShooter();
	
	
	// ENABLE BALL LAUNCHER MOTOR ON PIN D3
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3; // Port D
	HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= GPIO_PIN_3; // Enable Pin D3 for Digital I/O
	HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) |= GPIO_PIN_3; // Enable Pin D3 as Output
}

/****************************************************************************
Function: 		SetShooterPWM
Parameters: 	void
Returns:			void
Description:	Sets PWM for DC shooting motor.
****************************************************************************/
void SetShooterPWM(uint8_t DutyCycle) {
      if (DutyCycle == 0) {
				// Set the Duty cycle on A to 0% by programming the compare value
				// to 0. However, since the CmpADn action (set to one) wins, we also
				// need to disable the output  
				HWREG(PWM0_BASE+PWM_O_2_CMPA) = 0;
        HWREG(PWM0_BASE+PWM_O_ENABLE) &= ~PWM_ENABLE_PWM4EN;
				printf("Turning shooter motor off.\r\n");
      }
			else if (DutyCycle == 100) {
        // Set the Duty cycle on A to 100% by programming the compare value
        // to the load value. Since the CmpBDn action (set to one) wins, we get 100%
        HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM4EN;
        HWREG(PWM0_BASE+PWM_O_2_CMPA) = HWREG(PWM0_BASE+PWM_O_2_LOAD);
				printf("Turning shooter motor on.\r\n");
			} else {
        HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM4EN;
        uint32_t DutyCycleTicks = ((PeriodInMicroSeconds * PWMTicksPerMicroSecond)/2 * DutyCycle / 100);
				//uint32_t DutyCycleTicks = ((PeriodInMicroSeconds * PWMTicksPerMicroSecond)/2 * (100 - DutyCycle) / 100);
				//uint32_t DutyCycleTicks = HWREG(PWM0_BASE+PWM_O_2_LOAD)*DutyCycle/100;
				//printf("Period Ticks = %d , DutyCycleTicks = %d\n\r",PeriodInMicroSeconds*PWMTicksPerMicroSecond, DutyCycleTicks);
        HWREG(PWM0_BASE+PWM_O_2_CMPA) = DutyCycleTicks;
			}
			ServoReverse();
	return;
}

/****************************************************************************
Function: 		ServoForward
Parameters: 	void
Returns:			void
Description:	Moves the servo into the forward position to load a ballpopopop
****************************************************************************/
void ServoForward(void){
	HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM6EN;
	HWREG(PWM0_BASE+PWM_O_3_CMPA) = ((SERVO_FORWARD_PULSE_WIDTH * PWMTicksPerMicroSecond)>>1);
	printf("Servo arm in forward position.\r\n");
	return;
}

/****************************************************************************
Function: 		ServoReverse
Parameters: 	void
Returns:			void
Description:	Moves the servo into the forward position to load a ball
****************************************************************************/
void ServoReverse(void){
	HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM6EN;
	HWREG(PWM0_BASE+PWM_O_3_CMPA) = ((SERVO_REVERSE_PULSE_WIDTH * PWMTicksPerMicroSecond)>>1);
	printf("Servo arm in reverse position.\r\n");
	return;
}

void TurnOnShooter(void) {
	printf("Turning on the shooter motor.\r\n");
	HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA + ALL_BITS)) |= GPIO_PIN_3;
}

void TurnOffShooter(void) {
	printf("Turning off the shooter motor.\r\n");
	HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~GPIO_PIN_3;
}
	

/*------------------------------ Test Harness -----------------------------*/
#ifdef TEST 
/* Test Harness for the Ball Launcher Module */ 
int main(void) 
{ 
  // Set the clock to run at 40MhZ using the PLL and 16MHz external crystal
  SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
      | SYSCTL_XTAL_16MHZ);
  TERMIO_Init(); 
	//clrScrn();
	printf("In Test Harness for the Ball Launcher Module\n\r");
	InitBeaconSensingCapture();
	InitializeBallLauncher();
	InitializeDriveMotors();
	_HW_Timer_Init(ES_Timer_RATE_1mS);
	int pwm = 0;
	
	
	//IR Test
	SetShooterPWM(0);
		while(true){
			RotateCW(20,0);
			ServoReverse();
			if(IsBeaconSensed()){
				printf("Beacon sensed");
				//SetMotorDirection(FORWARD, FORWARD);
				StopMotors();
				//RotateCW(0,0);
				break;
			}
		}
		//HARDCODED TEST
			SetShooterPWM(100);
		//JANK ASS WAIT
		int waitTime1 = 2500;
		for(int i = 0; i < waitTime1; i++){
			printf("Waiting.... %d / %d \n\r", i, waitTime1);
		}
		for(int k = 0; k < 4; k++){
			ServoForward();
			int waitTime2 = 800;
			for(int i = 0; i < waitTime2; i++){
				printf("Waiting.... %d / %d \n\r", i, waitTime2);
			}
			ServoReverse();
			for(int i = 0; i < waitTime2; i++){
				printf("Waiting.... %d / %d \n\r", i, waitTime2);
			}
		}
		SetShooterPWM(0);
	}
		
		//Launch Test
	
/*		while(true){
			switch(CurrentState){
				case LAUNCH_SYSTEMS_OFF:
					SetShooterPWM(0);
				case MOTOR_STARTING:
					//Scale this later
					SetShooterPWM(100);
					break;
				case MOTOR_READY:
					ES_Timer_InitTimer(LOADER_READY_TIMER, LOADER_TIME);
					CurrentState = LOADER_FORWARD;
					ServoForward();
					break;
				case LOADER_FORWARD:
					break;
				case LOADER_BACK:
					break;
				default:
					break;
			}
	
	//Key Control
	while(true){		
		printf("Waiting for input...\n\r");
		// Use keyboard input to control motor
		char input = getchar();
		switch (input) {
			case 'w':
				ServoForward();
				printf("ServoForward\n\r");
				break;
			case 's':
				ServoReverse();
				printf("ServoReverse\n\r");
				break;
			case 'a':
				SetShooterPWM(pwm);
				printf("Set Shooter PWM = %d\n\r", pwm);
				pwm = pwm + 10;
				if(pwm > 100) pwm = 0;
				break;
		}
	}
}*/
#endif

/*------------------------------ End of file ------------------------------*/
