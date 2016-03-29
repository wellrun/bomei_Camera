/**
 * @filename sha204.h
 * @brief  the arithmetic  to sha204 drive head file.
 * Copyright (C) 2012 Anyka (Guangzhou) Software Technology Co., LTD
 * @author luheshan
 * @date 2012-07-02
 * @version 1.0
 */

#ifndef __SHA204_H_
#define __SHA204_H_

#define SHA204_I2C_ADDRESS              ((T_U8) 0xC8)

#define SHA204_CMD_SIZE_MIN             (7)
//! maximum size of command packet (CheckMac)
#define SHA204_CMD_SIZE_MAX             ((T_U8) 84)

// parameter range definitions
#define SHA204_KEY_ID_MAX               ((T_U8) 15)         //!< maximum value for key id
#define SHA204_OTP_BLOCK_MAX            ((T_U8)  1)         //!< maximum value for OTP block

// definitions for command packet indexes common to all commands
#define SHA204_COUNT_IDX                ( 0)                   //!< command packet index for count
#define SHA204_OPCODE_IDX               ( 1)                   //!< command packet index for op-code
#define SHA204_PARAM1_IDX               ( 2)                   //!< command packet index for first parameter
#define SHA204_PARAM2_IDX               ( 3)                   //!< command packet index for second parameter
#define SHA204_DATA_IDX                 ( 5)                   //!< command packet index for second parameter

// zone definitions
#define SHA204_ZONE_CONFIG              ((T_U8)  0x00)      //!< Configuration zone
#define SHA204_ZONE_OTP                 ((T_U8)  0x01)      //!< OTP (One Time Programming) zone
#define SHA204_ZONE_DATA                ((T_U8)  0x02)      //!< Data zone
#define SHA204_ZONE_COUNT_FLAG          ((T_U8)  0x80)      //!< Zone bit 7 set: Access 32 bytes, otherwise 4 bytes.
#define SHA204_ZONE_ACCESS_4            ((T_U8)     4)      //!< Read or write 4 bytes.
#define SHA204_ZONE_ACCESS_32           ((T_U8)    32)      //!< Read or write 32 bytes.
#define SHA204_ADDRESS_MASK_CONFIG      ((T_U8)  0x1F)      //!< Address bits 5 to 7 are 0 for Configuration zone.
#define SHA204_ADDRESS_MASK_OTP         ((T_U8)  0x0F)      //!< Address bits 4 to 7 are 0 for OTP zone.
#define SHA204_ADDRESS_MASK             ((T_U8) 0x007F)    //!< Address bit 7 to 15 are always 0.

// Read command definitions
#define READ_ZONE_IDX                   SHA204_PARAM1_IDX      //!< Read command index for zone
#define READ_ADDR_IDX                   SHA204_PARAM2_IDX      //!< Read command index for address
#define READ_COUNT                      SHA204_CMD_SIZE_MIN    //!< Read command packet size
#define READ_ZONE_MASK                  ((T_U8) 0x83)       //!< Read zone bits 2 to 6 are 0.
#define READ_ZONE_MODE_32_BYTES         ((T_U8) 0x80)       //!< Read mode: 32 bytes

// Write command definitions
#define WRITE_ZONE_IDX                  SHA204_PARAM1_IDX      //!< Write command index for zone
#define WRITE_ADDR_IDX                  SHA204_PARAM2_IDX      //!< Write command index for address
#define WRITE_VALUE_IDX                 SHA204_DATA_IDX        //!< Write command index for data
#define WRITE_MAC_VS_IDX                ( 9)                   //!< Write command index for MAC following short data
#define WRITE_MAC_VL_IDX                (37)                   //!< Write command index for MAC following long data
#define WRITE_COUNT_SHORT               (11)                   //!< Write command packet size with short data and no MAC
#define WRITE_COUNT_LONG                (39)                   //!< Write command packet size with long data and no MAC
#define WRITE_COUNT_SHORT_MAC           (43)                   //!< Write command packet size with short data and MAC
#define WRITE_COUNT_LONG_MAC            (71)                   //!< Write command packet size with long data and MAC
#define WRITE_MAC_SIZE                  (32)                   //!< Write MAC size
#define WRITE_ZONE_MASK                 ((T_U8) 0xC3)       //!< Write zone bits 2 to 5 are 0.
#define WRITE_ZONE_WITH_MAC             ((T_U8) 0x40)       //!< Write zone bit 6: write encrypted with MAC

// Lock command definitions
#define LOCK_ZONE_IDX                   SHA204_PARAM1_IDX      //!< Lock command index for zone
#define LOCK_SUMMARY_IDX                SHA204_PARAM2_IDX      //!< Lock command index for summary
#define LOCK_COUNT                      SHA204_CMD_SIZE_MIN    //!< Lock command packet size
#define LOCK_ZONE_NO_CONFIG             ((T_U8) 0x01)       //!< Lock zone is OTP or Data
#define LOCK_ZONE_NO_CRC                ((T_U8) 0x80)       //!< Lock command: Ignore summary.
#define LOCK_ZONE_MASK                  (0x81)                 //!< Lock parameter 1 bits 2 to 6 are 0.

