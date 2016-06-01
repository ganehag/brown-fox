/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief  XMEGA NVM routines.
 *
 *      This file contains an example application that demonstrates the Timer/
 *      Counter driver.
 *
 * \par Application note:
 *      AVR1306: Using the XMEGA Timer/Counter
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
 * $Date: 2012/05/24 22:36:35 $  \n
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

#include "avr_compiler.h"
#include "nvm.h"

/*! \Read the calibration row
 *  
 *	\param	*data	Pointer to data structure.
 */

void NVM_ReadCalib( uint8_t *data )
{

	/* Load the NVM Command register to read the calibration row. */
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	for (uint8_t fcc=0; fcc<PROD_SIGNATURES_SIZE; fcc++)
	{
		*(data++)=pgm_read_byte(fcc);
	}

	/* Clean up NVM Command register. */
 	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
}

/*! \Read the signature row
 *  
 *	\param	*data	Pointer to data structure.
 */
void NVM_ReadSign( uint8_t *data )
{
	/* Load the NVM Command register to read the calibration row. */
	NVM_CMD = NVM_CMD_READ_USER_SIG_ROW_gc;
	for (uint8_t fcc=0; fcc<32; fcc++) // The row is longer, but we do not support more than 32 bytes at the moment.
	{
		*(data++)=pgm_read_byte(fcc);
	}

	/* Clean up NVM Command register. */
 	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
}

void NVM_AppCrc( uint8_t *data )
{

	/* Load the NVM Command register to read the calibration row. */
	NVM_CMD = NVM_CMD_APP_CRC_gc;
	for (uint8_t fcc=0; fcc<CRC_SIZE; fcc++)
	{
		*(data++)=pgm_read_byte(fcc);
	}

	/* Clean up NVM Command register. */
 	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
}

void NVM_BootCrc( uint8_t *data )
{

	/* Load the NVM Command register to read the calibration row. */
	NVM_CMD = NVM_CMD_BOOT_CRC_gc;
	for (uint8_t fcc=0; fcc<CRC_SIZE; fcc++)
	{
		*(data++)=pgm_read_byte(fcc);
	}

	/* Clean up NVM Command register. */
 	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
}
