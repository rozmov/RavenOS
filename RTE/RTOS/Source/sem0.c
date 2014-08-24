/*! \file sem0.c
    \brief This file contains semaphore 0 related API implementations
		\details Defines the semaphore, and initializes it .
*/
#include "trace.h"
#include "osObjects.h"

osSemaphoreId sid_Semaphore0;                        ///< semaphore id
osSemaphoreDef (Semaphore0);                         ///< semaphore object

/*! \fn int Init_Semaphore0 (void) 
    \brief Initializing Semaphore0
		\return 0=successful; -1=failure
*/
int Init_Semaphore0 (void) 
{
  sid_Semaphore0 = osSemaphoreCreate(osSemaphore(Semaphore0), 1);
  if (!sid_Semaphore0)  {
    if (addTrace("sem0 could not create") != TRACE_OK)
		{
			stop_cpu;
		}
		return (-1);
  } 
  return(0);
}

/*! \fn int Delete_Semaphore0 (void)  
    \brief Deleting Semaphore0
    \return 0=successful; -1=failure
*/
int Delete_Semaphore0 (void) 
{
  if ( osSemaphoreDelete(sid_Semaphore0) != osOK)
	{
    if (addTrace("sem0 - could not delete") != TRACE_OK)
		{
			stop_cpu;
		}
		return (-1);
  } 
  return(0);
}
