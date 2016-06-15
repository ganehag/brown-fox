#ifndef USARTDEF
#define USARTDEF

#include <inttypes.h>
#include "usart_driver.h"

#define USART_GSM 		USARTC0
#define BAUDRATE_GSM 	115200

#define USART_MBUS		USARTD0
#define BAUDRATE_MBUS 	19200

#define USART_EXT 		USARTE0
#define BAUDRATE_EXT 	38400


#define RXC_USART_GSM  USARTC0_RXC_vect
#define DRE_USART_GSM  USARTC0_DRE_vect
 
#define RXC_USART_MBUS  USARTD0_RXC_vect
#define DRE_USART_MBUS  USARTD0_DRE_vect

#define RXC_USART_EXT  USARTE0_RXC_vect
#define DRE_USART_EXT  USARTE0_DRE_vect

#define PORT_GSM		PORTC
#define PORT_MBUS		PORTD
#define PORT_EXT		PORTE



/***USART Buffers ***/
extern USART_data_t USARTBuf_gsm;
extern USART_data_t USARTBuf_mbus;
extern USART_data_t USARTBuf_ext;


/*** Prototypes ***/

void init_usart_gsm(void);
void init_usart_mbus(void);
void init_usart_ext(void);

/*! \brief Transmit macro
 *
 *	\param data	character to send
 */
static inline uint8_t Transmit_gsm(uint8_t data)
{
	return USART_TXBuffer_PutByte(&USARTBuf_gsm, data);
}

/*! \brief Receive macro
 *
 *	\return	byte received
 */
static inline uint8_t Receive_gsm(void)
{
	return USART_RXBuffer_GetByte(&USARTBuf_gsm);
}

/*! \brief Transmit macro
 *
 *	\param data	character to send
 */
static inline uint8_t Transmit_mbus(uint8_t data)
{
	return USART_TXBuffer_PutByte(&USARTBuf_mbus, data);
}

/*! \brief Receive macro
 *
 *	\return	byte received
 */
static inline uint8_t Receive_mbus(void)
{
	return USART_RXBuffer_GetByte(&USARTBuf_mbus);
}

/*! \brief Transmit macro
 *
 *	\param data	character to send
 */
static inline uint8_t Transmit_ext(uint8_t data)
{
	return USART_TXBuffer_PutByte(&USARTBuf_ext, data);
}

/*! \brief Receive macro
 *
 *	\return	byte received
 */
static inline uint8_t Receive_ext(void)
{
	return USART_RXBuffer_GetByte(&USARTBuf_ext);
}


#endif
