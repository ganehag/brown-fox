#include <inttypes.h>
#include <onewire.h>
#include "ds18s20.h"
#include <stdio.h>

/********** Onewire/DS18S20 **********/
// Starts conversion. Could also be modified to skiprom to start conversion on several sensors.
uint8_t DS18S20_start_meas(unsigned char *id)
{
	uint8_t ret;

	if( OWI_DetectPresence() ) { // only send if bus is "idle" = high
		OWI_MatchRom(id);
		// Send start conversion command.
		OWI_SendByte(0x44);
		ret = 1;
	} 
	else { 
		printf( "DS18X20_start_meas: Short Circuit!\r" );
		ret = 0;
	}

	return ret;
}

/* Convert scatchpad to decicelcius */
int16_t DS18X20_raw_to_decicelsius( uint8_t sp[] )
{
	uint16_t measure;
	uint8_t  negative;
	int16_t  decicelsius;
	uint16_t fract;

	measure = sp[0] | (sp[1] << 8);
	//measure = 0xFF5E; // test -10.125
	//measure = 0xFE6F; // test -25.0625
 
    // 9 -> 12 bit if 18S20
		/* Extended measurements for DS18S20 contributed by Carsten Foss */
		measure &= (uint16_t)0xfffe;   // Discard LSB, needed for later extended precicion calc
		measure <<= 3;                 // Convert to 12-bit, now degrees are in 1/16 degrees units
		measure += (16 - sp[6]) - 4;   // Add the compensation and remember to subtract 0.25 degree (4/16)

	// check for negative 
	if ( measure & 0x8000 )  {
		negative = 1;       // mark negative
		measure ^= 0xffff;  // convert to positive => (twos complement)++
		measure++;
	}
	else {
		negative = 0;
	}

	decicelsius = (measure >> 4);
	decicelsius *= 10;

	// decicelsius += ((measure & 0x000F) * 640 + 512) / 1024;
	// 625/1000 = 640/1024
	fract = ( measure & 0x000F ) * 640;
	if ( !negative ) {
		fract += 512;
	}
	fract /= 1024;
	decicelsius += fract;

	if ( negative ) {
		decicelsius = -decicelsius;
	}

	if ( /* decicelsius == 850 || */ decicelsius < -550 || decicelsius > 1250 ) {
		return 9999;
	} else {
		return decicelsius;
	}
}


uint8_t read_scratchpad( unsigned char *id, uint8_t sp[], uint8_t n )
{
	uint8_t i;
	uint8_t crc=0;

	OWI_MatchRom(id);
	OWI_SendByte(0xBE);

	for ( i = 0; i < n; i++ ) {
		sp[i] = OWI_ReceiveByte();
		crc=OWI_ComputeCRC8(sp[i], crc);
	}

	return crc;
}

/* reads temperature (scratchpad) of sensor with rom-code id
   output: decicelsius 
   returns 0 on success */
uint8_t DS18S20_read_decicelsius( unsigned char *id, int16_t *decicelsius )
{
	uint8_t sp[9];
	uint8_t ret;
	
	OWI_DetectPresence();
	ret = read_scratchpad( id, sp, 9 );
	if ( ret == 0 ) {
		*decicelsius = DS18X20_raw_to_decicelsius( sp );
	}
	return ret;
}

uint8_t DS_scan(OWI_device *devices)
{
	uint8_t retr;
	uint8_t items=0;
	// Do the bus search until all ids are read without crc error.    
	for (retr=3; retr; retr--)
	{
		if (SearchBuses(devices, MAX_DEVICES) == SEARCH_SUCCESSFUL)
		{
			break;
		}
	}
	if (retr==0)
	{
		printf("1WErr\r\n"); // Error
	}
	else
	{
		for (uint8_t fcc=0; fcc<MAX_DEVICES; fcc++)
		{
			uint8_t cnt;
			if ((cnt=devices[fcc].itm)>0)
			{
				items++;
			}
		}
		printf("1s:%02d\r\n", items);
				
		for (uint8_t fcc=0; fcc<MAX_DEVICES; fcc++)
		{
			uint8_t cnt;
			if ((cnt=devices[fcc].itm)>0)
			{
				printf("%02d:", fcc+1);
				for (uint8_t bc=0; bc<8; bc++)
				{
					printf("%02x", devices[fcc].id[bc]);
				}
				printf("\r\n");
			}
		}
		printf("*END\r\n");
	}
	return items;
}

/********** End Onewire/DS18S20 **********/
