#ifndef MBUSRCDEF
#define MBUSRCDEF

#include <inttypes.h>
#include <avr/io.h>


#define MBUSCFG_PIN		PIN4_bm
#define MBUSRES_PIN		PIN1_bm

void mbus_init(void);
void mbus_configmode(uint8_t config);



#endif
