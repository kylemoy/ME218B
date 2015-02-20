/****************************************************************************
Module: EventCheckers.c
Description: Event checker functions used in the Events and Services framework.
Author: Kyle Moy, 2/18/15
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Events.h"
#include "ES_PostList.h"
#include "ES_ServiceHeaders.h"
#include "ES_Port.h"
#include "EventCheckers.h"


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
Function:			Check4Keystroke
Parameters:		void
Returns:			bool, true if a new key was detected & posted
Description:	Checks to see if a new key from the keyboard is detected,
							and, if so, retrieves the key and posts an ES_NewKey event.
Notes:
   The functions that actually check the serial hardware for characters
   and retrieve them are assumed to be in ES_Port.c
   Since we always retrieve the keystroke when we detect it, thus clearing the
   hardware flag that indicates that a new key is ready this event checker 
   will only generate events on the arrival of new characters, even though we
   do not internally keep track of the last keystroke that we retrieved.
****************************************************************************/
bool Check4Keystroke(void) {
  if (IsNewKeyReady()) // New key waiting?
  {
    ES_Event ThisEvent;
    ThisEvent.EventType = ES_NEW_KEY;
    ThisEvent.EventParam = GetNewKey();
    PostMapKeys( ThisEvent );
    return true;
  }
  return false;
}
