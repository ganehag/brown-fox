#include <avr/io.h>
#include <util/delay.h>
#include "rfm93.h"
#include <LUFA/Drivers/Board/LEDs.h> // Just to get the leds!
#include <stdio.h>

#define PIN_SS		PIN4_bm
#define PIN_RES		PIN0_bm
#define PIN_DIO0	PIN0_bm
#define PIN_DIO5	PIN1_bm

char character;
uint8_t currentMode = 0x81;
uint8_t msgBase = 1;
char msg[11];


/*! \brief SPI mastertransceive byte
 *
 *  This function clocks data in the DATA register to the slave, while data
 *  from the slave is clocked into the DATA register. The function does not
 *  check for ongoing access from other masters before initiating a transfer.
 *  For multimaster systems, checkers should be added to avoid bus contention.
 *
 *  SS line(s) must be pulled low before calling this function and released
 *  when finished.
 *
 *  \note This function is blocking and will not finish unless a successful
 *        transfer has been completed. It is recommended to use the
 *        interrupt-driven driver for applications where blocking
 *        functionality is not wanted.
 *
 *  \param spi        The SPI_Master_t struct instance.
 *  \param TXdata     Data to transmit to slave.
 *
 *  \return           Data received from slave.
 */
uint8_t SPI_MasterTransceiveByte(uint8_t TXdata)
{
	/* Send pattern. */
	SPIE.DATA = TXdata;

	/* Wait for transmission complete. */
	while(!(SPIE.STATUS & SPI_IF_bm)) {

	}
	/* Read received data. */
	uint8_t result = SPIE.DATA;

	return(result);
}

/////////////////////////////////////
//    Method:   Select Transceiver
//////////////////////////////////////
#define select() PORTE.OUTCLR=PIN_SS

/////////////////////////////////////
//    Method:   UNSelect Transceiver
//////////////////////////////////////
#define unselect() PORTE.OUTSET=PIN_SS


void rfmReset(void)
{
	PORTF.OUTSET=PIN_RES;
	PORTF.DIRSET=PIN_RES;
	_delay_ms(1);
	PORTF.OUTCLR=PIN_RES;
	PORTF.DIRCLR=PIN_RES;
	_delay_ms(10);
}

void txsetup(void) {                
  // initialize the pins
	SPIE.CTRL   = (SPI_PRESCALER_DIV16_gc | SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc);
	SPIE.INTCTRL = SPI_INTLVL_OFF_gc;
	PORTE.DIR  = 0b10111000;

	_delay_ms(100);
	
	// LoRa mode 
	setLoRaMode();
  
	// Turn on implicit header mode and set payload length
	writeRegister(REG_MODEM_CONFIG,IMPLICIT_MODE);
	writeRegister(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH);
  
	// Change the DIO mapping to 01 so we can listen for TxDone on the interrupt
	writeRegister(REG_DIO_MAPPING_1,0x40);
	writeRegister(REG_DIO_MAPPING_2,0x00);
  
	// Go to standby mode
	setMode(RF92_MODE_STANDBY);
  
	printf_P(PSTR("Setup Complete"));
}

void rxsetup(void) {                
  // initialize the pins
	SPIE.CTRL   = (SPI_PRESCALER_DIV16_gc | SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc);
	SPIE.INTCTRL = SPI_INTLVL_OFF_gc;
	PORTE.DIR  = 0b10111000;

	_delay_ms(100);
	
	// LoRa mode 
	setLoRaMode();
  
	startReceiving(); 
 
	printf_P(PSTR("Setup Complete"));
}


/*
// the loop routine runs over and over again forever:
void loop(void) {
  char tmp[] = {0,0,0,0,0,0,0,0,0,0};
  for(uint8_t i = 0;i<msgBase;i++){
    tmp[i] = payload[i]; 
  }
  msgBase++;
  if(msgBase>10){
    msgBase = 1; 
  }
  sendData(tmp);
  _delay_ms(3000);

}
*/
/////////////////////////////////////
//    Method:   Send TO BUFFER
//////////////////////////////////////
void sendData(char buffer[])
{
  printf_P(PSTR("Sending: "));
  
  setMode(RF92_MODE_STANDBY);

  writeRegister(REG_FIFO_TX_BASE_AD, 0x00);  // Update the address ptr to the current tx base address
  writeRegister(REG_FIFO_ADDR_PTR, 0x00); 
  
  select();
  // tell SPI which address you want to write to
  SPI_MasterTransceiveByte(REG_FIFO | 0x80);
  // loop over the payload and put it on the buffer 
  for (int i = 0; i < 10; i++){
    printf("%c",buffer[i]);
    SPI_MasterTransceiveByte(buffer[i]);
  }
  unselect();
  
  // go into transmit mode
  setMode(RF92_MODE_TX);
  
  // once TxDone has flipped, everything has been sent
  while((PORTD.IN&PIN_DIO0) == 0){
   //Serial.print("y");
  }
  
  printf_P(PSTR(" done sending!\n"));
  
  // clear the flags 0x08 is the TxDone flag
  writeRegister(REG_IRQ_FLAGS, 0x08); 
  
  // blink the LED
	LEDs_TurnOnLEDs(LEDS_GREEN);
 }

