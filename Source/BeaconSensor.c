//#define TEST

// C Libraries
#include <stdio.h>
#include <stdbool.h>

#include <stdint.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "bitdefs.h"

#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "bitdefs.h"
#include "Display.h"

#include "BeaconSensor.h"

// 40,000 ticks per mS assumes a 40Mhz clock
#define TicksPerMS 40000
#define PERIOD_ARRAY_SIZE 8
#define BEACON_PERIOD_THRESHOLD_LOW 1300//Corresponds to 1500 Hz
#define BEACON_PERIOD_THRESHOLD_HIGH 1800 //Corresponds to 1000 Hz

//Numbers for previous (obsolete) variations
//#define BEACON_PERIOD_THRESHOLD_LOW 2800 //Corresponds to 1500 Hz
//#define BEACON_PERIOD_THRESHOLD_HIGH 3200 //Corresponds to 1000 Hz
//#define BEACON_PERIOD_THRESHOLD_LOW 550 //Corresponds to 1500 Hz


static uint32_t Period;
static uint32_t LastCapture;
static uint32_t periodArray[PERIOD_ARRAY_SIZE];
int arrayCounter = 0;

// we will use Timer A in Wide Timer 0 to capture the beacon sensing input.
// This code is templated from Lab 7, so Periodic Timer B has been commented out, but can be activated if necessary.

