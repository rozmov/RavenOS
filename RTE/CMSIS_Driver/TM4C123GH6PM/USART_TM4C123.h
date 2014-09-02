/* -----------------------------------------------------------------------------
 * Copyright (c) 2013-2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        15. May 2014
 * $Revision:    V2.01
 *
 * Project:      USART Driver Definitions for TM4C123
 * -------------------------------------------------------------------------- */

#ifndef __USART_TM4C123_H
#define __USART_TM4C123_H

#include "Driver_USART.h"
#include "RTE_Device.h"
#include "CU_TM4C123GH6PM.h"            // CU_SYSC4906::Device:Startup
//#include "TM4C123GH6PM.h"	// Include this file if using the CMSIS Device standard definitions

/* USART Driver */
extern ARM_DRIVER_USART Driver_USART0;
 
// USART FIFO control register
#define USART_FCR_FIFOEN             (1 << 0)
#define USART_FCR_RXFIFORES          (1 << 1)
#define USART_FCR_TXFIFORES          (1 << 2)
#define USART_FCR_DMAMODE            (1 << 3)
#define USART_FCR_RXTRIGLVL_POS      (     6)
#define USART_FCR_RXTRIGLVL_MSK      (3 << USART_FCR_RXTRIGLVL_POS)


// USART Line control register
#define USART_LCR_WLS_POS            (     0)
#define USART_LCR_WLS_MSK            (3 << USART_LCR_WLS_POS)
#define USART_LCR_SBS                (1 << 2)
#define USART_LCR_PE                 (1 << 3)
#define USART_LCR_PS_POS             (     4)
#define USART_LCR_PS_MSK             (3 << USART_LCR_PS_POS)
#define USART_LCR_BC                 (1 << 6)
#define USART_LCR_DLAB               (1 << 7)

// USART IrDA control register
#define USART_ICR_IRDAEN             (1 << 0)
#define USART_ICR_FIXPULSEEN         (1 << 1)
#define USART_ICR_IRDAINV            (1 << 2)
#define USART_ICR_PULSEDIV_POS       (     3)
#define USART_ICR_PULSEDIV_MSK       (7 << USART_ICR_PULSEDIV_POS)

// USART Synchronous mode control register
#define USART_SYNCCTRL_SYNC          (1 << 0)
#define USART_SYNCCTRL_CSRC          (1 << 1)
#define USART_SYNCCTRL_FES           (1 << 2)
#define USART_SYNCCTRL_TSBYPASS      (1 << 3)
#define USART_SYNCCTRL_CSCEN         (1 << 4)
#define USART_SYNCCTRL_SSSDIS        (1 << 5)
#define USART_SYNCCTRL_CCCLR         (1 << 6)

// UART Modem control register
#define UART_MCR_DTRCTRL             (1 << 0)
#define UART_MCR_RTSCTRL             (1 << 1)
#define UART_MCR_LMS                 (1 << 4)
#define UART_MCR_RTSEN               (1 << 6)
#define UART_MCR_CTSEN               (1 << 7)

// UART Modem status register
#define UART_MSR_DCTS                (1 << 0)
#define UART_MSR_DDSR                (1 << 1)
#define UART_MSR_TERI                (1 << 2)
#define UART_MSR_DDCD                (1 << 3)
#define UART_MSR_CTS                 (1 << 4)
#define UART_MSR_DSR                 (1 << 5)
#define UART_MSR_RI                  (1 << 6)
#define UART_MSR_DCD                 (1 << 7)

// USART Transmitter enable register
#define USART_TER_TXEN               (1 << 0)


// USART flags
#define USART_FLAG_INITIALIZED       (1 << 0)
#define USART_FLAG_POWERED           (1 << 1)
#define USART_FLAG_CONFIGURED        (1 << 2)
#define USART_FLAG_TX_ENABLED        (1 << 3)
#define USART_FLAG_RX_ENABLED        (1 << 4)
#define USART_FLAG_SEND_ACTIVE       (1 << 5)

// USART synchronous xfer modes
#define USART_SYNC_MODE_TX           ( 1 )
#define USART_SYNC_MODE_RX           ( 2 )
#define USART_SYNC_MODE_TX_RX        (USART_SYNC_MODE_TX | \
                                      USART_SYNC_MODE_RX)

// Baudrate accepted error
#define UART_ACCEPTED_BAUDRATE_ERROR ( 3 )

// USART TX FIFO trigger level
#define USART_TRIG_LVL_1             (0x00)
#define USART_TRIG_LVL_4             (0x40)
#define USART_TRIG_LVL_8             (0x80)
#define USART_TRIG_LVL_14            (0xC0)

// USART Transfer Information (Run-Time)
typedef struct _USART_TRANSFER_INFO {
  uint32_t                rx_num;        // Total number of data to be received
  uint32_t                tx_num;        // Total number of data to be send
  uint8_t                *rx_buf;        // Pointer to in data buffer
  uint8_t                *tx_buf;        // Pointer to out data buffer
  uint32_t                rx_cnt;        // Number of data received
  uint32_t                tx_cnt;        // Number of data sent
  uint8_t                 tx_def_val;    // Transmit default value (used in USART_SYNC_MASTER_MODE_RX)
  uint8_t                 rx_dump_val;   // Receive dump value (used in USART_SYNC_MASTER_MODE_TX)
  uint8_t                 sync_mode;     // Synchronous mode
} USART_TRANSFER_INFO;

// USART Information (Run-Time)
typedef struct _USART_INFO {
  ARM_USART_SignalEvent_t cb_event;      // Event callback
  ARM_USART_STATUS        status;        // Status flags
  USART_TRANSFER_INFO     xfer;          // Transfer information
  uint8_t                 mode;          // USART mode
  uint8_t                 flags;         // USART driver flags
  uint32_t                baudrate;      // Baudrate
} USART_INFO;

// PIN struct

typedef const struct _PIN_ID {
	uint32_t                pin;
	uint32_t                base;
	uint32_t                bitfield;
} PIN_ID;

// USART Pin Configuration
typedef const struct _USART_PINS {
  PIN_ID                  tx;            // TX  Pin identifier
  PIN_ID                  rx;            // RX  Pin identifier
} USART_PINS;

// USART Clocks Configuration
typedef const struct _USART_CLOCK {
  uint32_t                base;       // USART peripheral clock configuration register
  uint32_t                periph;     // USART peripheral clock status register
} USART_CLOCKS;


// USART Resources definitions
typedef struct {
  ARM_USART_CAPABILITIES  capabilities;  // Capabilities
	uint32_t                periph_uart;   // UART peripheral
	UART0_Type             *reg;	         // Pointer to UART peripheral		
  USART_PINS              pins;          // USART pins configuration
  USART_CLOCKS            clk;           // USART clock peripheral	
  IRQn_Type               irq_num;       // USART IRQ Number
  uint32_t                trig_lvl;      // FIFO Trigger level
  USART_INFO             *info;          // Run-Time Information  
} const USART_RESOURCES;

#endif /* __USART_TM4C123_H */
