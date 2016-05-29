#ifndef gsmh
#define gsmh

/** \file
 *  \brief GSM definitions.
 *  \author Per Johansson
 *  \date 	2016-05-22
 *  \note See B2G_E Datasheet
 */

/** \ingroup GSM
 *  \defgroup GSMSW GSM module mid-level interface
 *  \brief GSM mid-level definitions.
 *
 *  GSM code for normal operation of the module.
 *
 *  @{
 */

		/* Defines: */
			/** Defines for hardware interface
			 */

#define GSM_RESP_LINE		8		/** New line response*/

#define GSM_PESP_REG		0		/** Registered to network*/
#define GSM_RESP_ISOPN		1		/** Internet OPEN **/
#define GSM_RESP_ISTRN		2		/** Internet CONNECT **/
#define GSM_RESP_ISDISC		3		/** Internet CONNECT **/
#define GSM_RESP_SMS		4		/** New SMS response*/
#define GSM_RESP_READSMS	5		/** Read SMS response*/
#define GSM_LIST_READSMS	6		/** List SMS response*/
#define GSM_RESP_SIMID		7		/** SIM-ID response*/

#define GSM_TIMEOUT			0xFF
#define GSM_PARSEERR		0xFE
#define GSM_OK				0x00

	/* Function Prototypes: */

			/** Function to wait for either 'OK' response or numeric response code 
			*			 
			*  \param numeric      If true, a numerical response is expected 	
			*
			*  \return GSM_OK if OK was retrieved. Unexpected characters gives GSM_PARSEERR.
			*   On timeout GSM_TIMEOUT is returned. Numerical codes other than 0-'ok' are also returned.
			*/	
uint8_t gsm_waitok(uint8_t numeric);

		/** Function to send a predefined command.
		*			 
		*  \param cmdidx      numrical index of the command string in the gsm_table structure 	
		*  \param addendum    null-terminated string to send after command-string (typically parameter value) 		
		*
		*  \return Allways return GSM_OK
		*/	
uint8_t gsm_sendcmd(uint8_t cmdidx, char* addendum);

			/** Function wait for a command response
			 *			 
			 *  \param responseidx      numerical index to the response string in gsm_response buffer
			 *
			 *	\param[out] remainder	Pointer to a buffer to store any additional data.
			 *
			 *	\param rem_length		Maximun number of bytes to put in remainder-buffer.
			 *
			 *  \return GSM_OK if OK was retrieved. Unexpected characters gives GSM_PARSEERR.
			 *   On timeout GSM_TIMEOUT is returned. 
			 *
			 */	
uint8_t gsm_waitresp(uint8_t responseidx, char * remainder, uint8_t rem_length);

			/** Function to send the init-commands
			 *	
			 *  \return GSM_OK if everything was acked as expected. Everything else are undefined errors.
			 *
			 */	
uint8_t gsm_init(void);

			/** Function poll for a command response. Tries to match against any string in gsm_response buffer 
			 *			 
			 *	\param[out] remainder	Pointer to a buffer to store any additional data.
			 *
			 *	\param rem_length		Maximun number of bytes to put in remainder-buffer.
			 *
			 *  \return GSM_OK if OK was retrieved. Unexpected characters gives GSM_PARSEERR.
			 *   On timeout GSM_TIMEOUT is returned. 
			 *
			 *  \todo Verify roubustness			 
			 */				 
uint8_t gsm_poll(char *remainder, uint8_t rem_length);

			/** Function identify module. Returns module identification string. 
			 *			 
			 *	\param[out] verstr	Pointer to a buffer to store identification/version string.
			 *
			 *	\param rem_length		Maximun number of bytes to put in version-buffer.
			 *
			 *  \return GSM_OK if OK was retrieved. Unexpected characters gives GSM_PARSEERR.
			 *   On timeout GSM_TIMEOUT is returned. 
			 *			 
			 */				 
uint8_t gsm_identify(char *verstr, uint8_t verlen);

			/** Function retrieve SIMcard ID. 
			 *			 
			 *	\param[out] verstr	Pointer to a buffer to store id-number.
			 *
			 *	\param rem_length		Maximun number of bytes to put in remainder-buffer.
			 *
			 *  \return GSM_OK if OK was retrieved. Unexpected characters gives GSM_PARSEERR.
			 *   On timeout GSM_TIMEOUT is returned. 
			 *			 
			 */				 
