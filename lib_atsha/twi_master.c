/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief
 *
 * 'WE=PD7
 *****************************************************************************/

#include "twi_master.h"


/*! \brief Initialize the TWI module.
 *
 *  TWI module initialization function.
 *  Enables master read and write interrupts.
 *  Remember to enable interrupts globally from the main application.
 *
 *  \param baudRateRegisterSetting  The baud rate register value.
 */
void TWI_MasterInit(uint8_t baudRateRegisterSetting)
{
	PORTD.OUTSET = (MEM_WP_PIN);
	PORTD.DIRSET = (MEM_WP_PIN);

	TWIE.MASTER.CTRLA = TWI_MASTER_INTLVL_LO_gc |
	                               TWI_MASTER_RIEN_bm |
	                               TWI_MASTER_WIEN_bm |
	                               TWI_MASTER_ENABLE_bm;
	TWIE.MASTER.BAUD = baudRateRegisterSetting;
	TWIE.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}




/*! \brief Returns the TWI bus state.
 *
 *  Returns the TWI bus state (type defined in device headerfile),
 *  unknown, idle, owner or busy.
 *
 *  \retval TWI_MASTER_BUSSTATE_UNKNOWN_gc Bus state is unknown.
 *  \retval TWI_MASTER_BUSSTATE_IDLE_gc    Bus state is idle.
 *  \retval TWI_MASTER_BUSSTATE_OWNER_gc   Bus state is owned by the master.
 *  \retval TWI_MASTER_BUSSTATE_BUSY_gc    Bus state is busy.
 */
uint8_t TWI_MasterState(void)
{
	uint8_t twi_status;
	twi_status = (TWIE.MASTER.STATUS & TWI_MASTER_BUSSTATE_gm);
	return twi_status;
}

/*! \brief Enable write
 *	Setting the WP pin to a HIGH condition will write-protect
 *  all addresses. The FM24CL16B will not acknowledge data bytes
 *  that are written to protected addresses
 */
void TWI_Unprotect(void)
{
	PORTD.DIRSET = (MEM_WP_PIN);
	PORTD.OUTCLR = (MEM_WP_PIN);
}

/*! \brief Returns true if transaction is ready.
 *
 *  This function returns a boolean whether the TWI Master is ready
 *  for a new transaction.
 *
 *  \retval true  If transaction could be started.
 *  \retval false If transaction could not be started.
 */
bool TWI_MasterReady(void)
{
	bool retval = (twi_status & TWIM_STATUS_READY);
	return retval;
}


/*! \brief TWI write and/or read transaction.
 *
 *  This function is a TWI Master write and/or read transaction. The function
 *  can be used to both write and/or read bytes to/from the TWI Slave in one
 *  transaction.
 *
 *  \param address        The slave address.
 *  \param writeData      Pointer to data to write.
 *  \param bytesToWrite   Number of bytes to write.
 *  \param bytesToRead    Number of bytes to read.
 *
 *  \retval true  If transaction could be started.
 *  \retval false If transaction could not be started.
 */
bool TWI_MasterWriteRead(uint8_t address,
                         uint8_t bytesToWrite,
                         uint8_t bytesToRead)
{
	/*Parameter sanity check. */
	if (bytesToWrite > TWIM_WRITE_BUFFER_SIZE) {
		return false;
	}
	if (bytesToRead > TWIM_READ_BUFFER_SIZE) {
		return false;
	}

	/*Initiate transaction if bus is ready. */
	if (twi_status == TWIM_STATUS_READY) {

		twi_status = TWIM_STATUS_BUSY;
		twi_result = TWIM_RESULT_UNKNOWN;

		address = address<<1;

//		/* Fill write data buffer. */
//		for (uint8_t bufferIndex=0; bufferIndex < bytesToWrite; bufferIndex++) {
//			twi_writeData[bufferIndex] = writeData[bufferIndex];
//		}

		twi_bytesToWrite = bytesToWrite;
		twi_bytesToRead = bytesToRead;
		twi_bytesWritten = 0;
		twi_bytesRead = 0;

		/* If write command, send the START condition + Address +
		 * 'R/_W = 0'
		 */
		if (twi_bytesToWrite > 0) {
			uint8_t writeAddress = address & ~0x01;
			TWIE.MASTER.ADDR = writeAddress;
		}

		/* If read command, send the START condition + Address +
		 * 'R/_W = 1'
		 */
		else if (twi_bytesToRead > 0) {
			TWIE.MASTER.ADDR |= 0x01;
		}
		return true;
	} else {
		return false;
	}
}


/*! \brief Common TWI master interrupt service routine.
 *
 *  Check current status and calls the appropriate handler.
 *
 *  \param twi  The TWI_Master_t struct instance.
 */
