/*! \file semaphores.c
    \brief Semaphore implementation according to CMSIS interfaces
		\details Defines semaphore defienition, creation and attributes manipulation.
*/

#include "cmsis_os.h" 
#include <stdlib.h>
#include "kernel.h"

//  ==== Semaphore Management Functions ====

#if (defined (osFeature_Semaphore)  &&  (osFeature_Semaphore != 0))     // Semaphore available

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


/// \fn uint32_t os_SearchThreadInSemaphoreQ (osThreadId thread_id, osSemaphoreId semaphore_id)
/// \brief Remove thread from a blocked semaphore queue.
/// \param[in]     thread_id  thread object.
/// \param[in]     semaphore_id  semaphore object.
/// \return the index within the semaphore queue; MAX_THREADS_SEM is not found or error
uint32_t os_SearchThreadInSemaphoreQ (osThreadId thread_id, osSemaphoreId semaphore_id);



osStatus os_SearchThreadSemaphoresExcept (osThreadId thread_id,osSemaphoreId semaphore_id, osSemaphoreId* semaphore_id_p, uint32_t* semaphore_p_p );



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
	}
	 
	semaphores[sem]->threads_q_cnt = 0;

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
	uint32_t ticks = 0;
	uint32_t idx,j;
	osThreadId curr_th = osThreadGetId();
	
	// semaphore sanity check
	if ( semaphore_id == NULL )
	{
		return osErrorParameter;
	}
	
	// check if semaphore is free
	// if free take semaphore
	// if not, add thread to the semaphore queue, mark thread as blocked and invoke scheduler 
	/// \todo do \a not add the thread currently holding the semaphore to the blocked queue 
  if ( millisec == 0 )
	{
		if ( semaphore_id->thread_id == NULL )
		{			
			if (semaphore_id->threads_q_cnt != MAX_THREADS_SEM)
			{
				semaphore_id->thread_id = curr_th;
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].threadId = curr_th;
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].expiryTime = 0;
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].ticks = 0;
				curr_th->semaphore_id = semaphore_id;
				curr_th->semaphore_p = semaphore_id->threads_q_cnt;
				semaphore_id->threads_q_cnt++;
				return 0;
			}
			return -1; // no room in queue
		}	
    return -1; // semaphore taken, but can't wait		
	}
	else // thread can wait on semaphore - the semaphore is not available
	{
		if ( semaphore_id->thread_id == NULL )
		{			
			if (semaphore_id->threads_q_cnt != MAX_THREADS_SEM)
			{
				semaphore_id->thread_id = curr_th;
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].threadId = curr_th;
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].expiryTime = 0;
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].ticks = 0;
				curr_th->semaphore_id = semaphore_id;
				curr_th->semaphore_p = semaphore_id->threads_q_cnt;				
				semaphore_id->threads_q_cnt++;
				return 0;
			}
			return -1; // no room in queue
		}	
    else // semaphore taken, add to semaphore queue
		{
			// enqueue thread to semaphore queue with appropriate time
			if (semaphore_id->threads_q_cnt != MAX_THREADS_SEM)
			{
				// set thread to blocked state and call scheduler
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].threadId = curr_th;
				if (osWaitForever == millisec)
				{
					semaphore_id->threads_q[semaphore_id->threads_q_cnt].expiryTime = osWaitForever;
					semaphore_id->threads_q[semaphore_id->threads_q_cnt].ticks = osWaitForever;
				}
				else
				{
					ticks = (uint32_t) osKernelSysTickMicroSec(microsec);
					semaphore_id->threads_q[semaphore_id->threads_q_cnt].expiryTime = ticks + osKernelSysTick() ;
					semaphore_id->threads_q[semaphore_id->threads_q_cnt].ticks = ticks;
				}
				curr_th->semaphore_id = semaphore_id;
				curr_th->semaphore_p = semaphore_id->threads_q_cnt;	
        				
				semaphore_id->threads_q_cnt++;				
				// while the semaphore is blocked, keep on waiting
				// if can no longer wait return fail
				while (semaphore_id->thread_id != NULL)
				{
					if ( semaphore_id->thread_id == osThreadGetId() )
					{
						// the thread has been granted semaphore control
						return 0;
					}
					
					// semaphore (still) busy, check if the thread can still wait
					if (semaphore_id->threads_q[curr_th->semaphore_p].expiryTime == osWaitForever &&
						  semaphore_id->threads_q[semaphore_id->threads_q_cnt].ticks == osWaitForever)
					{
						// can wait forever
					}
					else if (semaphore_id->threads_q[curr_th->semaphore_p].expiryTime == osKernelSysTick() )
					{
						// can no longer wait for the semaphore, unblock thread and return failure
						idx = curr_th->semaphore_p;
						
						if (idx != MAX_THREADS_SEM)
						{
							// remove thread from the semaphore queue
							for ( j = idx; j < (semaphore_id->threads_q_cnt) - 1 ; j++ )
							{
									semaphore_id->threads_q[j] = semaphore_id->threads_q[j+1];
								  if ( semaphore_id->threads_q[j].threadId != NULL)
									{
										semaphore_id->threads_q[j].threadId->semaphore_p = j; 
									}
							}		
							semaphore_id->threads_q_cnt--;															
						}

						// remove semaphore from thread
            curr_th->semaphore_id = NULL;
						curr_th->semaphore_p = MAX_THREADS_SEM;
						
						// return failure to access semaphore - timeout
						return -1;
					}
					// mark thread as blocked and yield
					curr_th->status = TH_BLOCKED;
					//invoke scheduler
					os_KernelInvokeScheduler ();					
				}
				
				// semaphore no longer busy, so take it and move on
				semaphore_id->thread_id = curr_th;
				
				return 0;
			}
			return -1; // no room in queue		
		}					
	}
}


