#ifndef TIMERS_H
#define TIMERS_H

#include "Primary.h"

extern unsigned long timer0[NUM_BUTTONS], timer1[NUM_BUTTONS];
extern unsigned long t2;
extern bool timerlock[NUM_BUTTONS];
extern int angle[NUM_SERVOS];
extern int pastangle[NUM_SERVOS];
extern bool gateflag;
extern bool quesend[NUM_BUTTONS];
extern int last_active;

#endif // TIMERS_H
