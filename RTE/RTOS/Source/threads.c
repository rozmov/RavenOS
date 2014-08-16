/*! \file threads.c
    \brief This file defines thread implementation using the CMSIS interface
		\details Defines thread defienition, creation and attributes manipulation.
*/

#include "cmsis_os.h" 
#include "kernel.h"

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

/// Thread Status : Running, Blocked or Asleep.
typedef enum os_thread_status 
{
	TH_RUNNING,
	TH_READY,
	TH_BLOCKED,
	TH_ASLEEP
} osThreadStatus;

/// Thead Control Block
struct os_thread_cb
{
	osPriority priority;   ///< Thread Priority
	osThreadStatus status; ///< Thread Status
	uint32_t rtr_q_p;      ///< Ready to Run Queue Pointer
	uint32_t stack_p;      ///< Stack Pointer
	uint32_t semaphore_p;  ///< Semapore Pointer (in semaphore queue) if waiting on a semaphore
	osSemaphoreId semaphore_id; ///< Semaphore ID for semaphore currently blocked on
	uint32_t time_count;   ///< Time until Timeout
	uint32_t timed_q_p;    ///< Timed Queue Pointer
	osStatus timed_ret_p;  ///< Exit Status from Sleep
};

/// \def MAX_THREADS Maximum number for threads supported
#define MAX_THREADS 10

osThreadId rtr_q[MAX_THREADS]; ///< Ready to Run Queue
osThreadId rtr_q_h;            ///< Ready to Run Queue Head 
uint32_t thread_counter = 0;   ///< Ready to Run Queue thread counter

/// Create a thread and add it to Active Threads and set it to state READY.
/// \param[in]     thread_def    thread definition referenced with \ref osThread.
/// \param[in]     argument      pointer that is passed to the thread function as start argument.
/// \return thread ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osThreadCreate shall be consistent in every CMSIS-RTOS.
osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument)
{
  /// If we are instantiating a thread and there is still room in the thread queue, add the thread to the queue.
	if ( (thread_def->instances > 0) && (thread_def->instances < (MAX_THREADS - thread_counter)))
	{
		thread_counter++;
	}
	else
	{
		return NULL;
	}
	
	rtr_q[thread_counter]->rtr_q_p = thread_counter;
	rtr_q[thread_counter]->priority = thread_def->tpriority;
	rtr_q[thread_counter]->semaphore_id = NULL;
	rtr_q[thread_counter]->semaphore_p = MAX_THREADS_SEM;
	rtr_q[thread_counter]->stack_p = PSP_array[thread_counter];
	rtr_q[thread_counter]->status = TH_READY;
	
	
	thread_def->stacksize;
	
	thread_def->pthread;
  
}
	

/// Return the thread ID of the current running thread.
/// \return thread ID for reference by other functions or NULL in case of error.
/// \note MUST REMAIN UNCHANGED: \b osThreadGetId shall be consistent in every CMSIS-RTOS.
osThreadId osThreadGetId (void)
{

}

/// Terminate execution of a thread and remove it from Active Threads.
/// \param[in]     thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadTerminate shall be consistent in every CMSIS-RTOS.
osStatus osThreadTerminate (osThreadId thread_id)
{

}

/// Pass control to next thread that is in state \b READY.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadYield shall be consistent in every CMSIS-RTOS.
osStatus osThreadYield (void)
{

}


/// Change priority of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \param[in]     priority      new priority value for the thread function.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osThreadSetPriority shall be consistent in every CMSIS-RTOS.
osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority)
{

}


/// Get current priority of an active thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
/// \return current priority value of the thread function.
/// \note MUST REMAIN UNCHANGED: \b osThreadGetPriority shall be consistent in every CMSIS-RTOS.
osPriority osThreadGetPriority (osThreadId thread_id)
{

}



