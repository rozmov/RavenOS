#ifndef _KERNEL_H
#define _KERNEL_H

#include <stdint.h>
#include "threads.h"
#include "cmsis_os.h" 

extern osThreadId rtr_q[MAX_THREADS];
extern uint32_t  PSP_array[MAX_THREADS];

void osKernelAskForContextSwitch (void);

#endif

