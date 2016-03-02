;******************************************************************************
;*
;* XMEGA Self-programming driver assembly source file.
;*
;*      This file contains the low-level implementations for the
;*      XMEGA Self-programming driver. It is written for the GCC Assembler.
;*
;*      If any SPM instructions are used, the linker file must define
;*      a segment named bootloader which must be located in the device Boot section.
;*      This can be done by passing "-Wl,--section-start=.BOOT=0x020000" to the
;*      linker with the correct address for the boot section.
;*
;*      None of these routines clean up the NVM Command Register after use.
;*      It is therefore important to write NVM_CMD_NO_OPERATION_gc (0x00) to this
;*      register when you are finished using any of the functions in this driver.
;*
;*      For all routines, it is important that any interrupt handlers do not
;*      perform any NVM operations. The user must implement a scheme for mutually
;*      exclusive access to the NVM. However, the 4-cycle timeout will work fine,
;*      since writing to the Configuration Change Protection register (CCP)
;*      automatically disables interrupts for 4 instruction cycles.
;*
;*      Note on IAR calling convention:
;*         Scratch registers:   R18-R27, R30-R31
;*         Preserved registers: R2-R17, R28-R29
;*         Parameter registers: R8-R25 (2-,4-, or 8- byte alignment)
;*         Return registers:    R18-R25 (up to 64-bit)
;*
;* Application note:
;*      AVR1316: XMEGA Self-programming
;*
;* Documentation
;*      For comprehensive code documentation, supported compilers, compiler
;*      settings and supported devices see readme.html
;*
;*      Atmel Corporation: http:;www.atmel.com \n
;*      Support email: avr@atmel.com
;*
;* $Revision: 1.1 $
;* $Date: 2012/05/24 22:40:41 $
;*
;* Copyright (c) 2007, Atmel Corporation All rights reserved.
;*
;* Redistribution and use in source and binary forms, with or without
;* modification, are permitted provided that the following conditions are met:
;*
;* 1. Redistributions of source code must retain the above copyright notice,
;* this list of conditions and the following disclaimer.
;*
;* 2. Redistributions in binary form must reproduce the above copyright notice,
;* this list of conditions and the following disclaimer in the documentation
;* and/or other materials provided with the distribution.
;*
;* 3. The name of ATMEL may not be used to endorse or promote products derived
;* from this software without specific prior written permission.
;*
;* THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
;* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
;* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
;* SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
;* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
;* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
;* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
;* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
;* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
;* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;******************************************************************************

#include <avr\io.h>

/* Define the size of the flash page if not defined in the header files. */
#ifndef APP_SECTION_PAGE_SIZE
	#error  APP_SECTION_PAGE_SIZE must be defined if not defined in header files.
	//#define APP_SECTION_PAGE_SIZE 512
#endif /*APP_SECTION_PAGE_SIZE*/

/* Defines not yet included in header file. */
#define NVM_CMD_NO_OPERATION_gc (0x00<<0)	// Noop/Ordinary LPM
#define NVM_CMD_READ_USER_SIG_ROW_gc (0x01<<0)	// Read user signature row
#define NVM_CMD_READ_CALIB_ROW_gc (0x02<<0)	// Read calibration row
#define NVM_CMD_READ_EEPROM_gc (0x06<<0)	// Read EEPROM
#define NVM_CMD_READ_FUSES_gc (0x07<<0)	// Read fuse byte
#define NVM_CMD_WRITE_LOCK_BITS_gc (0x08<<0)	// Write lock bits
#define NVM_CMD_ERASE_USER_SIG_ROW_gc (0x18<<0)	// Erase user signature row
#define NVM_CMD_WRITE_USER_SIG_ROW_gc (0x1A<<0)	// Write user signature row
#define NVM_CMD_ERASE_APP_gc (0x20<<0)	// Erase Application Section
#define NVM_CMD_ERASE_APP_PAGE_gc (0x22<<0)	// Erase Application Section page
#define NVM_CMD_LOAD_FLASH_BUFFER_gc (0x23<<0)	// Load Flash page buffer
#define NVM_CMD_WRITE_APP_PAGE_gc (0x24<<0)	// Write Application Section page
#define NVM_CMD_ERASE_WRITE_APP_PAGE_gc (0x25<<0)	// Erase-and-write Application Section page
#define NVM_CMD_ERASE_FLASH_BUFFER_gc (0x26<<0)	// Erase/flush Flash page buffer
#define NVM_CMD_ERASE_BOOT_PAGE_gc (0x2A<<0)	// Erase Boot Section page
#define NVM_CMD_WRITE_BOOT_PAGE_gc (0x2C<<0)	// Write Boot Section page
#define NVM_CMD_ERASE_WRITE_BOOT_PAGE_gc (0x2D<<0)	// Erase-and-write Boot Section page
#define NVM_CMD_ERASE_EEPROM_gc (0x30<<0)	// Erase EEPROM
#define NVM_CMD_ERASE_EEPROM_PAGE_gc (0x32<<0)	// Erase EEPROM page
#define NVM_CMD_LOAD_EEPROM_BUFFER_gc (0x33<<0)	// Load EEPROM page buffer
#define NVM_CMD_WRITE_EEPROM_PAGE_gc (0x34<<0)	// Write EEPROM page
#define NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc (0x35<<0)	// Erase-and-write EEPROM page
#define NVM_CMD_ERASE_EEPROM_BUFFER_gc (0x36<<0)	// Erase/flush EEPROM page buffer
#define NVM_CMD_APP_CRC_gc (0x38<<0)	// Generate Application section CRC
#define NVM_CMD_BOOT_CRC_gc (0x39<<0)	// Generate Boot Section CRC
#define NVM_CMD_FLASH_RANGE_CRC_gc (0x3A<<0)	// Generate Flash Range CRC
#define CCP_SPM_gc (0x9D<<0)	// SPM Instruction Protection
#define CCP_IOREG_gc (0xD8<<0)	// IO Register Protection