// Response size definitions
#define CHECKMAC_RSP_SIZE               4    //!< response size of DeriveKey command
#define DERIVE_KEY_RSP_SIZE             4    //!< response size of DeriveKey command
#define DEVREV_RSP_SIZE                 7    //!< response size of DevRev command returns 4 bytes
#define GENDIG_RSP_SIZE                 4    //!< response size of GenDig command
#define HMAC_RSP_SIZE                   35    //!< response size of HMAC command
#define LOCK_RSP_SIZE                   4    //!< response size of Lock command
#define MAC_RSP_SIZE                    35    //!< response size of MAC command
#define NONCE_RSP_SIZE_SHORT            4    //!< response size of Nonce command with mode[0:1] = 3
#define NONCE_RSP_SIZE_LONG             35    //!< response size of Nonce command
#define PAUSE_RSP_SIZE                  4    //!< response size of Pause command
#define RANDOM_RSP_SIZE                 35    //!< response size of Random command
#define READ_4_RSP_SIZE                 7    //!< response size of Read command when reading 4 bytes
#define READ_32_RSP_SIZE                35    //!< response size of Read command when reading 32 bytes
#define TEMP_SENSE_RSP_SIZE             7    //!< response size of TempSense command returns 4 bytes
#define UPDATE_RSP_SIZE                 4    //!< response size of UpdateExtra command
#define WRITE_RSP_SIZE                  4    //!< response size of Write command

// CheckMAC command definitions
#define CHECKMAC_MODE_IDX               SHA204_PARAM1_IDX      //!< CheckMAC command index for mode
#define CHECKMAC_KEYID_IDX              SHA204_PARAM2_IDX      //!< CheckMAC command index for key identifier
#define CHECKMAC_CLIENT_CHALLENGE_IDX   SHA204_DATA_IDX        //!< CheckMAC command index for client challenge
#define CHECKMAC_CLIENT_RESPONSE_IDX    (37)                   //!< CheckMAC command index for client response
#define CHECKMAC_DATA_IDX               (69)                   //!< CheckMAC command index for other data
#define CHECKMAC_COUNT                  (84)                   //!< CheckMAC command packet size
#define CHECKMAC_MODE_MASK              ((T_U8) 0x27)       //!< CheckMAC mode bits 3, 4, 6, and 7 are 0.
#define CHECKMAC_CLIENT_CHALLENGE_SIZE  (32)                   //!< CheckMAC size of client challenge
#define CHECKMAC_CLIENT_RESPONSE_SIZE   (32)                   //!< CheckMAC size of client response
#define CHECKMAC_OTHER_DATA_SIZE        (13)                   //!< CheckMAC size of "other data"

// DeriveKey command definitions
#define DERIVE_KEY_RANDOM_IDX           SHA204_PARAM1_IDX      //!< DeriveKey command index for random bit
#define DERIVE_KEY_TARGETKEY_IDX        SHA204_PARAM2_IDX      //!< DeriveKey command index for target slot
#define DERIVE_KEY_MAC_IDX              SHA204_DATA_IDX        //!< DeriveKey command index for optional MAC
#define DERIVE_KEY_COUNT_SMALL          SHA204_CMD_SIZE_MIN    //!< DeriveKey command packet size without MAC
#define DERIVE_KEY_COUNT_LARGE          (39)                   //!< DeriveKey command packet size with MAC
#define DERIVE_KEY_RANDOM_FLAG          ((T_U8) 4)             //!< DeriveKey 1. parameter
#define DERIVE_KEY_MAC_SIZE             (32)                   //!< DeriveKey MAC size

// GenDig command definitions
#define GENDIG_ZONE_IDX                 SHA204_PARAM1_IDX      //!< GenDig command index for zone
#define GENDIG_KEYID_IDX                SHA204_PARAM2_IDX      //!< GenDig command index for key id
#define GENDIG_DATA_IDX                 SHA204_DATA_IDX        //!< GenDig command index for optional data
#define GENDIG_COUNT                    SHA204_CMD_SIZE_MIN    //!< GenDig command packet size without "other data"
#define GENDIG_COUNT_DATA               (11)                   //!< GenDig command packet size with "other data"
#define GENDIG_OTHER_DATA_SIZE          (4)                    //!< GenDig size of "other data"
#define GENDIG_ZONE_OTP                 ((T_U8) 1)          //!< GenDig zone id OTP
#define GENDIG_ZONE_DATA                ((T_U8) 2)          //!< GenDig zone id data

