/*! \file semaphores.c
    \brief Semaphore implementation according to CMSIS interfaces
		\details Defines semaphore defienition, creation and attributes manipulation.
*/

#include "cmsis_os.h" 

//  ==== Semaphore Management Functions ====

#if (defined (osFeature_Semaphore)  &&  (osFeature_Semaphore != 0))     // Semaphore available

/// Define a Semaphore object.
/// \param         name          name of the semaphore object.
/// \note CAN BE CHANGED: The parameter to \b osSemaphoreDef shall be consistent but the
///       macro body is implementation specific in every CMSIS-RTOS.
#define osSemaphoreDef(name)  \
const osSemaphoreDef_t os_semaphore_def_##name = { 0 }

/// \def MAX_SEMAPHORES Maximum number for threads supported
#define MAX_SEMAPHORES 10

struct os_semaphore_cb
{
	osThreadId                 threads_q[MAX_THREADS_SEM];    ///< queue of threads blocked on a semaphore.
	osThreadId                 thread_id;                     ///< the thread currently controlling the semaphore.
};

osSemaphoreId semaphores[MAX_SEMAPHORES];
uint32_t sem_counter = 0;   ///< Ready to Run Queue thread counter


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

}


/// Wait until a Semaphore token becomes available.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \param[in]     millisec      timeout value or 0 in case of no time-out.
/// \return number of available tokens, or -1 in case of incorrect parameters.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreWait shall be consistent in every CMSIS-RTOS.
int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec)
{

}


/// Release a Semaphore token.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreRelease shall be consistent in every CMSIS-RTOS.
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id)
{

}


/// Delete a Semaphore that was created by \ref osSemaphoreCreate.
/// \param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osSemaphoreDelete shall be consistent in every CMSIS-RTOS.
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id)
{

}


#endif     // Semaphore available
