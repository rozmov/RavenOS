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
 * Driver:       Driver_USART0
 * Configured:   via RTE_Device.h configuration file
 * Project:      USART Driver for TI TM4C123
 * -----------------------------------------------------------------------------
 * Use the following configuration settings in the middleware component
 * to connect to this driver.
 *
 *   Configuration Setting                Value     UART Interface
 *   ---------------------                -----     --------------
 *   Connect to hardware via Driver_UART# = 0       use USART0
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 2.01
 *    - Added DMA support
 *    - Other Improvements (status checking, USART_Control, ...)
 *  Version 2.00
 *    - Updated to CMSIS Driver API V2.00
 *  Version 1.01
 *    - Based on API V1.10 (namespace prefix ARM_ added)
 *  Version 1.00
 *    - Initial release
 */

#include "USART_TM4C123.h"
#include "RTE_Components.h"


#define ARM_USART_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(2,01)

// Driver Version
static const ARM_DRIVER_VERSION usart_driver_version = { ARM_USART_API_VERSION, ARM_USART_DRV_VERSION };

// Trigger level definitions
// Can be user defined by C preprocessor
#ifndef USART0_TRIG_LVL
#define USART0_TRIG_LVL           USART_TRIG_LVL_1
#endif

// USART0
//#define RTE_USART0 1
static USART_INFO USART0_Info = {0};

static const USART_RESOURCES USART0_Resources = {
  {     // Capabilities
    1,  // supports UART (Asynchronous) mode
#if (RTE_USART0_UCLK_PIN_EN == 1)
    1,  // supports Synchronous Master mode
    1,  // supports Synchronous Slave mode
#else
    0,  // supports Synchronous Master mode
    0,  // supports Synchronous Slave mode
#endif
    0,  // supports UART Single-wire mode
    0,  // supports UART IrDA mode
    0,  // supports UART Smart Card mode
#if (RTE_USART0_UCLK_PIN_EN == 1)
    1,  // Smart Card Clock generator
#else
    0,
#endif
    0,  // RTS Flow Control available
    0,  // CTS Flow Control available
    0,  // Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
#if (RTE_USART0_DMA_RX_EN == 1)
    0,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
#else
    1,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
#endif
    0,  // RTS Line: 0=not available, 1=available
    0,  // CTS Line: 0=not available, 1=available
    0,  // DTR Line: 0=not available, 1=available
    0,  // DSR Line: 0=not available, 1=available
    0,  // DCD Line: 0=not available, 1=available
    0,  // RI Line: 0=not available, 1=available
    0,  // Signal CTS change event: \ref ARM_USART_EVENT_CTS
    0,  // Signal DSR change event: \ref ARM_USART_EVENT_DSR
    0,  // Signal DCD change event: \ref ARM_USART_EVENT_DCD
    0,  // Signal RI change event: \ref ARM_USART_EVENT_RI
  },
	  SYSCTL_PERIPH_UART0,
	  UART0,
	{
		{
			GPIO_PA1_U0TX,
			GPIOA_BASE,
			GPIO_PIN_1
		},
		{
			GPIO_PA0_U0RX,
			GPIOA_BASE,
			GPIO_PIN_0
		}
	},
    {
			UART0_BASE,
			SYSCTL_PERIPH_GPIOA
		},
    UART0_IRQn,
    USART0_TRIG_LVL,
    &USART0_Info
};


