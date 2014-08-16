/*! \file threads.c
    \brief This file defines thread implementation using the CMSIS interface
		\details Defines thread defienition, creation and attributes manipulation.
*/

#include "cmsis_os.h" 
#include "kernel.h"
#include <stdlib.h>

//  ==== Thread Management ====

/// Create a Thread Definition with function, priority, and stack requirements.
/// \param         name         name of the thread function.
/// \param         priority     initial priority of the thread function.
/// \param         instances    number of possible thread instances.
/// \param         stacksz      stack size (in bytes) requirements for the thread function.
/// \note CAN BE CHANGED: The parameters to \b osThreadDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osThreadDef(name, priority, instances, stacksz)  \
const osThreadDef_t os_thread_def_##name = \
{ (name), (priority), (instances), (stacksz)  }

osThreadId rtr_q[MAX_THREADS]; ///< Ready to Run Queue
uint32_t rtr_q_h;              ///< Ready to Run Queue Head 
uint32_t rtr_q_idx = 0;        ///< Ready to Run Queue thread counter

osThreadId timed_q[MAX_THREADS]; ///< Waiting Queue for sleeping threads
uint32_t timed_q_h;              ///< Waiting Queue Head 
uint32_t timed_q_idx = 0;        ///< Waiting Queue thread counter

uint32_t osThreadGetNextThread(void);
uint32_t osThreadGetBestThread(void);
void osThreadRemoveThread(osThreadId thread_id);

/// Create a thread and add it to Active Threads and set it to state READY.
/// \param[in]     thread_def    thread definition referenced with \ref osThread.
/// \param[in]     argument      pointer that is passed to the thread function as start argument.
/// \return thread ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osThreadCreate shall be consistent in every CMSIS-RTOS.
osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument)
{
  /// If we are instantiating a thread and there is still room in the thread queue, add the thread to the queue.
	if ( (thread_def->instances > 0) && (thread_def->instances < (MAX_THREADS - rtr_q_idx)))
	{
		rtr_q_idx++;
	}
	else
	{
		return NULL;
	}
	
	rtr_q[rtr_q_idx] = (osThreadId) calloc(1, sizeof(os_thread_cb));
	
	if (rtr_q[rtr_q_idx] == NULL)
	{
		return NULL;
	}
	
	rtr_q[rtr_q_idx]->rtr_q_p  = rtr_q_idx;
	rtr_q[rtr_q_idx]->priority = thread_def->tpriority;
	rtr_q[rtr_q_idx]->status   = TH_READY;
	
	// Kernel did not start yet, so no stack allocated
	rtr_q[rtr_q_idx]->stack_p  = NULL;
	
	if (thread_def->stacksize == 0)
	{
		rtr_q[rtr_q_idx]->stack_size = DEFAULT_STACK_SIZE;
	}
	else
	{
		rtr_q[rtr_q_idx]->stack_size = thread_def->stacksize;
	}
	
	rtr_q[rtr_q_idx]->semaphore_id = NULL;
	rtr_q[rtr_q_idx]->semaphore_p  = MAX_THREADS_SEM;
	
	rtr_q[rtr_q_idx]->timed_q_p   = NULL;
	rtr_q[rtr_q_idx]->timed_ret   = osOK;
	rtr_q[rtr_q_idx]->time_count  = 0;    /// Ready-to-Run
	
	rtr_q[rtr_q_idx]->start_p = thread_def->pthread;
  
	// Update RTR head based on priority
	// The kernel should not be running at this point
	rtr_q_h = osThreadGetBestThread();
	
	return rtr_q[rtr_q_idx];
}
	

/// Return the thread ID of the current running thread.
/// \return thread ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osThreadGetId shall be consistent in every CMSIS-RTOS.
osThreadId osThreadGetId (void)
{
  return rtr_q[rtr_q_h];
}

/// Terminate execution of a thread and remove it from Active Threads.
/// \param[in]     thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadTerminate shall be consistent in every CMSIS-RTOS.
osStatus osThreadTerminate (osThreadId thread_id)
{
	if (thread_id == NULL)
	{		
		return osErrorValue;
  }	
	// if terminating current task, let's get the next best thing	
	if (rtr_q[rtr_q_h] == thread_id)
	{
		// remove thread from various queues
		osThreadRemoveThread(thread_id);		
		// search for next thread to run
		rtr_q_h = osThreadGetBestThread();	
	}
	else
	{
		// remove thread from various queues
		osThreadRemoveThread(thread_id);
	}
	
	free(thread_id);
	return osOK;
}

