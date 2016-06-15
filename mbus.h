#ifndef mbush
#define mbush

/** \file
 *  \brief Radiocraft RC1180-MBUS hardware definitions.
 *  \author Per Johansson
 *  \date 	2016-04-25
 *  \note See RC1180 Datasheet
 */

/** \ingroup MBUS
 *  \defgroup MBUSHW MBus module interface
 *  \brief Radiocraft RC1180-Mbus Device management definitions.
 *
 *  Mbus module related definitions common to all architectures. This module contains definitions which
 *  are used when the MBUS is initialized.
 *
 *  @{
 */


/* Public Interface - Could be used in end-application: */
		/* Defines: */
			/** Defines for return values for mbus_get_package
			 */

#define MBERR_TMO	1		/**< Timeout error
									*/
#define MBERR_SYNC 2		/**< Syncronisation lost error (dropped byte)
									*/
#define MBWAIT	0			/**< Not done yet
									*/
#define MBDONE	0x80		/**< Reception ready
									*/

		/* Function Prototypes: */
			/** Function to automatically enter config mode. This function will set and unset the config pin
			 *  See also \ref mbus_configmode 		 *
			 *
			 *  \return True (1) if config prompt '>' was retrieved.
			 */									
uint8_t mbus_enter_config(void);

			/** Function to probe for RC1180 module. Prints hardware and firmware revision to stdout.
			 *  
			 *  \return True (1) if module was identified.
			 */	
uint8_t mbus_probe(void);

			/** Function to validate configuration. This function will read configuration and calculate
			 *  CRC-16 checksum for the first 64 bytes
			 *
			 *  \return CRC-16 checksum for the active configuration.
			 */	
uint16_t mbus_validate(void);

			/** Function to validate configuration. This function will read configuration and calculate
			 *  CRC-16 checksum for the first 64 bytes
			 *			 
			 *  \param[in] config      Pointer to the configuration array. 
			 *  \note config array consists of adress,data pairs. Last pair should be 0xff,0xff.
			 *
			 *  \return True (1) on successful parameter load.
			 */	
uint8_t mbus_setup(const uint8_t *config);

			/** Function for retrieving data. It should be continously polled from main loop
			 *			 
			 *  \param[out] buffer      Pointer to the databuffer. \note This must be big enough. (257byte) 	
			 *
			 *  \return MBDONE when a complete package has been retrieved. During ongoing reception MBWAIT are returned.
			 *   On timeout MBERR_TMO is returned. A lost character or wrong sync char results in MBERR_SYNC
			 *
			 *  \note databuffer [0] contains length. Databuffer[length] contains RSSI-value.
			 */	
uint8_t mbus_get_package(uint8_t *buf);

			/** Function send data. 
			 *	
			 *  \param[in] len		   Number of bytes to send.
			 *  \param[in] buffer      Pointer to the databuffer. \note This must contain LEN for the radio package 	
			 *
			 */	
void mbus_send_package(uint8_t len, uint8_t *buf);

#endif
/** @} */
