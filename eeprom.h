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
extern const uint8_t MB_CONF[][2];

#endif
/** @} */