/// Pass control to next thread that is in state \b READY.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadYield shall be consistent in every CMSIS-RTOS.
osStatus osThreadYield (void)
{
	uint32_t next;
	// search for next thread to run
	next = osThreadGetNextThread();

	if (next != rtr_q_h)
	{
		// leave the current head in the ready to run queue, but make the next best thing active
		rtr_q[rtr_q_h]->status = TH_READY;
		rtr_q_h = next;
		rtr_q[rtr_q_h]->status = TH_RUNNING;	
		
		//need a context switch
		osKernelAskForContextSwitch();
	}
	
	// otherwise there are no other threads to run, to staying with the current one
	
	return osOK;
}


/// Change priority of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \param[in]     priority      new priority value for the thread function.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadSetPriority shall be consistent in every CMSIS-RTOS.
osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority)
{
	if (thread_id == NULL)
	{		
		return osErrorValue;
  }
  thread_id->priority = priority;
	return osOK;
}


/// Get current priority of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \return current priority value of the thread function.
/// \note MUST REMAIN UNCHANGED: \b osThreadGetPriority shall be consistent in every CMSIS-RTOS.
osPriority osThreadGetPriority (osThreadId thread_id)
{
	if (thread_id == NULL)
	{		
		return osPriorityError;
  }
  
	return thread_id->priority;
}

/// \fn uint32_t nextThread()
/// \brief Get thread with next highest priority.
/// \return Thread ID of the next thread to run
uint32_t osThreadGetNextThread(void)
{
	uint32_t i, temp = rtr_q_h;
	osPriority priority = osPriorityIdle;
	
	// search for any thread that is ready to go except head
	for ( i = 0; i < rtr_q_idx + 1 ; i++ )
	{
		if ( i != rtr_q_h )
		{
			if ( rtr_q[i]->priority > priority )
			{
				priority = rtr_q[i]->priority;
				temp = i;
			}
		}
	}
	// if something is found, it will be returned, otherwise, curent head is going back
	return temp;
}

/// \fn uint32_t bestThread()
/// \brief Get thread with highest priority.
/// \return Thread ID of the best thread to run
uint32_t osThreadGetBestThread(void)
{
	uint32_t i, temp = rtr_q_h;
	osPriority priority = osPriorityIdle;
	
	// search for any thread that is ready to go except head
	for ( i = 0; i < rtr_q_idx + 1 ; i++ )
	{
		if ( rtr_q[i]->priority > priority )
		{
			priority = rtr_q[i]->priority;
			temp = i;
		}
	}
	// if something is found, it will be returned, otherwise, curent head is going back
	return temp;
}

/// \fn void removeThread(uint32_t thread_idx)
/// \brief Remove thread from all the lists.
/// \param thread_idx  Thread ID of the thread to remove
void osThreadRemoveThread(osThreadId thread_id)
{
	uint32_t i, idx;
	
	// remove from RTR and update the queue
	for ( i = 0; i < rtr_q_idx + 1 ; i++ )
	{
		if (rtr_q[i] == thread_id )
		{
			rtr_q[i] = NULL;
			idx = i;
		}
	}
	
	for ( i = idx; i < (rtr_q_idx + 1) - 1 ; i++ )
	{
			rtr_q[i] = rtr_q[i+1];
		  rtr_q[i]->rtr_q_p = i; 	
	}
	
	rtr_q_idx--;
	
	// remove from timed and update the queue
	for ( i = 0; i < timed_q_idx + 1 ; i++ )
	{
		if (timed_q[i] == thread_id )
		{
			timed_q[i] = NULL;
			idx = i;
		}
	}
	
	for ( i = idx; i < (timed_q_idx + 1) - 1 ; i++ )
	{
			timed_q[i] = timed_q[i+1];
		  timed_q[i]->timed_q_p = i; 	
	}
	
	timed_q_idx--;	
	
	// remove from any semaphore blocked queue and update the queue
	osSemaphoreRemoveThread(thread_id);
	
}

