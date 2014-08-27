/// \file semaphores.c
/// \brief Semaphore implementation according to CMSIS interfaces
/// \details Defines a semaphore and semaphore creation and attributes manipulation

#include "cmsis_os.h" 
#include <stdlib.h>
#include "kernel.h"

//  ==== Semaphore Management Functions ====

/// Define a Semaphore object.
/// \param         name          name of the semaphore object.
/// \note CAN BE CHANGED: The parameter to \b osSemaphoreDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osSemaphoreDef(name)  \
const osSemaphoreDef_t os_semaphore_def_##name = { 0 }

/// Access a Semaphore definition.
/// \param         name          name of the semaphore object.
/// \note CAN BE CHANGED: The parameter to \b osSemaphore shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osSemaphore(name)  \
&os_semaphore_def_##name


osSemaphoreId semaphores[MAX_SEMAPHORES]; ///< Semaphore Queue
uint32_t sem_counter = 0;                 ///< Semaphore Queue counter


// Prototypes
osStatus os_RemoveThreadFromSemaphoreBlockedQ (osThreadId thread_id, osSemaphoreId semaphore_id);
osStatus os_RemoveThreadFromSemaphoreOwnerQ (osThreadId thread_id, osSemaphoreId semaphore_id);
osStatus os_InsertThreadInSemaphoreOwnerQ (osThreadId thread_id, osSemaphoreId semaphore_id);
osStatus os_InsertThreadInSemaphoreBlockedQ (osThreadId thread_id, osSemaphoreId semaphore_id, uint32_t expiryTime, uint32_t ticks);
uint32_t os_SearchThreadInSemaphoreOwnerQ (osThreadId thread_id, osSemaphoreId semaphore_id);
uint32_t os_SearchThreadInSemaphoreBlockedQ (osThreadId thread_id, osSemaphoreId semaphore_id);
osStatus os_SearchThreadAllSemaphoresBlockedQ (osThreadId thread_id, osSemaphoreId* semaphore_id_p, uint32_t* semaphore_p_p );

/// Create and Initialize a Semaphore object used for managing resources.
/// \param[in]     semaphore_def semaphore definition referenced with \ref osSemaphore.
/// \param[in]     count         number of available resources.
/// \return semaphore ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreCreate shall be consistent in every CMSIS-RTOS.
osSemaphoreId osSemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count)
{
	uint32_t j, sem;
	/// If we are instantiating a thread and there is still room in the thread queue, add the thread to the queue.
	if ( 0 < (MAX_SEMAPHORES - sem_counter))
	{
		sem = sem_counter;
		sem_counter++;
	}
	else
	{
		return NULL;
	}
	
	if ( count <=0 )
	{
		return NULL;
	}

	if ( count > MAX_THREADS_SEM )
	{
		return NULL;
	}	
	
	semaphores[sem] = (osSemaphoreId) calloc(1, sizeof(os_semaphore_cb));
	// no more memory available, so do not create semaphore
	if (semaphores[sem] == NULL)
	{
		sem_counter--;
		return NULL;
	}
	
	for ( j = 0; j < MAX_THREADS_SEM ; j++ )
	{
		semaphores[sem]->threads_q[j].threadId = NULL;
		semaphores[sem]->threads_q[j].expiryTime = 0;
		semaphores[sem]->threads_q[j].ticks = 0;
		
		semaphores[sem]->threads_own_q[j] = NULL;
	}
	 
	semaphores[sem]->threads_q_cnt = 0;
	semaphores[sem]->threads_own_q_cnt = 0;
	semaphores[sem]->ownCount = count;
	
	return semaphores[sem];
}


