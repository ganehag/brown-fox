/*----------------------------------------------------------------------------
 Copyright:      
 Author:         
 Remarks:        
 known Problems: none
 Version:        16.12.2009
 Description:    RF22 Funkmodule

RX

------------------------------------------------------------------------------*/


//#define BAND_868

#define RF22_PORT 	PORTE_OUT//  = PIN0_bm;  //Messung aktiv
#define RF22_DDR  	PORTE_DIR 
#define RF22_PIN  	PORTE_IN

#define SHDN	1 
#define CS		4
#define SDI		5
#define SDO		6
#define SCK		7

#define nIRQ	(PORTF_IN&(1<<0))

void rf22_init(void);											// initialize module
void rf22_setfreq(unsigned short freq);

void rf22_write(unsigned char addr, unsigned char data);
unsigned char rf22_read(unsigned char addr);

void rf22_rxmode(void);

unsigned char rf22_getpacket(unsigned char *data);
void rf22_sendpacket(unsigned char *data, unsigned char size);


#ifdef BAND_868
 #define RF22FREQ(freq)	((freq-860.0)*3200)
#else
 #define RF22FREQ(freq)	((freq-430.0)*6400)
#endif


