#include "Anyka_types.h"
#include "Eng_debug.h"
#include "fwl_osmalloc.h"
#include "fwl_oscom.h"
#include "sha204.h"
#include "Fwl_osfs.h"
#include "Eng_string_UC.h"
#include "Eng_string.h"
#include "Eng_DataConvert.h"
#include "ctl_flashPlayer.h"
#include "gpio_config.h"
#if SHA204_DEVICE_PERSONALIZATION

/*!
 *	*** DEVICE Modes Address ***
 */
#define DEVICE_MODES_ADDRESS			((T_U16) 0x0004)
#define DEVICE_MODES_BYTE_SIZE			(4)			


//Key ID in 16 bit boundaries
#define KEY_ID_0						((T_U16) 0x0000)
#define KEY_ID_1						((T_U16) 0x0001)
#define KEY_ID_2						((T_U16) 0x0002)
#define KEY_ID_3						((T_U16) 0x0003)
#define KEY_ID_4						((T_U16) 0x0004)
#define KEY_ID_5						((T_U16) 0x0005)
#define KEY_ID_6						((T_U16) 0x0006)
#define KEY_ID_7						((T_U16) 0x0007)
#define KEY_ID_8						((T_U16) 0x0008)
#define KEY_ID_9						((T_U16) 0x0009)
#define KEY_ID_10						((T_U16) 0x000A)
#define KEY_ID_11						((T_U16) 0x000B)
#define KEY_ID_12						((T_U16) 0x000C)
#define KEY_ID_13						((T_U16) 0x000D)
#define KEY_ID_14						((T_U16) 0x000E)
#define KEY_ID_15						((T_U16) 0x000F)

//Slot configuration address
#define SLOT_CONFIG_0_1_ADDRESS			((T_U16) 0x0005)
#define SLOT_CONFIG_2_3_ADDRESS			((T_U16) 0x0006)
#define SLOT_CONFIG_4_5_ADDRESS			((T_U16) 0x0007)
#define SLOT_CONFIG_6_7_ADDRESS			((T_U16) 0x0008)
#define SLOT_CONFIG_8_9_ADDRESS			((T_U16) 0x0009)
#define SLOT_CONFIG_10_11_ADDRESS		((T_U16) 0x000A)
#define SLOT_CONFIG_12_13_ADDRESS		((T_U16) 0x000B)
#define SLOT_CONFIG_14_15_ADDRESS		((T_U16) 0x000C)

//Slot key address
#define SLOT_0_ADDRESS					((T_U16) 0x0000)
#define SLOT_1_ADDRESS					((T_U16) 0x0008)
#define SLOT_2_ADDRESS					((T_U16) 0x0010)
#define SLOT_3_ADDRESS					((T_U16) 0x0018)
#define SLOT_4_ADDRESS					((T_U16) 0x0020)
#define SLOT_5_ADDRESS					((T_U16) 0x0028)
#define SLOT_6_ADDRESS					((T_U16) 0x0030)
#define SLOT_7_ADDRESS					((T_U16) 0x0038)
#define SLOT_8_ADDRESS					((T_U16) 0x0040)
#define SLOT_9_ADDRESS					((T_U16) 0x0048)
#define SLOT_10_ADDRESS					((T_U16) 0x0050)
#define SLOT_11_ADDRESS					((T_U16) 0x0058)
#define SLOT_12_ADDRESS					((T_U16) 0x0060)
#define SLOT_13_ADDRESS					((T_U16) 0x0068)
#define SLOT_14_ADDRESS					((T_U16) 0x0070)
#define SLOT_15_ADDRESS					((T_U16) 0x0078)


/*!
 * *** Read granularity and address specifiers ***
 */
#define CONFIG_READ_SHORT				((T_U8)0x00)
#define CONFIG_READ_LONG				((T_U8)0x80)

#define OTP_READ_SHORT					((T_U8)0x01)
#define OTP_READ_LONG					((T_U8)0x81)
#define OTP_BLOCK_0_ADDRESS				((T_U16)0x0000)			//!< Base address of the first 32 bytes of the OTP region
#define OTP_BLOCK_1_ADDRESS				((T_U16)0x0008)			//!< Base address of the second 32 bytes of the OTP region