// HMAC command definitions
#define HMAC_MODE_IDX                   SHA204_PARAM1_IDX      //!< HMAC command index for mode
#define HMAC_KEYID_IDX                  SHA204_PARAM2_IDX      //!< HMAC command index for key id
#define HMAC_COUNT                      SHA204_CMD_SIZE_MIN    //!< HMAC command packet size
#define HMAC_MODE_MASK                  ((T_U8) 0x74)       //!< HMAC mode bits 0, 1, 3, and 7 are 0.

// Mac command definitions
#define MAC_MODE_IDX                    SHA204_PARAM1_IDX      //!< MAC command index for mode
#define MAC_KEYID_IDX                   SHA204_PARAM2_IDX      //!< MAC command index for key id
#define MAC_CHALLENGE_IDX               SHA204_DATA_IDX        //!< MAC command index for optional challenge
#define MAC_COUNT_SHORT                 SHA204_CMD_SIZE_MIN    //!< MAC command packet size without challenge
#define MAC_COUNT_LONG                  (39)                   //!< MAC command packet size with challenge
#define MAC_MODE_BLOCK2_TEMPKEY         ((T_U8) 0x01)       //!< MAC mode bit   0: second SHA block from TempKey
#define MAC_MODE_BLOCK1_TEMPKEY         ((T_U8) 0x02)       //!< MAC mode bit   1: first SHA block from TempKey
#define MAC_MODE_SOURCE_FLAG_MATCH      ((T_U8) 0x04)       //!< MAC mode bit   2: match TempKey.SourceFlag
#define MAC_MODE_PASSTHROUGH            ((T_U8) 0x07)       //!< MAC mode bit 0-2: pass-through mode
#define MAC_MODE_INCLUDE_OTP_88         ((T_U8) 0x10)       //!< MAC mode bit   4: include first 88 OTP bits
#define MAC_MODE_INCLUDE_OTP_64         ((T_U8) 0x20)       //!< MAC mode bit   5: include first 64 OTP bits
#define MAC_MODE_INCLUDE_SN             ((T_U8) 0x50)       //!< MAC mode bit   6: include serial number
#define MAC_CHALLENGE_SIZE              (32)                   //!< MAC size of challenge
#define MAC_MODE_MASK                   ((T_U8) 0x77)       //!< MAC mode bits 3 and 7 are 0.

// Nonce command definitions
#define NONCE_MODE_IDX                  SHA204_PARAM1_IDX      //!< Nonce command index for mode
#define NONCE_PARAM2_IDX                SHA204_PARAM2_IDX      //!< Nonce command index for 2. parameter
#define NONCE_INPUT_IDX                 SHA204_DATA_IDX        //!< Nonce command index for input data
#define NONCE_COUNT_SHORT               (27)                   //!< Nonce command packet size for 20 bytes of data
#define NONCE_COUNT_LONG                (39)                   //!< Nonce command packet size for 32 bytes of data
#define NONCE_MODE_MASK                 ((T_U8) 3)          //!< Nonce mode bits 2 to 7 are 0.
#define NONCE_MODE_SEED_UPDATE          ((T_U8) 0x00)       //!< Nonce mode: update seed
#define NONCE_MODE_NO_SEED_UPDATE       ((T_U8) 0x01)       //!< Nonce mode: do not update seed
#define NONCE_MODE_INVALID              ((T_U8) 0x02)       //!< Nonce mode 2 is invalid.
#define NONCE_MODE_PASSTHROUGH          ((T_U8) 0x03)       //!< Nonce mode: pass-through
#define NONCE_NUMIN_SIZE                (20)                   //!< Nonce data length
#define NONCE_NUMIN_SIZE_PASSTHROUGH    (32)                   //!< Nonce data length in pass-through mode (mode = 3)

// Random command definitions
#define RANDOM_MODE_IDX                 SHA204_PARAM1_IDX      //!< Random command index for mode
#define RANDOM_PARAM2_IDX               SHA204_PARAM2_IDX      //!< Random command index for 2. parameter
#define RANDOM_COUNT                    SHA204_CMD_SIZE_MIN    //!< Random command packet size
#define RANDOM_SEED_UPDATE              ((T_U8) 0x00)       //!< Random mode for automatic seed update
#define RANDOM_NO_SEED_UPDATE           ((T_U8) 0x01)       //!< Random mode for no seed update

// UpdateExtra command definitions
#define UPDATE_MODE_IDX                  SHA204_PARAM1_IDX     //!< UpdateExtra command index for mode
#define UPDATE_VALUE_IDX                 SHA204_PARAM2_IDX     //!< UpdateExtra command index for new value
#define UPDATE_COUNT                     SHA204_CMD_SIZE_MIN   //!< UpdateExtra command packet size
#define UPDATE_CONFIG_BYTE_86            ((T_U8) 0x01)      //!< UpdateExtra mode: update Config byte 86

