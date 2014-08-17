/*! \file scheduler.c
    \brief This file contains the OS scheduler implementation
		\details The scheduler is invoked:
		           - at every system tick by the \ref SysTick_Handler
							 - at a thread yield
*/

#include "scheduler.h"
#include <stdint.h>
#include "stdio.h"
#include "osObjects.h" 

extern uint32_t  curr_task;     ///< Current task
extern uint32_t  next_task;     ///< Next task

// Thread related information 
extern osThreadId th_q[MAX_THREADS];
extern uint32_t th_q_h;
extern uint32_t th_q_cnt;

uint32_t os_ThreadGetNextThread(void);
uint32_t os_ThreadGetBestThread(void);

/*! \fn void scheduler(void)
    \brief Prepares the next task to be run and sets \red next_task.
*/
void scheduler(void)
{
	uint32_t next;
	
	// Simple task round robin scheduler
  switch(curr_task) {
    case(0): next_task=1; break;
    case(1): next_task=2; break;
		case(2): next_task=3; break;
    case(3): next_task=0; break;
    default: next_task=0;
      printf("ERROR:curr_task = %x\n", curr_task);
      stop_cpu;
      break; // Should not be here
    }
	
	
////	// search for next thread to run
////	next = os_ThreadGetNextThread();
//	// search for next thread to run
//	next = os_ThreadGetBestThread();

//	if (next != th_q_h)
//	{
//		// leave the current head in the ready to run queue, but make the next best thing active
//		th_q[th_q_h]->status = TH_READY;
//		th_q_h = next;
//		th_q[th_q_h]->status = TH_RUNNING;	
//		
//		//need a context switch - let the kernel handle this
//		next_task = next;
//	}	
		
}

/// \fn uint32_t os_ThreadGetNextThread(void)
/// \brief Get thread with next highest priority.
/// \return Thread ID of the next thread to run
uint32_t os_ThreadGetNextThread(void)
{
	uint32_t i, temp = th_q_h;
	osPriority priority = osPriorityIdle;
	
	// search for any thread that is ready to go except head
	for ( i = 0; i < th_q_cnt ; i++ )
	{
		// This is not the currently running process
		// The candidate process is in ready state
		if ( i != th_q_h && th_q[i]->status  )
		{
			if ( th_q[i]->priority > priority )
			{
				priority = th_q[i]->priority;
				temp = i;
			}
		}
	}
	// if something is found, it will be returned, otherwise, curent head is going back
	return temp;
}

/// \fn uint32_t os_ThreadGetBestThread(void)
/// \brief Get thread with highest priority.
/// \return Thread ID of the best thread to run
uint32_t os_ThreadGetBestThread(void)
{
	// need to take into account blocked state and re-evaluate
	// need to re-evaluate sleeping threads
	
	uint32_t i, temp = th_q_h;
	osPriority priority = osPriorityIdle;
	
	// search for any thread that is ready to go except head
	for ( i = 0; i < th_q_cnt ; i++ )
	{
		if ( th_q[i]->priority > priority )
		{
			priority = th_q[i]->priority;
			temp = i;
		}
	}
	// if something is found, it will be returned, otherwise, curent head is going back
	return temp;
}

