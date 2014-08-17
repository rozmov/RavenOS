/*! \file kernel.c
    \brief Implements kernel level operations.
	  \details Maps operations to CMSIS interface. Performs scheduling and context switching.
*/

#include "TM4C123.h"
#include "stdio.h"
#include "osObjects.h"                      // RTOS object definitions
#include "kernel.h"
#include "scheduler.h"

/*! \def HW32_REG(ADDRESS) 
         Macros for word accesses. */
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
	
#define os_sysTickTicks 16000 ///< Number of ticks between two system timer interrupts. This would generate 1000 interruts/s on a 16MHz clock.

void __svc(0x00) os_start(void); ///< OS start scheduler
void __svc(0x01) thread_yield(void); ///< Thread needs to schedule a switch of context
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
	// nothing for now
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
      // The OS is not running at this point
		  
      // Create stack frame for threads
		  for (i = 0 ; i < th_q_cnt ;i++)
			{
				th_q[i]->stack_p = (uint32_t) task_stack[i];
//				PSP_array[i] = ((unsigned int) rtr_q[i]->stack_p) + (sizeof task_stack[i]) - 18*4;
				PSP_array[i] = ((unsigned int) th_q[i]->stack_p) + (th_q[i]->stack_size) - 18*4;
				HW32_REG((PSP_array[i] + (16<<2))) = (unsigned long) th_q[i]->start_p; // initial Program Counter
				HW32_REG((PSP_array[i] + (17<<2))) = 0x01000000;            // initial xPSR
				HW32_REG((PSP_array[i]          )) = 0xFFFFFFFDUL;          // initial EXC_RETURN
				HW32_REG((PSP_array[i] + ( 1<<2))) = 0x3;// initial CONTROL : unprivileged, PSP, no FP		
				
				th_q[i]->stack_p = PSP_array[i];
			}
//      // Create stack frame for task0
//      PSP_array[0] = ((unsigned int) task0_stack) + (sizeof task0_stack) - 18*4;
//      HW32_REG((PSP_array[0] + (16<<2))) = (unsigned long) thread0; // initial Program Counter
//      HW32_REG((PSP_array[0] + (17<<2))) = 0x01000000;            // initial xPSR
//      HW32_REG((PSP_array[0]          )) = 0xFFFFFFFDUL;          // initial EXC_RETURN
//      HW32_REG((PSP_array[0] + ( 1<<2))) = 0x3;// initial CONTROL : unprivileged, PSP, no FP

//      // Create stack frame for task1
//      PSP_array[1] = ((unsigned int) task1_stack) + (sizeof task1_stack) - 18*4;
//      HW32_REG((PSP_array[1] + (16<<2))) = (unsigned long) thread1; // initial Program Counter
//      HW32_REG((PSP_array[1] + (17<<2))) = 0x01000000;            // initial xPSR
//      HW32_REG((PSP_array[1]          )) = 0xFFFFFFFDUL;          // initial EXC_RETURN
//      HW32_REG((PSP_array[1] + ( 1<<2))) = 0x3;// initial CONTROL : unprivileged, PSP, no FP

//      // Create stack frame for task2
//      PSP_array[2] = ((unsigned int) task2_stack) + (sizeof task2_stack) - 18*4;
//      HW32_REG((PSP_array[2] + (16<<2))) = (unsigned long) thread2; // initial Program Counter
//      HW32_REG((PSP_array[2] + (17<<2))) = 0x01000000;            // initial xPSR
//      HW32_REG((PSP_array[2]          )) = 0xFFFFFFFDUL;          // initial EXC_RETURN
//      HW32_REG((PSP_array[2] + ( 1<<2))) = 0x3;// initial CONTROL : unprivileged, PSP, no FP

//      // Create stack frame for task3
//      PSP_array[3] = ((unsigned int) task3_stack) + (sizeof task3_stack) - 18*4;
//      HW32_REG((PSP_array[3] + (16<<2))) = (unsigned long) thread3; // initial Program Counter
//      HW32_REG((PSP_array[3] + (17<<2))) = 0x01000000;            // initial xPSR
//      HW32_REG((PSP_array[3]          )) = 0xFFFFFFFDUL;          // initial EXC_RETURN
//      HW32_REG((PSP_array[3] + ( 1<<2))) = 0x3;// initial CONTROL : unprivileged, PSP, no FP
	
 	    // Starting the task scheduler
//      curr_task = 0; // Switch to task #0 (Current task)

			// Update thread to be run based on priority
	    scheduler();
      curr_task = next_task; // Switch to head ready-to-run task (Current task)		
			th_q_h = curr_task;
			th_q[curr_task]->status = TH_RUNNING;
      svc_exc_return = HW32_REG((PSP_array[curr_task])); // Return to thread with PSP
      __set_PSP((PSP_array[curr_task] + 10*4));  // Set PSP to @R0 of task 0 exception stack frame
      NVIC_SetPriority(PendSV_IRQn, 0xFF); // Set PendSV to lowest possible priority
      if (SysTick_Config(os_sysTickTicks) == 0)   // 1000 Hz SysTick interrupt on 16MHz core clock
			{
				printf("ERROR: Impossible SysTick_Config number of ticks");
			}
      __set_CONTROL(0x3);                  // Switch to use Process Stack, unprivileged state
      __ISB();       // Execute ISB after changing CONTROL (architectural recommendation)			
		  break;
    case (1): // Thread Yield
			//next_task = rtr_q_h;
			// Run scheduler to determine if a context switch is needed
			scheduler();		  
			if (curr_task!=next_task)
			{ 
				// Context switching needed
				ScheduleContextSwitch();
			}			
			break;
    default:
      printf("ERROR: Unknown SVC service number");
      printf("- SVC number 0x%x\n", svc_number);
      stop_cpu;
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
  if (curr_task!=next_task)
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
  printf("[HardFault]");
  printf ("curr_task = %d\n", curr_task);
  printf ("next_task = %d\n", next_task);
  printf ("PSP #0 = %x\n", PSP_array[0]);
  printf ("PSP #1 = %x\n", PSP_array[1]);
  printf ("PSP #2 = %x\n", PSP_array[2]);
  printf ("Stacked PC = %x\n", svc_args[6]);
  stop_cpu;	
}
// -------------------------------------------------------------------------


