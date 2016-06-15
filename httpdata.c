#include "uart.h"
#include "httpdata.h"
#include <stdio.h>
#include <avr/pgmspace.h>
#include <string.h>

const unsigned char httppre[] PROGMEM ="POST /ngw/p.php HTTP/1.1\r\nHost: www.hoj.nu";
const unsigned char httpcon[] PROGMEM ="\r\nContent-Type: application/json\r\nContent-Length: ";

void httpsend(char *host, char *payload)
{
	unsigned char chr;
	unsigned char *adr;
	uint8_t len;
	unsigned char slen[8];
	
	flush_gsm();
	
	adr=(unsigned char *)httppre;

	while ((chr=pgm_read_byte(adr++)))
	{
		while(Transmit_gsm(chr)==0);
		putchar(chr);
	} 
/*	
	while ((chr=*host++)!=0)
	{
		Transmit_gsm(chr);
		putchar(chr);
	}
*/	
	adr=(unsigned char *)httpcon;
	
	while ((chr=pgm_read_byte(adr++)))
	{
		while(Transmit_gsm(chr)==0);
	} 

	len=strlen(payload);
	sprintf((char *)slen, "%d\r\n\r\n",len);

	adr=slen;
		
	while ((chr=*adr++)!=0)
	{
		while(Transmit_gsm(chr)==0);
	}
		
	while ((chr=*payload++)!=0)
	{
		while(Transmit_gsm(chr)==0);
	}
}

uint8_t http_response(char *bigbuf)
{

	uint8_t retval=HTTP_ERR;
/*
	uint8_t strpos=0;
	uint16_t adr;
	uint8_t toklen,copylen;
	uint8_t rxd;
	char *startpos;
	
	if (USART_RXBufferData_CRInBuffer(&USARTBuf_gsm)) // We got CR in buffer.
	{
		do 
		{
			rxd=USART_RXBuffer_GetByte(&USARTBuf_gsm); // read byte
			if (strpos<(rem_length-1)) // Make space for \0
			{
				remainder[strpos++]=rxd;
			}		
		} while (rxd!='\r'); // Copy all data until end of line
		remainder[strpos]=0;	// Trim end of string.
		for (uint8_t vpos=0; vpos<(sizeof(gsm_response)/sizeof(gsm_response[0])); vpos++)
		{
			adr=(uint16_t)pgm_read_word(&(gsm_response[vpos]));
			if ((startpos=strstr_PF ( remainder, adr ))!=0)
			{
			
				toklen=strlen_PF(adr);
				copylen=strlen(startpos)-toklen;
				// Do some basic sanity checks before memmove
				if(copylen>0) 
				{									
					memmove(remainder, startpos+toklen, copylen );// Trim the URC
					remainder[copylen]=0;
				}
				retval=vpos;
				break;
			}
	
		}
	}
	
	return retval;

*/
	return retval;
}