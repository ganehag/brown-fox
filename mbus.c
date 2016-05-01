#include "mbrc1180.h"
#include "usart_driver.h"
#include "uart.h"
#include "gdefs.h"
#include <avr/pgmspace.h>
#include <stdio.h>
#include <util/crc16.h>
#include "mbus.h"

#include "eeprom.h"

const uint8_t verifikat[12] PROGMEM ={'R','C','1','1','8','0','-','M','B','U','S','\0'};


/* Typedef of a enumeration of the possible states and error status */
typedef enum
{
	MB_STOPPED = 0,
	MB_SYNC_START,
	MB_KNOWN_LEN,
	MB_WAIT,
	MB_SYNC_TRAIL,
	MB_DONE
} MB_STATE_t;

uint8_t mbus_enter_config(void)
{
	uint8_t rxd;
	uint8_t retval=0;
	//Clear buffer

	while(USART_RXBufferData_Available(&USARTBuf_mbus))
	{
		// Read out byte from buffer
		USART_RXBuffer_GetByte(&USARTBuf_mbus);
	}
		
	mbus_configmode(1); // Set config pin
	
	_delay_ms(1); 
	
	if (USART_RXBufferData_Available(&USARTBuf_mbus))
	{
		rxd=USART_RXBuffer_GetByte(&USARTBuf_mbus);
		
		if (rxd=='>')
		{
			retval=1;
		}
	}
	mbus_configmode(0);
	return retval;
}

uint8_t poll_and_purge(void) /* Get first byte and scrap the rest in buffer. Timeout after 2 ticks */
{
	uint8_t pos=0;
	uint8_t retval=0;
	uint8_t rtcc_start;
	
	rtcc_start=rtcc;
	
	do 
	{
		if (USART_RXBufferData_Available(&USARTBuf_mbus))
		{
			if (pos==1)
			{
				retval=USART_RXBuffer_GetByte(&USARTBuf_mbus);
			}
			else
			{
				USART_RXBuffer_GetByte(&USARTBuf_mbus); // Scrap
			}
			pos++;
		}
	} while ((pos<3)&&((rtcc-rtcc_start))<2);
	return retval;
}

uint8_t mbus_probe(void)
{
	uint8_t retval=0;
	uint8_t vpos=0;
	if (mbus_enter_config())
	{
		for (uint8_t tcc=0x61; tcc<=0x6c; tcc++)
		{
			USART_TXBuffer_PutByte(&USARTBuf_mbus,'Y');	// Enter read mode
			USART_TXBuffer_PutByte(&USARTBuf_mbus,tcc);
			if(poll_and_purge()==pgm_read_byte(verifikat+vpos))
			{
				vpos++;
			}
		}
		
		if (vpos==11) // OK, it verfied
		{
			printf_P((const char *)verifikat);
			printf_P(PSTR(" found. HW: "));
			retval=1;
		
		
			for (uint8_t tcc=0x6e; tcc<=0x71; tcc++)
			{
				USART_TXBuffer_PutByte(&USARTBuf_mbus,'Y');	// Enter read mode
				USART_TXBuffer_PutByte(&USARTBuf_mbus,tcc);
				putchar(poll_and_purge());
			}
			
			printf_P(PSTR(" FW: "));
		
			for (uint8_t tcc=0x73; tcc<=0x76; tcc++)
			{
				USART_TXBuffer_PutByte(&USARTBuf_mbus,'Y');	// Enter read mode
				USART_TXBuffer_PutByte(&USARTBuf_mbus,tcc);
				putchar(poll_and_purge());
			}
			
			putchar('\n');
			putchar('\r');
			
		}
		USART_TXBuffer_PutByte(&USARTBuf_mbus,'X'); // Exit config mode
	
	}
	return retval;
}

