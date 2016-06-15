#ifndef ds18s20def
#define ds18s20def

#include <inttypes.h>

#define MAX_DEVICES 4       //!< Max number of 1-wire devices to search for.


uint8_t DS18S20_start_meas(unsigned char *id);
int16_t DS18X20_raw_to_decicelsius( uint8_t sp[] );
uint8_t read_scratchpad( unsigned char *id, uint8_t sp[], uint8_t n );
uint8_t DS18S20_read_decicelsius( unsigned char *id, int16_t *decicelsius );
uint8_t DS_scan(OWI_device *devices);

#endif