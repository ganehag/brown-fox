#include <stdio.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "uart.h"
#include "gdefs.h"
#include "gsm.h"
#include "gsmbg2.h"
#include <avr/sleep.h>
#include <wdt.h>
#include <avr/pgmspace.h>
#include "eeprom.h"



#define SHA204_COMMAND_FUNCTIONS


#include "lib_atsha/sha204_lib_return_codes.h"  // declarations of function return codes
#include "lib_atsha/sha204_comm_marshaling.h"   // definitions and declarations for the Command module

uint8_t handle_cfgsms(char *sender, char* cfg)
{
	char *paramid;
	char *dataval;
	char *hash;
	
	uint8_t param;
	uint8_t retval=0xff;
	
	uint8_t command[CHECKMAC_COUNT];
	uint8_t challenge[CHECKMAC_CLIENT_CHALLENGE_SIZE];
		// data for challenge in MAC mode 0 command
	uint8_t other[CHECKMAC_OTHER_DATA_SIZE] = {
		0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00
	};
	
	static const uint8_t response_ok[CHECKMAC_RSP_SIZE]={0x04,0x00,0x03,0x40};
	
	uint8_t response[CHECKMAC_CLIENT_RESPONSE_SIZE];
	
	// Make the response buffer the size of a MAC response.
	uint8_t result[CHECKMAC_RSP_SIZE];

	
	uint8_t fcc, ret_code;
	
	// Trim whitespace
	
	paramid=strtok(cfg,"<"); // Get message status;
	dataval=strtok(NULL,"<"); // Get message id;
	hash=strtok(NULL,"<"); // Get message id;
	
	if ((paramid!=0)&&(dataval!=0)&&(hash!=0))	// Sanity check
	{
		if ((paramid[0]>='0')&&(paramid[1]>='0'))
		{
			param=ZIP_htoi(paramid[0])*16;	// Since all buffers are non-zero, we don't need to check index range (Size ARE larger than 2)
			param+=ZIP_htoi(paramid[1]);

			if (param<100)
			{
				memset(challenge,0,CHECKMAC_CLIENT_CHALLENGE_SIZE);		// Fill buffer
	
				strncpy((char *)challenge,dataval,CHECKMAC_CLIENT_CHALLENGE_SIZE);	// Copy data  
				other[2]=param;

				str_htoi((char *)&other[7], sender, CHECKMAC_OTHER_DATA_SIZE-7);
				
				str_htoi((char *)response, hash, CHECKMAC_CLIENT_RESPONSE_SIZE);
	

				// Wake up the device.
				ret_code = sha204c_wakeup(&result[0]);
				if (ret_code != SHA204_SUCCESS) {
					retval=1;
				}
	
				memset(result,0,CHECKMAC_RSP_SIZE);		// Fill buffer
	
				ret_code=sha204m_check_mac(command, result ,0 , 0, challenge, response, other);
				if (ret_code != SHA204_SUCCESS) {
					retval=2;
				}

				for (fcc = 0; fcc < 4; fcc++) {
					if (result[fcc]!=response_ok[fcc])
					{
						break;
					}
				}
				if (fcc==4)
				{
					printf("\nIdentical!\n");
					retval=update_param(param, dataval);
				}
				// Put device to sleep.
				ret_code = sha204p_sleep();
			}
		}
	}
	return retval;
}