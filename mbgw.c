#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include "eeprom.h"
//#include <avr/wdt.h>
#include <util/delay.h>
#include <stdio.h>
#include "avr_compiler.h"
#include "tc_driver.h"
#include <spdriver.h>
#include <onewire.h>
#include <wdt.h>
#include <nvm.h>
#include "rfm93.h"
#include "gsmbg2.h"
#include "ds18s20.h"
#include "usbclassdef.h"
#include "usart_driver.h"
#include "xmegabaud.h"
#include "uart.h"
#include "mbrc1180.h"
#include "mbus.h"

#include "lib_atsha/sha204_lib_return_codes.h"  // declarations of function return codes
#include "lib_atsha/sha204_comm_marshaling.h"   // definitions and declarations for the Command module


// Rev history
// 0.1 Initial.


#define WATCHDOG_TIMEOUT        WDT_PER_2KCLK_gc // aprox 2s


#define PRECHARGE_TIME		5 // Precharge GSM powersupply 

volatile uint8_t rtcc=0;

// FIX Move to LIB header
/*! \brief Structure to hold low and high byte in a 16-bit word. */
struct int16_8
{
	uint8_t low;
	uint8_t high;
};

/*! \brief Union of 16-bit word and 2 bytes. */
typedef union
{
	uint16_t word;
	struct int16_8	byte;	

} cnv16_8;


/*************** USB ***************/
#include "DualVirtualSerial.h"

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs
 */

static FILE USBSerialStream;

void SetupHardware(void)
{
	uint8_t res=0;

	
	// Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it
	if (XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU))
		res|=1<<0;
		
	if (XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL))
		res|=1<<1;

	// Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference
	if (XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ))
		res|=1<<2;
	if (XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB))
		res|=1<<3;
	
	if (XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32KHZ))
		res|=1<<4;
		
	CLK.RTCCTRL= CLK_RTCSRC_RCOSC32_gc|CLK_RTCEN_bm;
	RTC.CTRL=RTC_PRESCALER_DIV1_gc;
	RTC.INTCTRL=RTC_OVFINTLVL_HI_gc;
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;

	/* Hardware Initialization */
//	Buttons_Init();
	LEDs_Init();
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	USB_Init();


		
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial1_CDC_Interface);
	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial2_CDC_Interface);

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial1_CDC_Interface);
	CDC_Device_ProcessControlRequest(&VirtualSerial2_CDC_Interface);
}

void Boot_Jump(void)
{
	/* This code is not needed for smaller devices (<128k) 
	*  where a jmp could be done. For portabillity, this 
	*  could always be used. Just adjust the vector:
	* ATXmega16: 0x0020FE
	* Jump to 0x041FC = BOOT_SECTION_START + 0x1FC which is
	* the stated location of the bootloader entry (AVR1916).
	* Note the address used is the word address = byte addr/2.
	* My ASM fu isn't that strong, there are probably nicer
	* ways to do this with, yennow, defined symbols .. */

	asm ("ldi r30, 0xFE\n"  /* Low byte to ZL */
	  "ldi r31, 0x20\n" /* mid byte to ZH */
	  "ldi r24, 0x00\n" /* high byte to EIND which lives */
	  "out 0x3c, r24\n" /* at addr 0x3c in I/O space */
	  "eijmp":  :: "r24", "r30", "r31");
}


/*************** END USB ***************/



/*
void send_version(uint8_t hwver, uint16_t crcv)
{
	printf_P(PSTR("Noda "SWTYPE" "));
	printf_P(PSTR("ver: " VERSION " Hw: %d - "), hwver);
   	printf_P(PSTR(__DATE__ " " __TIME__ " CRC: %x\r\n"),crcv);
}
*/

/*! \brief Inits basic hardware modules
 *
 *  For more information see ATMEL datasheets
 *
 *	\return 		none 
 */