// Local Function
/**
  \fn          int32_t USART_SetBaudrate (uint32_t         baudrate,
                                          USART_RESOURCES *usart)
  \brief       Set baudrate
  \param[in]   baudrate  Usart baudrate
  \param[in]   usart     Pointer to USART resources
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t USART_SetBaudrate (uint32_t         baudrate,
                           USART_RESOURCES *usart) {

    ROM_UARTConfigSetExpClk( usart->clk.base, ROM_SysCtlClockGet(), baudrate,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
														 
    usart->info->baudrate = baudrate;
  return 0;
}

// Function Prototypes
static int32_t USART_Receive (void            *data,
                              uint32_t         num,
                              USART_RESOURCES *usart);

// USART Driver functions

/**
  \fn          ARM_DRIVER_VERSION USARTx_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION USARTx_GetVersion (void) {
  return usart_driver_version;
}

/**
  \fn          ARM_USART_CAPABILITIES USART_GetCapabilities (USART_RESOURCES *usart)
  \brief       Get driver capabilities
  \param[in]   usart     Pointer to USART resources
  \return      \ref ARM_USART_CAPABILITIES
*/
static ARM_USART_CAPABILITIES USART_GetCapabilities (USART_RESOURCES *usart) {
  return usart->capabilities;
}

