#include <stdio.h>
#include <avr/pgmspace.h>
#include "uart.h"




const char string_1[] PROGMEM = "ATE0"; 		// Turn off echo
const char string_2[] PROGMEM = "ATV0";		// Numeric result codes
const char string_3[] PROGMEM = "ATI";  		// delivers a product information text.
const char string_4[] PROGMEM = "AT^SSYNC=1";	// Enables the GPIO5/LED pin to drive a status LED
const char string_5[] PROGMEM = "AT+CREG=2";	// Activates extended URC mode. 
const char string_6[] PROGMEM = "AT+CNMI=2,1"; // Turn on notification of SMS. Indication is: +CMTI: "MT",[idx]
const char string_7[] PROGMEM = "AT^SICS=0,conType,GPRS0";			// Select interface
const char string_8[] PROGMEM = "AT^SICS=0,\"inactTO\", \"20\"";		// Set timeout
const char string_9[] PROGMEM = "AT^SICS=0,apn,\"online.telia.se\""; 	// 
const char string_a[] PROGMEM = "AT^SISS=0,SRVTYPE,TRANSPARENT";		// Set transparent mode
const char string_b[] PROGMEM = "AT^SICS=0,ALPHABET,1"; 	 			// Selecet alphadet, teminal
const char string_c[] PROGMEM = "AT^SISS=0,ALPHABET,1";				// Yepp 	
const char string_d[] PROGMEM = "AT^SISS=0,ADDRESS,\"www.hoj.nu:80\"";	// 

const char string_e[] PROGMEM = "AT+CSQ";			// Signal quality
const char string_f[] PROGMEM = "AT^SMSO";		// Switch Off BG2-E

//char string_x[] PROGMEM = "AT^SMGL";		// List SMS without setting read status
const char string_g[] PROGMEM = "AT+CMGR=";	// read SMS, AT+CMGR=[idx]
const char string_h[] PROGMEM = "AT+CMGD=";	// delete SMS, AT+CMGD=[idx]
const char string_i[] PROGMEM = "AT+CMGL";	// list SMS

const char string_j[] PROGMEM = "AT^SISC=0"; // Close
const char string_k[] PROGMEM = "AT^SISO=0"; // Open
const char string_l[] PROGMEM = "AT^SIST=0"; // Transarent

#define GSM_INIT		0
#define GSM_INITLEN		12 
#define GSM_SIGNAL_Q	13
#define GSM_OFF			14
#define GSM_READSMS		15
#define GSM_DELSMS		16
#define GSM_LISTSMS		17
#define GSM_OPENIS		18
#define GSM_CLOSEIS		19
#define GSM_CONNECTIS	20

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
};

uint8_t gsm_sendcmd(uint8_t cmdidx, char* addendum)
{
	uint8_t chr;
	uint16_t adr;
	
	adr=(uint16_t)&(gsm_table[cmdidx]);
	do
	{
		chr=pgm_read_byte(adr++);
		Transmit_gsm(chr);
		putchar(chr);
	} while (chr!=0);
	
	Transmit_gsm('\n');
	Transmit_gsm('\r');

	return 0;
}

uint8_t gsm_waitresp(uint8_t responseidx, char * remainder, uint8_t rem_length)
{


}

uint8_t gsm_init(uint8_t cmd)
{
	if (cmd<GSM_INITLEN)
	{
		gsm_sendcmd(cmd,0);
		cmd++;
		return cmd;
	}
	else
		return 0;
}