/// Wait until a Semaphore token becomes available.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out.
/// \return number of available tokens, or -1 in case of incorrect parameters.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreWait shall be consistent in every CMSIS-RTOS.
int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec)
{	
	uint64_t microsec = ((uint64_t) millisec) * 1000;
	uint32_t ticks = 0, wait = 0;
	osThreadId curr_th = osThreadGetId();
	osStatus rc;
	
	// semaphore sanity check
	if ( semaphore_id == NULL )
	{
		return (-1);
	}	

	if ( (rc = os_InsertThreadInSemaphoreOwnerQ(curr_th,semaphore_id)) == osOK)
	{
		// semaphore is free -> take semaphore
		return (semaphore_id->ownCount - semaphore_id->threads_own_q_cnt);
	}	
	
	// check for osErrorResource to block thread
	if (rc != osErrorResource)
	{
		return (-1);
	}
	
	
  if ( millisec == 0 )
	{
		// all tokens taken for this semaphore, but can't wait, so return unsuccessful
		return (-1);			
	}
	 
	// thread can wait on semaphore	
	// add thread to the semaphore queue, mark thread as blocked and invoke scheduler  
	
	// enqueue thread to semaphore queue with appropriate time
	if (semaphore_id->threads_q_cnt == MAX_THREADS_SEM)
	{
		return (-1); // no room in queue	
	}
				
	// add thread to blocked queue on semaphore
	if (osWaitForever == millisec)
	{		
		if (os_InsertThreadInSemaphoreBlockedQ(curr_th,semaphore_id, osWaitForever, osWaitForever) != osOK)
		{
			return (-1); // no room in queue or something else went wrong and cannot block on this semaphore
		}			
	}
	else
	{
		ticks = (uint32_t) osKernelSysTickMicroSec(microsec);
		wait = ticks + osKernelSysTick();
		if (os_InsertThreadInSemaphoreBlockedQ(curr_th,semaphore_id, wait, ticks) != osOK)
		{
			return (-1); // no room in queue or something else went wrong and cannot block on this semaphore
		}	
	}			
				
	// while the semaphore is taken, keep on waiting
	// if can no longer wait, return fail
	while (semaphore_id->threads_own_q_cnt == semaphore_id->ownCount &&
		     semaphore_id->threads_q[curr_th->semaphore_p].ticks != 0)
	{
		// set thread to blocked state and yield
		curr_th->status = TH_BLOCKED;
		//invoke scheduler
		os_KernelInvokeScheduler ();						
	}
	
	// semaphore no longer busy or thread wait time expired
	ticks = semaphore_id->threads_q[curr_th->semaphore_p].ticks;
	// unblock thread and move on
	curr_th->status = TH_READY;
	if ((rc = os_RemoveThreadFromSemaphoreBlockedQ(curr_th, semaphore_id)) != osOK)
	{
		return (-1);
	}	
	
	if (ticks == 0)
	{
		return (-1); // timeout...
	}

	if (os_InsertThreadInSemaphoreOwnerQ(curr_th,semaphore_id) == osOK)
	{
		// semaphore is free -> take semaphore
		return (semaphore_id->ownCount - semaphore_id->threads_own_q_cnt);
	}	
	
	return (-1);
				
}


/// Release a Semaphore token.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreRelease shall be consistent in every CMSIS-RTOS.
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id)
{
	uint32_t j, idx, sem_p;
	osPriority maxPriority = osPriorityIdle;
	osThreadId thread_id = osThreadGetId();
	osStatus rc;
	osSemaphoreId sem_id;
	
	if ( semaphore_id == NULL )
	{ // semaphore has not been created yet
		return osErrorParameter;
	}
	if (semaphore_id->threads_own_q_cnt == 0)
	{ // no owners, so nothing to release
		return osOK;
	}	
	
	if ((rc = os_RemoveThreadFromSemaphoreOwnerQ(thread_id, semaphore_id)) != osOK)
	{
		return rc;
	}
	// check if not blocked on other semaphore
	if ( (rc = os_SearchThreadAllSemaphoresBlockedQ(thread_id, &sem_id, &sem_p)) != osOK)
	{
		return rc;
	}		

	if (sem_p == MAX_THREADS_SEM)
	{
		// not blocked on any other semaphores, continue to run
	}
	else
	{
		// need to block thread on semaphore (already in the blocked queue)
		thread_id->semaphore_id = sem_id;
		thread_id->semaphore_p = sem_p;
		thread_id->status = TH_BLOCKED;
	}
	
	// unblock a thread from the queue based on thread priority if any left and assign the semaphore to it

	if (semaphore_id->threads_q_cnt == 0)
	{
		// no threads blocked on this semaphore
		return osOK;
	}
	
	// unblock the thread with highest priority waiting in the queue
	
	idx = semaphore_id->threads_q_cnt;
	maxPriority = osPriorityIdle;
		
	// search for the thread with highest priority waiting in the queue
	for ( j = 0; j < semaphore_id->threads_q_cnt ; j++ )
	{		
		//sanity
		if ( semaphore_id->threads_q[j].threadId != NULL)
		{
			if (semaphore_id->threads_q[j].threadId->priority > maxPriority )
			{
				maxPriority = semaphore_id->threads_q[j].threadId->priority;
				idx = j; // remember the highest priority thread in the queue so far
				thread_id = semaphore_id->threads_q[j].threadId;
			}
		}
	}
	
	// assign the semaphore to the newly found thread and 
	// unblock the thread (if any found) from this semaphore
	if (idx == semaphore_id->threads_q_cnt)
	{
		return osErrorValue;
	}	
	
	// unblock thread 				
	thread_id->status = TH_READY;			
	
	// Thread(s) status change - invoke scheduler to re-evaluate running thread
	os_KernelInvokeScheduler ();					

	return osOK;
}

