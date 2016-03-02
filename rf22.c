/*----------------------------------------------------------------------------
 Copyright:      
 Author:         
 Remarks:        
 known Problems: none
 Version:        16.12.2009
 Description:    RF22 Funkmodule
------------------------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "rf22.h"
#include <util/delay.h>
//#include "tools.h"

//###############################################################################################################
static inline unsigned char rf22_trans(unsigned char addr, unsigned char data)
{	
	RF22_PORT &= ~(1<<CS);
	SPIE_DATA = addr;
	while(!(SPIE.STATUS & (1<<SPI_IF_bp)));
	SPIE_DATA = data;
	while(!(SPIE.STATUS & (1<<SPI_IF_bp)));
	RF22_PORT |= (1<<CS);
	return SPIE_DATA;
}


//###############################################################################################################
void rf22_write(unsigned char addr, unsigned char data)
{
	rf22_trans(128|addr,data);
}


//###############################################################################################################
unsigned char rf22_read(unsigned char addr)
{
	return rf22_trans(addr&127,0);
}


//###############################################################################################################
void rf22_init(void)
{


  	PORTE.DIRSET   = PIN7_bm | PIN5_bm  | PIN4_bm; //PC4=.CS PC2=SHDN
  	PORTE.DIRCLR   = PIN6_bm | PIN3_bm;	 //PC3=IRQ
	
	PORTF.DIRSET = PIN1_bm; // SHDN
	PORTF.OUTCLR = PIN1_bm;

	
	SPIE.CTRL =  SPI_MODE_0_gc | (0<< SPI_CLK2X_bp)| SPI_PRESCALER_DIV64_gc | (1<< SPI_ENABLE_bp) | (1<< SPI_MASTER_bp);
	SPIE.INTCTRL = SPI_INTLVL_OFF_gc;
  	SPIE.STATUS = 0;


	
	_delay_ms(50);
    rf22_write(0x07, 0x80);		// software reset
	_delay_ms(50); // wait until software reset done

    rf22_write(0x05, 0x02);		// valid packed received interrupt on

    rf22_write(0x06, 0x00);		// all interrupts off
    rf22_write(0x07, 0x01);		//* operating mode: ready mode
    rf22_write(0x09, 0x7f);		//* xtal load capacitance
    rf22_write(0x0A, 0x02);		// uC CLK: 10MHz
    
    rf22_write(0x0b, 0xf5);		// GPIO0: RX_ANT - f5 
    rf22_write(0x0c, 0xf2);		// GPIO1: TX ANT - f2
    rf22_write(0x0d, 0x00);		//* GPIO2: uC Clock out
    rf22_write(0x0e, 0x00);		//*
    rf22_write(0x0f, 0x70);		// ADC Input: GND
    rf22_write(0x10, 0x00);		//* ADC offset: 0
    rf22_write(0x12, 0x00);		// temp sensor calibration off
    rf22_write(0x13, 0x00);		//* temp sensor offset: 0
//    rf22_write(0x1d, 0x40);		// enable AFC
    rf22_write(0x1d, 0x44);		// enable AFC
    rf22_write(0x1e, 0x0A);		//* afc timing
	rf22_write(0x1f, 0x03);		//* afc timing

//	rf22_write(0x1C, 0x05);		// IF bandwidth
//	rf22_write(0x20, 0x83);		// Clock Recovery Oversampling Rate
//	rf22_write(0x21, 0xC0);		// Clock Recovery Offset 2
//	rf22_write(0x22, 0x13);		// Clock Recovery Offset 1
//	rf22_write(0x23, 0xA9);		// Clock Recovery Offset 0
//	rf22_write(0x24, 0x00);		// Clock Recovery Timing Loop Gain 1
//	rf22_write(0x25, 0x04);		// Clock Recovery Timing Loop Gain 0
//	rf22_write(0x2A, 0x24);

	rf22_write(0x1C, 0x01);		// IF bandwidth
	rf22_write(0x20, 0x64);		// Clock Recovery Oversampling Rate
	rf22_write(0x21, 0x01);		// Clock Recovery Offset 2
	rf22_write(0x22, 0x47);		// Clock Recovery Offset 1
	rf22_write(0x23, 0xAE);		// Clock Recovery Offset 0
	rf22_write(0x24, 0x02);		// Clock Recovery Timing Loop Gain 1
	rf22_write(0x25, 0x8F);		// Clock Recovery Timing Loop Gain 0
	rf22_write(0x2A, 0x00);


	rf22_write(0x27, 0x10);		// RSSI Threashold: -120dB

//    rf22_write(0x30, 0x8c);		// data access: RX/TX packet handling, enable crc: CCIT
//    rf22_write(0x32, 0xff);		// header check enable
    rf22_write(0x30, 0x8d);		// data access: RX/TX packet handling, enable crc: CCIT
    rf22_write(0x32, 0x0c);		// header check enable

//    rf22_write(0x33, 0x42);		// 2 word synchronisation
    rf22_write(0x33, 0x22);		// 2 word synchronisation

//    rf22_write(0x34, 0x10);		// preamble length: 16 nibbles, = 64bits
//    rf22_write(0x35, 0x30);		// preamble detection control: 6 nibbles = 24bits
//    rf22_write(0x36, 0x2d);		// sync word 3
//    rf22_write(0x37, 0xd4);		// sync word 2
    rf22_write(0x34, 0x08);		// preamble length: 16 nibbles, = 64bits
    rf22_write(0x35, 0x2a);		// preamble detection control: 6 nibbles = 24bits
    rf22_write(0x36, 0x2d);		// sync word 3
    rf22_write(0x37, 0xd4);		// sync word 2

//    rf22_write(0x38, 0xAA);		// sync word 1
//    rf22_write(0x39, 0xAA);		// sync word 0
    rf22_write(0x3a, 'h');		// transmit header 3
    rf22_write(0x3b, 'o');		// transmit header 2
    rf22_write(0x3c, 'p');		// transmit header 1
    rf22_write(0x3d, 'e');		// transmit header 0
    rf22_write(0x3e, 17);		// packet length
    rf22_write(0x3f, 'h');		// check header 3
    rf22_write(0x40, 'o');		// check header 2
    rf22_write(0x41, 'p');		// check header 1
    rf22_write(0x42, 'e');		// check header 0
    rf22_write(0x43, 0xff);		// header enable mask 3
    rf22_write(0x44, 0xff);		// header enable mask 2
    rf22_write(0x45, 0xff);		// header enable mask 1
    rf22_write(0x46, 0xff);		// header enable mask 0

	rf22_write(0x69, 0x60);		// AGC on
    rf22_write(0x6a, 0x0b);		// agc override 2
	rf22_write(0x6d, 0x0F);		// tx power: +17dBm
	
//	rf22_write(0x6E,0x13);		// set baud high
//	rf22_write(0x6F,0xa9);		// set baud low

	rf22_write(0x6E,0x0A);		// set baud high
	rf22_write(0x6F,0x3D);		// set baud low


//    rf22_write(0x70, 0x2C);		// modulation control
    rf22_write(0x70, 0x0C);		// modulation control
//    rf22_write(0x71, 0x22);		// modulation control 2: FIFO mode, OOK  //0x21 / 0x00
    rf22_write(0x71, 0x23);		// modulation control 2: FIFO mode, OOK  //0x21 / 0x00
	
//    rf22_write(0x72, 0x50);		// frequency deviation: 45kHz
    rf22_write(0x72, 0x20);		// frequency deviation: 45kHz

	rf22_write(0x73, 0x00);		// offset: 0
	rf22_write(0x74, 0x00);		// offset: 0
	
	rf22_write(0x79, 0x0);		// frequency hopping off
    rf22_write(0x7a, 0x0);		// frequency hopping off
	
#ifdef BAND_868
	rf22_write(0x75, 0x73);		// 860-880MHz range
	#warning 860MHz
#else
	rf22_write(0x75, 0x53);		// 430-440MHz range
#endif


}


//###############################################################################################################
void rf22_rxmode(void)
{	
	rf22_read(0x03);			// clear interrupt status
    rf22_read(0x04);			// clear interrupt status
	rf22_write(0x07, 0x01);		// to_ready_mode();
	
	rf22_write(0x07, 0x01);		// to_ready_mode();
	rf22_write(0x7e, 0x17);		// threshold for rx almost full, interrupt when 1 byte received
	
	rf22_write(0x08, 0x03);		// clear RX fifo
	rf22_write(0x08, 0x00);		// clear fifo, disable multi packet
	
	rf22_write(0x07, 0x05);		// RX on
	
    rf22_read(0x03);			// clear interrupt status
    rf22_read(0x04);			// clear interrupt status
}


//###############################################################################################################
void rf22_setfreq(unsigned short freq)
{	
    rf22_write(0x76, (freq&0xFF00)>>8);
    rf22_write(0x77, (freq&0x00FF));
}

//###############################################################################################################
void rf22_sendpacket(unsigned char *data, unsigned char size)
{	
	unsigned char i;

	if (size>64) size=64;
	
	rf22_write(0x07, 0x03);		// switch to ready mode
	rf22_read(0x03);			// clear interrupt status
	rf22_read(0x04);			// clear interrupt status

	rf22_write(0x08, 0x01);		// clear TX fifo
	rf22_write(0x08, 0x00);		// clear TX fifo
	
	rf22_write(0x34, 32);		// premable length: 32 nibbles -> 128 Bits
	rf22_write(0x3e, size);		// packet length
	
	for (i = 0; i<size; i++)
	{    
		rf22_write(0x7f, *data++);
	}
	
	rf22_write(0x05, 0x04);		// enable packet send
	rf22_write(0x07, 0x09);		// TX on
	
	while (nIRQ);
	
	rf22_write(0x07, 0x01);		//	switch to ready mode
}


//###############################################################################################################
unsigned char rf22_getpacket(unsigned char *data)
{	
	unsigned char i, cnt;

	if(rf22_read(0x31)&0x1A)		// receiving a packet
	{
		if ((rf22_read(0x03)&2)&&(!(rf22_read(0x02)&32)))	// packet received & not read
		{
			cnt=rf22_read(0x4B);		// packet length
			
			for (i=0; i<cnt; i++)		// Daten (cnt - 2 für CRC)
			{
				*data++=rf22_read(0x7f);
			}
			
			rf22_rxmode();	
			return (cnt);
		}
	}
	return 0;
}
