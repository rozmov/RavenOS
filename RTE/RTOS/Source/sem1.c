/*! \file sem1.c
    \brief UART Semaphore
		\details Contains semaphore 1 related API implementations. Defines the semaphore, and initializes it .
*/
#include <string.h>
#include "trace.h"
#include "osObjects.h"

osSemaphoreId sid_Semaphore1;                        ///< semaphore id
osSemaphoreDef (Semaphore1);                         ///< semaphore object

/*! 
    \brief Initializing Semaphore
		\return 0=successful; -1=failure
*/
int Init_Semaphore1 (void) 
{
  sid_Semaphore1 = osSemaphoreCreate(osSemaphore(Semaphore1), 1);
  if (!sid_Semaphore1)  {
    ADD_TRACE("sem1 could not create") ;
		return (-1);
  } 
  return(0);
}

/*!  
    \brief Deleting Semaphore
    \return 0=successful; -1=failure
*/
int Delete_Semaphore1 (void) 
{
  if ( osSemaphoreDelete(sid_Semaphore1) != osOK)
	{
    ADD_TRACE("sem1 - could not delete") ;
		return (-1);
  } 
  return(0);
}