/// Delete a Semaphore that was created by \ref osSemaphoreCreate.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreDelete shall be consistent in every CMSIS-RTOS.
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id)
{
	uint32_t i, idx = MAX_SEMAPHORES;
	
	if (semaphore_id == NULL)
	{
		return osErrorParameter;
	}
	
	// check if the semaphore is currently in use or it contains threads in queue
	// return error if semaphore still in use/contains blocked threads pending
	// should only delete a semaphore if not in use or with threads pending
	if (semaphore_id->threads_q_cnt != 0)
	{
		return osErrorValue;
	}	
	if (semaphore_id->threads_own_q_cnt != 0)
	{
		return osErrorValue;
	}	
	
	// check if any semaphores defined in semaphores queue
	if (sem_counter == 0) // this should not happen
	{
		return osOK;
	}
	
	// search for the semaphore in the semaphore queue 
	// break the link to it if found and remember index for shifting queue
	for ( i = 0; i < sem_counter ; i++ )
	{	
		if (semaphores[i] == semaphore_id)
		{
			semaphores[i] = NULL;
			idx = i;
		}
	}		
	
	// found the semaphore in the semaphore queue
	// shifting queue
	if (idx != MAX_SEMAPHORES)
	{
		for ( i = idx; i < sem_counter-1 ; i++ )
		{
				semaphores[i] = semaphores[i+1];	
		}		
		semaphores[sem_counter-1] = NULL;
	}			
	
	
	sem_counter--;
	
	free(semaphore_id);
	
	return osOK;
}

/// Remove thread from all semaphore queues.
/// \param     thread_id  thread object.
/// \return status code that indicates the execution status of the function.
osStatus os_SemaphoreRemoveThread (osThreadId thread_id)
{
	uint32_t i;
	
	if (sem_counter == 0)
	{
		return osOK;
	}
	
	if ( thread_id == NULL)
	{
		return osErrorParameter;
	}
	
	for ( i = 0; i < sem_counter ; i++ )
	{		
		os_RemoveThreadFromSemaphoreBlockedQ(thread_id,semaphores[i]);
		os_RemoveThreadFromSemaphoreOwnerQ(thread_id,semaphores[i]);	
	}
	
	return osOK;
}

/// Remove thread from a blocked semaphore queue.
/// \param     thread_id  thread object.
/// \param     semaphore_id  semaphore object
/// \return status code that indicates the execution status of the function.
osStatus os_RemoveThreadFromSemaphoreBlockedQ (osThreadId thread_id, osSemaphoreId semaphore_id)
{
	uint32_t j, idx = MAX_THREADS_SEM;
//	osStatus rc = osOK;

	if ( thread_id == NULL)
	{
		return osErrorParameter;
	}
	
	if (semaphore_id == NULL)
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
	
	if (semaphore_id->threads_q_cnt == 0)
	{
		return osErrorParameter;
	}
	
  // search for the thread in the given semaphore queue	
	idx = os_SearchThreadInSemaphoreBlockedQ(thread_id,semaphore_id);

	if (idx == MAX_THREADS_SEM)
	{
		// this thread was not in the provided semaphore's queue
		return osOK;
	}
	
	// if the thread is blocked on this queue, then it must be pointing to it; need to clear
	semaphore_id->threads_q[idx].threadId->semaphore_id = NULL;
	semaphore_id->threads_q[idx].threadId->semaphore_p = MAX_THREADS_SEM;
	// remove the thread from the semaphore's queue
	semaphore_id->threads_q[idx].threadId = NULL;
	semaphore_id->threads_q[idx].expiryTime = 0;	
	semaphore_id->threads_q[idx].ticks = 0;
	
	for ( j = idx; j < (semaphore_id->threads_q_cnt) - 1 ; j++ )
	{
		semaphore_id->threads_q[j] = semaphore_id->threads_q[j+1];
		if (semaphore_id->threads_q[j].threadId == NULL)
		{
			// one of the threads in the queue is NULL
			return osErrorValue;
		}
		semaphore_id->threads_q[j].threadId->semaphore_p = j; 	
	}	
	
	semaphore_id->threads_q[(semaphore_id->threads_q_cnt) - 1].threadId = NULL;
	semaphore_id->threads_q[(semaphore_id->threads_q_cnt) - 1].expiryTime = 0;
	semaphore_id->threads_q[(semaphore_id->threads_q_cnt) - 1].ticks = 0;
	
	semaphore_id->threads_q_cnt--;

	return osOK;
}

