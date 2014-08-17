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

osSemaphoreId semaphores[MAX_SEMAPHORES]; ///< Semaphore Queue
uint32_t sem_counter = 0;                 ///< Semaphore Queue counter


/// Access a Semaphore definition.
/// \param         name          name of the semaphore object.
/// \note CAN BE CHANGED: The parameter to \b osSemaphore shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osSemaphore(name)  \
&os_semaphore_def_##name

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
	
	for ( j = 0; j < semaphores[sem]->threads_q_cnt ; j++ )
	{
		semaphores[sem]->threads_q[j].threadId = NULL;
		semaphores[sem]->threads_q[j].expiryTime = 0;
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
	
	// semaphore sanity check
	if ( semaphore_id == NULL )
	{
		return osErrorParameter;
	}
	
	// check if semaphore is free
	// if free take semaphore
	// if not, add thread to the semaphore queue, mark thread as blocked and invoke scheduler 
  if ( millisec == 0 )
	{
		if ( semaphore_id->thread_id == NULL )
		{			
			if (semaphore_id->threads_q_cnt != MAX_THREADS_SEM)
			{
				semaphore_id->thread_id = th_q[th_q_h];
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].threadId = th_q[th_q_h];
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].expiryTime = 0;
				th_q[th_q_h]->semaphore_id = semaphore_id;
				th_q[th_q_h]->semaphore_p = semaphore_id->threads_q_cnt;
				semaphore_id->threads_q_cnt++;
				return 0;
			}
			return -1; // no room in queue
		}	
    return -1; // semaphore taken, but can't wait		
	}
	else // thread can wait on semaphore of the semaphore is not available
	{
		ticks = (uint32_t) osKernelSysTickMicroSec(microsec);
		
		if ( semaphore_id->thread_id == NULL )
		{			
			if (semaphore_id->threads_q_cnt != MAX_THREADS_SEM)
			{
				semaphore_id->thread_id = th_q[th_q_h];
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].threadId = th_q[th_q_h];
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].expiryTime = 0;
				th_q[th_q_h]->semaphore_id = semaphore_id;
				th_q[th_q_h]->semaphore_p = semaphore_id->threads_q_cnt;				
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
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].threadId = th_q[th_q_h];
				semaphore_id->threads_q[semaphore_id->threads_q_cnt].expiryTime = ticks;
				
				// set thread to blocked state and call scheduler
				th_q[th_q_h]->semaphore_id = semaphore_id;
				th_q[th_q_h]->semaphore_p = semaphore_id->threads_q_cnt;
				th_q[th_q_h]->status = TH_BLOCKED;
				semaphore_id->threads_q_cnt++;
				
				//invoke scheduler
				os_KernelInvokeScheduler ();
				
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
	uint32_t j;
	
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
	
	// Remove semaphore from thread 
	thread_id->semaphore_p = MAX_THREADS_SEM;
	thread_id->semaphore_id = NULL;
	
  // Remove thread from semaphore queue
  for ( j = 0; j < semaphore_id->threads_q_cnt ; j++ )
	{		
		if (semaphore_id->threads_q[j].threadId == thread_id )
		{	
			semaphore_id->threads_q[j].threadId = NULL;
			semaphore_id->threads_q[j].expiryTime = 0;
		}
	}
	
	// Release semaphore
	semaphore_id->thread_id = NULL;
	
	return osOK;
}


/// Delete a Semaphore that was created by \ref osSemaphoreCreate.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreDelete shall be consistent in every CMSIS-RTOS.
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id)
{
	uint32_t i,j, idx = MAX_SEMAPHORES;
	
	if (semaphore_id == NULL)
	{
		return osErrorParameter;
	}
		
	// unblock any threads waiting for this semaphore
	for ( j = 0; j < semaphore_id->threads_q_cnt ; j++ )
	{
		semaphore_id->threads_q[j].threadId->semaphore_id = NULL;
		semaphore_id->threads_q[j].threadId->semaphore_p = MAX_THREADS_SEM;
		semaphore_id->threads_q[j].threadId->status = TH_READY; // if a thread can block on multiple semaphores, check if the thread is in queue for other semaphores
		semaphore_id->threads_q[j].threadId = NULL;
		semaphore_id->threads_q[j].expiryTime = 0;
	}
	
	semaphore_id->thread_id = NULL; 
	semaphore_id->threads_q_cnt = 0;
	
	// remove from semaphores queue
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
	}			
	
	sem_counter--;
	
	free(semaphore_id);
	return osOK;
}

/// \fn osStatus osSemaphoreRemoveThread (osThreadId thread_id)
/// \brief Delete a Thread from all semaphore queues.
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
	
	for ( i = 0; i < sem_counter ; i++ )
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
			
			if (idx != MAX_THREADS_SEM)
			{
				for ( j = idx; j < (semaphores[i]->threads_q_cnt) - 1 ; j++ )
				{
						semaphores[i]->threads_q[j] = semaphores[i]->threads_q[j+1];
						semaphores[i]->threads_q[j].threadId->semaphore_p = j; 	
				}		
			}			
			semaphores[i]->threads_q_cnt--;				
	}
	
	return osOK;
}


#endif     // Semaphore available
