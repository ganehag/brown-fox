#include <stdio.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "uart.h"
#include "gdefs.h"
#include "gsm.h"
#include "gsmbg2.h"
#include <avr/sleep.h>
#include <wdt.h>
#include "msghnd.h"


const unsigned char sendpre[] PROGMEM = "0011000-91"; // Beginning of string
const unsigned char sendstr[] PROGMEM = "0000AA02CF25"; // OK response

const char response_1[] PROGMEM = "+CREG:"; 	// Registered on network
const char response_2[] PROGMEM = "^SIS";	// Internet service
const char response_3[] PROGMEM = "+CMTI"; 	// SMS
const char response_4[]	PROGMEM = "+CMGR:";	// Read SMS
//const char response_4[]	PROGMEM = "^SMGR:";	// Read SMS (no read mark)
const char response_5[] PROGMEM = "+CMGL:"; // List SMS
//const char response_5[] PROGMEM = "^SMGL:"; // List SMS
const char response_6[]	PROGMEM = "^SCID:";
const char response_7[]	PROGMEM = "";	// ATI response

PGM_P const gsm_response[] PROGMEM =
{
	response_1,
	response_2,
	response_3,
	response_4,
	response_5,
	response_6,	
	response_7,
};
		
const char string_1[] PROGMEM = "ATE0"; 		// Turn off echo
const char string_2[] PROGMEM = "ATV0";		// Numeric result codes

const char string_3[] PROGMEM = "ATI";  		// delivers a product information text.
const char string_4[] PROGMEM = "AT^SSYNC=1";	// Enables the GPIO5/LED pin to drive a status LED
const char string_5[] PROGMEM = "AT+CREG=2";	// Activates extended URC mode. 
const char string_6[] PROGMEM = "AT+CNMI=2,1"; // Turn on notification of SMS. Indication is: +CMTI: "MT",[idx]
const char string_7[] PROGMEM = "AT^SICS=0,conType,GPRS0";			// Select interface
const char string_8[] PROGMEM = "AT^SICS=0,\"inactTO\", \"20\"";		// Set timeout
const char string_9[] PROGMEM = "AT^SISS=0,SRVTYPE,TRANSPARENT";		// Set transparent mode
const char string_a[] PROGMEM = "AT^SICS=0,ALPHABET,1"; 	 			// Selecet alphadet, teminal
const char string_b[] PROGMEM = "AT^SISS=0,ALPHABET,1";				// Yepp 	
const char string_c[] PROGMEM = "AT^SICS=0,apn,\"online.telia.se\""; 	// 
const char string_d[] PROGMEM = "AT^SISS=0,ADDRESS,\"www.hoj.nu:80\"";	// 

const char string_e[] PROGMEM = "AT+CSQ";			// Signal quality
const char string_f[] PROGMEM = "AT^SMSO";		// Switch Off BG2-E

//const char string_g[] PROGMEM = "AT^SMGR=";	// read SMS, without setting read status AT^SMGR=[idx]
const char string_g[] PROGMEM = "AT+CMGR=";	// read SMS, AT+CMGR=[idx]
const char string_h[] PROGMEM = "AT+CMGD=";	// delete SMS, AT+CMGD=[idx]
const char string_i[] PROGMEM = "AT+CMGL=4";	// list SMS
//const char string_i[] PROGMEM = "AT^SMGL";		// List SMS without setting read status
const char string_j[] PROGMEM = "AT+CMGS=";		// Send SMS to network

const char string_k[] PROGMEM = "AT^SISC=0"; // Close
const char string_l[] PROGMEM = "AT^SISO=0"; // Open
const char string_m[] PROGMEM = "AT^SIST=0"; // Transarent

const char string_n[] PROGMEM = "AT^SCID"; // Transarent

#define GSM_ATE			0
#define GSM_ATV			1
#define GSM_ATI			2
#define GSM_FIXED_INIT	3
#define GSM_INIT_LEN	8
#define GSM_SET_APN		11
#define GSM_SET_HOST 	12
#define GSM_SIGNAL_Q	13
#define GSM_OFF			14
#define GSM_READSMS		15
#define GSM_DELSMS		16
#define GSM_LISTSMS		17
#define GSM_SENDSMS		18
#define GSM_OPENIS		19
#define GSM_CLOSEIS		20
#define GSM_CONNECTIS	21
#define GSM_SIMID		22

