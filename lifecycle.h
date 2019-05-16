#ifndef lifecycle_h
#define lifecycle_h

#include "schedule.h"

void init_process();
void init(int, int, int, int);
int interrupt();
Schedule* life_cycle();

#endif