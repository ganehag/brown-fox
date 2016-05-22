#ifndef eepdef
#define eepdef

#include <avr/eeprom.h>


/** \file
 *  \brief Radiocraft RC1180-MBUS hardware definitions.
 *
 *  \note See RC1180 Datasheet
 */

/** \ingroup EEPROM
 *  \defgroup EEPROM EEProm memory organisation
 *  \brief Contains all eeprom structures.
 *
 *
 *  @{
 */
 		/* EEprom Variables: */
			/** RC1180 configuration array.
			 *  Each row should contain two bytes, adress and data. See RC1180 datasheet
			 *
			 *  \attention Last row pair must be 0xff,0xff.
			 *
			 *  \ingroup EEPROM
			 */
extern uint8_t MB_CONF[][2];


			/** GSM configuration array.
			 *  Each parameter sould  me either numeric or string. Strings are 32 bytes, numbers 2.
			 *
			 *  \attention Last row pair must be 0xff,0xff.
			 *
			 *  \ingroup EEPROM
			 */

#define PARTYP_STR		1
#define PARTYP_NUMB		2

#define EEPAR_1_TYP		PARTYP_STR
#define EEPAR_1_LEN		32		
extern uint8_t EEPAR_1[EEPAR_1_LEN];
#define EEPAR_2_TYP		PARTYP_STR
#define EEPAR_2_LEN		32		
extern uint8_t EEPAR_2[EEPAR_2_LEN];

#define EEPAR_3_TYP		PARTYP_NUMB
#define EEPAR_3_LEN		2		
extern uint8_t EEPAR_3[EEPAR_3_LEN];


uint8_t update_param(uint8_t param, char *value);

#endif
/** @} */