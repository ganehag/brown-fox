#include "eeprom.h"
#include <avr/eeprom.h>
#include <string.h>
#include "gsm.h"

uint16_t EEMEM MB_CRC=0x91a2;
uint8_t  EEMEM MB_CONF[][2]={{0x05,0x01},{0x36,0x04},{0x3A,0x01},{0xff,0xff}};

uint8_t EEMEM EEPAR_1[EEPAR_1_LEN]={00};
uint8_t EEMEM EEPAR_2[EEPAR_2_LEN]={00};
uint8_t EEMEM EEPAR_3[EEPAR_3_LEN]={00};

uint8_t update_param(uint8_t param, char *value)
{
	uint8_t ptype=0;
	uint8_t psize=0;

	uint8_t *padr=0;

	uint8_t retval=0x80;
	
	if (param==1)
	{
		ptype=EEPAR_1_TYP;
		psize=EEPAR_1_LEN;
		padr =EEPAR_1;
	}
	
	else if (param==2)
	{
		ptype=EEPAR_2_TYP;
		psize=EEPAR_2_LEN;
		padr =EEPAR_2;
	}
	
	else if (param==3)
	{
		ptype=EEPAR_3_TYP;
		psize=EEPAR_3_LEN;
		padr =EEPAR_3;
	}
	
	else
	{
		retval=0x81;
	}
	
	while (!eeprom_is_ready( )); // Wait

	if (ptype==PARTYP_STR)
	{
		uint8_t len;
		if ((len=(strlen(value)+1))<psize)
		{
			eeprom_update_block (value, padr, len );
			retval=0;
		}
	
	}
	else if (ptype==PARTYP_NUMB)
	{

		uint16_t word;
		uint8_t num[3];

		str_htoi((char*)num, value, 2);
		
		word=(num[0]<<8)+num[1];
		
		eeprom_update_word((uint16_t*)padr, word);
		retval=0;
	}
	return retval;
}