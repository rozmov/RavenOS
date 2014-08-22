/*! \file kernel.c
    \brief Implements kernel level operations.
	  \details Maps operations to CMSIS interface. Performs scheduling and context switching.
*/
#include "threadIdle.h"
#include "TM4C123.h"
#include "stdio.h"
#include "osObjects.h"                      // RTOS object definitions
#include "kernel.h"
#include "scheduler.h"
#include "trace.h"


/*! \def HW32_REG(ADDRESS) 
         Macros for word accesses. */
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
	
#define os_sysTickTicks 16000 ///< Number of ticks between two system timer interrupts. This would generate 1000 interruts/s on a 16MHz clock.

void __svc(0x00) os_start(void);              ///< OS start scheduler
void __svc(0x01) thread_yield(void);          ///< Thread needs to schedule a switch of context
void __svc(0x02) stack_alloc(int thread_idx); ///< Initialize the process stack pointer PSP_array[thread_idx]
void SVC_Handler_C(unsigned int * svc_args);
void HardFault_Handler_C(unsigned int * svc_args);
void ScheduleContextSwitch(void);

/// \var systick_count Event to tasks
volatile uint32_t systick_count=0;

/// Stack for each task ( \ref DEFAULT_STACK_SIZE bytes)
uint8_t task_stack[MAX_THREADS][DEFAULT_STACK_SIZE];

/// Data used by OS
uint32_t  curr_task=0;     ///< Current task
uint32_t  next_task=1;     ///< Next task
uint32_t  PSP_array[MAX_THREADS];    ///< Process Stack Pointer for each task
uint32_t  svc_exc_return;  ///< EXC_RETURN use by SVC

uint32_t kernel_running = 0; ///< flag whether the kernel is running of not

//  ==== Kernel Control Functions ====

/// \fn osStatus osKernelInitialize (void)
/// \brief Initialize the RTOS Kernel for creating objects.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osKernelInitialize shall be consistent in every CMSIS-RTOS.
osStatus osKernelInitialize (void)
{
	// Enable double-word stack alignment
	SCB->CCR |= SCB_CCR_STKALIGN_Msk; // Set STKALIGN bit (bit 9) of CCR
	
	// Initialize the Idle thread
	if (Init_threadIdle() != 0)
	{
		while(1)
		{		
				// Should not be here
		};
	}
	return osOK;
}

/// \fn osStatus osKernelStart (void)
/// \brief Start the RTOS Kernel.
/// \return status code that indicates the execution status of the function.
/// \note MUST REMAIN UNCHANGED: \b osKernelStart shall be consistent in every CMSIS-RTOS.
osStatus osKernelStart (void)
{
	kernel_running = 1;
	os_start();
	return osOK;
}

/// \fn int32_t osKernelRunning(void)
/// \brief Check if the RTOS kernel is already started.
/// \note MUST REMAIN UNCHANGED: \b osKernelRunning shall be consistent in every CMSIS-RTOS.
/// \return 0 RTOS is not started, 1 RTOS is started.
int32_t osKernelRunning(void)
{
	return kernel_running;
}

#if (defined (osFeature_SysTick)  &&  (osFeature_SysTick != 0))     // System Timer available

/// \fn uint32_t osKernelSysTick (void)
/// \brief Get the RTOS kernel system timer counter 
/// \note MUST REMAIN UNCHANGED: \b osKernelSysTick shall be consistent in every CMSIS-RTOS.
/// \return RTOS kernel system timer as 32-bit value 
uint32_t osKernelSysTick (void)
{
  return systick_count;
}

#endif    // System Timer available

/// \fn void os_KernelInvokeScheduler (void)
/// \brief Perform an SVC call to invoke scheduler.
void os_KernelInvokeScheduler (void)
{
	thread_yield();
  return ;
}

/// \fn void os_KernelStackAlloc (void)
/// \brief Perform an SVC call to allocate stack for a thread
/// \param thread_idx The thread index in the PSP table to initialize
void os_KernelStackAlloc (uint32_t thread_idx)
{
	stack_alloc(thread_idx);
  return ;
}