PGM_P const gsm_table[] PROGMEM = 
{
    string_1,
    string_2,
    string_3,
    string_4,
    string_5,
    string_6,
    string_7,
    string_8,
    string_9,
    string_a,
    string_b,
    string_c,
    string_d,
    string_e,
    string_f,
    string_g,
    string_h,
    string_i,
    string_j,
    string_k,
    string_l,
	string_m,
	string_n,	
};

uint8_t gsm_sendcmd(uint8_t cmdidx, char* addendum)
{
	uint8_t chr;
	uint16_t adr;
	
	adr=(uint16_t)pgm_read_word(&(gsm_table[cmdidx]));
	while ((chr=pgm_read_byte(adr++)))
	{
		Transmit_gsm(chr);
	} 
	
	while ((chr=*addendum++)!=0)
	{
		Transmit_gsm(chr);
	}
		// We could compare to received echo and verify that we got exclusive access to serial line. But we don't do that check at the moment.

	
	Transmit_gsm('\r');
//	Transmit_gsm('\n');
	return 0;
}

uint8_t gsm_waitok(uint8_t numeric)
{
	uint8_t retval=0xff;
	uint8_t rxd=0;
	uint8_t pos=0;

	rtc_alarm(60000); 
	do 
	{
		if (USART_RXBufferData_CRInBuffer(&USARTBuf_gsm)) // We got CR in buffer. 
		{
			do {
				rxd=USART_RXBuffer_GetByte(&USARTBuf_gsm); // read byte
				if (rxd=='\n') // reset pos if we got multiline.
				{
					pos=0;
				}
				else if ((pos==0)&&((rxd==0x20)||(rxd=='\r')))
				{
					// Nothing. Pos will stay 0 and leading whitespace trimmed.
				}
				else if ((numeric)&&(pos==0))	// For numberic response, just capture the digit
				{
					if ((rxd>='0')&&(rxd<='9'))
					{
						retval=rxd-'0';
					}
					pos=2;
				}
				else if (pos==0)	// If verbal response, first look for 'O'
				{
					if (rxd=='O')
					{
						pos=1;
					}
				}
				else if (pos==1)	// Then look for 'K'
				{
					if (rxd=='K')
					{
						retval=0;
						pos=2;
					}
				}
				else
				{
					pos++; // Unexpected
				}
				
				if ((pos>2)&&(rxd!='\r')) // Not done? Error!
				{
					retval=0xFE;
					putchar(rxd);
				}
				
			} while (rxd!='\r'); // Until end of line, which we know is in buffer
		}
	} while ((chk_alarm()==0)&&(retval==0xff)); // Until timeout or end of line
	return retval;
}

/* Todo: This could me merged with above */

uint8_t gsm_waitresp(uint8_t responseidx, char * remainder, uint8_t rem_length)
{
	uint8_t retval=0xff;
	uint8_t rxd=0;
	uint8_t pos=0;
	uint8_t strpos=0;
	uint8_t crpos=0;

	uint16_t adr;
	int8_t toklen,copylen;
	char *startpos;
	
	rtc_alarm(60000); 
	do 
	{
		if (USART_RXBufferData_CRInBuffer(&USARTBuf_gsm)) // We got CR in buffer. 
		{
			do {
				rxd=USART_RXBuffer_GetByte(&USARTBuf_gsm); // read byte
				if (strpos<rem_length)
				{
					remainder[strpos]=rxd;
				}
				if (rxd=='\n') // reset pos if we got multiline.
				{
					pos=0;
				}
				
				else if (pos==0)	// Numberic response, just capture the digit after \n
				{
					if (rxd=='0')
					{
						crpos=strpos;	// The last 0 will be substututed to NULL
					}
					pos=2;
				}
				else if ((rxd=='\r')&&((strpos-1)==crpos)) // previous character was 0
				{
					retval=0;
				}
				
				if ((pos>2)&&(rxd!='\r')) // Not done? Error!
				{
					retval=0xFE;
				}
				
				strpos++;
				
			} while (USART_RXBufferData_Available(&USARTBuf_gsm)); // Until end of line, which we know is in buffer
		}
	} while ((chk_alarm()==0)&&(retval>=0xfe)); // Until timeout or end of line
	
	remainder[crpos]=0;	// Trim OK response.
	
	if (retval==0)
	{
		adr=(uint16_t)pgm_read_word(&(gsm_response[responseidx]));
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
		}
		else
		{
			retval=0xFD;
		}
	}

	return retval;
}