ISR(TWIE_TWIM_vect)
{
	uint8_t currentStatus = TWIE.MASTER.STATUS;

	/* If arbitration lost or bus error. */
	if ((currentStatus & TWI_MASTER_ARBLOST_bm) ||
	    (currentStatus & TWI_MASTER_BUSERR_bm)) {

		TWI_MasterArbitrationLostBusErrorHandler();
	}

	/* If master write interrupt. */
	else if (currentStatus & TWI_MASTER_WIF_bm) {
		TWI_MasterWriteHandler();
	}

	/* If master read interrupt. */
	else if (currentStatus & TWI_MASTER_RIF_bm) {
		TWI_MasterReadHandler();
	}

	/* If unexpected state. */
	else {
		twi_result = TWIM_RESULT_FAIL;
		twi_status = TWIM_STATUS_READY;
	}
}


/*! \brief TWI master arbitration lost and bus error interrupt handler.
 *
 *  Handles TWI responses to lost arbitration and bus error.
 *
 *  \param twi  The TWI_Master_t struct instance.
 */
void TWI_MasterArbitrationLostBusErrorHandler(void)
{
	uint8_t currentStatus = TWIE.MASTER.STATUS;

	/* If bus error. */
	if (currentStatus & TWI_MASTER_BUSERR_bm) {
		twi_result = TWIM_RESULT_BUS_ERROR;
	}
	/* If arbitration lost. */
	else {
		twi_result = TWIM_RESULT_ARBITRATION_LOST;
	}

	/* Clear interrupt flag. */
	TWIE.MASTER.STATUS = currentStatus | TWI_MASTER_ARBLOST_bm;

	twi_status = TWIM_STATUS_READY;
}


/*! \brief TWI master write interrupt handler.
 *
 *  Handles TWI transactions (master write) and responses to (N)ACK.
 *
 *  \param twi The TWI_Master_t struct instance.
 */
void TWI_MasterWriteHandler(void)
{
	/* Local variables used in if tests to avoid compiler warning. */
	uint8_t bytesToWrite  = twi_bytesToWrite;
	uint8_t bytesToRead   = twi_bytesToRead;

	/* If NOT acknowledged (NACK) by slave cancel the transaction. */
	if (TWIE.MASTER.STATUS & TWI_MASTER_RXACK_bm) {
		TWIE.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		twi_result = TWIM_RESULT_NACK_RECEIVED;
		twi_status = TWIM_STATUS_READY;
	}

	/* If more bytes to write, send data. */
	else if (twi_bytesWritten < bytesToWrite) {
		uint8_t data = twi_writeData[twi_bytesWritten];
		TWIE.MASTER.DATA = data;
		++twi_bytesWritten;
	}

	/* If bytes to read, send repeated START condition + Address +
	 * 'R/_W = 1'
	 */
	else if (twi_bytesRead < bytesToRead) {
		PORTD.OUTSET = (MEM_WP_PIN); // Write is completed. Writeprotect
		TWIE.MASTER.ADDR |= 0x01;
	}

	/* If transaction finished, send STOP condition and set RESULT OK. */
	else {
		PORTD.OUTSET = (MEM_WP_PIN); // Write is completed. Writeprotect
		TWIE.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		twi_result = TWIM_RESULT_OK;
		twi_status = TWIM_STATUS_READY;
	}
}


/*! \brief TWI master read interrupt handler.
 *
 *  This is the master read interrupt handler that takes care of
 *  reading bytes from the TWI slave.
 *
 *  \param twi The TWI_Master_t struct instance.
 */
void TWI_MasterReadHandler(void)
{
	/* Fetch data if bytes to be read. */
	if (twi_bytesRead < TWIM_READ_BUFFER_SIZE) {
		uint8_t data = TWIE.MASTER.DATA;
		twi_readData[twi_bytesRead] = data;
		twi_bytesRead++;
	}

	/* If buffer overflow, issue STOP and BUFFER_OVERFLOW condition. */
	else {
		TWIE.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		twi_result = TWIM_RESULT_BUFFER_OVERFLOW;
		twi_status = TWIM_STATUS_READY;
	}

	/* Local variable used in if test to avoid compiler warning. */
	uint8_t bytesToRead = twi_bytesToRead;

	/* If more bytes to read, issue ACK and start a byte read. */
	if (twi_bytesRead < bytesToRead) {
		TWIE.MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
	}

	/* If transaction finished, issue NACK and STOP condition. */
	else {
		TWIE.MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
		twi_result = TWIM_RESULT_OK;
		twi_status = TWIM_STATUS_READY;
	}
}
