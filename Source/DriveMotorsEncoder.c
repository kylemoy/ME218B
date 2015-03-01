
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "bitdefs.h"

// Framework Libraries
#include "ES_Configure.h"
#include "ES_Framework.h"

#include "DriveMotorEncoder.h"
#include "SM_Master.h"

// 40,000 ticks per mS assumes a 40Mhz clock
#define TicksPerMS 40000
#define TicksPerMin 2400000000
#define PulsesPerRev 28

//#define TEST

static uint32_t PeriodR;
static uint32_t PeriodL;
static uint32_t LastCaptureR;
static uint32_t LastCaptureL;
static uint32_t RPMR;
static uint32_t RPML;

static uint32_t TickCountR = 0;
static uint32_t TickCountL = 0;
static uint32_t TickCountAverage = 0;
static uint32_t TargetTickCount;

// we will use Timer A in Wide Timer 0 to capture the input
void InitInputCapturePeriod( void ){
  // start by enabling the clock to the timer (Wide Timer 0)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R0;
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R1;
	// enable the clock to Port C
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
	// since we added this Port C clock init, we can immediately start
  // into configuring the timer, no need for further delay
  
  // make sure that timer (Timer A) is disabled before configuring
  HWREG(WTIMER0_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TBEN;
	HWREG(WTIMER1_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
  
// set it up in 32bit wide (individual, not concatenated) mode
// the constant name derives from the 16/32 bit timer, but this is a 32/64
// bit timer so we are setting the 32bit mode
  HWREG(WTIMER0_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
	HWREG(WTIMER1_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;

// we want to use the full 32 bit count, so initialize the Interval Load
// register to 0xffff.ffff (its default value :-)
  HWREG(WTIMER0_BASE+TIMER_O_TBILR) = 0xffffffff;
	HWREG(WTIMER1_BASE+TIMER_O_TBILR) = 0xffffffff;

// we don't want any prescaler (it is unnecessary with a 32 bit count)
//  HWREG(WTIMER0_BASE+TIMER_O_TAPR) = 0;

// set up timer A in capture mode (TAMR=3, TAAMS = 0), 
// for edge time (TACMR = 1) and up-counting (TACDIR = 1)
  HWREG(WTIMER0_BASE+TIMER_O_TBMR) = 
      (HWREG(WTIMER0_BASE+TIMER_O_TBMR) & ~TIMER_TBMR_TBAMS) | 
        (TIMER_TBMR_TBCDIR | TIMER_TBMR_TBCMR | TIMER_TBMR_TBMR_CAP);
	HWREG(WTIMER1_BASE+TIMER_O_TAMR) = 
      (HWREG(WTIMER1_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAAMS) | 
        (TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);

// To set the event to rising edge, we need to modify the TAEVENT bits 
// in GPTMCTL. Rising edge = 00, so we clear the TAEVENT bits
  HWREG(WTIMER0_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TBEVENT_M;
	HWREG(WTIMER1_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEVENT_M;

// Now Set up the port to do the capture (clock was enabled earlier)
// start by setting the alternate function for Port C bit 4 (WT0CCP0)
	HWREG(GPIO_PORTC_BASE+GPIO_O_AFSEL) |= (BIT5HI | BIT6HI);

// Then, map bit 4's alternate function to WT0CCP0
// 7 is the mux value to select WT0CCP0, 16 to shift it over to the
// right nibble for bit 4 (4 bits/nibble * 4 bits)

// bit 5? shift 20?
	HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) = 
    (HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) & 0xfff0ffff) + (7<<20);
	HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) = 
    (HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) & 0xfff0ffff) + (7<<24);

// Enable pin on Port C for digital I/O
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= (BIT5HI | BIT6HI);
	
// make pin 4 on Port C into an input
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) &= (BIT5LO & BIT6LO);

// back to the timer to enable a local capture interrupt
  HWREG(WTIMER0_BASE+TIMER_O_IMR) |= TIMER_IMR_CBEIM;
	HWREG(WTIMER1_BASE+TIMER_O_IMR) |= TIMER_IMR_CAEIM;

// enable the Timer A in Wide Timer 0 interrupt in the NVIC
// it is interrupt number 94 so appears in EN2 at bit 30
  HWREG(NVIC_EN2) |= BIT31HI;
	HWREG(NVIC_EN3) |= BIT0HI;

// make sure interrupts are enabled globally
  __enable_irq();

// now kick the timer off by enabling it and enabling the timer to
// stall while stopped by the debugger
  HWREG(WTIMER0_BASE+TIMER_O_CTL) |= (TIMER_CTL_TBEN | TIMER_CTL_TBSTALL);
	HWREG(WTIMER1_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
}

void RDriveCaptureResponse( void ){
  uint32_t ThisCaptureR;
	//start by clearing the source of the interrupt, the input capture event
  HWREG(WTIMER0_BASE+TIMER_O_ICR) = TIMER_ICR_CBECINT;

	//now grab the captured value and calculate the period
  ThisCaptureR = HWREG(WTIMER0_BASE+TIMER_O_TBR);
	PeriodR = ThisCaptureR - LastCaptureR;
	RPMR = TicksPerMin / (PeriodR*PulsesPerRev);
			    
	//update LastCapture to prepare for the next edge  
  LastCaptureR = ThisCaptureR;
	
	// Update the tick count
	TickCountR++;
	TickCountAverage = (TickCountR + TickCountL) / 2;
	printf("TickCountAverage = %d\r\n", TickCountAverage);
	if (TickCountAverage > TargetTickCount) {
		ES_Event Event = {E_MOTOR_TICK_TIMEOUT, 0};
		PostMasterSM(Event);
	}
}

void LDriveCaptureResponse( void ){
  uint32_t ThisCaptureL;
  
	//start by clearing the source of the interrupt, the input capture event
  HWREG(WTIMER1_BASE+TIMER_O_ICR) = TIMER_ICR_CAECINT;
		
	//now grab the captured value and calculate the period
  ThisCaptureL = HWREG(WTIMER1_BASE+TIMER_O_TAR);
	PeriodL = ThisCaptureL - LastCaptureL;
	RPML = TicksPerMin / (PeriodL*PulsesPerRev);
	
	//update LastCapture to prepare for the next edge  
  LastCaptureL = ThisCaptureL;
	
	// Update the tick count
	TickCountL++;
	TickCountAverage = (TickCountR + TickCountL) / 2;
	printf("TickCountAverage = %d\r\n", TickCountAverage);
	if (TickCountAverage > TargetTickCount) {
		ES_Event Event = {E_MOTOR_TICK_TIMEOUT, 0};
		PostMasterSM(Event);
	}
	
}

uint32_t GetPeriodR( void ){
  return PeriodR;
}

uint32_t GetPeriodL( void ){
  return PeriodL;
}

uint32_t GetRPMR(void){
	if ((HWREG(WTIMER0_BASE+TIMER_O_TBV) - LastCaptureR) > 6000000){ 
		return 0;
	}
	return RPMR;
}

uint32_t GetRPML(void){	
	if ((HWREG(WTIMER1_BASE+TIMER_O_TAV) - LastCaptureL) > 6000000){ 
		return 0;
	}
	return RPML;
}

void SetTargetTickCount(uint32_t NumberOfTicks) {
	TickCountR = 0;
	TickCountL = 0;
	TickCountAverage = 0;
	TargetTickCount = NumberOfTicks;
}

/*------------------------------- Footnotes -------------------------------*/
#ifdef TEST
#include "termio.h"
int main(void){
	TERMIO_Init();
	InitInputCapturePeriod();
	while (1){
		printf("\rRPM: %d\r\n", RPMR);
	}
}
#endif
/*------------------------------ End of file ------------------------------*/
