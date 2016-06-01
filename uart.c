#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "usart_driver.h"
#include "xmegabaud.h"
#include "uart.h"

/***USART Buffers ***/
USART_data_t USARTBuf_gsm;
USART_data_t USARTBuf_mbus;
USART_data_t USARTBuf_ext;

void init_usart_gsm(void)
{

	/* Select USART and initialize buffers. */
	USART_InterruptDriver_Initialize(&USARTBuf_gsm, &USART_GSM, USART_DREINTLVL_LO_gc);

	/* 8 Data bits, No Parity, 1 Stop bit. */
	USART_Format_Set(USARTBuf_gsm.usart, USART_CHSIZE_8BIT_gc,
                     USART_PMODE_DISABLED_gc, false);

	/* Enable RXC interrupt. */
	USART_RxdInterruptLevel_Set(USARTBuf_gsm.usart, USART_RXCINTLVL_HI_gc); //TEST

	/* Set Baudrate */
	USART_Baudrate_Set(USARTBuf_gsm.usart, BSEL(F_CPU,BAUDRATE_GSM) , BSCALE(F_CPU,BAUDRATE_GSM));

	// Enable both RX and TX. 
	USART_Rx_Enable(USARTBuf_gsm.usart);
	USART_Tx_Enable(USARTBuf_gsm.usart);

	// Set direction of UART pins
  	// PD3 (TXD0) as output. 
	PORT_GSM.DIRSET   = PIN3_bm;
	PORT_GSM.OUTSET   = PIN3_bm; 

	// PD2 (RXD0) as input. 
	PORT_GSM.DIRCLR   = PIN2_bm;	

	// Enable PMIC interrupt level low. 
	PMIC.CTRL |= PMIC_LOLVLEX_bm|PMIC_HILVLEX_bm;
}

void init_usart_mbus(void)
{

	/* Select USART and initialize buffers. */
	USART_InterruptDriver_Initialize(&USARTBuf_mbus, &USART_MBUS, USART_DREINTLVL_LO_gc);

	/* 8 Data bits, No Parity, 1 Stop bit. */
	USART_Format_Set(USARTBuf_mbus.usart, USART_CHSIZE_8BIT_gc,
                     USART_PMODE_DISABLED_gc, false);

	/* Enable RXC interrupt. */
	USART_RxdInterruptLevel_Set(USARTBuf_mbus.usart, USART_RXCINTLVL_LO_gc);

	/* Set Baudrate */
	USART_Baudrate_Set(USARTBuf_mbus.usart, BSEL(F_CPU,BAUDRATE_MBUS) , BSCALE(F_CPU,BAUDRATE_MBUS));

	// Enable both RX and TX. 
	USART_Rx_Enable(USARTBuf_mbus.usart);
	USART_Tx_Enable(USARTBuf_mbus.usart);

	// Set direction of UART pins
  	// PD3 (TXD0) as output. 
	PORT_MBUS.DIRSET   = PIN3_bm;
	PORT_MBUS.OUTSET   = PIN3_bm; 

	// PD2 (RXD0) as input. 
	PORT_MBUS.DIRCLR   = PIN2_bm;	

	// Enable PMIC interrupt level low. 
	PMIC.CTRL |= PMIC_LOLVLEX_bm;
}

void init_usart_ext(void)
{

	/* Select USART and initialize buffers. */
	USART_InterruptDriver_Initialize(&USARTBuf_ext, &USART_EXT, USART_DREINTLVL_LO_gc);

	/* 8 Data bits, No Parity, 1 Stop bit. */
	USART_Format_Set(USARTBuf_ext.usart, USART_CHSIZE_8BIT_gc,
                     USART_PMODE_DISABLED_gc, false);

	/* Enable RXC interrupt. */
	USART_RxdInterruptLevel_Set(USARTBuf_ext.usart, USART_RXCINTLVL_LO_gc);

	/* Set Baudrate */
	USART_Baudrate_Set(USARTBuf_ext.usart, BSEL(F_CPU,BAUDRATE_EXT) , BSCALE(F_CPU,BAUDRATE_EXT));

	// Enable both RX and TX. 
	USART_Rx_Enable(USARTBuf_ext.usart);
	USART_Tx_Enable(USARTBuf_ext.usart);
	
	// Set direction of UART pins
  	// PD3 (TXD0) as output. 
	PORT_EXT.DIRSET   = PIN3_bm;
	PORT_EXT.OUTSET   = PIN3_bm; 

	// PD2 (RXD0) as input. 
	PORT_EXT.DIRCLR   = PIN2_bm;	

	// Enable PMIC interrupt level low. 
	PMIC.CTRL |= PMIC_LOLVLEX_bm;
}





/*! \brief Receive complete interrupt service routine.
 *
 *  Receive complete interrupt service routine.
 *  Calls the common receive complete handler with pointer to the correct USART
 *  as argument.
 */
ISR(RXC_USART_GSM)
{
	USART_RXComplete(&USARTBuf_gsm);
}


/*! \brief Data register empty  interrupt service routine.
 *
 *  Data register empty  interrupt service routine.
 *  Calls the common data register empty complete handler with pointer to the
 *  correct USART as argument.
 */
ISR(DRE_USART_GSM)
{
	USART_DataRegEmpty(&USARTBuf_gsm);
}

/*! \brief Receive complete interrupt service routine.
 *
 *  Receive complete interrupt service routine.
 *  Calls the common receive complete handler with pointer to the correct USART
 *  as argument.
 */
ISR(RXC_USART_MBUS)
{
	USART_RXComplete(&USARTBuf_mbus);
}


/*! \brief Data register empty  interrupt service routine.
 *
 *  Data register empty  interrupt service routine.
 *  Calls the common data register empty complete handler with pointer to the
 *  correct USART as argument.
 */
ISR(DRE_USART_MBUS)
{
	USART_DataRegEmpty(&USARTBuf_mbus);
}

/*! \brief Receive complete interrupt service routine.
 *
 *  Receive complete interrupt service routine.
 *  Calls the common receive complete handler with pointer to the correct USART
 *  as argument.
 */
ISR(RXC_USART_EXT)
{
	USART_RXComplete(&USARTBuf_ext);
}


/*! \brief Data register empty  interrupt service routine.
 *
 *  Data register empty  interrupt service routine.
 *  Calls the common data register empty complete handler with pointer to the
 *  correct USART as argument.
 */
ISR(DRE_USART_EXT)
{
	USART_DataRegEmpty(&USARTBuf_ext);
}