/////////////////////////////////////
//    Method:   Read Register
//////////////////////////////////////
uint8_t readRegister(uint8_t addr)
{
  select();
  SPI_MasterTransceiveByte(addr & 0x7F);
  uint8_t regval = SPI_MasterTransceiveByte(0);
  unselect();
  return regval;
}

/////////////////////////////////////
//    Method:   Write Register
//////////////////////////////////////

void writeRegister(uint8_t addr, uint8_t value)
{
  select();
  SPI_MasterTransceiveByte(addr | 0x80); // OR address with 10000000 to indicate write enable;
  SPI_MasterTransceiveByte(value);
  unselect();
}

/////////////////////////////////////
//    Method:   Receive FROM BUFFER
//////////////////////////////////////
void receiveMessage(char *message)
{
  
  // clear the rxDone flag
  writeRegister(REG_IRQ_FLAGS, 0x40); 
  
  int x = readRegister(REG_IRQ_FLAGS); // if any of these are set then the inbound message failed
  //Serial.println(x);
  
  // check for payload crc issues (0x20 is the bit we are looking for
  if((x & 0x20) == 0x20)
  {
    printf_P(PSTR("Oops there was a crc problem!!"));
    printf("%02x",x);
    // reset the crc flags
    writeRegister(REG_IRQ_FLAGS, 0x20); 
  }
  else{
    uint8_t currentAddr = readRegister(REG_FIFO_RX_CURRENT_ADDR);
    uint8_t receivedCount = readRegister(REG_RX_NB_BYTES);
    printf_P(PSTR("Packet! RX Current Addr:"));
    printf("%02x",currentAddr);
    printf_P(PSTR("Number of bytes received:"));
    printf("%d ", receivedCount);

    writeRegister(REG_FIFO_ADDR_PTR, currentAddr);   
    // now loop over the fifo getting the data
    for(int i = 0; i < receivedCount; i++)
    {
      message[i] = (char)readRegister(REG_FIFO);
    }
  } 
}

/////////////////////////////////////
//    Method:   Read ALL Registers
//////////////////////////////////////
void readAllRegs(void)
{
  uint8_t regVal;
        
  for (uint8_t regAddr = 1; regAddr <= 0x46; regAddr++)
  {
    select();
    SPI_MasterTransceiveByte(regAddr & 0x7f);        // send address + r/w bit
    regVal = SPI_MasterTransceiveByte(0);
    unselect();
  
    printf("%02x - %02x\n", regAddr, regVal);
  }
}

/////////////////////////////////////
//    Method:   Change the mode
//////////////////////////////////////
void setMode(uint8_t newMode)
{
  if(newMode == currentMode)
    return;  
  
  switch (newMode) 
  {
    case RF92_MODE_RX_CONTINUOS:
      printf_P(PSTR("Changing to Receive Continous Mode"));
      writeRegister(REG_PA_CONFIG, PA_OFF_BOOST);  // TURN PA OFF FOR RECIEVE??
      writeRegister(REG_LNA, LNA_MAX_GAIN);  // MAX GAIN FOR RECIEVE
      writeRegister(REG_OPMODE, newMode);
      currentMode = newMode; 
      break;
    case RF92_MODE_TX:
      printf_P(PSTR("Changing to Transmit Mode"));
      writeRegister(REG_LNA, LNA_OFF_GAIN);  // TURN LNA OFF FOR TRANSMITT
      writeRegister(REG_PA_CONFIG, PA_MAX_BOOST);    // TURN PA TO MAX POWER
      writeRegister(REG_OPMODE, newMode);
      currentMode = newMode; 
      
      break;
    case RF92_MODE_SLEEP:
      printf_P(PSTR("Changing to Sleep Mode")); 
      writeRegister(REG_OPMODE, newMode);
      currentMode = newMode; 
      break;
    case RF92_MODE_STANDBY:
      printf_P(PSTR("Changing to Standby Mode"));
      writeRegister(REG_OPMODE, newMode);
      currentMode = newMode; 
      break;
    default: return;
  } 
  
  if(newMode != RF92_MODE_SLEEP){
    while((PORTF.IN&PIN_DIO5) == 0)
    {
      printf("z");
    } 
  }
   
  printf_P(PSTR(" Mode Change Done"));
  return;
}

/////////////////////////////////////
//    Method:   Enable LoRa mode
//////////////////////////////////////
void setLoRaMode(void)
{
  printf_P(PSTR("Setting LoRa Mode"));
  setMode(RF92_MODE_SLEEP);
  writeRegister(REG_OPMODE,0x80);
   
  printf_P(PSTR("LoRa Mode Set"));
  return;
}
  

/////////////////////////////////////
//    Method:   Setup to receive continuously
//////////////////////////////////////
void startReceiving(void)
{
	// LoRa mode 
	setLoRaMode();

	// Turn on implicit header mode and set payload length
	writeRegister(REG_MODEM_CONFIG,IMPLICIT_MODE);
	writeRegister(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH);
	writeRegister(REG_HOP_PERIOD,0xFF);
	writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_BASE_AD));   
  
	// Setup Receive Continous Mode
	setMode(RF92_MODE_RX_CONTINUOS); 
}

// RXpoll:
void rxpoll(void) {  
  
	if((PORTD.IN&PIN_DIO0) == PIN_DIO0)
	{
		LEDs_TurnOnLEDs(LEDS_GREEN);
		receiveMessage(msg);
		msg[10]=0;
		printf("Rx: %s\n",msg);
	}
}