// -------------------------------------------------------------------------
/*! \fn __asm void SVC_Handler(void)
    \brief SVC exception handler
    \details Extracts the stack frame location, saves the current EXC_RETURN, 
    runs the C part of the handler, and restores the new EXC_RETURN.
*/
__asm void SVC_Handler(void)
{
  TST    LR, #4   // Extract stack frame location
  ITE    EQ
  MRSEQ  R0, MSP
  MRSNE  R0, PSP
  LDR    R1,=__cpp(&svc_exc_return) // Save current EXC_RETURN
  STR    LR,[R1]	
  BL     __cpp(SVC_Handler_C)       // Run C part of SVC_Handler
  LDR    R1,=__cpp(&svc_exc_return) // Load new EXC_RETURN
  LDR    LR,[R1]
  BX     LR
  ALIGN  4
}

/*! \fn void SVC_Handler_C(unsigned int * svc_args)
    \brief C part of the SVC exception handler

     SVC 0 is initializing the OS and starting the scheduler.
     Each thread stack frame is initialized.
     
    \param svc_args Used to extract the SVC number 
*/
void SVC_Handler_C(unsigned int * svc_args)
{
  uint8_t svc_number, i;	
  svc_number = ((char *) svc_args[6])[-2]; // Memory[(Stacked PC)-2]
  switch(svc_number) {
    case (0): // OS start		  
 	    // Starting the task scheduler
			// Update thread to be run based on priority
	    scheduler();
      curr_task = next_task; // Switch to head ready-to-run task (Current task)		
			th_q_h = curr_task;
			th_q[curr_task]->status = TH_RUNNING;
		  if (PSP_array[curr_task] == NULL)
			{
				//printf("ERROR: Stack not allocated for current task (task %u), allocating. \n\r", curr_task);
				i = curr_task;
				th_q[i]->stack_p = (uint32_t) task_stack[i];
				PSP_array[i] = ((unsigned int) th_q[i]->stack_p) + (th_q[i]->stack_size) - 18*4;
				HW32_REG((PSP_array[i] + (16<<2))) = (unsigned long) th_q[i]->start_p; // initial Program Counter
				HW32_REG((PSP_array[i] + (17<<2))) = 0x01000000;            // initial xPSR
				HW32_REG((PSP_array[i]          )) = 0xFFFFFFFDUL;          // initial EXC_RETURN
				HW32_REG((PSP_array[i] + ( 1<<2))) = 0x3;// initial CONTROL : unprivileged, PSP, no FP		
				
				th_q[i]->stack_p = PSP_array[i];				
			}

			svc_exc_return = HW32_REG((PSP_array[curr_task])); // Return to thread with PSP
			__set_PSP((PSP_array[curr_task] + 10*4));  // Set PSP to @R0 of task 0 exception stack frame

      NVIC_SetPriority(PendSV_IRQn, 0xFF);       // Set PendSV to lowest possible priority
      if (SysTick_Config(os_sysTickTicks) != 0)  // 1000 Hz SysTick interrupt on 16MHz core clock
			{
				//printf("ERROR: Impossible SysTick_Config number of ticks\n\r");
			}
      __set_CONTROL(0x3);                  // Switch to use Process Stack, unprivileged state
      __ISB();       // Execute ISB after changing CONTROL (architectural recommendation)			
		  break;
    case (1): // Thread Yield
			// Run scheduler to determine if a context switch is needed
			scheduler();		  
			if (curr_task != next_task)
			{ 
				// Context switching needed
				ScheduleContextSwitch();
		  }	
      //__set_CONTROL(0x3);                  // Switch to use Process Stack, unprivileged state			
      __ISB();       // Execute ISB after changing CONTROL (architectural recommendation)						
			break;
    case (2): // Stack Allocation
      // Create stack frame for thread
		  i = svc_args[0];  

			th_q[i]->stack_p = (uint32_t) task_stack[i];
			PSP_array[i] = ((unsigned int) th_q[i]->stack_p) + (th_q[i]->stack_size) - 18*4;
			HW32_REG((PSP_array[i] + (16<<2))) = (unsigned long) th_q[i]->start_p; // initial Program Counter
			HW32_REG((PSP_array[i] + (17<<2))) = 0x01000000;            // initial xPSR
			HW32_REG((PSP_array[i]          )) = 0xFFFFFFFDUL;          // initial EXC_RETURN
			HW32_REG((PSP_array[i] + ( 1<<2))) = 0x3;// initial CONTROL : unprivileged, PSP, no FP		
			
			th_q[i]->stack_p = PSP_array[i];
		
		  //__set_CONTROL(0x3);                  // Switch to use Process Stack, unprivileged state	
      __ISB();       // Execute ISB after changing CONTROL (architectural recommendation)						
			break;			
    default:
//      printf("ERROR: Unknown SVC service number\n\r");
//      printf("- SVC number 0x%x\n\r", svc_number);
      stop_cpu2;
      break;
  } // end switch
}	