/// Release a Semaphore token.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreRelease shall be consistent in every CMSIS-RTOS.
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id)
{
	osThreadId thread_id = NULL;
	uint32_t j, idx, sem_p;
	osPriority maxPriority = osPriorityIdle;
	osThreadId curr_th = osThreadGetId();
	osStatus rc;
	osSemaphoreId sem_id;
	
	// semaphore sanity check
	if ( semaphore_id == NULL )
	{
		return osErrorParameter;
	}
		
	thread_id = semaphore_id->thread_id;
	
	// if there is no thread currently holding the semaphore, we are done
	if ( thread_id == NULL )
	{
		return osOK;
	}
	
	// Is the thread holding the semaphore the current thread?
	if ( thread_id != curr_th )
	{
		return osErrorParameter;
	}	
	
	// Remove semaphore from thread 
	thread_id->semaphore_p = MAX_THREADS_SEM;
	thread_id->semaphore_id = NULL;
	
	if (semaphore_id->threads_q_cnt != 0)
	{
		if ((rc = os_RemoveThreadFromSemaphore(thread_id, semaphore_id)) != osOK)
		{
			return rc;
		}
		// unblock the thread, if not blocked on other semaphore
		if ( os_SearchThreadAllSemaphores(thread_id, &sem_id, &sem_p) == osOK)
		{
			if (sem_p == MAX_THREADS_SEM)
			{
				// unblock thread 
				thread_id->status = TH_READY;
			}
		}
	
	  // unblock a process from the queue if any left and assign the semaphore to it

		idx = semaphore_id->threads_q_cnt;
		maxPriority = osPriorityIdle;
    
		// unblock the thread with highest priority waiting in the queue
		
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
		// unblock the thread, if any found and not blocked on other semaphore
		if (idx != semaphore_id->threads_q_cnt)
		{
			// assign thread to semaphore
			semaphore_id->thread_id = thread_id;
			
			if ( os_SearchThreadSemaphoresExcept(thread_id, semaphore_id, &sem_id, &sem_p) == osOK)
			{
				// if thread is not blocked on any other semaphores,
				// or the semaphore it is blocked on is 
				if (sem_p == MAX_THREADS_SEM || sem_id->thread_id == thread_id)
				{
					// unblock thread 					
					semaphore_id->thread_id->status = TH_READY;
					// Thread(s) status change - invoke scheduler to re-evaluate running thread
					os_KernelInvokeScheduler ();					
				}			
				else
				{
           // thread blocked on another semaphore
				}
			}
			else
			{
				return osErrorValue;
			}
		}
	}
	else
	{
		// Release semaphore
		semaphore_id->thread_id = NULL;
	}
	
	return osOK;
}


