/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief  XMEGA TWI master driver header file.
 *
 *      This file contains the function prototypes and enumerator definitions
 *      for various configuration parameters for the XMEGA TWI master driver.
 *
 *      The driver is not intended for size and/or speed critical code, since
 *      most functions are just a few lines of code, and the function call
 *      overhead would decrease code performance. The driver is intended for
 *      rapid prototyping and documentation purposes for getting started with
 *      the XMEGA TWI master module.
 *
 *      For size and/or speed critical code, it is recommended to copy the
 *      function contents directly into your application instead of making
 *      a function call.
 *
 * \par Application note:
 *      AVR1308: Using the XMEGA TWI
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Revision: 1569 $
 * $Date: 2008-04-22 13:03:43 +0200 (ti, 22 apr 2008) $  \n
 *
 * Copyright (c) 2008, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#ifndef TWI_MASTER_DRIVER_H
#define TWI_MASTER_DRIVER_H

#include "avr_compiler.h"

/*! Baud register setting calculation. Formula described in datasheet. */
#define TWI_BAUD(F_TWI) ((F_CPU / (2 * F_TWI)) - 5)


/*! Transaction status defines. */
#define TWIM_STATUS_READY              0
#define TWIM_STATUS_BUSY               1


/*! Transaction result enumeration. */

#define TWIM_RESULT_UNKNOWN          (0x00<<0)
#define TWIM_RESULT_OK               (0x01<<0)
#define TWIM_RESULT_BUFFER_OVERFLOW  (0x02<<0)
#define TWIM_RESULT_ARBITRATION_LOST (0x03<<0)
#define TWIM_RESULT_BUS_ERROR        (0x04<<0)
#define TWIM_RESULT_NACK_RECEIVED    (0x05<<0)
#define TWIM_RESULT_FAIL             (0x06<<0)

/*! Buffer size defines */
#define TWIM_WRITE_BUFFER_SIZE         8
#define TWIM_READ_BUFFER_SIZE          8

#define MEM_WP_PIN	PIN7_bm


/*! \brief TWI master driver struct
 *
 *  TWI master struct. Holds pointer to TWI module,
 *  buffers and necessary varibles.
 */
uint8_t twi_writeData[TWIM_WRITE_BUFFER_SIZE];  /*!< Data to write */
uint8_t twi_readData[TWIM_READ_BUFFER_SIZE];    /*!< Read data */
uint8_t twi_bytesToWrite;                       /*!< Number of bytes to write */
uint8_t twi_bytesToRead;                        /*!< Number of bytes to read */
volatile uint8_t twi_bytesWritten;              /*!< Number of bytes written */
volatile uint8_t twi_bytesRead;                 /*!< Number of bytes read */
volatile uint8_t twi_status;                    /*!< Status of transaction */
volatile uint8_t twi_result;                    /*!< Result of transaction */

void TWI_Unprotect(void);
void TWI_MasterInit(uint8_t baudRateRegisterSetting);
uint8_t TWI_MasterState(void);
bool TWI_MasterReady(void);
bool TWI_MasterWriteRead(uint8_t address, uint8_t bytesToWrite, uint8_t bytesToRead);
void TWI_MasterInterruptHandler(void);
void TWI_MasterArbitrationLostBusErrorHandler(void);
void TWI_MasterWriteHandler(void);
void TWI_MasterReadHandler(void);


#endif /* TWI_MASTER_DRIVER_H */
