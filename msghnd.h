#ifndef msghndh
#define msghndh

/** \file
 *  \brief GSM logic definitions.
 *  \author Per Johansson
 *  \date 	2016-05-22
 *  \note See ATSHA204A Datasheet
 */

/** \ingroup GSM
 *  \defgroup GSMLW GSM module high-level interface
 *  \brief GSM high-level definitions to handle the config SMS.
 *
 *  Code for SMS-configuration.
 *
 *	The SMS should have the following structure: paramid<config<hash
 *	paramid is a hex value in range 0-100 (dec).
 *	config is the data stored in eeprom
 *	hash is the hash to compare against.
 *
 *	There exists a script to create valid sms.
 *
 *  @{
 */
			/** Function test an SMS config request
			 *
			 *  This function first calculates a hash of sender number, parameter number and config data.
			 *  Then this is compared to the Hash-part of the packet.
			 *  If the hash matches, eeprom is updated with the parameter
			 *
			 *
			 *
			 *	\param sender		String with sender number  
			 *
			 *	\param cfg			Config packet
			 *
			 *			 
			 *  \return 0 if everything was ok. 
			 *
			 */
uint8_t handle_cfgsms(char *sender, char* cfg);

/** @} */
#endif
