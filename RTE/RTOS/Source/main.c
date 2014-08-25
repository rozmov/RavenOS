/*! \file main.c
    \brief A program that demonstrates threads
		\details Initialize threads and start the OS
**/

#define osObjectExternal                    // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "peripherals.h"                    // Peripheral definitions
#include "stdio.h"

// -------------------------------------------------------------------------
/*! \fn int main(void)
    \brief Start of main program
		\details Initializes the peripherals and starts the OS
*/
int main(void)
{
	osKernelInitialize ();                    // initialize CMSIS-RTOS
	
	// initialize peripherals
	UART_initialize();
  printf("\n\rRavenOS demo\n\r");
  LED_initialize(); 
	
	printf("Initializing threads\n\r");
  if (Init_thread0() != 0)
	{
		stop_cpu;
	}
  if (Init_thread1() != 0)
	{
		stop_cpu;
	}
  if (Init_thread2() != 0)
	{
		stop_cpu;
	}	
  if (Init_thread3() != 0)
	{
		stop_cpu;
	}
	
	printf("Initializing semaphores\n\r");
  if (Init_Semaphore0() != 0)
	{
		stop_cpu;
	}	
  if (Init_Semaphore1() != 0)
	{
		stop_cpu;
	}		
	printf("Start kernel\n\r");
	osKernelStart ();                         // start thread execution 
  
  while(1)
	{		
			// Should not be here
  };
}