uint8_t gsm_identify(char *verstr, uint8_t verlen)
{

	gsm_sendcmd(GSM_ATI,0);
//	_delay_ms(100);
	return gsm_waitresp(GSM_RESP_LINE,verstr,verlen);
}

uint8_t gsm_simid(char *verstr, uint8_t verlen)
{

	gsm_sendcmd(GSM_SIMID,0);
//	_delay_ms(100);
	return gsm_waitresp(GSM_RESP_SIMID,verstr,verlen);
}

uint8_t gsm_init(void)
{
	uint8_t retval=0;
	if (!gsm_is_on())
	{
		return 0xff;
	}

	gsm_sendcmd(GSM_ATE,0);
	retval+=gsm_waitok(0);
	_delay_ms(100);
	
	if (retval)	// Maybe we already was in numeric mode... Test.
	{
		gsm_sendcmd(GSM_ATE,0);
		retval=gsm_waitok(0);
		_delay_ms(100);
	}
	
	if (retval)
	{
		return retval; // If we failed already, there probably are no-one listening, so just fail.... 
	}
	
	gsm_sendcmd(GSM_ATV,0);
	retval+=gsm_waitok(1);
	_delay_ms(100);
			
	wdt_reset();

	for (uint8_t idx=GSM_FIXED_INIT; idx<GSM_FIXED_INIT+GSM_INIT_LEN; idx++)
	{
		gsm_sendcmd(idx,0);
		retval+=gsm_waitok(1);
		_delay_ms(100);
		wdt_reset();
		
	}
	return retval;
}
/* Todo: Verify roubustness */

uint8_t gsm_poll(char *remainder, uint8_t rem_length)
{
	uint8_t strpos=0;
	uint16_t adr;
	uint8_t toklen,copylen;
	uint8_t retval=0xFF;
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

}


// Decode PDU:

/*! \brief Convert hex to int
 *
 *  This function will take a hex in char format and return int value
 *
 *  \param    hex     Hex value to convert
 *
 *  \retval   int     Integer representation of hex-value
 */
uint8_t ZIP_htoi( unsigned char hex )
{

    if( ( hex >= 'A' ) && ( hex <= 'F' ) )    //Test if hex is A-->F?
    {
        return hex - 'A' + 10;
    }
	else if( ( hex >= 'a' ) && ( hex <= 'f' ) )    //Test if hex is A-->F?
    {
        return hex - 'a' + 10;
    }
    else                                      //Must have something else then:0-->9
    {
        return hex - '0';
    }
}

void str_htoi(char *dest, char *src, uint8_t len)
{
	uint8_t chr, numbr;
	uint8_t opos;
	
	for (opos=0; (opos<len); opos++)
	{
		chr=*src++;
			
		if(chr>='0')
		{
			numbr=ZIP_htoi(chr);
		}
		else
		{
			numbr=0;
			break;
		}

		chr=*src++;
			
		if(chr>='0')
		{
			numbr=numbr*16;
			numbr+=ZIP_htoi(chr);
			dest[opos]=numbr;
		}
		else
		{
			//Keep value from first
			dest[opos]=numbr;
			break;
		}
	}
	for (opos++; (opos<len); opos++)
	{
		dest[opos]=0;	// Set remaining bytes to 0
	}
}

const unsigned char mask[7]              = {1,3,7,15,31,63,127};  //!< Lookup table -> mask

/*! \brief ZIP_decompress will decode a message in PDU format into a readable string
 *
 *  \param    *compressed PDU encoded text
 *  \param    *decompressed Pointer to return array
 *
 *  \retval   iii Number of decoded chars
 */
