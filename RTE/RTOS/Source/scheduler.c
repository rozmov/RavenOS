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
#include "threadIdle.h"

extern uint32_t  curr_task;     ///< Current task
extern uint32_t  next_task;     ///< Next task

uint32_t os_ThreadGetBestThread(void);
void os_ReevaluateBlockedThread(void);
osStatus os_ReevaluateThread(osThreadId thread_id);

/*! 
    \brief Prepares the next task to be run and sets \ref next_task.
*/
void scheduler(void)
{
	uint32_t next;
	
	if (th_q_cnt == 0)
	{
		stop_cpu;
	}	
	
	// re-evaluate blocked threads
	os_ReevaluateBlockedThread();
	
	// re-evaluate sleeping threads	
	
	// search for next thread to run
	next = os_ThreadGetBestThread();

	if (next != th_q_h)
	{
		// leave the current head in the ready to run queue, but make the next best thing active
		if ( th_q[th_q_h]->status == TH_RUNNING )
		{
			th_q[th_q_h]->status = TH_READY;
		}	
		th_q_h = next;
		th_q[th_q_h]->status = TH_RUNNING;	
		
		//need a context switch - let the kernel handle this	
	}	
	
	next_task = next;
	return;	
}


/// \brief Get ready/running thread with highest priority.
/// \return Thread ID of the best thread to run
uint32_t os_ThreadGetBestThread(void)
{
	uint32_t i, temp = th_q_h;
	osPriority priority = osPriorityIdle;
	
	// search for any thread that is ready to go
	for ( i = 0; i < th_q_cnt ; i++ )
	{
		if (th_q[i]->status == TH_READY || th_q[i]->status == TH_RUNNING)
		{
			if ( th_q[i]->priority > priority )
			{
				priority = th_q[i]->priority;
				temp = i;
			}			
		}
	}
	
	// check that there is a runnable thread up, otherwise scheduling the Idle thread
	if (priority == osPriorityIdle)
	{
		if (temp != tid_threadIdle->th_q_p )
		{
			temp = tid_threadIdle->th_q_p; 
		}
	}
	
	// if something is found, it will be returned, otherwise, curent thread is going back
	return temp;
}

/// \brief Re-evaluate all blocked threads 
/// \details If a thread expires on a semaphore, but the semaphore is still taken, 
///          take it out of the blocked state and into the ready to run queue
void os_ReevaluateBlockedThread(void)
{
	uint32_t i;
	
	for ( i = 0; i < th_q_cnt ; i++ )
	{
		if (th_q[i]->status != TH_BLOCKED)
		{
			continue;
		}
		if (os_ReevaluateThread(th_q[i]) != osOK)
		{
			continue;
		}			
	}	
	return;
}

/// \brief Re-evaluate all blocked threads 
/// \details If a thread expires on a semaphore, but the semaphore is still taken, 
///          take it out of the blocked state and into the ready to run queue
osStatus os_ReevaluateThread(osThreadId thread_id)
{
	uint32_t semaphore_p;
	osSemaphoreId semaphore_id;

	if ( thread_id == NULL)
	{
		return osErrorParameter;
	}	
	if ( thread_id->semaphore_id == NULL)
	{
		return osErrorParameter;
	}
	
	if (thread_id->semaphore_p == MAX_THREADS_SEM)
	{
		return osErrorParameter;				
	}
		
	semaphore_id = thread_id->semaphore_id;
	semaphore_p  = thread_id->semaphore_p;
				
	//check if the semaphore is free
	if (semaphore_id->threads_own_q_cnt < semaphore_id->ownCount )
	{
		// give the thread a chance to run 
		thread_id->status = TH_READY;
		return osOK;
	}
	
	// if it can wait forever, continue with the next thread
	if (osWaitForever == semaphore_id->threads_q[semaphore_p].expiryTime &&
			osWaitForever == semaphore_id->threads_q[semaphore_p].ticks)
	{
		return osOK;
	}
	else // check if time expired
	if (osKernelSysTick() == semaphore_id->threads_q[semaphore_p].expiryTime &&
			0 == semaphore_id->threads_q[semaphore_p].ticks)
	{			
				// unblock thread 
				thread_id->status = TH_READY;	
				return osOK;					
	}		
	else // time did not expire yet, reduce the number of ticks
	{
		if (0 < semaphore_id->threads_q[semaphore_p].ticks)
		{
			semaphore_id->threads_q[semaphore_p].ticks--;
		}
		return osOK;
	}
}