void ioinit (void) 
{
	//	Timer 1, Led flash and timekeeping		
	// Enable overflow interrupt at low level 
	TC0_SetOverflowIntLevel( &TCE0, TC_OVFINTLVL_LO_gc );
	PMIC.CTRL |= PMIC_LOLVLEN_bm;

	// Start Timer/Counter. 
	TC0_ConfigClockSource( &TCE0, TC_CLKSEL_DIV256_gc );

	// FIX Move to dedicated UART lib
	// Set direction of UART pins
  	// PD3 (TXD0) as output. 
	PORTB.DIRSET   = PIN0_bm;
	PORTB.OUTCLR   = PIN0_bm; 
	PORTB.DIRSET   = PIN1_bm;
	PORTB.OUTCLR   = PIN1_bm; 

	// PD1 (1Wire power) as output. 
	PORTD.DIRSET   = PIN1_bm;

	
}


/*! \brief gets a hex word from UART
 *
 *  Allways expects 4 digits
 *
 *	\param retval 	pointer to return struct (integer)
 *	\return 		Allways returns 0 on success, otherwise 1 
 */
 /*
uint8_t serial_get_hexword(uint16_t *retval)
{
	uint8_t pos=4; // Expect 4 digits
	uint8_t digit;
	uint16_t tmpval=0;
	uint8_t error=0;

	while (USART_RXBufferData_Available(&USART_data)&&(pos)) 
	{
		tmpval=tmpval<<4;
		pos--;
		digit=USART_RXBuffer_GetByte(&USART_data);
		if ((digit >= 0x30)&&(digit <= 0x39))
		{
			tmpval+=(digit-0x30);
		}
		else if ((digit >= 0x41)&&(digit <= 0x46))
		{
			tmpval+=(digit-0x37);
		}
		else if ((digit >= 0x61)&&(digit <= 0x66))
		{
			tmpval+=(digit-0x57);
		}
		else
		{
			error=1;
			break;
		}
	}
	*retval=tmpval;
	if (pos)
		error=1;
		
	return error;	
}
*/
/*void send_verbose(void)
{
	printf ("CPU ID: %02x%02x%02x Rev: %02x\n\r", MCU.DEVID0, MCU.DEVID1, MCU.DEVID2, MCU.REVID);
	printf ("UID: %02x\n\r", MCU.JTAGUID);
	printf ("Unique ID: ");
	
	NVM_PROD_SIGNATURES_t ps;
	
	NVM_ReadCalib((uint8_t *)&ps);
	
	uint8_t *uid=(uint8_t *)&(ps.LOTNUM0);

	for (uint8_t tst=0; tst<6; tst++)
	{
		printf("%02x", *(uid++));
	}

	printf("\n\rWafer :%02x %02x%02x, ", ps.WAFNUM, ps.COORDX1, ps.COORDX0);
	printf("%02x%02x\n\r", ps.COORDY1, ps.COORDY0);
}

*/

void test_sha(void)
{
	// declared as "volatile" for easier debugging
	volatile uint8_t ret_code;

	uint8_t i,c=0;

	// Make the command buffer the size of the MAC command.
	static uint8_t command[MAC_COUNT_LONG];

	// Make the response buffer the size of a MAC response.
	static uint8_t response[MAC_RSP_SIZE];

   // expected MAC response in mode 0
	static const uint8_t mac_mode0_response_expected[MAC_RSP_SIZE] =
	{
			MAC_RSP_SIZE,                                   // count
			0x06, 0x67, 0x00, 0x4F, 0x28, 0x4D, 0x6E, 0x98,
			0x62, 0x04, 0xF4, 0x60, 0xA3, 0xE8, 0x75, 0x8A,
			0x59, 0x85, 0xA6, 0x79, 0x96, 0xC4, 0x8A, 0x88,
			0x46, 0x43, 0x4E, 0xB3, 0xDB, 0x58, 0xA4, 0xFB,
			0xE5, 0x73                                       // CRC
	};

	// data for challenge in MAC mode 0 command
	const uint8_t challenge[MAC_CHALLENGE_SIZE] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
	};

	for (i = 0; i < sizeof(response); i++)
		response[i] = 0;

	// Wake up the device.
	ret_code = sha204c_wakeup(&response[0]);
	if (ret_code != SHA204_SUCCESS)
		printf("WakeErr");

	// Mac command with mode = 0.
	ret_code = sha204m_execute(SHA204_MAC, 0, 0, MAC_CHALLENGE_SIZE, (uint8_t *) challenge,
					0, NULL, 0, NULL, sizeof(command), &command[0], sizeof(response), &response[0]);
	if (ret_code != SHA204_SUCCESS) {
		printf("ExecErr %x ",ret_code);
			
	}

	// Put device to sleep.
	ret_code = sha204p_sleep();

	// Compare returned MAC with expected one.
	ret_code = SHA204_SUCCESS;
	printf("\nResponse: \t");
	for (i = 0; i < SHA204_RSP_SIZE_MAX; i++) {
		printf("%x, ", response[i] );
	}
	printf("\nExpected: \t");
	for (i = 0; i < SHA204_RSP_SIZE_MAX; i++) {
		printf("%x, ", mac_mode0_response_expected[i]);
		if (response[i]==mac_mode0_response_expected[i])
		{
			c++;
		}
	}
	if (c==SHA204_RSP_SIZE_MAX)
	{
		printf("\nIdentical!\n");
	}

}