int PDU_decompress( unsigned char *compressed, unsigned char *decompressed )
{

    //! Local variables
    int i,ii,iii;                                                           //String index
    unsigned char rest_c, ans_c ,dec_c , this_c, next_c;                    //Read and temorary variables

    for( i = ii = iii = rest_c = 0; (this_c = compressed[i++]) != '\0'; )   //Run through complete string
    {
        //Read:
        next_c = compressed[i++];                                           //Read from in buffer in AVR_SMS_com.c
        //Convert:
        dec_c = 16 * ZIP_htoi( this_c ) + ZIP_htoi( next_c );               //Decimal value of the two chars
        ans_c = dec_c & mask[6 - ii];                                       //Mask out the correct bits
        ans_c <<= ii;														//Left shift proper correct of times
        decompressed[iii++] = ans_c + rest_c;   
		//Store
        rest_c = (dec_c & ~mask[6 - ii]) >> ( 7 - ii );                     //Saving up for next time

        if( ii == 6)                                                        //Do carry
        {
            ii = 0;
            decompressed[ iii++ ] = rest_c;
            rest_c = 0;
        }

        else
        {
            ii++;
        }
    }

    decompressed[ iii ] = '\0';                                             //Terminate string in a proper manner
    return iii;                                                             //Return length
}

/*! \brief Get start of compressed string
 *
 *  When a new message has been read from a given index, thsi function\n
 *  will run through it, and find start of the encoded user text.
 *
 *  \param    string buffer
 *
 *  \retval   in_handle if error in_handle points to '\0', else it will point to the string ready for decompression
 *
 */
unsigned char* TOOLS__decodeCMGR( unsigned char* in_handle, unsigned char *sender )
{

	//! Local variables
	int i, j, len, field_length;
	unsigned char* tmp_handle;
	
	len=strlen((char *)in_handle);
	
	tmp_handle=in_handle;
		
	if ((*tmp_handle=='\n'))
	{
		tmp_handle++;
	}

    for( i = 0; ( i < len ) && ( *tmp_handle++ != '\n' ); i++)   //Loop until we get '\n'
    {
        ;
    }
		
    for( j = 0; j < len ; j++)   //Loop until we get next '\n' and trim string
    {
        if (tmp_handle[j]=='\n')
			tmp_handle[j]='\0';
    }

    //Error
    if( i >= len )
    {
        tmp_handle = '\0';
        return tmp_handle;
    }
    //Everything OK
    else
    {
//		tmp_handle[90]='\0';
//        field_length = 16 * ZIP_htoi( *in_handle++ );
		tmp_handle++;
        field_length = ZIP_htoi( *tmp_handle++ );

        tmp_handle += 2*field_length + 2;                        //Skip Service center nmbr and First octet of this SMS-DELIVER message

        //Find length of sender address
//        field_length = 16 * ZIP_htoi( *in_handle++ );
		tmp_handle++;
        field_length = ZIP_htoi( *tmp_handle++ );

        if( field_length%2 )
        {
            field_length++;                                     //Check for trailing F
        }

        tmp_handle += 2;										// Skip length
		if (sender)												// If we supplied pointer to save sender
		{
			uint8_t idx=0;
			uint8_t tmpc;
			for (uint8_t fcc=0; fcc<field_length;) // inc is done in loop
			{
				tmpc=tmp_handle[fcc++];
				sender[idx++]=tmp_handle[fcc++];

				if (tmpc>='0'&&tmpc<='9') // Skip last 'F'
				{
					sender[idx++]=tmpc;
				}
			}
			sender[idx]=0;
		}

		tmp_handle+=field_length;                          		//Skip sender fields in header

        tmp_handle += 14 + 4;                                    //Skip more fields...TP-PID, TP-DCS and TP-SCTS

//		len=ZIP_htoi(tmp_handle[0])*16;
//		len+=ZIP_htoi(tmp_handle[1])-1;
        return ( tmp_handle+2 );                               //Return correct pointer (strip size field)
    }
}

/*Returns: 	<stat>, [<alpha>], <length>
			<pdu>
			
			where stat: 0 Unread, 1 Read. alpha to be ignored, length of the actual TP data unit in octets (i.e. the
			RP layer SMSC address octets are not counted in the length) pdu is data.
*/		
			

