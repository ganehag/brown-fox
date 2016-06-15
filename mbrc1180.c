#include <inttypes.h>
#include <avr/io.h>
#include "mbrc1180.h"

void mbus_init(void)
{
	PORTD.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc|PORT_SRLEN_bm; //GSMSON Pullup
	PORTD.OUTSET=MBUSRES_PIN; // Normal operation
	PORTD.DIRSET=MBUSRES_PIN;	
	
	PORTD.OUTSET=MBUSCFG_PIN; // Normal operation
	PORTD.DIRSET=MBUSCFG_PIN;
}

void mbus_configmode(uint8_t config)
{
	if (config)
	{
		PORTD.OUTCLR=MBUSCFG_PIN; // Config mode
	}
	else
	{
		PORTD.OUTSET=MBUSCFG_PIN; // Normal operation
	}
}

void mbus_reset(uint8_t reset)
{
	if (reset)
	{
		PORTD.OUTCLR=MBUSRES_PIN; // Config mode
	}
	else
	{
		PORTD.OUTSET=MBUSRES_PIN; // Normal operation
	}
}