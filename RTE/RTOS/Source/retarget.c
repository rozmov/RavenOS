/*!
 *****************************************************************************
 ** @file RETARGET.C
 ** @brief 'Retarget' layer for target-dependent low level functions      
 ** @copyright {
 This file is part of the uVision/ARM development tools.                    
 Copyright (c) 2005-2009 Keil Software. All rights reserved.                
 This software may only be used under the terms of a valid, current,        
 end user licence from KEIL for a compatible version of KEIL software       
 development tools. Nothing else gives you the right to use this software.  
 }
******************************************************************************/

#include <stdio.h>
#include <time.h>
#include <rt_misc.h>
#include <stdint.h>
#include "CU_TM4C123.h"
#include "USART_TM4C123.h"
#pragma import(__use_no_semihosting_swi)

/// stdio retargeting handle
struct __FILE { int handle; /*!< file handle */ };
FILE __stdout; /*!< standard output */
FILE __stdin;  /*!< standard input */

static uint8_t c; ///< using a global to pass around between stdlib and UART driver

/*! \fn void UART0_putc(char ch)
    \brief Output a character to UART
    \param ch The character to print
*/
void UART0_putc(char ch)
{ 
	c = (uint8_t) ch;
  Driver_USART0.Send(&c, 1);
  return;
}

/*! \fn char UART0_getc(void)
    \brief Read a character from UART
    \return Character read
*/
char UART0_getc(void)
{ 
	return ROM_UARTCharGet(UART0_BASE);
}

/*! \fn int fputc(int ch, FILE *f)
    \brief Re-targeting the print
    \param ch The character to print
    \param *f pointer to file descriptor (ex. standard output)
    \return Character read
*/
int fputc(int ch, FILE *f) 
{
  UART0_putc(ch);
  return (ch);
}

/*! \fn int fgetc(FILE *f)
    \brief Re-targeting the input
    \param *f pointer to file descriptor (ex. standard input)
    \return Character read
*/
int fgetc(FILE *f) 
{
  return ((int) (UART0_getc()));
}

/*! \fn int ferror(FILE *f)
    \brief Re-targeting the error (none for now)
    \param *f pointer to file descriptor (ex. standard input)
    \return Error - EOF
*/
int ferror(FILE *f) 
{
  /* Your implementation of ferror */
  return EOF;
}

/*! \fn void _ttywrch(int ch) 
    \brief Re-targeting the print
    \param ch The character to print
*/
void _ttywrch(int ch) 
{
  UART0_putc(ch);
}

/*! \fn void _sys_exit(int return_code)
    \brief Re-targeting the system exit function to an infinite loop
    \param return_code Exit code
*/
void _sys_exit(int return_code) 
{
label:  goto label;  /* endless loop */
}
