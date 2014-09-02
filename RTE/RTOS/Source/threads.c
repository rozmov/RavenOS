/*! \file threads.c
    \brief This file defines thread implementation using the CMSIS interface
		\details Thread definition, creation and attributes manipulation.
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

osThreadId th_q[MAX_THREADS]; ///< Ready to Run Queue (thread queue)
uint32_t th_q_h = 0;          ///< Ready to Run Queue Head 
uint32_t th_q_cnt = 0;        ///< Ready to Run Queue thread counter

osThreadId timed_q[MAX_THREADS]; ///< Waiting Queue for sleeping threads
uint32_t timed_q_h;              ///< Waiting Queue Head 
uint32_t timed_q_cnt = 0;        ///< Waiting Queue thread counter

void os_ThreadRemoveThread(osThreadId thread_id);

/// Create a thread and add it to Active Threads and set it to state READY.
/// \param[in]     thread_def    thread definition referenced with \ref osThread.
/// \param[in]     argument      pointer that is passed to the thread function as start argument.
/// \return thread ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osThreadCreate shall be consistent in every CMSIS-RTOS.
osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument)
{
	uint32_t th, instances, dead = MAX_THREADS;
	
	// the definition does not exist, nothing to feed from, so exiting
	if ( thread_def == NULL )
	{
		return NULL;
	}	
	
	// check multiple instances of the thread function already present 
	// and check the maximum presented has been met
	if ( thread_def->instances <= 0) 
	{	
		return NULL;
	}	
	
	instances = 0;
	for (th = 0; th < th_q_cnt; th++)
	{
		if (th_q[th] != NULL)
		{
			if (th_q[th]->start_p == thread_def->pthread)
			{
				instances++;
			
				// Check if this is a previously terminated thread
				if (th_q[th]->status == TH_DEAD)
				{
					dead = th;
					break;
				}
			}
		}
	}
	
	if (dead == MAX_THREADS)
	{
		if (instances >= thread_def->instances)
		{
			// thread function instance maximum already met, cannot create another one
			return NULL;
		}
	}
	// check that stack size requested can be supported
	if ( thread_def->stacksize > DEFAULT_STACK_SIZE )
	{
		return NULL;
	}	
	
	// check that the priority is whithin limits
	if ( (thread_def->tpriority < osPriorityIdle) || (thread_def->tpriority > osPriorityRealtime) )
	{
		return NULL;
	}
	
	if (dead == MAX_THREADS)
	{
		// If there is still room in the thread queue, add the thread to the queue.
		if ( 0 < (MAX_THREADS - th_q_cnt))
		{
			// Instantiate only one thread, since we can only return one thread id...
			// If the user would like to instantiate another thread with the same name, 
			// the user would need to call create again with a decrement in the number of instances.
			th = th_q_cnt;
			
			// check if the thread was already instantiated previously (should not happen)
			if (th_q[th] != NULL)
			{
				return NULL;
			}
			
			th_q_cnt++;
			// could also consider re-using dead threads in order to avoid burning out threads
		}
		else
		{
			return NULL;
		}
		
		// allocate the TCB for this newly created thread
		th_q[th] = (osThreadId) calloc(1, sizeof(os_thread_cb));
		// no more heap memory available, so do not create thread
		if (th_q[th] == NULL)
		{
			th_q_cnt--;
			return NULL;
		}
	}
	else
	{
		th = dead;
	}
	
	th_q[th]->th_q_p   = th;
	th_q[th]->priority = thread_def->tpriority;
	th_q[th]->status   = TH_READY;
	
	th_q[th]->semaphore_id = NULL;
	th_q[th]->semaphore_p  = MAX_THREADS_SEM;
	
	th_q[th]->timed_q_p   = NULL;
	th_q[th]->timed_ret   = osOK;
	th_q[th]->time_count  = 0;    /// Ready-to-Run	
	
	if (thread_def->stacksize == 0)
	{
		th_q[th]->stack_size = DEFAULT_STACK_SIZE;
	}
	else
	{
		th_q[th]->stack_size = thread_def->stacksize;
	}
	
	th_q[th]->start_p = thread_def->pthread;
	
	// Whether already running or not, the kernel should be able to allocate stack space to the thread
	// Need the stack_size and the start_p already initialized in order to allocate the stack for the thread
	os_KernelStackAlloc(th);	

	return th_q[th];
}
	

/// Return the thread ID of the current running thread.
/// \return thread ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osThreadGetId shall be consistent in every CMSIS-RTOS.
osThreadId osThreadGetId (void)
{
  return th_q[th_q_h];
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
	// if terminating current task, invoke the scheduler to setup another thread
	if (th_q[th_q_h] == thread_id)
	{
		// remove thread from various queues
		os_ThreadRemoveThread(thread_id);		
	
		// yield execution
		os_KernelInvokeScheduler ();
	}
	else
	{
		// remove thread from various queues
		os_ThreadRemoveThread(thread_id);
	}
	
	return osOK;
}

/// Pass control to next thread that is in state \b READY.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadYield shall be consistent in every CMSIS-RTOS.
osStatus osThreadYield (void)
{
	//invoke scheduler
	os_KernelInvokeScheduler ();
		
	// back
	
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


/// Remove thread from all the lists.
/// \param thread_id  Thread ID of the thread to remove
void os_ThreadRemoveThread(osThreadId thread_id)
{
	uint32_t i, idx;

	// remove from any semaphore queues
	os_SemaphoreRemoveThread(thread_id);
	
	// remove from timed queue and update the queue
	if (timed_q_cnt != 0)
	{
		for ( i = 0; i < timed_q_cnt ; i++ )
		{
			if (timed_q[i] == thread_id )
			{
				timed_q[i] = NULL;
				idx = i;
			}
		}
		
		for ( i = idx; i < timed_q_cnt - 1 ; i++ )
		{
				timed_q[i] = timed_q[i+1];
				timed_q[i]->timed_q_p = i; 	
		}
		timed_q[timed_q_cnt - 1] = NULL;
		timed_q_cnt--;	
	}	
	
	// set the thread in dead state
	thread_id->status = TH_DEAD;
	// stack size already allocated, so just park the thread in TH_DEAD state
	// this particular thread cannot be 'revived', it will be dead until the end of the program/forever
	return;
}

