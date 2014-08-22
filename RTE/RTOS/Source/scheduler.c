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

uint32_t os_ThreadGetNextThread(void);
uint32_t os_ThreadGetBestThread(void);
void os_ReevaluateBlockedThread(void);

/*! \fn void scheduler(void)
    \brief Prepares the next task to be run and sets \red next_task.
*/
void scheduler(void)
{
	uint32_t next;
	
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
	
	if (th_q_cnt == 0)
	{
		next = 0;
		while(1)
		{		
				// Should not be here
		};
	}
	
	next_task = next;
	return;	
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
	
	// if something is found, it will be returned, otherwise, curent head is going back
	return temp;
}

/// \fn void os_ReevaluateBlockedThread(void)
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

/// \fn void os_ReevaluateBlockedThread(void)
/// \brief Re-evaluate all blocked threads 
/// \details If a thread expires on a semaphore, but the semaphore is still taken, 
///          take it out of the blocked state and into the ready to run queue
osStatus os_ReevaluateThread(osThreadId thread_id)
{
	uint32_t semaphore_p;
	osSemaphoreId semaphore_id;
	//sanity
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
		if (semaphore_id->thread_id == NULL)
		{
			// give the thread a chance to run 
			thread_id->status = TH_READY;
			return osOK;
		}
		
		// if it can wait forever, continue with the next thread
		if (osWaitForever == semaphore_id->threads_q[semaphore_p].expiryTime)
		{
			return osOK;
		}
		else // check if time expired
		if (osKernelSysTick() == semaphore_id->threads_q[semaphore_p].expiryTime)
		{			
			if ( os_RemoveThreadFromSemaphore(thread_id, semaphore_id) != osOK)
			{
				return osErrorTimeoutResource;
			}
			
			if ( os_SearchThreadAllSemaphores(thread_id, &semaphore_id, &semaphore_p) == osOK)
			{
				if (semaphore_p == MAX_THREADS_SEM)
				{
					// unblock thread 
					thread_id->status = TH_READY;	
				}
				else
				{
					thread_id->semaphore_id = semaphore_id;
					thread_id->semaphore_p  = semaphore_p;
				}					
			}
			else
			{
				return osOK;
			}
		
			return osOK;				
		}		
}