; ---
; This routine reads a byte from flash given by the address in
; R25:R24:R23:R22.
;
; Input:
;     R25:R24:R23:R22.
;
; Returns:
;     R24 - Read byte.
; ---

.section .text
.global SP_ReadByte

SP_ReadByte:
	in	r19, RAMPZ      ; Save RAMPZ.
	out	RAMPZ, r24      ; Load RAMPZ with the MSB of the address.
	movw	ZL, r22         ; Move the low bytes to the Z pointer
	elpm	r24, Z          ; Extended load byte from address pointed to by Z.
	out	RAMPZ, r19      ; Restore RAMPZ register.
	ret



; ---
; This routine reads a word from flash given by the address in
; R25:R24:R23:R22.
;
; Input:
;     R25:R24:R23:R22.
;
; Returns:
;     R25:R24 - Read word.
; ---

.section .text
.global SP_ReadWord

SP_ReadWord:
	in	r19, RAMPZ      ; Save RAMPZ.
	out	RAMPZ, r24      ; Load RAMPZ with the MSB of the address.
	movw	ZL, r22         ; Move the low bytes to the Z pointer
	elpm	r24, Z+         ; Extended load byte from address pointed to by Z.
	elpm	r25, Z          ; Extended load byte from address pointed to by Z.
	out	RAMPZ, r19      ; Restore RAMPZ register.
	ret



; ---
; This routine reads the calibration byte given by the index in R24.
;
; Input:
;     R24 - Byte index.
;
; Returns:
;     R24 - Calibration byte.
; ---

.section .text
.global SP_ReadCalibrationByte	

SP_ReadCalibrationByte:
	ldi	r20, NVM_CMD_READ_CALIB_ROW_gc    ; Prepare NVM command in R20.
	rjmp	SP_CommonLPM                      ; Jump to common LPM code.



; ---
; This routine reads the user signature byte given by the index in R25:R24.
;
; Input:
;     R25:R24 - Byte index.
;
; Returns:
;     R24 - Signature byte.
; ---

.section .text	
.global SP_ReadUserSignatureByte

SP_ReadUserSignatureByte:
	ldi	r20, NVM_CMD_READ_USER_SIG_ROW_gc  ; Prepare NVM command in R20.
	rjmp	SP_CommonLPM                       ; Jump to common LPM code.



; ---
; This routine reads the fuse byte given by the index in R24.
;
; Input:
;     R24 - Byte index.
;
; Returns:
;     R24 - Fuse byte.
; ---

.section .text	
.global SP_ReadFuseByte

SP_ReadFuseByte:
	sts	NVM_ADDR0, r24              ; Load fuse byte index into NVM Address Register 0.
	clr	r24                         ; Prepare a zero.
	sts	NVM_ADDR1, r24              ; Load zero into NVM Address Register 1.
	sts	NVM_ADDR2, r24              ; Load zero into NVM Address Register 2.
	ldi	r20, NVM_CMD_READ_FUSES_gc  ; Prepare NVM command in R20.
	rcall	SP_CommonCMD                ; Jump to common NVM Action code.
	movw	r24, r22                    ; Move low byte to 1 byte return address.
	ret