// command op-code definitions
#define SHA204_CHECKMAC                 ((T_U8) 0x28)       //!< CheckMac command op-code
#define SHA204_DERIVE_KEY               ((T_U8) 0x1C)       //!< DeriveKey command op-code
#define SHA204_DEVREV                   ((T_U8) 0x30)       //!< DevRev command op-code
#define SHA204_GENDIG                   ((T_U8) 0x15)       //!< GenDig command op-code
#define SHA204_HMAC                     ((T_U8) 0x11)       //!< HMAC command op-code
#define SHA204_LOCK                     ((T_U8) 0x17)       //!< Lock command op-code
#define SHA204_MAC                      ((T_U8) 0x08)       //!< MAC command op-code
#define SHA204_NONCE                    ((T_U8) 0x16)       //!< Nonce command op-code
#define SHA204_PAUSE                    ((T_U8) 0x01)       //!< Pause command op-code
#define SHA204_RANDOM                   ((T_U8) 0x1B)       //!< Random command op-code
#define SHA204_READ                     ((T_U8) 0x02)       //!< Read command op-code
#define SHA204_TEMPSENSE                ((T_U8) 0x18)       //!< TempSense command op-code
#define SHA204_UPDATE_EXTRA             ((T_U8) 0x20)       //!< UpdateExtra command op-code
#define SHA204_WRITE                    ((T_U8) 0x12)       //!< Write command op-code

//sha204 lib return state value
#define SHA204_SUCCESS                  ((T_U8)  0x00) //!< Function succeeded.
#define SHA204_PARSE_ERROR              ((T_U8)  0xD2) //!< response status byte indicates parsing error
#define SHA204_CMD_FAIL                 ((T_U8)  0xD3) //!< response status byte indicates command execution error
#define SHA204_STATUS_CRC               ((T_U8)  0xD4) //!< response status byte indicates CRC error
#define SHA204_STATUS_UNKNOWN           ((T_U8)  0xD5) //!< response status byte is unknown
#define SHA204_FUNC_FAIL                ((T_U8)  0xE0) //!< Function could not execute due to incorrect condition / state.
#define SHA204_GEN_FAIL                 ((T_U8)  0xE1) //!< unspecified error
#define SHA204_BAD_PARAM                ((T_U8)  0xE2) //!< bad argument (out of range, null pointer, etc.)
#define SHA204_INVALID_ID               ((T_U8)  0xE3) //!< invalid device id, id not set
#define SHA204_INVALID_SIZE             ((T_U8)  0xE4) //!< Count value is out of range or greater than buffer size.
#define SHA204_BAD_CRC                  ((T_U8)  0xE5) //!< incorrect CRC received
#define SHA204_RX_FAIL                  ((T_U8)  0xE6) //!< Timed out while waiting for response. Number of bytes received is > 0.
#define SHA204_RX_NO_RESPONSE           ((T_U8)  0xE7) //!< Not an error while the Command layer is polling for a command response.
#define SHA204_RESYNC_WITH_WAKEUP       ((T_U8)  0xE8) //!< re-synchronization succeeded, but only after generating a Wake-up
#define SHA204_COMM_FAIL                ((T_U8)  0xF0) //!< Communication with device failed. Same as in hardware dependent modules.
#define SHA204_TIMEOUT                  ((T_U8)  0xF1) //!< Timed out while waiting for response. Number of bytes received is 0.


/** \brief This function is to initail sha204 i2c two pin.
 *
 * \param[in] pin_scl i2c of scl pin
 * \param[in] pin_sda i2c of sda pin
 * \return status of the void
 */
T_VOID sha204_I2C_init(T_U32 pin_scl, T_U32 pin_sda);


/** \brief This function creates a command packet, sends it, and receives its response.
 *
 * \param[in] op_code command op-code
 * \param[in] param1 first parameter
 * \param[in] param2 second parameter
 * \param[in] datalen1 number of bytes in first data block
 * \param[in] data1 pointer to first data block
 * \param[in] datalen2 number of bytes in second data block
 * \param[in] data2 pointer to second data block
 * \param[in] datalen3 number of bytes in third data block
 * \param[in] data3 pointer to third data block
 * \param[in] tx_size size of tx buffer
 * \param[in] tx_buffer pointer to tx buffer
 * \param[in] rx_size size of rx buffer
 * \param[out] rx_buffer pointer to rx buffer
 * \return status of the operation
 */
T_U8 sha204_I2C_execute(T_U8 op_code, T_U8 param1, T_U16 param2,
            T_U8 datalen1, T_U8 *data1, T_U8 datalen2, T_U8 *data2, T_U8 datalen3, T_U8 *data3,
            T_U8 tx_size, T_U8 *tx_buffer, T_U8 rx_size, T_U8 *rx_buffer);

#endif