uint8_t gsm_read_sms(char *msgidx, char *msgdata, uint8_t datalen)
{
	uint8_t resp;
	unsigned char *msg;
	unsigned char nbuf[16];
	unsigned char sms[160];
	uint8_t txlen;
//	uint8_t chr, rxd;
	gsm_sendcmd(GSM_READSMS, msgidx);
	resp=gsm_waitresp(GSM_RESP_READSMS, msgdata, datalen);
	
	if(resp==0) // OK
	{
		msg=TOOLS__decodeCMGR((unsigned char*)msgdata, nbuf);
		if (PDU_decompress(msg,sms))
		{
			if (handle_cfgsms((char *)nbuf, (char *)sms)==0) // Successful
			{
				txlen=gsm_make_ok_response((char *)nbuf, msgdata, datalen);
				sprintf((char *)sms, "%d",txlen); // Reusing sms buffer
				gsm_send_sms((char *)sms,(char *)msgdata);
			}
		}
		else
		{
			resp=1;	// Signal error.
		}
	}

	return resp;
}

uint8_t gsm_make_ok_response(char *number, char *msgdata, uint8_t datalen)
{
	uint8_t chra, chrb;
	uint8_t hpos=0;
	uint8_t len=0;
	uint8_t spos=0;
	const uint8_t *adr;
	
	adr=sendpre;
	
	while (((chra=pgm_read_byte(adr++))!=0)&&(datalen--))
	{
		if (chra=='-')
		{
			spos=hpos;
		}
		msgdata[hpos++]=chra;
	}
	
	while (((chra=*number++)!=0)&&(datalen--))
	{
		if ((chrb=*number)!=0)
		{
			number++;
			len++;
			msgdata[hpos++]=chrb;
		}
		else
		{
			msgdata[hpos++]='F';
		}
		msgdata[hpos++]=chra;
		len++;
	}
	
	if((len>=0)&&(len<=9))
	{
		msgdata[spos]=len+'0';
	}
	else if((len>=0x0A)&&(len<=0x0F))
	{
		msgdata[spos]=len-0x0a+'A';
	}

	adr=sendstr;

	while (((chra=pgm_read_byte(adr++))!=0)&&(datalen--))
	{
		msgdata[hpos++]=chra;
	}
	msgdata[hpos++]=0x1A; // Ctrl-Z
	msgdata[hpos]=0;
	return (strlen(msgdata)>>1)-1;
}

uint8_t gsm_send_sms(char *tlen, char *msgdata)
{
	uint8_t rxd=0, chr;
	uint8_t retval;
	gsm_sendcmd(GSM_SENDSMS,tlen);
	rtc_alarm(20000);
	do 
	{	
		if (USART_RXBufferData_Available(&USARTBuf_gsm)) // We got data in buffer. 
		{
			rxd=USART_RXBuffer_GetByte(&USARTBuf_gsm); // read byte
			if (rxd=='>')
				break;
		}
	} while (chk_alarm()==0);
				
	if (rxd=='>')
	{
		while ((chr=*msgdata++)!=0)
		{
			Transmit_gsm(chr);
			//		putchar(chr);
		}
	}
	retval=gsm_waitok(1);
	return retval;
}

uint8_t gsm_delete_sms(char *msgidx)
{
	uint8_t retval;
	gsm_sendcmd(GSM_DELSMS,msgidx);
	retval=gsm_waitok(1);
	
	return retval;
}


uint8_t gsm_poll_sms(void)
{
	// Let the main poller take care of data, just send the command 
	return gsm_sendcmd(GSM_LISTSMS, 0);;
}

uint8_t gsm_getline(char *buf, uint8_t buflen)
{
	return gsm_waitresp(GSM_RESP_LINE, buf, buflen);
}

void gsm_off(void)
{
	gsm_sendcmd(GSM_OFF,0);
}

void flush_gsm(void)
{
	while (USART_RXBufferData_Available(&USARTBuf_gsm)) // We got CR in buffer. 	
	{
		USART_RXBuffer_GetByte(&USARTBuf_gsm); // read byte
	}
}