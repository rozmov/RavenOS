/*! \file main.c
    \brief This header file defines all objects when included in a C/C++ source file
		\details 
*/

#ifndef _PERIPHERALS_H
#define _PERIPHERALS_H

#include <stdint.h>

#define LED0              0
#define LED1              1
#define LED2              2


/*! Prototypes */
void LED_initialize(void); ///< Initialize LED
void LED_blink( uint32_t led ); ///< Blink LED
void UART_initialize(void); ///< Initialize UART

void count1Sec(void); ///< 1s busy incrementing a counter

#endif //_PERIPHERALS_H