; ---
; This routine calculates a CRC for the application section.
;
; Input:
;     Nothing.
;
; Returns:
;     R25:R24:R23:R22 - 32-bit CRC result (actually only 24-bit used).
; ---

.section .text	
.global SP_ApplicationCRC

SP_ApplicationCRC:
	ldi	r20, NVM_CMD_APP_CRC_gc    ; Prepare NVM command in R20.
	rjmp	SP_CommonCMD               ; Jump to common NVM Action code.



; ---
; This routine calculates a CRC for the Boot section.
;
; Input:
;     Nothing.
;
; Returns:
;     R25:R24:R23:R22 - 32-bit CRC result (actually only 24-bit used).
; ---

.section .text
.global SP_BootCRC

SP_BootCRC:
	ldi	r20, NVM_CMD_BOOT_CRC_gc   ; Prepare NVM command in R20.
	rjmp	SP_CommonCMD               ; Jump to common NVM Action code.



; ---
; This routine locks all further access to SPM operations until next reset.
;
; Input:
;     Nothing.
;
; Returns:
;     Nothing.
; ---

.section .text
.global SP_LockSPM

SP_LockSPM:
	ldi	r18, CCP_IOREG_gc     ; Prepare Protect IO-register signature in R18.
	sts	CCP, r18              ; Enable IO-register operation (this disables interrupts for 4 cycles).
	ldi	r18, NVM_SPMLOCK_bm   ; Prepare bitmask for locking SPM into R18.
	sts	NVM_CTRLB, r18        ; Load bitmask into NVM Control Register B, which locks SPM.
	ret
	


; ---
; This routine wait for the SPM to finish and clears the command register.
;
; Note that this routine is blocking, and will halt any execution until the SPM
; is finished.
;
; Input:
;     Nothing.
;
; Returns:
;     Nothing.
; ---

.section .text
.global SP_WaitForSPM		

SP_WaitForSPM:
	lds	r18, NVM_STATUS     ; Load the NVM Status register.
	sbrc	r18, NVM_NVMBUSY_bp ; Check if bit is cleared.
	rjmp	SP_WaitForSPM       ; Repeat check if bit is not cleared.
	clr	r18
	sts	NVM_CMD, r18        ; Clear up command register to NO_OPERATION.
	ret



; ---
; This routine is called by several other routines, and contains common code
; for executing an NVM command, including the return statement itself.
;
; If the operation (NVM command) requires the NVM Address registers to be
; prepared, this must be done before jumping to this routine.
;
; Note that R25:R24:R23:R22 is used for returning results, even if the
; C-domain calling function only expects a single byte or even void.
;
; Input:
;     R20 - NVM Command code.
;
; Returns:
;     R25:R24:R23:R22 - 32-bit result from NVM operation.
; ---

.section .text		

SP_CommonCMD:
	sts	NVM_CMD, r20        ; Load command into NVM Command register.
	ldi	r18, CCP_IOREG_gc   ; Prepare Protect IO-register signature in R18.
	ldi	r19, NVM_CMDEX_bm   ; Prepare bitmask for setting NVM Command Execute bit into R19.
	sts	CCP, r18            ; Enable IO-register operation (this disables interrupts for 4 cycles).
	sts	NVM_CTRLA, r19      ; Load bitmask into NVM Control Register A, which executes the command.
	lds	r22, NVM_DATA0      ; Load NVM Data Register 0 into R22.
	lds	r23, NVM_DATA1      ; Load NVM Data Register 1 into R23.
	lds	r24, NVM_DATA2      ; Load NVM Data Register 2 into R24.
	clr	r25                 ; Clear R25 in order to return a clean 32-bit value.
	ret



; ---
; This routine is called by several other routines, and contains common code
; for executing an LPM command, including the return statement itself.
;
; Note that R24 is used for returning results, even if the
; C-domain calling function expects a void.
;
; Input:
;     R25:R24 - Low bytes of Z pointer.
;     R20     - NVM Command code.
;
; Returns:
;     R24     - Result from LPM operation.
; ---

.section .text		

SP_CommonLPM:
	movw	ZL, r24             ; Load index into Z.
	sts	NVM_CMD, r20        ; Load prepared command into NVM Command register.
	lpm	r24,Z
	ret

