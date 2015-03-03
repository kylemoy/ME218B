/****************************************************************************
Module: KartSwitchAndLED.h
Description:
	This module initializes the Kart Switch and LED hardware, and it contains
	functions to determine the Kart Select (Kart1, Kart2, or Kart3) and to turn
	on/off an LED to indicate that a race is in progress.
Author: Kyle Moy, 3/1/15
****************************************************************************/

#ifndef KartSwitchAndLED_H
#define KartSwitchAndLED_H

/*----------------------- Public Function Prototypes ----------------------*/
void InitializeKartSwitchAndLED(void);
uint8_t ReadKartSwitch(void);
void TurnOnRaceLED(void);
void TurnOffRaceLED(void);

#endif /* KartSwitchAndLED_H */