uint8_t gsm_simid(char *verstr, uint8_t verlen);

			/** Function read SMS. 
			 *
			 *	\param msgidx		Index of SMS to read.
			 *			 
			 *	\param[out] msgdata	Pointer to a buffer to store id-number.
			 *
			 *	\param datalen		Maximun number of bytes to put in message-buffer.
			 *
			 *  \return GSM_OK if OK was retrieved. Unexpected characters gives GSM_PARSEERR.
			 *   On timeout GSM_TIMEOUT is returned. 
			 *
			 *  \todo Verify roubustness			 
			 */
uint8_t gsm_read_sms(char *msgidx, char *msgdata, uint8_t datalen);

			/** Function send SMS. 
			 *
			 *	\param tlen		Length of data excuding the RP layer SMSC address octets
			 *			 
			 *	\param[out] msgdata	Message (and adress) in PDU format.
			 *
			 *  \return GSM_OK if OK was retrieved. Unexpected characters gives GSM_PARSEERR.
			 *   On timeout GSM_TIMEOUT is returned. 
			 *
			 */
uint8_t gsm_send_sms(char *tlen, char *msgdata);

			/** Function send OK-SMS. 
			 *
			 *	\param number	string with receiver in 46xxxxx format.
			 *			 
			 *	\param msgdata	Message (and adress) in PDU format.
			 *
			 *  \return length of data for gsm_send_sms function
			 *
			 */
uint8_t gsm_make_ok_response(char *number, char *msgdata, uint8_t datalen);

			/** Function delete SMS. 
			 *
			 *	\param msgidx		Index of SMS to read.
			 *			 
			 *
			 *  \return GSM_OK if OK was retrieved. Unexpected characters gives GSM_PARSEERR.
			 *   On timeout GSM_TIMEOUT is returned. 
			 */
uint8_t gsm_delete_sms(char *msgidx);

			/** Function list sms. Only sends a command, leaving the response to the periodic (gsm_poll) function
			 *
			 *
			 *  \return GSM_OK if OK was retrieved. Unexpected characters gives GSM_PARSEERR.
			 *   On timeout GSM_TIMEOUT is returned. 
			 *
			 */
uint8_t gsm_poll_sms(void);

			/** Function getline. 
			 *
			 *	\param[out] buf		data buffer.
			 *
			 *	\param buflen		Maximum number of bytes to put in buf
			 *			 
			 *  \return GSM_OK if OK was retrieved. Unexpected characters gives GSM_PARSEERR.
			 *   On timeout GSM_TIMEOUT is returned. 
			 *
			 */
uint8_t gsm_getline(char *buf, uint8_t buflen);

			/** Function gsm_off. Turns module off
			 *	
			 *
			 */
void gsm_off(void);

			/** Function ZIP_htoi. Converts a hex-nibble to integer
			 *
			 *	\param hex		Hex-nibble to convert.
			 *			 
			 *  \return integer 
			 *
			 */
uint8_t ZIP_htoi( unsigned char hex );

			/** Function str_htoi. converts a hex-string to integer-array
			 *
			 *	\param[out] dst		integer data buffer.
			 *
			 *	\param src			character string 
			 *
			 *	\param len			Maximum number of bytes to put in dst
			 *
			 *			 
			 *  \return none
			 *
			 */
void str_htoi(char *dest, char *scr, uint8_t len);


			/** Function flush_gsm. Just empty buffer
			 *
			 *			 
			 *  \return none
			 *
			 */
void flush_gsm(void);

			/** Function gsm_open_data. Open data connectoin
			 *
			 *			 
			 *  \return none
			 *
			 */
void gsm_open_data(void);

			/** Function gsm_close_data. Close an open data connection
			 *
			 *			 
			 *  \return none
			 *
			 */
void gsm_close_data(void);

			/** Function gsm_connect_data. Connect serial interface to the open data interface
			 *
			 *			 
			 *  \return none
			 *
			 */
void gsm_connect_data(void);

			/** Function gsm_drop_data. Disconnect serial interface to the open data interface
			 *
			 *			 
			 *  \return none
			 *
			 */
void gsm_drop_data(void);

			/** Function gsm_data_trans. converts a hex-string to integer-array
			 *
			 *	\param txbuf		String to send.
			 *
			 *	\param txlen		Length of string to send 
			 *
			 *  \param[out] rxbuf	Buffer for receive data
			 *
			 *	\param rxlen		Maximum number of bytes to put in rxbuf
			 *
			 *			 
			 *  \return 	Number fo bytes received or 0 if error or no data.
			 *
			 */
uint8_t gsm_data_trans(char * txbuf, uint8_t txlen, char *rxbuf, uint8_t rxlen);

/** @} */
#endif