#define DATA_READ_SHORT					((T_U8)0x02)
#define DATA_READ_LONG					((T_U8)0x82)

#define CONFIG_BLOCK_0_ADDRESS			((T_U16)0x0000)
#define CONFIG_BLOCK_1_ADDRESS			((T_U16)0x0008)
#define CONFIG_BLOCK_2_ADDRESS			((T_U16)0x0010)


/*!
 * Word base addresses for UseFlag and UpdateCount bits 
 *	Even bytes address UseFlag
 *  Odd bytes address UpdateCount
 */
#define SLOT_0_1_USE_UPDATE_ADDRESS		((T_U16) 0x000D)		// Word 13
#define SLOT_2_3_USE_UPDATE_ADDRESS		((T_U16) 0x000E)		// Word 14
#define SLOT_4_5_USE_UPDATE_ADDRESS		((T_U16) 0x000F)		// Word 15
#define SLOT_6_7_USE_UPDATE_ADDRESS		((T_U16) 0x0010)		// Word 16

/*!
 *	*** LAST KEY USE ADDRESS AND SIZE ***
 */
#define LAST_KEY_USE_ADDRESS			((T_U16) 0X0011)		// Word 17
#define LAST_KEY_USE_BYTE_SIZE			((T_U8) 0x10)		// 16 bytes
/*!
 *	*** USER EXTRA, SELECTOR, and LOCK bytes address
 */
#define EXTRA_SELECTOR_LOCK_ADDRESS		((T_U16) 0x0015)		// Word 21

//													 { I2C_Address,	TempOffset,	   OTPmode,	SelectorMode}; 
const T_U8 DEVICE_MODES[SHA204_ZONE_ACCESS_4] = {	      0xC8,       0x00,       0x55,         0x00};								


/*!
 *  *** SLOT CONFIGURATION ***
 *
 * Configure the access restrictions governing each slot
 *
 */
//													       { LSB,  MSB,    LSB,  MSB};				
const T_U8 SLOT_CONFIG_00_01[SHA204_ZONE_ACCESS_4] = {0x8F, 0x80,   0x8F, 0x80};  /* Access rights for slots 0 and 1   */
const T_U8 SLOT_CONFIG_02_03[SHA204_ZONE_ACCESS_4] = {0x8F, 0x80,   0x8F, 0x80};  /* Access rights for slots 2 and 3   */
const T_U8 SLOT_CONFIG_04_05[SHA204_ZONE_ACCESS_4] = {0x8F, 0x80,   0x8F, 0x80};  /* Access rights for slots 4 and 5   */
const T_U8 SLOT_CONFIG_06_07[SHA204_ZONE_ACCESS_4] = {0x8F, 0x80,   0x8F, 0x80};  /* Access rights for slots 6 and 7   */
const T_U8 SLOT_CONFIG_08_09[SHA204_ZONE_ACCESS_4] = {0x8F, 0x80,   0x8F, 0x80};  /* Access rights for slots 8 and 9   */
const T_U8 SLOT_CONFIG_10_11[SHA204_ZONE_ACCESS_4] = {0x8F, 0x80,   0x8F, 0x80};  /* Access rights for slots 10 and 11 */
const T_U8 SLOT_CONFIG_12_13[SHA204_ZONE_ACCESS_4] = {0x8F, 0x80,   0x8F, 0x80};  /* Access rights for slots 12 and 13 */
const T_U8 SLOT_CONFIG_14_15[SHA204_ZONE_ACCESS_4] = {0x8F, 0x80,   0x8F, 0x80};  /* Access rights for slots 14 and 15 */

/*!
 *	*** Cconfugre USE FLAGS AND UPDATE COUNT bytes ***
 *
 * Use flags and update counts apply restrictions to enable limits on key usage.
 *	- For each 4-byte word:
 *		- Byte 0 is the UseFlag byte for the lower slot
 *		- Byte 1 is the UpdateCount byte for the lower slot
 *		- Byte 2 is the UseFlag byte for the upper slot
 *		- Byte 3 is the UpdateCount byte for the upper slot
 */
