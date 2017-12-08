/*! \file sem0.c
    \brief  LED Semaphore
		\details Contains semaphore 0 related API implementations. Defines the semaphore, and initializes it .
*/
#include <string.h>
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
    ADD_TRACE("sem0 could not create") ;
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
    ADD_TRACE("sem0 - could not delete") ;
		return (-1);
  } 
  return(0);
}