/// Remove thread from a blocked semaphore queue.
/// \param     thread_id  thread object.
/// \param     semaphore_id  semaphore object
/// \return status code that indicates the execution status of the function.
osStatus os_RemoveThreadFromSemaphoreOwnerQ (osThreadId thread_id, osSemaphoreId semaphore_id)
{
	uint32_t j, idx = MAX_THREADS_SEM;
//	osStatus rc = osOK;

	//sanity
	if ( thread_id == NULL)
	{
		return osErrorParameter;
	}
	if (semaphore_id == NULL)
	{
		return osErrorParameter;
	}		
	
	if (semaphore_id->threads_own_q_cnt == 0)
	{
		return osErrorParameter;
	}
	
  // search for the thread in the given semaphore queue	
	idx = os_SearchThreadInSemaphoreOwnerQ(thread_id,semaphore_id);

	if (idx == MAX_THREADS_SEM)
	{
		// this thread was not in the provided semaphore's queue
		return osOK;
	}

	// remove the thread from the semaphore's queue
	semaphore_id->threads_own_q[idx] = NULL;
			
	for ( j = idx; j < (semaphore_id->threads_own_q_cnt) - 1 ; j++ )
	{
		semaphore_id->threads_own_q[j] = semaphore_id->threads_own_q[j+1];
		if (semaphore_id->threads_own_q[j] == NULL)
		{
			// one of the threads in the queue is NULL
			return osErrorValue;
		}
//		semaphore_id->threads_own_q[j]->semaphore_p = j; 	
	}

	semaphore_id->threads_own_q[(semaphore_id->threads_own_q_cnt) - 1] = NULL;
	semaphore_id->threads_own_q_cnt--;	
		
	return osOK;
}

/// Insert thread in the owners semaphore queue.
/// \param     thread_id  thread object.
/// \param     semaphore_id  semaphore object
/// \return status code that indicates the execution status of the function.
osStatus os_InsertThreadInSemaphoreOwnerQ (osThreadId thread_id, osSemaphoreId semaphore_id)
{
	if ( thread_id == NULL)
	{
		return osErrorParameter;
	}
	if (semaphore_id == NULL)
	{
		return osErrorParameter;
	}		
	
	if (semaphore_id->threads_own_q_cnt == MAX_THREADS_SEM)
	{
		return osErrorResource;			
	}	
	
	if (semaphore_id->threads_own_q_cnt == semaphore_id->ownCount)
	{
		return osErrorResource;			
	}		
	
	// check if thread already in semaphore queue
	if (os_SearchThreadInSemaphoreOwnerQ(thread_id, semaphore_id) != MAX_THREADS_SEM)
	{
		// already own this semaphore
		return osOK;
	}
	
	semaphore_id->threads_own_q[semaphore_id->threads_own_q_cnt] = thread_id;
	semaphore_id->threads_own_q_cnt++;
	
	return osOK;
}

