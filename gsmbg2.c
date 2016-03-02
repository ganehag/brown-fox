#include <inttypes.h>
#include <avr/io.h>
#include "util/delay.h" 
#include "gsmbg2.h"


void gsm_hwinit(void)
{
	PORTA.DIRSET=(GSMPC|GSMVEN);
	PORTA.OUTCLR=(GSMPC|GSMVEN);
	PORTB.OUTSET=GSMON; // Inverted
	PORTC.PIN6CTRL=PORT_OPC_PULLUP_gc; //GSMSON Pullup
	PORTC.DIRSET=GSMOUTS;
	PORTB.DIRSET=0xFF; // All port!

}

void gsm_on(void)
{
	// Inverted
	PORTB.OUTCLR=(GSMON);
	_delay_ms(100);
	PORTB.OUTSET=(GSMON);
}