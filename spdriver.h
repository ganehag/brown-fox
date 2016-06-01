/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief  XMEGA Self-programming driver header file.
 *
 *      This file contains the function prototypes for the
 *      XMEGA Self-programming driver.
 *      If any SPM instructions are used, the linker file must define
 *      a segment named BOOT which must be located in the device boot section.
 *
 *
 *      None of these functions clean up the NVM Command Register after use.
 *      It is therefore important to write NVMCMD_NO_OPERATION (0x00) to this
 *      register when you are finished using any of the functions in this driver.
 *
 *      For all functions, it is important that no interrupt handlers
 *      perform any NVM operations. The user must implement a scheme for mutually
 *      exclusive access to the NVM. However, the 4-cycle timeout will work fine,
 *      since writing to the Configuration Change Protection register (CCP)
 *      automatically disables interrupts for 4 instruction cycles.
 *
 * \par Application note:
 *      AVR1316: XMEGA Self-programming
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Revision: 1.1 $
 * $Date: 2012/05/24 22:40:41 $  \n
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
#ifndef SP_DRIVER_H
#define SP_DRIVER_H

#include "avr_compiler.h"

/* Define the size of the flash page if not defined in the header files. */
#ifndef APP_SECTION_PAGE_SIZE
	#error  APP_SECTION_PAGE_SIZE must be defined if not defined in header files.
	//#define APP_SECTION_PAGE_SIZE 512
#endif /*APP_SECTION_PAGE_SIZE*/

/* Define the Start of the application table if not defined in the header files. */
#ifndef APPTABLE_SECTION_START
	#error  APPTABLE_SECTION_START must be defined if not defined in header files.
#	//#define APPTABLE_SECTION_START 0x01E000 //APPTABLE address for ATxmega128A1
#endif /*APPTABLE_SECTION_START*/

/*! \brief Read a byte from flash.
 *
 *  This function reads one byte from the flash.
 *
 *  \note Both IAR and GCC have functions to do this, but
 *        we include the fucntions for easier use.
 *
 *  \param address Address to the location of the byte to read.
 *
 *  \retval Byte read from flash.
 */
uint8_t SP_ReadByte(uint32_t address);

/*! \brief Read a word from flash.
 *
 *  This function reads one word from the flash.
 *
 *  \note Both IAR and GCC have functions to do this automatically, but
 *        we include the fucntions for easier use.
 *
 *  \param address Address to the location of the word to read.
 *
 *  \retval word read from flash.
 */
uint16_t SP_ReadWord(uint32_t address);

/*! \brief Read calibration byte at given index.
 *
 *  This function reads one calibration byte from the Calibration signature row.
 *
 *  \param index  Index of the byte in the calibration signature row.
 *
 *  \retval Calibration byte
 */
uint8_t SP_ReadCalibrationByte(uint8_t index);

/*! \brief Read fuse byte from given index.
 *
 *  This function reads the fuse byte at the given index.
 *
 *  \param index  Index of the fuse byte.
 *
 *  \retval Fuse byte
 */
uint8_t SP_ReadFuseByte(uint8_t index);

/*! \brief Read lock bits.
 *
 *  This function reads the lock bits.
 *
 *  \retval Lock bits
 */
uint8_t SP_ReadLockBits(void);

/*! \brief Read user signature at given index.
 *
 *  This function reads one byte from the user signature row.
 *
 *  \param index  Index of the byte in the user signature row.
 *
 *  \retval User signature byte
 */
uint8_t SP_ReadUserSignatureByte(uint16_t index);

/*! \brief Generate CRC from application section.
 *
 *  \retval 24-bit CRC value
 */
uint32_t SP_ApplicationCRC(void);

/*! \brief Generate CRC from boot section.
 *
 *  \retval 24-bit CRC value
 */
uint32_t SP_BootCRC(void);

/*! \brief Lock SPM instruction.
 *
 *   This function locks the SPM instruction, and will disable the use of
 *   SPM until the next reset occurs.
 */
void SP_LockSPM(void);

/*! \brief Wait for SPM to finish.
 *
 *   This routine waits for the SPM to finish and clears the command register.
 */
void SP_WaitForSPM(void);

#endif /* SP_DRIVER_H */
