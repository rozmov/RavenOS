/*! \file sem0.c
    \brief This file contains semaphore 0 related API implementations
		\details Defines the semaphore, and initializes it .
*/
#include <cmsis_os.h>                                           // CMSIS RTOS header file

//void Thread_Semaphore (void const *argument);                   // thread function
//osThreadId tid_Thread_Semaphore;                                // thread id
//osThreadDef (Thread_Semaphore, osPriorityNormal, 1, 0);         // thread object

osSemaphoreId sid_Thread_Semaphore;                        ///< semaphore id
osSemaphoreDef (Semaphore0);                               ///< semaphore object

/*! \fn int Init_Semaphore0 (void) 
    \brief Initializing Semaphore0
*/
int Init_Semaphore0 (void) 
{

  sid_Thread_Semaphore = osSemaphoreCreate(osSemaphore(Semaphore0), 1);
  if (!sid_Thread_Semaphore)  {
    ; // Semaphore object not created, handle failure
  }
  
//  tid_Thread_Semaphore  = osThreadCreate (osThread(Thread_Semaphore), NULL);
//  if(!tid_Thread_Semaphore) return(-1);
//  
  return(0);
}

//void Thread_Semaphore(void const *argument) {
//  int32_t val;

//  while(1) {
//    ; // Insert thread code here...

//    val = osSemaphoreWait (sid_Thread_Semaphore, 10);           // wait 10 mSec
//    switch (val) {
//      case osOK:
//        ; // Use protected code here...
//        osSemaphoreRelease (sid_Thread_Semaphore);              // Return a token back to a semaphore
//        break;
//      case osErrorResource:
//        break;
//      case osErrorParameter:
//        break;
//      default:
//        break;
//    }

//    osThreadYield();        // suspend thread
//  }
//}