const T_U8 SLOT_0_1_USE_UPDATE[SHA204_ZONE_ACCESS_4] = {0xFF, 0xFF,  0xFF, 0xFF};
const T_U8 SLOT_2_3_USE_UPDATE[SHA204_ZONE_ACCESS_4] = {0xFF, 0xFF,  0x1F, 0xFF};
const T_U8 SLOT_4_5_USE_UPDATE[SHA204_ZONE_ACCESS_4] = {0xFF, 0x07,  0xFF, 0x3F};
const T_U8 SLOT_6_7_USE_UPDATE[SHA204_ZONE_ACCESS_4] = {0xFF, 0xFF,  0xFF, 0xFF};


/*!
 *	***	LAST KEY USE ***
 *
 *	Control limited use for KeyID 15.  Factory defaults are 0xFF
 *
 */
const T_U8 LAST_KEY_USE [LAST_KEY_USE_BYTE_SIZE] = {	0x00, 0x00, 0x00, 0x00,		// Bytes 68 - 71
														0x00, 0x00, 0x00, 0x00,		// Bytes 72 - 75
														0x00, 0x00, 0x00, 0x80,		// Bytes 76 - 79
														0xFF, 0xFF, 0xFF, 0xFF		// Bytes 80 - 83																				
													};

/*!
 *	***	Configure USER EXTRA and SELECTOR bytes ***
 *
 *	- Byte 0 configures UserExtra
 *	- Byte 1 configures Selector
 *	- Bytes 2 and 3 are ignored as they can only be modified via LOCK commands.
 *
 * These bytes are modifiable only through UpdateExtra and Lock commands only
 */

/*!
 *	*** INITIAL OTP CONTENT ***
 *
 *	512 Bits in total
 */
 const T_U8 OTP[2 * SHA204_ZONE_ACCESS_32] = {	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
													0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
													0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
													0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
													0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
													0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
													0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
													0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
												};

/*!
 *	*** INITIAL SLOT CONTENT ***
 *
 *	The initial slot content can be keys or data depending on custom security configuration.
 * 
 * The slots in this example are populated with values easy to remember for easy illustration.
 * Atmel strongly advices use of random values preferably from the high quality on-chip RNG 
 * (random number generator) for initial key values.
 *
 */

const T_U8 SLOT_00_CONTENT [SHA204_ZONE_ACCESS_32] = {	'A', 't', 'm', 'e' , 'l' , ' ', 'W', 'i',
															'd', 'g', 'e', 't', ' ', 'M', 'o', 'd',
															'e', 'l', ' ', '5', '5', '5', '5', 0x00,
															0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
														};

const T_U8 SLOT_01_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
															0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
															0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
															0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
														};

const T_U8 SLOT_02_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
															0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
															0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
															0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
														};

const T_U8 SLOT_03_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
															0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
															0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
															0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03
														};

const T_U8 SLOT_04_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
															0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
															0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
															0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04
														};

const T_U8 SLOT_05_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
															0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
															0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
															0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05
														};

const T_U8 SLOT_06_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
															0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
															0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
															0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06
														};

const T_U8 SLOT_07_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
															0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
															0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
															0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07
														};

const T_U8 SLOT_08_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
															0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
															0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
															0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
														};

const T_U8 SLOT_09_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
															0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
															0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
															0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09
														};

const T_U8 SLOT_10_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0xA,
															0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0xA,
															0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0xA,
															0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0xA
														};

const T_U8 SLOT_11_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
															0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
															0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
															0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B
														};

const T_U8 SLOT_12_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
															0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
															0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
															0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C
														};

const T_U8 SLOT_13_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
															0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
															0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
															0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D
														};

const T_U8 SLOT_14_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
															0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
															0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
															0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E
														};

const T_U8 SLOT_15_CONTENT [SHA204_ZONE_ACCESS_32] = {	0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
															0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
															0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
															0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
														};