// -------------------------------------------------------------------------
/*! \fn __asm void PendSV_Handler(void)
    \brief Handles context switch.

     Saves the current process context (stack, registers, pointer to stack).
     Loads the next process context.
*/
__asm void PendSV_Handler(void)
{ 
  // Save current context
  MRS      R0, PSP     // Get current process stack pointer value
  TST      LR, #0x10   // Test bit 4. If zero, need to stack floating point regs
  IT       EQ
  VSTMDBEQ R0!, {S16-S31} // Save floating point registers
  MOV      R2, LR
  MRS      R3, CONTROL
  STMDB    R0!,{R2-R11}// Save LR,CONTROL and R4 to R11 in task stack (10 regs)
  LDR      R1,=__cpp(&curr_task)
  LDR      R2,[R1]     // Get current task ID
  LDR      R3,=__cpp(&PSP_array)
  STR      R0,[R3, R2, LSL #2] // Save PSP value into PSP_array
  // -------------------------
  // Load next context
  LDR      R4,=__cpp(&next_task)
  LDR      R4,[R4]     // Get next task ID
  STR      R4,[R1]     // Set curr_task = next_task
  LDR      R0,[R3, R4, LSL #2] // Load PSP value from PSP_array
  LDMIA    R0!,{R2-R11}// Load LR, CONTROL and R4 to R11 from task stack (10 regs)
  MOV      LR, R2
  MSR      CONTROL, R3
  TST      LR, #0x10   // Test bit 4. If zero, need to unstack floating point regs
  IT       EQ
  VLDMIAEQ R0!, {S16-S31} // Save floating point registers
  MSR      PSP, R0     // Set PSP to next task
  BX       LR          // Return
  ALIGN  4
}
// -------------------------------------------------------------------------
/*! \fn void SysTick_Handler(void)
    \brief Invokes the scheduler

     Increment systick counter, invoke scheduler and flag any context switching needed.
*/
void SysTick_Handler(void) // 1KHz
{ // Increment systick counter 
  systick_count++;
	// Run scheduler to determine if a context switch is needed
  scheduler();
  if (curr_task != next_task)
	{ 
		// Context switching needed
    ScheduleContextSwitch();
  }
  return;	
}

/*! \fn void ScheduleContextSwitch(void)
    \brief Schedules a context switch

     Sets PendSV to pending.
*/
void ScheduleContextSwitch(void)
{
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set PendSV to pending
	return;
}


// -------------------------------------------------------------------------
/*! \fn __asm void HardFault_Handler(void)
    \brief Report HardFault when things goes wrong
*/
__asm void HardFault_Handler(void) 
{
  TST    LR, #4
  ITE    EQ
  MRSEQ  R0, MSP
  MRSNE  R0, PSP
  B      __cpp(HardFault_Handler_C)
}	

/*! \fn void HardFault_Handler_C(unsigned int * svc_args)
    \brief C part of the handler

     Printing process ID, processes stacks and stacked PC
    
    \param svc_args Stack
*/
void HardFault_Handler_C(unsigned int * svc_args)
{
//	uint32_t i;
	
//  printf("[HardFault]\n\r");
//	printf("Buffered trace:\n\r");
//	dumpTrace();
//	printf("Environment:\n\r");
//  printf ("curr_task = %d\n\r", curr_task);
//  printf ("next_task = %d\n\r", next_task);
//	for ( i = 0; i < MAX_THREADS ; i++ )
//	{
//		printf ("PSP # %d = %x\n\r",i, PSP_array[i]);
//	}
//  printf ("Stacked PC = %x\n\r", svc_args[6]);
  stop_cpu2;	
}
// -------------------------------------------------------------------------


