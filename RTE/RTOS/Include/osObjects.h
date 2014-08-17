/*! \file osObjects.h
    \brief This header file defines all objects when included in a C/C++ source file
		\details Defines the stop_cpu as a breakpoint, threads, semaphores.
**/

#ifndef _OSOBJECTS_H
#define _OSOBJECTS_H

#include "cmsis_os.h"                                   // CMSIS RTOS header file

/*! \def stop_cpu2
     Use Breakpoint to stop when error is detected (KEIL MDK specific intrinsic) */
/*! \def stop_cpu
     Stop when error is detected - while(1) version */
#define stop_cpu   while(1) {	}
#define stop_cpu2 __breakpoint(0)

/// Thread definition
extern osThreadId tid_thread0;
extern void thread0 (void const *argument);             // function prototype
int Init_thread0 (void);

extern osThreadId tid_thread1;
extern void thread1 (void const *argument);             // function prototype
int Init_thread1 (void);

extern osThreadId tid_thread2;
extern void thread2 (void const *argument);             // function prototype
int Init_thread2 (void);

extern osThreadId tid_thread3;
extern void thread3 (void const *argument);             // function prototype
int Init_thread3 (void);

/// Semaphore definition
extern void sem0 (void const *argument);             // function prototype
int Init_Semaphore0 (void);

#endif // _OSOBJECTS_H