/**
  \fn          int32_t USART_Initialize (ARM_USART_SignalEvent_t  cb_event
                                         USART_RESOURCES         *usart)
  \brief       Initialize USART Interface.
  \param[in]   cb_event  Pointer to \ref ARM_USART_SignalEvent
  \param[in]   usart     Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Initialize (ARM_USART_SignalEvent_t  cb_event,
                                 USART_RESOURCES         *usart) {

  if (usart->info->flags & USART_FLAG_POWERED) {
    // Device is powered - could not be re-initialized
    return ARM_DRIVER_ERROR;
  }

  if (usart->info->flags & USART_FLAG_INITIALIZED) {
    // Driver is already initialized
    return ARM_DRIVER_OK;
  }

  // Initialize USART Run-time Resources
  usart->info->cb_event = cb_event;

  usart->info->status.tx_busy          = 0;
  usart->info->status.rx_busy          = 0;
  usart->info->status.tx_underflow     = 0;
  usart->info->status.rx_overflow      = 0;
  usart->info->status.rx_break         = 0;
  usart->info->status.rx_framing_error = 0;
  usart->info->status.rx_parity_error  = 0;

  usart->info->mode = 0;
  usart->info->xfer.tx_def_val = 0;

	//
	// Enable Peripheral Clocks 
	//
	MAP_SysCtlPeripheralEnable(usart->periph_uart);
	MAP_SysCtlPeripheralEnable(usart->clk.periph);

	//
	// Enable pin PA0 for UART0 U0RX
	//
	MAP_GPIOPinConfigure(usart->pins.rx.pin);
	MAP_GPIOPinTypeUART(usart->pins.rx.base, usart->pins.rx.bitfield);

	//
	// Enable pin PA1 for UART0 U0TX
	//
	MAP_GPIOPinConfigure(usart->pins.tx.pin);
	MAP_GPIOPinTypeUART(usart->pins.tx.base, usart->pins.tx.bitfield);
	
	
//    // \note for potential interrupt support
//    // Enable processor interrupts.
//    //
//    ROM_IntMasterEnable();

//    //
//    // Enable the UART interrupt.
//    //
//    ROM_IntEnable(INT_UART0);
//    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
	
  usart->info->flags = USART_FLAG_INITIALIZED;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_Uninitialize (USART_RESOURCES *usart)
  \brief       De-initialize USART Interface.
  \param[in]   usart     Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Uninitialize (USART_RESOURCES *usart) {

  if (usart->info->flags & USART_FLAG_POWERED) {
    // Driver is powered - could not be uninitialized
    return ARM_DRIVER_ERROR;
  }

  if (usart->info->flags == 0) {
    // Driver not initialized
    return ARM_DRIVER_OK;
  }

  // Reset USART status flags
  usart->info->flags = 0;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_PowerControl (ARM_POWER_STATE state)
  \brief       Control USART Interface Power.
  \param[in]   state  Power state
  \param[in]   usart  Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_PowerControl (ARM_POWER_STATE  state,
                                   USART_RESOURCES *usart) {

  if ((usart->info->flags & USART_FLAG_INITIALIZED) == 0) {
    // Return error, if USART is not initialized
    return ARM_DRIVER_ERROR;
  }

  if (usart->info->status.rx_busy == 1) {
    // Receive busy
    return ARM_DRIVER_ERROR_BUSY;
  }

  if (usart->info->flags & USART_FLAG_SEND_ACTIVE) {
    // Transmit busy
    return ARM_DRIVER_ERROR_BUSY;
  }

  switch (state) {
    case ARM_POWER_OFF:
      if ((usart->info->flags & USART_FLAG_POWERED) == 0)
        return ARM_DRIVER_OK;

//      // Disable USART IRQ			
			ROM_UARTIntDisable(usart->clk.base, UART_INT_RX | UART_INT_RT);

      usart->info->flags = USART_FLAG_INITIALIZED;
      break;

    case ARM_POWER_LOW:
      return ARM_DRIVER_ERROR_UNSUPPORTED;

    case ARM_POWER_FULL:
      if (usart->info->flags & USART_FLAG_POWERED)
        return ARM_DRIVER_OK;

      usart->info->flags = USART_FLAG_POWERED | USART_FLAG_INITIALIZED;

      // Clear and Enable USART IRQ
//      ROM_UARTIntClear(usart->clk.base, UART_INT_RX | UART_INT_RT);
//      ROM_UARTIntEnable(usart->clk.base, UART_INT_RX | UART_INT_RT);

      break;

    default: return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_Send (const void            *data,
                                         uint32_t         num,
                                         USART_RESOURCES *usart)
  \brief       Start sending data to USART transmitter.
  \param[in]   data  Pointer to buffer with data to send to USART transmitter
  \param[in]   num   Number of data items to send
  \param[in]   usart Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Send (const void            *data,
                                 uint32_t         num,
                                 USART_RESOURCES *usart) {

  if ((data == NULL) || (num == 0)) {
    // Invalid parameters
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0) {
    // USART is not configured (mode not selected)
    return ARM_DRIVER_ERROR;
  }

  if (usart->info->flags & USART_FLAG_SEND_ACTIVE) {
    // Send is not completed yet
    return ARM_DRIVER_ERROR_BUSY;
  }

  // Set Send active flag
  usart->info->flags |= USART_FLAG_SEND_ACTIVE;

  // Save transmit buffer info
  usart->info->xfer.tx_buf = (uint8_t *)data;
  usart->info->xfer.tx_num = num;
  usart->info->xfer.tx_cnt = 0;

  //
  // Loop while there are more characters to send.
  //
  while(num--)
  {
    //
    // Write the next character to the UART.
    //	
		__disable_irq();
		ROM_UARTCharPut(usart->clk.base, usart->info->xfer.tx_buf[usart->info->xfer.tx_cnt++]);
		__DMB();
		__enable_irq();
		ROM_SysCtlDelay(ROM_SysCtlClockGet() / (1000 * 3));
  }	

  // Clear Send active flag
  usart->info->flags &= ~USART_FLAG_SEND_ACTIVE ;
		
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_Receive (void            *data,
                                      uint32_t         num,
                                      USART_RESOURCES *usart)
  \brief       Start receiving data from USART receiver. - Prototype
  \param[out]  data  Pointer to buffer for data to receive from USART receiver
  \param[in]   num   Number of data items to receive
  \param[in]   usart Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Receive (void            *data,
                              uint32_t         num,
                              USART_RESOURCES *usart) {

  if ((data == NULL) || (num == 0)) {
    // Invalid parameters
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0) {
    // USART is not configured (mode not selected)
    return ARM_DRIVER_ERROR;
  }

  // Check if receiver is busy
  if (usart->info->status.rx_busy == 1) 
    return ARM_DRIVER_ERROR_BUSY;

  // Set RX busy flag
  usart->info->status.rx_busy = 1;

  // Save number of data to be received
  usart->info->xfer.rx_num = num;

  // Clear RX statuses
  usart->info->status.rx_break          = 0;
  usart->info->status.rx_framing_error  = 0;
  usart->info->status.rx_overflow       = 0;
  usart->info->status.rx_parity_error   = 0;

  // Save receive buffer info
  usart->info->xfer.rx_buf = (uint8_t *)data;
  usart->info->xfer.rx_cnt =            0;

  // Synchronous mode
  if ((usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_MASTER) ||
      (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {
    if (usart->info->xfer.sync_mode == USART_SYNC_MODE_TX) {
      // Dummy DMA reads (do not increment destination address)
    }
  }

  // DMA mode
  if (usart->reg->DMACTL) {

  // Interrupt mode
  } else {
    // Enable receive data available interrupt
  }
	
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_Transfer (const void             *data_out,
                                             void             *data_in,
                                             uint32_t          num,
                                             USART_RESOURCES  *usart)
  \brief       Start sending/receiving data to/from USART transmitter/receiver. - Prototype
  \param[in]   data_out  Pointer to buffer with data to send to USART transmitter
  \param[out]  data_in   Pointer to buffer for data to receive from USART receiver
  \param[in]   num       Number of data items to transfer
  \param[in]   usart     Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Transfer (const void             *data_out,
                                     void             *data_in,
                                     uint32_t          num,
                                     USART_RESOURCES  *usart) {
  int32_t status;

  if ((data_out == NULL) || (data_in == NULL) || (num == 0)) {
    // Invalid parameters
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0) {
    // USART is not configured
    return ARM_DRIVER_ERROR;
  }

  if ((usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_MASTER) ||
      (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {

    // Set xfer mode
    usart->info->xfer.sync_mode = USART_SYNC_MODE_TX_RX;

    // Receive
    status = USART_Receive (data_in, num, usart);
    if (status != ARM_DRIVER_OK) return status;

    // Send
    status = USART_Send (data_out, num, usart);
    if (status != ARM_DRIVER_OK) return status;

  } else {
    // Only in synchronous mode
    return ARM_DRIVER_ERROR;
  }
  return ARM_DRIVER_OK;
}

/**
  \fn          uint32_t USART_GetTxCount (USART_RESOURCES *usart)
  \brief       Get transmitted data count.
  \param[in]   usart     Pointer to USART resources
  \return      number of data items transmitted
*/
static uint32_t USART_GetTxCount (USART_RESOURCES *usart) {
  return usart->info->xfer.tx_cnt;
}

