#ifndef GSMBG2
#define GSMBG2

#include <inttypes.h>
#include <avr/io.h>
#include "gsmbg2.h"

#define GSMPC 	PIN5_bm
#define GSMVEN	PIN6_bm
#define GSMON	PIN6_bm
#define GSMRING	PIN1_bm
#define GSMCTS	PIN2_bm
#define GSMDTR	PIN4_bm
#define GSMRTS	PIN5_bm
#define GSMSON	PIN6_bm
#define GSMRST	PIN7_bm

#define GSMOUTS	GSMRTS|GSMDTR|GSMRST // and GSMTXD but it is handeld by USART. 


#define gsm_vcc_precharge()	PORTA.OUTSET=GSMPC
#define gsm_vcc_enable()	PORTA.OUTSET=GSMVEN
#define gsm_vcc_disable()	PORTA.OUTCLR=(GSMPC|GSMVEN)

// En=RTSLOW=Request (=+12V)
#define gsm_rts_enable()	PORTC.OUTCLR=GSMRTS
#define gsm_rts_disable()	PORTC.OUTSET=GSMRTS


// En=DTRLOW=Ready (=+12V)
#define gsm_dtr_enable()	PORTC.OUTCLR=GSMDTR
#define gsm_dtr_disable()	PORTC.OUTSET=GSMDTR


void gsm_hwinit(void);
void gsm_on(void);

#endif