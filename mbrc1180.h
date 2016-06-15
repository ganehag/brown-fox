#ifndef MBUSRCDEF
#define MBUSRCDEF

#include <inttypes.h>
#include <avr/io.h>

/** \file
 *  \brief Radiocraft RC1180-MBUS hardware definitions.
 *  \author Per Johansson
 *  \date 	2016-04-25
 *  \note See RC1180 Datasheet
 */

/** \ingroup MBUS
 *  \defgroup MBUSSW MBus module low-level interface
 *  \brief Radiocraft RC1180-Mbus low-level definitions.
 *
 *  Mbus module related definitions common to all architectures. This module contains definitions which
 *  are used when the MBUS is initialized.
 *
 *  @{
 */

/* Private Interface - Should not be used in end-application: */
		/* Defines: */
			/** Defines for hardware interface
			 */
#define MBUSCFG_PIN		PIN4_bm		/**< Config pin connection
									*/
				                    
#define MBUSRES_PIN		PIN1_bm		/**< Reset pin connection
									*/
				                    

		/* Function Prototypes: */
			/** Function to initialize module hardware
			 *
			 */
void mbus_init(void);

			/** Function to control config hardware-line.
			 *	See also \ref mbus_enter_config
			 *
			 *  \param[in] config                 	True (1) will set the module to config mode
			 *
			 *  \note Config mode should be reset when the config prompt '>' is received.
			 *        By latest, it must be reset before sending the 'X' exit command
			 */
void mbus_configmode(uint8_t config);

			/** Function to control reset hardware-line.
			 *
			 *  \param[in] reset                 	True (1) will set the module to reset
			 *
			 *  \note Reset mode must be reset manually by calling this function again with false as parameter.
			 */
void mbus_reset(uint8_t reset);

#endif
/** @} */