int main (void)
{

	uint8_t rst,hwv;
	uint16_t crc;
	uint8_t lasttick=0;
	char txdata[10];

	uint8_t sermux=0;
	
	uint8_t gsm_precharge=PRECHARGE_TIME;
	uint8_t last_gsm=0;
	
	static OWI_device devices[MAX_DEVICES];
	uint8_t mbbuf[256];
	uint8_t items;

	uint8_t gsminit=0;
	uint8_t ist=0;
	
	rst=RST.STATUS&0x3f;

	RST.STATUS|=0x3f; // Clear flags

	hwv=SP_ReadFuseByte(0);

	
	wdt_Disable();
	wdt_EnableAndSetTimeout(WATCHDOG_TIMEOUT);
	wdt_reset();
	
    ioinit ();
	// Enable global interrupts. DO NOT TOUCH!!!

	sei();
	
	/*************** USB ***************/
	
	SetupHardware();

	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	CDC_Device_CreateStream(&VirtualSerial1_CDC_Interface, &USBSerialStream);

	stdin = &USBSerialStream;
	stdout = &USBSerialStream;

	
	sei();

	/*************** END USB ***************/
	
	_delay_ms(1000);
		
	wdt_reset();
	
	/*** NOTE! Onewire is compiled for 32Mhz main clock ***/
	OWI_Init(); // One-wire init
	PORTF.DIRSET=PIN4_bm;
	PORTF.OUTSET=PIN4_bm;

	gsm_hwinit();
	
	items=DS_scan(devices);
	
	if (items)
	{
		DS18S20_start_meas(devices[0].id);
	}


	sha204p_init(); // Start up TWI interface for ATSHA204A.
	
	mbus_init();
	
	init_usart_ext(); // Start up external UART
	init_usart_gsm(); // Start up GSM UART
	init_usart_mbus(); //Start up MBus UART
	
	printf("\nStart precharge");
	gsm_vcc_precharge();
	
/*
	// Init RFM
	if (hwv==2)
	{	
		txsetup();
	}
	else
	{
		rxsetup();
	}
*/

	for (;;)
	{
	
		// USB
		/* Must throw away unused bytes from the host, or it will lock up while waiting for the device */


		/* Read both USB interfaces! */
		int16_t ReceivedByte2 = CDC_Device_ReceiveByte(&VirtualSerial2_CDC_Interface);
		int16_t ReceivedByte = CDC_Device_ReceiveByte(&VirtualSerial1_CDC_Interface);

		if (ReceivedByte>0)
		{
			uint8_t getkey;
			LEDs_TurnOnLEDs(LEDS_BLUE);

			getkey=	ReceivedByte&0xff;
			
			if (getkey=='B')
			{
				uint8_t fcc;
				wdt_reset(); // extra WDR
				
				printf_P(PSTR("Boot loader started\n"));
				CDC_Device_Flush(&VirtualSerial1_CDC_Interface);

				for (fcc=0; fcc<200; fcc++)
				{
					CDC_Device_USBTask(&VirtualSerial1_CDC_Interface);
					USB_USBTask();
				}
				USB_Detach();	// Shutdown USB
				USB_Disable();
				cli();
				_delay_ms(500); // Settle things...
				//CCPWrite( &RST.CTRL, RST_SWRST_bm );
				//asm("jmp 0x201FE");
				Boot_Jump();
			}
			else if (getkey=='o')
			{
				gsm_on();
			}
			else if (getkey=='O')
			{
				gsminit=1;
				ist=0;
			}
			else if (getkey=='p')
			{
				mbus_probe();
			}

			else if (getkey=='V')
			{
				mbus_configmode(1);
			}
			else if (getkey=='v')
			{
				mbus_configmode(0);
			}
			else if (getkey=='t')
			{
				printf_P(PSTR("Tic:%d\n"),rtcc);
			}
			else if (getkey=='h')
			{
				printf_P(PSTR("Hwv:%x, Rst:%x\n"),hwv,rst);
			}
			else if (getkey=='r')
			{			
			//	readAllRegs();
				gsm_vcc_precharge();
			}
			else if (getkey=='R')
			{			
			//	startReceiving();
				gsm_vcc_disable();
			}
			else if (getkey=='s')
			{	
			//	sendData(txdata);
				gsm_vcc_enable();
			}
			else if (getkey=='i')
			{			
				printf("MB Cfg crc: %x\n", mbus_validate());
			}
			else if (getkey=='I')
			{
				test_sha();
			}
			else if (getkey=='u')
			{
				mbus_setup((const uint8_t *)MB_CONF);
				
			}
			else if (getkey=='g')
			{			
				uint8_t gstat;
				gstat=PORTC.IN;
				printf("Ring=%d, CTS=%d, ON=%d", gstat&&0x01, gstat&&0x02, gstat&&0x40);
			}
			else if (getkey=='m')
			{
				sermux++;

				if (sermux==0)
				{
					printf("Serial: None\n");
				}
				else if (sermux==1)
				{
					printf("Serial: External (P6)\n");
				}
				else if (sermux==2)
				{
					printf("Serial: GSM\n");
				}
				else if (sermux==3)
				{
					printf("Serial: Mbus\n");
				}
				else
				{
					sermux=0;
				}

	
			}			
			// ______1W
			else if (getkey=='*')
			{
				items=DS_scan(devices);
			}

			else if (getkey=='p')
			{
				int16_t temp;

				if (DS18S20_start_meas(devices[0].id))
					printf("SOK");
				
				_delay_ms( 750 );
				
				if (DS18S20_read_decicelsius(devices[0].id, &temp)==0)
				{
					printf("Temp: %d", temp);
					sprintf(txdata,"!%04d-%04d",rtcc,temp);
				}
			}
			else
			{
				printf("Commands: [m] select 2nd seral endpoint. [o] Start GSM. [V/v] Set/Clr Mbus config pin. [I] test ATSHA204\n");
			}

		}

		// USARTS
		// Got anything on USB interface?? Then send to selected interface
		if ((ReceivedByte2 > 0))
		{
			ReceivedByte2&=0x00ff;

			if(sermux==1)
			{
				USART_TXBuffer_PutByte(&USARTBuf_ext,(ReceivedByte2&0xff));
			}
			else if(sermux==2)
			{
				USART_TXBuffer_PutByte(&USARTBuf_gsm,(ReceivedByte2&0xff));
			}
			else if(sermux==3)
			{
				USART_TXBuffer_PutByte(&USARTBuf_mbus,(ReceivedByte2&0xff));
			}
		}

		// Then check the interfaces one at the time
		if (USART_RXBufferData_Available(&USARTBuf_ext))
		{
			uint8_t rxd;
			// Read out byte from buffer
			rxd=USART_RXBuffer_GetByte(&USARTBuf_ext);
			
			// Are we selected? Then send
			if (sermux==1)
			{
				 CDC_Device_SendByte(&VirtualSerial2_CDC_Interface, rxd);
			}
		}
		if (USART_RXBufferData_Available(&USARTBuf_gsm))
		{
			uint8_t rxd;
			Endpoint_WaitUntilReady();
			while(USART_RXBufferData_Available(&USARTBuf_gsm))
			{
				// Read out byte from buffer
				rxd=USART_RXBuffer_GetByte(&USARTBuf_gsm);
			
				// Are we selected? Then send
				if (sermux==2)
				{
//				
					CDC_Device_SendByte(&VirtualSerial2_CDC_Interface, rxd);
//					USART_TXBuffer_PutByte(&USARTBuf_ext,rxd);
				}
			}
		}
		
		if (sermux!=3)
		{	
			if(mbus_get_package((uint8_t*)&mbbuf)==MBDONE)
			{
				uint8_t len;
				len=mbbuf[0];
				
				for (uint8_t fcb=1; fcb<len; fcb++)
					printf("%x",mbbuf[fcb]);
					
				printf("RSSI=-%d", (mbbuf[len]/2));
				if ((mbbuf[len]%2))
					printf(".5\n");
				else
					printf(".0\n");
			}
		}
		else if (USART_RXBufferData_Available(&USARTBuf_mbus))
		{
			uint8_t rxd;
			Endpoint_WaitUntilReady();
			while(USART_RXBufferData_Available(&USARTBuf_mbus))
			{
				// Read out byte from buffer
				rxd=USART_RXBuffer_GetByte(&USARTBuf_mbus);
			
				// Are we selected? Then send
				if (sermux==3)
				{
//				
					CDC_Device_SendByte(&VirtualSerial2_CDC_Interface, rxd);
//					USART_TXBuffer_PutByte(&USARTBuf_ext,rxd);
				}
			}
		}

		//USB
		CDC_Device_USBTask(&VirtualSerial1_CDC_Interface);
		CDC_Device_USBTask(&VirtualSerial2_CDC_Interface);
		USB_USBTask();
		// END USB
		wdt_reset();
		
		
		// Periodic, fires @ aprox 1Hz 
		if (rtcc!=lasttick)
		{
			int16_t temp;
			lasttick=rtcc;


			if (items>0)
			{
				DS18S20_read_decicelsius(devices[0].id, &temp);				
				DS18S20_start_meas(devices[0].id);
			}

			
		//	printf("Tick: %03d", rtcc);
				
			if (sermux==0)
			{
				CDC_Device_SendByte(&VirtualSerial2_CDC_Interface, '+');
			}
			if (items>0)
			{
				int16_t temp;
			
				if (DS18S20_read_decicelsius(devices[0].id, &temp)==0)
				{
					printf("\t Temp: %d\n", temp);
				}
			
				DS18S20_start_meas(devices[0].id);
			}
			
			if (gsm_precharge)
			{
				gsm_precharge--;
				
				if (gsm_precharge==0)
				{
					printf("Starting main GSM VCC regulator\n");
					gsm_vcc_enable();
				}
			}
			else if (gsminit)
			{
				ist=gsm_init(ist);
				if (ist==0)
					gsminit=0;
			
			}
			//rxpoll();
		}
		
		if (last_gsm!=PORTC.IN)
		{
			last_gsm=PORTC.IN;
			printf("GSM: ");
			
			if (last_gsm&0x01)
				printf("RING! ");
			if (last_gsm&0x02)
				printf("CTS ");
			if ((last_gsm&0x40)==0)
				printf("ON");
			else
				printf("off");
			
			printf("\n");

		}
		
		
	}
    return (0);
}


ISR(TCE0_OVF_vect)
{
	LEDs_ToggleLEDs(LEDS_YELLOW);
	LEDs_TurnOffLEDs(LEDS_RED|LEDS_BLUE);
}

ISR(RTC_OVF_vect)
{
	rtcc++;
}