/// Insert thread in the blocked semaphore queue.
/// \param     thread_id  thread object.
/// \param     semaphore_id  semaphore object
/// \param     expiryTime to  give up waiting on semaphore
/// \param     ticks to spend on semaphore
/// \return status code that indicates the execution status of the function.
osStatus os_InsertThreadInSemaphoreBlockedQ (osThreadId thread_id, osSemaphoreId semaphore_id, uint32_t expiryTime, uint32_t ticks)
{
	if ( thread_id == NULL)
	{
		return osErrorParameter;
	}
	if (semaphore_id == NULL)
	{
		return osErrorParameter;
	}		
	
	if (semaphore_id->threads_q_cnt == MAX_THREADS_SEM)
	{ // no room in queue
		return osErrorResource; 
	}
	
	// check if thread already in semaphore blocked queue
	if (os_SearchThreadInSemaphoreBlockedQ(thread_id, semaphore_id) != MAX_THREADS_SEM)
	{
		// already blocked on this semaphore
		return osOK;
	}
	
	semaphore_id->threads_q[semaphore_id->threads_q_cnt].threadId = thread_id;
	semaphore_id->threads_q[semaphore_id->threads_q_cnt].expiryTime = expiryTime;
	semaphore_id->threads_q[semaphore_id->threads_q_cnt].ticks = ticks;
	
	thread_id->semaphore_id = semaphore_id;
	thread_id->semaphore_p = semaphore_id->threads_q_cnt;
	
	semaphore_id->threads_q_cnt++;
	
	return osOK;
}

/// Remove thread from a blocked semaphore queue.
/// \param     thread_id  thread object.
/// \param     semaphore_id  semaphore object.
/// \return the index within the semaphore queue; MAX_THREADS_SEM is not found or error
uint32_t os_SearchThreadInSemaphoreBlockedQ (osThreadId thread_id, osSemaphoreId semaphore_id)
{
	uint32_t j, idx = MAX_THREADS_SEM;

	if ( thread_id == NULL)
	{
		return MAX_THREADS_SEM;
	}
	if (semaphore_id == NULL)
	{
		return MAX_THREADS_SEM;
	}		
		
  // search for the thread in the given semaphore queue	
	for ( j = 0; j < semaphore_id->threads_q_cnt ; j++ )
	{		
		if (semaphore_id->threads_q[j].threadId == thread_id )
		{
			idx = j;
			break;
		}
	}	
	
	return idx;
}

/// Search for thread in the owners semaphore queue.
/// \param     thread_id  thread object.
/// \param     semaphore_id  semaphore object.
/// \return the index within the semaphore queue; MAX_THREADS_SEM is not found or error
uint32_t os_SearchThreadInSemaphoreOwnerQ (osThreadId thread_id, osSemaphoreId semaphore_id)
{
	uint32_t j, idx = MAX_THREADS_SEM;

	if ( thread_id == NULL)
	{
		return MAX_THREADS_SEM;
	}
	if (semaphore_id == NULL)
	{
		return MAX_THREADS_SEM;
	}		
		
  // search for the thread in the given semaphore queue	
	for ( j = 0; j < semaphore_id->threads_own_q_cnt ; j++ )
	{		
		if (semaphore_id->threads_own_q[j] == thread_id )
		{
			idx = j;
			break;
		}
	}	
	
	return idx;
}

/// Search for the thread in all blocked semaphore queue.
/// \param     thread_id  thread object.
/// \param     semaphore_id_p  pointer to semaphore object for returning
/// \param     semaphore_p_p  pointer to index to semaphore object for returning
/// \return index withing one of the semaphores the thread is blocked; 
///   - MAX_THREADS_SEM if not found or error
osStatus os_SearchThreadAllSemaphoresBlockedQ (osThreadId thread_id, osSemaphoreId* semaphore_id_p, uint32_t* semaphore_p_p )
{
	uint32_t i, j;

	if ( thread_id == NULL)
	{
		return osErrorParameter;
	}	

	if ( semaphore_id_p == NULL)
	{
		return osErrorParameter;
	}	

	if ( semaphore_p_p == NULL)
	{
		return osErrorParameter;
	}
	
	*semaphore_p_p = MAX_THREADS_SEM;
	*semaphore_id_p = NULL;
	
  // search for the thread in all semaphore queues
	for ( i = 0; i < sem_counter ; i++ )
	{	
		if (semaphores[i]->threads_q_cnt != 0 )
		{	
			for ( j = 0; j < semaphores[i]->threads_q_cnt ; j++ )
			{		
				if (semaphores[i]->threads_q[j].threadId == thread_id)
				{
					*semaphore_p_p = j;
					*semaphore_id_p = semaphores[i];
					break;
				}
			}	
		}
	}
	
	return osOK;
}