T_U8 atsha204_device_personalization(T_VOID)
{		
	static T_U8 status = SHA204_SUCCESS;
	static T_U8 transmit_buffer[SHA204_CMD_SIZE_MAX];
	static T_U8 response_buffer[READ_32_RSP_SIZE]; 

	sha204_I2C_init(GPIO_I2C_SCL, GPIO_I2C_SDA);
	// Read lock bytes and verify successful lock
	status |= sha204_I2C_execute(SHA204_READ,SHA204_ZONE_CONFIG,EXTRA_SELECTOR_LOCK_ADDRESS,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer); 	
	status |= response_buffer[4] /* LockConfig */; 
	if(status!=0x55)
		return status;
	/*!
	 * Successful status here, i.e. status == SHA204_SUCCESS, means ATSHA204 was completely personalized and successfully locked.  Congratulations!
	 */

	
	/*!
	 *	*** ENTER PERSONALIZATON PREFERENCES INTO THE CONFIGURATION MEMORY ***
	 */
	
	// Device Operation Parameters
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,DEVICE_MODES_ADDRESS,SHA204_ZONE_ACCESS_4,DEVICE_MODES,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 
	
	// *** SLOT CONFIGURATION ***
	// Slots 0 and 1
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,SLOT_CONFIG_0_1_ADDRESS,SHA204_ZONE_ACCESS_4,SLOT_CONFIG_00_01,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Slots 2 and 3
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,SLOT_CONFIG_2_3_ADDRESS,SHA204_ZONE_ACCESS_4,SLOT_CONFIG_02_03,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Slots 4 and 5
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,SLOT_CONFIG_4_5_ADDRESS,SHA204_ZONE_ACCESS_4,SLOT_CONFIG_04_05,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Slots 6 and 7
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,SLOT_CONFIG_6_7_ADDRESS,SHA204_ZONE_ACCESS_4,SLOT_CONFIG_06_07,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Slots 8 and 9
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,SLOT_CONFIG_8_9_ADDRESS,SHA204_ZONE_ACCESS_4,SLOT_CONFIG_08_09,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Slots 10 and 11
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,SLOT_CONFIG_10_11_ADDRESS,SHA204_ZONE_ACCESS_4,SLOT_CONFIG_10_11,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Slots 12 and 13
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,SLOT_CONFIG_12_13_ADDRESS,SHA204_ZONE_ACCESS_4,SLOT_CONFIG_12_13,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Slots 14 and 15
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,SLOT_CONFIG_14_15_ADDRESS,SHA204_ZONE_ACCESS_4,SLOT_CONFIG_14_15,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// *** USE FLAG and UPDATE COUNT Region
	// Slots 0 and 1
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,SLOT_0_1_USE_UPDATE_ADDRESS,SHA204_ZONE_ACCESS_4,SLOT_0_1_USE_UPDATE,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Slots 2 and 3
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,SLOT_0_1_USE_UPDATE_ADDRESS,SHA204_ZONE_ACCESS_4,SLOT_2_3_USE_UPDATE,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Slots 4 and 5
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,SLOT_0_1_USE_UPDATE_ADDRESS,SHA204_ZONE_ACCESS_4,SLOT_4_5_USE_UPDATE,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Slots 6 and 7
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,SLOT_0_1_USE_UPDATE_ADDRESS,SHA204_ZONE_ACCESS_4,SLOT_6_7_USE_UPDATE,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// *** LAST KEY USE Region ***
	// First word
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,LAST_KEY_USE_ADDRESS+0,SHA204_ZONE_ACCESS_4,&LAST_KEY_USE[0x0],0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Second word
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,LAST_KEY_USE_ADDRESS+1,SHA204_ZONE_ACCESS_4,&LAST_KEY_USE[0x4],0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Third word
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,LAST_KEY_USE_ADDRESS+2,SHA204_ZONE_ACCESS_4,&LAST_KEY_USE[0x8],0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Fourth word
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_CONFIG,LAST_KEY_USE_ADDRESS+3,SHA204_ZONE_ACCESS_4,&LAST_KEY_USE[0xC],0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 
	
	//* *** OPTIONAL READ and VERIFY ****
	// The read data is contained in the response buffer.  Note that the buffer does not accumulate data. It's content is refreshed on each read so break and inspect buffer after each command execution.
	status |= sha204_I2C_execute(SHA204_READ,SHA204_ZONE_CONFIG|SHA204_ZONE_COUNT_FLAG ,CONFIG_BLOCK_0_ADDRESS,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	status |= sha204_I2C_execute(SHA204_READ,SHA204_ZONE_CONFIG|SHA204_ZONE_COUNT_FLAG ,CONFIG_BLOCK_1_ADDRESS,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	status |= sha204_I2C_execute(SHA204_READ,SHA204_ZONE_CONFIG,CONFIG_BLOCK_2_ADDRESS+0,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	status |= sha204_I2C_execute(SHA204_READ,SHA204_ZONE_CONFIG,CONFIG_BLOCK_2_ADDRESS+1,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	status |= sha204_I2C_execute(SHA204_READ,SHA204_ZONE_CONFIG,CONFIG_BLOCK_2_ADDRESS+2,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	status |= sha204_I2C_execute(SHA204_READ,SHA204_ZONE_CONFIG,CONFIG_BLOCK_2_ADDRESS+3,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	status |= sha204_I2C_execute(SHA204_READ,SHA204_ZONE_CONFIG,CONFIG_BLOCK_2_ADDRESS+4,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	status |= sha204_I2C_execute(SHA204_READ,SHA204_ZONE_CONFIG,CONFIG_BLOCK_2_ADDRESS+5,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);

	/*!
	 *	*** LOCK CONFIG ***
	 **********************
	 *
	 *	Forever lock the custom configuration from future modification.  Writing of Data or OTP regions requires prior execution of this command.
	 */
	
    status |= sha204_I2C_execute(SHA204_LOCK, LOCK_ZONE_NO_CRC, 0, 0,NULL, 0, NULL, 0, NULL, sizeof(transmit_buffer), transmit_buffer, sizeof(response_buffer), response_buffer);

	/*!
	 *	*** WRITE INITIAL DATA TO DATA SLOTS ***
	 *******************************************
	 *
	 *  Write initial content to data slots.  This is the only opportunity to to write non-modifiable information e.g. model numbers and certain keys.
	 */
	
	// Write initial content for slot 0
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_0_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_00_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 1
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_1_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_01_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 2
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_2_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_02_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 3
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_3_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_03_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 4
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_4_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_04_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 5
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_5_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_05_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 6
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_6_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_06_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 7
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_7_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_07_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 8
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_8_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_08_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 9
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_9_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_09_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 10
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_10_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_10_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 11
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_11_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_11_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 12
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_12_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_12_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 13
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_13_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_13_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 14
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_14_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_14_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write initial content for slot 15
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG,SLOT_15_ADDRESS,SHA204_ZONE_ACCESS_32,SLOT_15_CONTENT,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	
	/*!
	 *	*** WRITE INITIAL OTP DATA INTO THE OTP REGION ***
	 *****************************************************
	 *  Write initial information to the OTP region.  This is the only opportunity to do so.  After locking data and OTP regions, write accesses to this region will be controlled by 
	 *  custom access privileges defined in the configuration region.
	 */
	// Write the first 32 bytes of the 64-byte OTP block
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_OTP|SHA204_ZONE_COUNT_FLAG,OTP_BLOCK_0_ADDRESS,SHA204_ZONE_ACCESS_32,&OTP[0],0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	 

	// Write the second 32 bytes of the 64-byte OTP block
	status |= sha204_I2C_execute(SHA204_WRITE,SHA204_ZONE_OTP|SHA204_ZONE_COUNT_FLAG,OTP_BLOCK_1_ADDRESS,SHA204_ZONE_ACCESS_32,&OTP[8],0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);

	/*!
	 *	*** LOCK VALUE ***
	 *********************
	 *
	 *	Forever lock the data and OTP regions.  After lock data, access to these regions will be controlled by access rights defined in the configuration region.
	 */
    //status |= sha204_I2C_execute(SHA204_LOCK,LOCK_ZONE_NO_CONFIG|LOCK_ZONE_NO_CRC,LOCK_PARAM2_NO_CRC,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);
	
	
	/*!
	 *	*** VERIFY SUCCESSFUL COMPLETION OF THE PERSONALIZATION PROCESS ***
	 **********************************************************************
	 *
	 * Check that all functions executed without errors and that the chip is actually locked.
	 */
	
	// Read lock bytes and verify successful lock
	status |= sha204_I2C_execute(SHA204_READ,SHA204_ZONE_CONFIG,EXTRA_SELECTOR_LOCK_ADDRESS,0,NULL,0,NULL,0,NULL,sizeof(transmit_buffer),transmit_buffer,sizeof(response_buffer),response_buffer);		
	status |= response_buffer[3] /* LockValue */ & response_buffer[4] /* LockConfig */; 
	
	/*!
	 * Successful status here, i.e. status == SHA204_SUCCESS, means ATSHA204 was completely personalized and successfully locked.  Congratulations!
	 */
	AkDebugOutput("Sha204_device_personalization statu:%x,lock_config%x\n",status,response_buffer[4]);
	return status;
}

T_VOID Flash_WriteKeyFromFile(T_VOID)
{
	T_hFILE file;
	T_U16	rBuf[2];
	T_U32	i,j = 0;
	T_U32   key_num = 0;
	T_U8 	ret_code;
	T_U8 SLOT_CONTENT [16][32] = {'0'};
	// Make the command buffer the size of the MAC command.
	static T_U8 command[WRITE_COUNT_LONG];
	// Make the response buffer the size of a READ response.
	static T_U8 response[WRITE_RSP_SIZE];

	file = Fwl_FileOpen(_T("D:/IcKey.txt"), FS_MODE_READ, FS_MODE_READ);
	if(file == FS_INVALID_HANDLE)
		Fwl_FileOpen(_T("D:/IcKey.txt"), FS_MODE_CREATE, FS_MODE_CREATE);

	for(j=0; j<16; j++)
	{
		i = 0;
		while(1)
		{
			memset(rBuf[0], 0, sizeof(T_U16));
			if(sizeof(T_U16) != Fwl_FileRead(file, &rBuf[0], sizeof(T_U16)))
			{
				if(32 == i)
					key_num++;
				
				break;
			}
				
			rBuf[1]=0;
			//0xa0d == "\n\r"
			if(0xa0d==rBuf[0] || 32==i)
			{
				key_num++;
				break;
			}
			else if((AToI16(&rBuf[0]) || 0==strcmp(&rBuf[0], "00")) && i < 32)
			{
				SLOT_CONTENT[j][i] = AToI16(&rBuf[0]);
				i++;
			}			
		}
		
	}

	// Init encrypt Ic device
	sha204_I2C_init(GPIO_I2C_SCL, GPIO_I2C_SDA);
	
	for(j=0; j<key_num; j++)
	{
		// write key to slot
		ret_code = sha204_I2C_execute(SHA204_WRITE, SHA204_ZONE_COUNT_FLAG|SHA204_ZONE_DATA, 
						j*8 , SHA204_ZONE_ACCESS_32, &SLOT_CONTENT[j][0],
						0, AK_NULL, 0, AK_NULL , WRITE_COUNT_LONG, &command[0],
						WRITE_RSP_SIZE, &response[0]);
		if ((ret_code != SHA204_SUCCESS) || (0 != response[1]))
		{
			AK_DEBUG_OUTPUT("sha204 Write fail:%x\r\n",ret_code);
		}
	}

	for(j=0; j<key_num; j++)
	{
		AK_DEBUG_OUTPUT("key%d:", j);
		for (i = 0; i < 32; i++)
		{
			AK_DEBUG_OUTPUT("0X%02x,",SLOT_CONTENT[j][i]);
		}
	
		AK_DEBUG_OUTPUT("\r\n");
	}
	
	if(file != FS_INVALID_HANDLE)
		Fwl_FileClose(file);
}
#endif