/**
  \fn          uint32_t USART_GetRxCount (USART_RESOURCES *usart)
  \brief       Get received data count.
  \param[in]   usart     Pointer to USART resources
  \return      number of data items received
*/
static uint32_t USART_GetRxCount (USART_RESOURCES *usart) {
  return usart->info->xfer.rx_cnt;
}

/**
  \fn          int32_t USART_Control (uint32_t          control,
                                      uint32_t          arg,
                                      USART_RESOURCES  *usart)
  \brief       Control USART Interface. - Prototype (partially implemented)
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \param[in]   usart    Pointer to USART resources
  \return      common \ref execution_status and driver specific \ref usart_execution_status
*/
static int32_t USART_Control (uint32_t          control,
                              uint32_t          arg,
                              USART_RESOURCES  *usart) {
  uint32_t mode;
  uint32_t mcr;

  if ((usart->info->flags & USART_FLAG_POWERED) == 0) {
    // USART not powered
    return ARM_DRIVER_ERROR;
  }

  switch (control & ARM_USART_CONTROL_Msk) {
    case ARM_USART_MODE_ASYNCHRONOUS:
      mode = ARM_USART_MODE_ASYNCHRONOUS;
      break;
    case ARM_USART_MODE_SYNCHRONOUS_MASTER:
      if (usart->capabilities.synchronous_master) {
        // Enable synchronous master (SCLK out) mode
      } else return ARM_USART_ERROR_MODE;
      mode = ARM_USART_MODE_SYNCHRONOUS_MASTER;
      break;
    case ARM_USART_MODE_SYNCHRONOUS_SLAVE:
      if (usart->capabilities.synchronous_slave) {
        // Enable synchronous slave (SCLK in) mode
      } else return ARM_USART_ERROR_MODE;
      mode = ARM_USART_MODE_SYNCHRONOUS_SLAVE;
      break;
    case ARM_USART_MODE_IRDA:
      if (usart->capabilities.irda) {
        // Enable IrDA mode
      } else return ARM_USART_ERROR_MODE;
      mode = ARM_USART_MODE_IRDA;
      break;
    // Default TX value
    case ARM_USART_SET_DEFAULT_TX_VALUE:
      usart->info->xfer.tx_def_val = arg;
      return ARM_DRIVER_OK;

    // IrDA pulse
    case ARM_USART_SET_IRDA_PULSE:
      if (usart->capabilities.irda) {
        if (arg == 0) {
          usart->reg->ICR &= ~(USART_ICR_FIXPULSEEN);
        } else {

        }
      } else return ARM_DRIVER_ERROR;
      return ARM_DRIVER_OK;

    // Control TX
    case ARM_USART_CONTROL_TX:
      if (arg) {
        if (usart->info->mode != ARM_USART_MODE_SMART_CARD) {
          // USART TX pin function selected
        }
        usart->info->flags |= USART_FLAG_TX_ENABLED;
      } else {
        usart->info->flags &= ~USART_FLAG_TX_ENABLED;
        if (usart->info->mode != ARM_USART_MODE_SMART_CARD) {
          // GPIO pin function selected

        }
      }
      return ARM_DRIVER_OK;

    // Control RX
    case ARM_USART_CONTROL_RX:
      // RX Line interrupt enable (overrun, framing, parity error, break)
      return ARM_DRIVER_OK;

    // Control break
    case ARM_USART_CONTROL_BREAK:
      return ARM_DRIVER_OK;

    // Abort Send
    case ARM_USART_ABORT_SEND:
      // Disable transmit holding register empty interrupt

      // Clear Send active flag
      usart->info->flags &= ~USART_FLAG_SEND_ACTIVE;
      return ARM_DRIVER_OK;

    // Abort receive
    case ARM_USART_ABORT_RECEIVE:
      // Disable receive data available interrupt

      // Clear RX busy status
      usart->info->status.rx_busy = 0;
      return ARM_DRIVER_OK;

    // Abort transfer
    case ARM_USART_ABORT_TRANSFER:
     // Disable transmit holding register empty and 
      // receive data available interrupts

      // Clear busy statuses
      usart->info->status.rx_busy = 0;
      usart->info->flags &= ~USART_FLAG_SEND_ACTIVE;
     
      return ARM_DRIVER_OK;

    // Unsupported command
    default: return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  // Check if Receiver/Transmitter is busy
  if ( usart->info->status.rx_busy ||
      (usart->info->flags & USART_FLAG_SEND_ACTIVE)) {
    return ARM_DRIVER_ERROR_BUSY;
  }

  // USART Flow control (RTS and CTS lines are only available on USART1)
  if (usart->reg) {
//    mcr = usart->reg->MCR & ~(UART_MCR_RTSEN | UART_MCR_CTSEN);
    switch (control & ARM_USART_FLOW_CONTROL_Msk) {
      case ARM_USART_FLOW_CONTROL_NONE:
        break;
      case ARM_USART_FLOW_CONTROL_RTS:
        if (usart->capabilities.flow_control_rts)
          mcr |= UART_MCR_RTSEN;
        else return ARM_USART_ERROR_FLOW_CONTROL;
        break;
      case ARM_USART_FLOW_CONTROL_CTS:
        if (usart->capabilities.flow_control_cts)
          mcr |= UART_MCR_CTSEN;
        else return ARM_USART_ERROR_FLOW_CONTROL;
        break;
      case ARM_USART_FLOW_CONTROL_RTS_CTS:
        if (usart->capabilities.flow_control_rts && 
            usart->capabilities.flow_control_cts) {
          mcr |= (UART_MCR_RTSEN | UART_MCR_CTSEN);
        } else return ARM_USART_ERROR_FLOW_CONTROL;
        break;
      default:
        return ARM_USART_ERROR_FLOW_CONTROL;
    }
  }

  // Clock setting for synchronous mode
  if ((mode == ARM_USART_MODE_SYNCHRONOUS_MASTER) ||
      (mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {

    // Only CPOL0 - CPHA1 combination available

    // USART clock polarity
    if ((control & ARM_USART_CPOL_Msk) != ARM_USART_CPOL0)
      return ARM_USART_ERROR_CPOL;

    // USART clock phase
    if ((control & ARM_USART_CPHA_Msk) != ARM_USART_CPHA1)
      return ARM_USART_ERROR_CPHA;
  }

  // USART Baudrate
  if (USART_SetBaudrate (arg, usart) == -1)
    return ARM_USART_ERROR_BAUDRATE;    
	
  // Configuration is OK - Mode is valid
  usart->info->mode = mode;

  // Configure TX pin regarding mode and transmitter state

  // Configure RX pin regarding mode and receiver state

  // Configure CLK pin regarding mode

  // Configure SYNCCRTL register (only in synchronous mode)

  // Configure ICR register (only in IrDA mode)

  // Configure MCR register (modem line for USART1)

  // Configure Line control register

	ROM_UARTEnable(usart->clk.base);
  // Set configured flag
  usart->info->flags |= USART_FLAG_CONFIGURED;

  return ARM_DRIVER_OK;
}

/**
  \fn          ARM_USART_STATUS USART_GetStatus (USART_RESOURCES *usart)
  \brief       Get USART status. - Prototype
  \param[in]   usart     Pointer to USART resources
  \return      USART status \ref ARM_USART_STATUS
*/
static ARM_USART_STATUS USART_GetStatus (USART_RESOURCES *usart) {
  return usart->info->status;
}

/**
  \fn          int32_t USART_SetModemControl (ARM_USART_MODEM_CONTROL  control,
                                              USART_RESOURCES         *usart)
  \brief       Set USART Modem Control line state. - Prototype
  \param[in]   control   \ref ARM_USART_MODEM_CONTROL
  \param[in]   usart     Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_SetModemControl (ARM_USART_MODEM_CONTROL  control,
                                      USART_RESOURCES         *usart) {

  if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0) {
    // USART is not configured
    return ARM_DRIVER_ERROR;
  }

  // Only UART1 supports modem lines

  return ARM_DRIVER_OK;
}

/**
  \fn          ARM_USART_MODEM_STATUS USART_GetModemStatus (USART_RESOURCES *usart)
  \brief       Get USART Modem Status lines state. - Prototype
  \param[in]   usart     Pointer to USART resources
  \return      modem status \ref ARM_USART_MODEM_STATUS
*/
static ARM_USART_MODEM_STATUS USART_GetModemStatus (USART_RESOURCES *usart) {
  ARM_USART_MODEM_STATUS modem_status;
  uint32_t msr;

  if (usart->reg &&
     (usart->info->flags & USART_FLAG_CONFIGURED)) {

//    msr = usart->reg->MSR;

    modem_status.cts = (msr & UART_MSR_CTS ? (1) : (0));
    modem_status.dsr = (msr & UART_MSR_DSR ? (1) : (0));
    modem_status.ri  = (msr & UART_MSR_RI  ? (1) : (0));
    modem_status.dcd = (msr & UART_MSR_DCD ? (1) : (0));
  } else {
     modem_status.cts = 0;
     modem_status.dsr = 0;
     modem_status.ri  = 0;
     modem_status.dcd = 0;
  }

  return modem_status;
}

/**
  \fn          void USART_IRQHandler (UART_RESOURCES *usart)
  \brief       USART Interrupt handler. - Prototype
  \param[in]   usart     Pointer to USART resources
*/
static void USART_IRQHandler (USART_RESOURCES *usart) {
  uint32_t event;

  event = 0;
    // Transmit holding register empty

   // Receive line status

    // Character time-out indicator

    // Modem interrupt (UART1 only)
#if (RTE_UART1)
    if (usart->uart_reg) {
      if ((iir & USART_IIR_INTID_MSK) == UART_IIR_INTID_MS) {
        // CTS state changed
        if (usart->uart_reg->MSR & UART_MSR_DCTS)
          event |= ARM_USART_EVENT_CTS;
        // DSR state changed
        if (usart->uart_reg->MSR & UART_MSR_DDSR)
          event |= ARM_USART_EVENT_DSR;
        // Ring indicator
        if (usart->uart_reg->MSR & UART_MSR_TERI)
          event |= ARM_USART_EVENT_RI;
        // DCD state changed
        if (usart->uart_reg->MSR & UART_MSR_DDCD)
          event |= ARM_USART_EVENT_DCD;
      }
    }
#endif
//  }
  if (usart->info->cb_event && event)
    usart->info->cb_event (event);
}

// USART0 Driver Wrapper functions
static ARM_USART_CAPABILITIES USART0_GetCapabilities (void) {
  return USART_GetCapabilities (&USART0_Resources);
}
static int32_t USART0_Initialize (ARM_USART_SignalEvent_t cb_event) {
  return USART_Initialize (cb_event, &USART0_Resources);
}
static int32_t USART0_Uninitialize (void) {
  return USART_Uninitialize(&USART0_Resources);
}
static int32_t USART0_PowerControl (ARM_POWER_STATE state) {
  return USART_PowerControl (state, &USART0_Resources);
}
static int32_t USART0_Send (const void *data, uint32_t num) {
  return USART_Send (data, num, &USART0_Resources);
}
static int32_t USART0_Receive (void *data, uint32_t num) {
  return USART_Receive (data, num, &USART0_Resources);
}
static int32_t USART0_Transfer (const void      *data_out,
                                      void      *data_in,
                                      uint32_t   num) {
  return USART_Transfer (data_out, data_in, num, &USART0_Resources);
}
static uint32_t USART0_GetTxCount (void) {
  return USART_GetTxCount (&USART0_Resources);
}
static uint32_t USART0_GetRxCount (void) {
  return USART_GetRxCount (&USART0_Resources); 
}
static int32_t USART0_Control (uint32_t control, uint32_t arg) {
  return USART_Control (control, arg, &USART0_Resources);
}
static ARM_USART_STATUS USART0_GetStatus (void) {
  return USART_GetStatus (&USART0_Resources);
}
static int32_t USART0_SetModemControl (ARM_USART_MODEM_CONTROL control) {
  return USART_SetModemControl (control, &USART0_Resources);
}
static ARM_USART_MODEM_STATUS USART0_GetModemStatus (void) {
  return USART_GetModemStatus (&USART0_Resources);
}
void UART0_IRQHandler (void) {
  USART_IRQHandler (&USART0_Resources);
}

// USART0 Driver Control Block
ARM_DRIVER_USART Driver_USART0 = {
    USARTx_GetVersion,
    USART0_GetCapabilities,
    USART0_Initialize,
    USART0_Uninitialize,
    USART0_PowerControl,
    USART0_Send, 
    USART0_Receive,
    USART0_Transfer,
    USART0_GetTxCount,
    USART0_GetRxCount,
    USART0_Control,
    USART0_GetStatus,
    USART0_SetModemControl,
    USART0_GetModemStatus
};