/// Delete a Semaphore that was created by \ref osSemaphoreCreate.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreDelete shall be consistent in every CMSIS-RTOS.
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id)
{
	uint32_t i, idx = MAX_SEMAPHORES;
//  uint32_t j, ctsw = 0;
	
	if (semaphore_id == NULL)
	{
		return osErrorParameter;
	}
	
	// check if the semaphore is currently in use or it contains threads in queue
	// return error if semaphore still in use/contains blocked threads pending
	// should only delete a semaphore if not in use or with threads pending
	if ((semaphore_id->thread_id != NULL) || (semaphore_id->threads_q_cnt != 0))
	{
		return osErrorValue;
	}	
	
	// check if any semaphores defined in semaphores queue
	if (sem_counter == 0) // this should not happen
	{
		return osOK;
	}
	
//	if (semaphore_id->threads_q_cnt != 0 )
//	{	
//		// unblock any threads waiting for this semaphore
//		for ( j = 0; j < semaphore_id->threads_q_cnt ; j++ )
//		{
//			semaphore_id->threads_q[j].threadId->semaphore_id = NULL;
//			semaphore_id->threads_q[j].threadId->semaphore_p = MAX_THREADS_SEM;
//			semaphore_id->threads_q[j].threadId->status = TH_READY; // if a thread can block on multiple semaphores, check if the thread is in queue for other semaphores
//			semaphore_id->threads_q[j].threadId = NULL;
//			semaphore_id->threads_q[j].expiryTime = 0;
//			ctsw = 1;
//		}
//		
//		semaphore_id->thread_id = NULL; 
//		semaphore_id->threads_q_cnt = 0;
//	}
	
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
	}			
	
	sem_counter--;
	
	free(semaphore_id);
	
//	// there were threads unblocked, need to invoke the scheduler to re-evaluate running thread
//	// a context switch may be needed
//	if (ctsw == 1)
//	{
//		os_KernelInvokeScheduler ();
//	}
	
	return osOK;
}

/// \fn osStatus osSemaphoreRemoveThread (osThreadId thread_id)
/// \brief Remove thread from all semaphore queues.
/// \param[in]     thread_id  thread object.
/// \return status code that indicates the execution status of the function.
osStatus os_SemaphoreRemoveThread (osThreadId thread_id)
{
	uint32_t i,j, idx = MAX_THREADS_SEM;
	osStatus rc = osOK;
	
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
		if (semaphores[i]->threads_q_cnt != 0 )
		{
			for ( j = 0; j < semaphores[i]->threads_q_cnt ; j++ )
			{		
				if (semaphores[i]->threads_q[j].threadId == thread_id )
				{
					// check if this is the thread currently controlling the semaphore
					if (semaphores[i]->thread_id == thread_id )
					{
						// Release the semaphore
						if ((rc = osSemaphoreRelease(semaphores[i])) != osOK)
						{
							return rc;
						}
					}
					else
					{
						// remove the thread from the semaphore's queue
						semaphores[i]->threads_q[j].threadId = NULL;
						semaphores[i]->threads_q[j].expiryTime = 0;
						idx = j;
					}
				}
			}
			
			if (idx != semaphores[i]->threads_q_cnt)
			{
				for ( j = idx; j < (semaphores[i]->threads_q_cnt) - 1 ; j++ )
				{
						semaphores[i]->threads_q[j] = semaphores[i]->threads_q[j+1];
						semaphores[i]->threads_q[j].threadId->semaphore_p = j; 	
				}	
				semaphores[i]->threads_q_cnt--;
			}							
		}			
	}
	
	return osOK;
}

/// \fn osStatus os_RemoveThreadFromOwnSemaphore (osThreadId thread_id)
/// \brief Remove thread from currently blocked semaphore queue.
/// \param[in]     thread_id  thread object.
/// \return status code that indicates the execution status of the function.
osStatus os_RemoveThreadFromOwnSemaphore (osThreadId thread_id)
{
	uint32_t j, semaphore_p;
	osStatus rc = osOK;
	osSemaphoreId semaphore_id;

	//sanity
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
	
	semaphore_id = thread_id->semaphore_id;
	semaphore_p  = thread_id->semaphore_p;
	
	if (semaphore_id->threads_q[semaphore_p].threadId == NULL)
	{
		return osErrorParameter;
	}

	if (semaphore_id->threads_q[semaphore_p].threadId != thread_id)
	{
		return osErrorParameter;
	}
	
	// remove thread from the semaphore queue
	semaphore_id->threads_q[semaphore_p].threadId = NULL;
	semaphore_id->threads_q[semaphore_p].expiryTime = 0;
	
	for ( j = semaphore_p; j < (semaphore_id->threads_q_cnt) - 1 ; j++ )
	{
			semaphore_id->threads_q[j] = semaphore_id->threads_q[j+1];
			if ( semaphore_id->threads_q[j].threadId != NULL)
			{
				semaphore_id->threads_q[j].threadId->semaphore_p = j; 
			}
	}		
	semaphore_id->threads_q_cnt--;															
			
	
	thread_id->semaphore_p = MAX_THREADS_SEM;
	thread_id->semaphore_id = NULL;

	return rc;
}

