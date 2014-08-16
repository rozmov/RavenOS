/*! \file main.c
    \brief A program that demonstrates threads
		\details Initialize threads and start the OS
*/

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
  printf("Simple context switching demo\n\r");
  LED_initialize(); 
	
	// create 'thread' functions that start executing
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
	
  if (Init_Semaphore0() != 0)
	{
		stop_cpu;
	}	
	
	osKernelStart ();                         // start thread execution 
  
  while(1)
	{		
    stop_cpu;// Should not be here
  };
}
