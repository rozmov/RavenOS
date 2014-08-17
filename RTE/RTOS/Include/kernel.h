/*! \file kernel.h
    \brief This header file defines all kernel related data
		\details Makes available the context switch API for threads.
*/

#ifndef _KERNEL_H
#define _KERNEL_H

#include <stdint.h>
#include "threads.h"
#include "cmsis_os.h" 

void os_KernelInvokeScheduler (void);

#endif

