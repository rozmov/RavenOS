/*! \file peripherals.h
    \brief This header file defines peripheral APIs, tools and constants.
*/

#ifndef _PERIPHERALS_H
#define _PERIPHERALS_H

#include <stdint.h>

#define LED0              0  ///< External definition to red colour of the LED
#define LED1              1  ///< External definition to green colour of the LED
#define LED2              2  ///< External definition to blue colour of the LED

/*! Prototypes */
void LED_initialize(void);      ///< Initialize LED
void LED_blink( uint32_t led ); ///< Blink LED
void UART_initialize(void);     ///< Initialize UART

void count1Sec(void);           ///< 1s busy incrementing a counter

#endif //_PERIPHERALS_H