uint16_t mbus_validate(void)	//Get CRC of config
{
	uint8_t pos=0;
	uint16_t crc=0xffff;
	uint8_t rtcc_start;
	
	rtcc_start=rtcc;

	if (mbus_enter_config())
	{
		USART_TXBuffer_PutByte(&USARTBuf_mbus,'0');	// Read all config.
	
		do 
		{
			if (USART_RXBufferData_Available(&USARTBuf_mbus))
			{
				uint8_t rx;
				rx=USART_RXBuffer_GetByte(&USARTBuf_mbus);
				crc=_crc16_update(crc, rx);
				pos++;
			}
		
		} while ((pos<65)&&((rtcc-rtcc_start))<2); // Check 64 first bytes. Later, we have serialnumber, keys and versions.
		
		_delay_ms(1); // Wait for transfer to complete
		
		while (USART_RXBufferData_Available(&USARTBuf_mbus)) // Thow away the rest data
		{
			USART_RXBuffer_GetByte(&USARTBuf_mbus);
		}
	}
	USART_TXBuffer_PutByte(&USARTBuf_mbus,'X'); // Exit config mode
	return crc;
}

uint8_t mbus_setup(const uint8_t *config)	// Load config to module NV-Mem (NOT for frequent use)
{
	uint8_t *eepos;
	uint8_t addr, data, rxd;
	
	eepos=config;


	
	if (mbus_enter_config())
	{	
		USART_TXBuffer_PutByte(&USARTBuf_mbus,'M');
		
		_delay_ms(3); // Twait * 2
		
		if (USART_RXBufferData_Available(&USARTBuf_mbus))
		{
			rxd=USART_RXBuffer_GetByte(&USARTBuf_mbus);
		
			if (rxd!='>')
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	
		do {
			if (eeprom_is_ready())
			{
			
				addr=eeprom_read_byte(eepos++);
				data=eeprom_read_byte(eepos++);
			
				if (addr==0xff)
				{
					USART_TXBuffer_PutByte(&USARTBuf_mbus,addr); // Send 0xff to quit
					printf("End");
				}
				else
				{
					USART_TXBuffer_PutByte(&USARTBuf_mbus,addr);
					USART_TXBuffer_PutByte(&USARTBuf_mbus,data);					
				}
			}
		} while (addr!=0xff);
		
		_delay_ms(100); // TMemory * 3
		
		while (USART_RXBufferData_Available(&USARTBuf_mbus)) // Thow away the rest data
		{
			USART_RXBuffer_GetByte(&USARTBuf_mbus);
		}
		
		USART_TXBuffer_PutByte(&USARTBuf_mbus,'X'); // Exit config mode
		return 1;
	}
	return 0;
}

uint8_t mbus_get_package(uint8_t *buf)
{
	static MB_STATE_t mb_state=MB_STOPPED;
	
	uint8_t rxd;
	uint8_t retval=MBWAIT;
	static uint8_t len=0;
	static uint8_t cnt=0;
	static uint8_t stick;
		
	if (mb_state==MB_DONE)
		mb_state=MB_STOPPED;

	while (USART_RXBufferData_Available(&USARTBuf_mbus)) 
	{
		rxd=USART_RXBuffer_GetByte(&USARTBuf_mbus);

		if (mb_state==MB_STOPPED)
		{
			// Just throw away
			if (rxd==0x68)
			{
				mb_state=MB_SYNC_START;
				stick=rtcc;
			}
		}
		else if (mb_state==MB_SYNC_START)
		{
			//Next byte is length
			len=rxd;
			cnt=0;
			mb_state=MB_KNOWN_LEN;
			buf[cnt]=rxd;
		}
		else if (mb_state==MB_KNOWN_LEN)
		{
			cnt++;
			buf[cnt]=rxd;
			if (cnt==len) // End
			{
				mb_state=MB_SYNC_TRAIL;
			}
		}
		else if (mb_state==MB_SYNC_TRAIL)
		{
			if (rxd==0x16)
			{
				mb_state=MB_DONE;
				retval=MBDONE;
			}
			else if (rxd==0x68)
			{
				mb_state=MB_SYNC_START;
				retval=MBERR_SYNC;
			}			
			else if ((rtcc-stick)>2)
			{
				mb_state=MB_DONE;
				retval=MBERR_TMO;
			}
		}
	}
	return retval;
}

void mbus_send_package(uint8_t len, uint8_t *buf)
{
	for (uint8_t bpos=0; bpos<len; bpos++)
	{
		USART_TXBuffer_PutByte(&USARTBuf_mbus,buf[bpos]); 
	}
}