/// \fn osStatus os_RemoveThreadFromSemaphore (osThreadId thread_id, osSemaphoreId semaphore_id)
/// \brief Remove thread from a blocked semaphore queue.
/// \param[in]     thread_id  thread object.
/// \param[in]     semaphore_id  semaphore object
/// \return status code that indicates the execution status of the function.
osStatus os_RemoveThreadFromSemaphore (osThreadId thread_id, osSemaphoreId semaphore_id)
{
	uint32_t j, idx = MAX_THREADS_SEM;
	osStatus rc = osOK;

	//sanity
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
	
	// check if this is the thread currently controlling the semaphore
	if (semaphore_id->thread_id == thread_id && thread_id->semaphore_id == semaphore_id)
	{
		if ((rc = os_RemoveThreadFromOwnSemaphore(thread_id)) != osOK)
		{
			return rc;
		}
		return osOK;
	}
	
  // search for the thread in the given semaphore queue	
	idx = os_SearchThreadInSemaphoreQ(thread_id,semaphore_id);

	if (idx != MAX_THREADS_SEM)
	{
		// remove the thread from the semaphore's queue
		semaphore_id->threads_q[idx].threadId = NULL;
		semaphore_id->threads_q[idx].expiryTime = 0;
				
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
		semaphore_id->threads_q_cnt--;
		rc = osOK;
	}			
	else
	{
		// this thread was not in the provided semaphore's queue
		rc = osOK;
	}
	
	return rc;
}

/// \fn uint32_t os_SearchThreadInSemaphoreQ (osThreadId thread_id, osSemaphoreId semaphore_id)
/// \brief Remove thread from a blocked semaphore queue.
/// \param[in]     thread_id  thread object.
/// \param[in]     semaphore_id  semaphore object.
/// \return the index within the semaphore queue; MAX_THREADS_SEM is not found or error
uint32_t os_SearchThreadInSemaphoreQ (osThreadId thread_id, osSemaphoreId semaphore_id)
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

/// \fn osStatus os_SearchThreadAllSemaphores (osThreadId thread_id, osSemaphoreId* semaphore_id_p, uint32_t* semaphore_p_p )
/// \brief Search for the thread in all blocked semaphore queue.
/// \param[in]     thread_id  thread object.
/// \param[in]     semaphore_id_p  pointer to semaphore object for returning
/// \param[in]     semaphore_p_p  pointer to index to semaphore object for returning
/// \return index withing one of the semaphores the thread is blocked; 
///   - MAX_THREADS_SEM is not found or error
osStatus os_SearchThreadAllSemaphores (osThreadId thread_id, osSemaphoreId* semaphore_id_p, uint32_t* semaphore_p_p )
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

/// \fn osStatus os_SearchThreadAllSemaphoresBlockedQ (osThreadId thread_id, osSemaphoreId* semaphore_id_p, uint32_t* semaphore_p_p )
/// \brief Search for the thread in all blocked semaphore queue.
/// \param[in]     thread_id  thread object.
/// \param[in]     semaphore_id_p  pointer to semaphore object for returning
/// \param[in]     semaphore_p_p  pointer to index to semaphore object for returning
/// \return index withing one of the semaphores the thread is blocked; 
///   - MAX_THREADS_SEM is not found or error
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
				if (semaphores[i]->threads_q[j].threadId == thread_id &&
					  semaphores[i]->threads_q[j].threadId->status == TH_BLOCKED)
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



/// \fn osStatus os_SearchThreadSemaphoresExcept (osThreadId thread_id,osSemaphoreId semaphore_id, osSemaphoreId* semaphore_id_p, uint32_t* semaphore_p_p )
/// \brief Search for the thread in all blocked semaphore queue.
/// \param[in]     thread_id  thread object.
/// \param[in]     semaphore_id semaphore object to exclude in search
/// \param[in]     semaphore_id_p  pointer to semaphore object for returning
/// \param[in]     semaphore_p_p  pointer to pointer to semaphore object for returning
/// \return index withing one of the semaphores the thread is blocked; 
///   - MAX_THREADS_SEM is not found or error
osStatus os_SearchThreadSemaphoresExcept (osThreadId thread_id,osSemaphoreId semaphore_id, osSemaphoreId* semaphore_id_p, uint32_t* semaphore_p_p )
{
	uint32_t i, j;

	if ( thread_id == NULL)
	{
		return osErrorParameter;
	}	

	if ( semaphore_id == NULL)
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
		if (semaphores[i] != semaphore_id && semaphores[i]->threads_q_cnt != 0 )
		{	
			for ( j = 0; j < semaphores[i]->threads_q_cnt ; j++ )
			{		
				if (semaphores[i]->threads_q[j].threadId == thread_id && 
					  semaphores[i]->threads_q[j].threadId->status == TH_BLOCKED)
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

#endif     // Semaphore available
