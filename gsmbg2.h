#ifndef GSMBG2
#define GSMBG2

#include <inttypes.h>
#include <avr/io.h>
#include "gsmbg2.h"

/** \file
 *  \brief B2G_E GSM low-level definitions.
 *  \author Per Johansson
 *  \date 	2016-05-22
 *  \note See B2G_E Datasheet
 */

/** \ingroup GSMLL
 *  \defgroup GSMLLSW GSM module low-level interface
 *  \brief GSM low-level definitions.
 *
 *  GSM code for hardware init of B2G_E module.
 *
 *  @{
 */

		/* Defines: */
			/** Defines for hardware interface pins
			 */

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

			/** Function to start pre-charge (high impedance) of VCC.			 */	
#define gsm_vcc_precharge()	PORTA.OUTSET=GSMPC

			/** Function to turn on VCC.			 */	
#define gsm_vcc_enable()	PORTA.OUTSET=GSMVEN

			/** Function to turn of VCC.			 */	
#define gsm_vcc_disable()	PORTA.OUTCLR=(GSMPC|GSMVEN)

// En=RTSLOW=Request (=+12V)
			/** Function set RTS. En=RTSLOW=Request (=+12V)			 */	
#define gsm_rts_enable()	PORTC.OUTCLR=GSMRTS

			/** Function clear RTS En=RTSLOW=Request (=+12V).		 */	
#define gsm_rts_disable()	PORTC.OUTSET=GSMRTS


// En=DTRLOW=Ready (=+12V)

			/** Function set DTR En=DTRLOW=Ready (=+12V).		 */	
#define gsm_dtr_enable()	PORTC.OUTCLR=GSMDTR

			/** Function clear DTR En=DTRLOW=Ready (=+12V).		 */	
#define gsm_dtr_disable()	PORTC.OUTSET=GSMDTR

			/** Function turn on module.		 */	
#define gsm_is_on()			((~PORTC.IN)&GSMON)

			/** Test CTS-line.		 */	
#define gsm_is_cts()		(PORTC.IN&GSMCTS)

			/** Test Ring-line.		 */	
#define gsm_is_ring()		(PORTC.IN&GSMRING)

/** Function set up all hardware pins for GSM module.		 */	
void gsm_hwinit(void);

/** Function send an ON-pulse to the GSM-module		 */	
void gsm_on(void);
/** @} */
#endif