void InitBeaconSensingCapture( void ){

  // start by enabling the clock to the timer (Wide Timer 5)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R5;
	// enable the clock to Port D
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
	// since we added this Port D clock init, we can immediately start
  // into configuring the timer, no need for further delay
  // make sure that timer (Timer A) is disabled before configuring
  HWREG(WTIMER5_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
	// make sure that timer (Timer B) is disabled before configuring
  //HWREG(WTIMER0_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TBEN;
// set it up in 32bit wide (individual, not concatenated) mode
// the constant name derives from the 16/32 bit timer, but this is a 32/64
// bit timer so we are setting the 32bit mode
  HWREG(WTIMER5_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
// we want to use the full 32 bit count, so initialize the Interval Load
// register to 0xffff.ffff (its default value :-)
  HWREG(WTIMER5_BASE+TIMER_O_TAILR) = 0xffffffff;
// we don't want any prescaler (it is unnecessary with a 32 bit count)
//  HWREG(WTIMER0_BASE+TIMER_O_TAPR) = 0;
// set up timer A in capture mode (TAMR=3, TAAMS = 0), 
// for edge time (TACMR = 1) and up-counting (TACDIR = 1)
  HWREG(WTIMER5_BASE+TIMER_O_TAMR) = 
      (HWREG(WTIMER5_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAAMS) | 
        (TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);
// set up timer B in periodic mode (TBMR=2, TBAMS = 0), 
// for edge time (TBCMR = 1) and up-counting (TBCDIR = 1)
 // HWREG(WTIMER0_BASE+TIMER_O_TBMR) = 
      //(HWREG(WTIMER0_BASE+TIMER_O_TBMR) & ~TIMER_TBMR_TBMR_M) | TIMER_TBMR_TBMR_PERIOD;
//set timeout to 20 mS
	//HWREG(WTIMER0_BASE + TIMER_O_TBILR) = TicksPerMS * 20;
// To set the event to rising edge, we need to modify the TAEVENT bits 
// in GPTMCTL. Rising edge = 00, so we clear the TAEVENT bits
  HWREG(WTIMER5_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEVENT_M;
// Now Set up the port to do the capture (clock was enabled earlier)
// start by setting the alternate function for Port D bit 6 (WT5CCP0)
	HWREG(GPIO_PORTD_BASE+GPIO_O_AFSEL) |= BIT6HI;
// Then, map bit 6's alternate function to WT5CCP0
// 7 is the mux value to select WT0CCP0, 16 to shift it over to the
// right nibble for bit 4 (4 bits/nibble * 4 bits)
// So for PD6, we use << 24 (4 bits/nibble * 6 bits)
	HWREG(GPIO_PORTD_BASE+GPIO_O_PCTL) = 
    (HWREG(GPIO_PORTD_BASE+GPIO_O_PCTL) & 0xfff0ffff) + (7<<24);
// Enable pin on Port D for digital I/O
	HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= BIT6HI;
// make pin 6 on Port D into an input
	HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) &= BIT6LO;
// back to the timer to enable a local capture interrupt
  HWREG(WTIMER5_BASE+TIMER_O_IMR) |= TIMER_IMR_CAEIM;
//enable a local timeout interrupt
//	HWREG(WTIMER5_BASE + TIMER_O_IMR) |= TIMER_IMR_TBTOIM;
// enable the Timer A in Wide Timer 5 interrupt in the NVIC
// it is interrupt number 104 so appears in EN3 at bit 8
  HWREG(NVIC_EN3) |= BIT8HI;
	// enable the Timer B in Wide Timer 0 interrupt in the NVIC
// it is interrupt number 95 so appears in EN2 at bit 31
 // HWREG(NVIC_EN2) |= BIT31HI;
	//Change priority of Timer B (Interrupt #95) to 2
	//HWREG(NVIC_PRI23) |= ((HWREG(NVIC_PRI23) & ~NVIC_PRI23_INTD_M) | 2<<5);
// make sure interrupts are enabled globally
  __enable_irq();
// now kick timer a off by enabling it and enabling the timer to
// stall while stopped by the debugger
  HWREG(WTIMER5_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
	// now kick timer b off by enabling it and enabling the timer to
// stall while stopped by the debugger
  //HWREG(WTIMER0_BASE+TIMER_O_CTL) |= (TIMER_CTL_TBEN | TIMER_CTL_TBSTALL);
	Period = 0;
}

void BeaconSensedCaptureResponse(void ){
	//printf("INTERRUPT!");
  uint32_t ThisCapture;
// start by clearing the source of the interrupt, the input capture event
    HWREG(WTIMER5_BASE+TIMER_O_ICR) = TIMER_ICR_CAECINT;
// now grab the captured value and calculate the period
    ThisCapture = HWREG(WTIMER5_BASE+TIMER_O_TAR);
    Period = ThisCapture - LastCapture;
		Period = Period << 1;
    
// update LastCapture to prepare for the next edge  
    LastCapture = ThisCapture;

    periodArray[arrayCounter] = Period;
    arrayCounter++;
    if(arrayCounter > PERIOD_ARRAY_SIZE - 1) {
    	arrayCounter = 0;
    }
		
		uint32_t average = GetAveragePerioduS();
		//printf("Period is: %d \n\r", GetPerioduS());
		//printf("Average period: %duS   ", average);
		if(average > BEACON_PERIOD_THRESHOLD_LOW && average < BEACON_PERIOD_THRESHOLD_HIGH){
			//printf("Beacon is ON\n\r");
			//POST TO SM
		}
  //printf("Beacon is OFF\n\r");
}

uint32_t GetPeriod( void ){
  return Period;
}

uint32_t GetPerioduS(void){
	return (Period*1000/TicksPerMS);
}

uint32_t GetAveragePerioduS(void){
	uint32_t average = 0;
	for(int i = 0; i < PERIOD_ARRAY_SIZE; i++){
		average += periodArray[i];
	}
	average = average*1000/TicksPerMS/PERIOD_ARRAY_SIZE;
	return average;
}

bool IsBeaconSensed(){
	uint32_t average = GetAveragePerioduS();
	//printf("Period is: %d \n\r", GetPerioduS());
	//printf("Average period: %duS   ", average);
	if(average > BEACON_PERIOD_THRESHOLD_LOW && average < BEACON_PERIOD_THRESHOLD_HIGH){
		//printf("Beacon is ON\n\r");
		for(int i = 0; i < PERIOD_ARRAY_SIZE; i++){
			periodArray[i] = 0;
		}
		return true;
	}
  //printf("Beacon is OFF\n\r");
	return false;
}

#ifdef TEST 
/* Test Harness for the Ball Launcher Module */ 
int main(void) 
{ 
	InitBeaconSensingCapture();
  // Set the clock to run at 40MhZ using the PLL and 16MHz external crystal
  //SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
  //    | SYSCTL_XTAL_16MHZ);
  TERMIO_Init(); 
	//clrScrn();
	printf("In Test Harness for the Beacon Module\n\r");
	_HW_Timer_Init(ES_Timer_RATE_1mS);


	
	while(true){
		IsBeaconSensed();
	}
}
	
	#endif
