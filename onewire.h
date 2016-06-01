// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* \li File:               OWIHighLevelFunctions.h
* \li Compiler:           IAR EWAAVR 3.20a
* \li Support mail:       avr@atmel.com
*
* \li Supported devices:  All AVRs.
*
* \li Application Note:   AVR318 - Dallas 1-Wire(R) master.
*                         
*
* \li Description:        Header file for OWIHighLevelFunctions.c
*
*                         $Revision: 1.1 $
*                         $Date: 2012/05/24 22:38:21 $
****************************************************************************/

#ifndef _OWI_ROM_FUNCTIONS_H_
#define _OWI_ROM_FUNCTIONS_H_

#define SEARCH_SUCCESSFUL               0x00
#define SEARCH_CRC_ERROR                0x01

#define     OWI_CRC_OK      0x00    //!< CRC check succeded
#define     OWI_CRC_ERROR   0x01    //!< CRC check failed


/*! \brief  Data type used to hold information about slave devices.
 *  
 *  The OWI_device data type holds information about what bus each device
 *  is connected to, and its 64 bit identifier.
 */
typedef struct
{
	unsigned char itm;
    unsigned char id[8];    //!< The 64 bit identifier.
} OWI_device;


void OWI_SendByte(unsigned char data);
unsigned char OWI_ReceiveByte(void);
void OWI_SkipRom(void);
void OWI_ReadRom(unsigned char * romValue);
void OWI_MatchRom(unsigned char * romValue);
unsigned char OWI_SearchRom(unsigned char * bitPattern, unsigned char lastDeviation);
unsigned char SearchBuses(OWI_device * devices, unsigned char len);
unsigned char OWI_ComputeCRC8(unsigned char inData, unsigned char seed);
unsigned int OWI_ComputeCRC16(unsigned char inData, unsigned int seed);
unsigned char OWI_CheckRomCRC(unsigned char * romValue);
unsigned char OWI_TouchBit(unsigned char outValue);
void OWI_Init(void);
void OWI_WriteBit1(void);
void OWI_WriteBit0(void);
unsigned char OWI_ReadBit(void);
unsigned char OWI_DetectPresence(void);

#endif
