/**
 * @file tscr_command.h
 * @brief Define commands for tscr function
 * 
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2006-01-16
 * @version 1.0
 */



/** @defgroup TouchScreen  
 *	@ingroup M3PLATFORM
 */
/*@{*/


 ///@cond TSCR_COMMAND


#ifndef __H_TSCR_COMMAND_
#define __H_TSCR_COMMAND_

/******************* command list ***********************/
/*
 * host command
 */
#define ePH_CMD_WAKE_UP				0x33 /** the command of wake up*/

#define ePH_CMD_SET_RECOG_MODE		0x10 /** the command of recognize the input word*/

#define ePH_CMD_SWITCH_INK			0x14 /** the command of switch ink*/

#define ePH_CMD_SET_PENUP_TIME		0x1a /** the command of set the penup time*/

#define ePH_CMD_QUICK_RECOG			0x1b /** the command of quick recognize word*/

#define ePH_CMD_HOST_READY			0x1c /** the command of the host is ready*/

#define ePH_CMD_DECR_PENUP_TIME		0x1e /** the command of decrease the penup time*/

#define ePH_CMD_SET_CODE_TBL		0x41 /** the command of set code table*/

#define ePH_CMD_SOFT_RESET			0x42 /** the command of reset soft key*/

#define ePH_CMD_ABORT_INKING		0x43 /** the command of abort inking*/

#define ePH_CMD_CALIBRATION			0x44 /** the command of calibration*/

#define ePH_CMD_SET_WRITE_AREA		0x46 /** the command of set write area*/

#define ePH_CMD_SET_MODE			0x49 /** the command of set mode*/

#define ePH_CMD_SET_POWER_SAVING	0x4a /** the command of set power saving*/

#define ePH_CMD_ROTATE_PANEL		0x4c /** the command of rotate panel*/

/*
 * ePH1100 respone
 */
#define ePH_RESPONE_ACK_ERROR		0x00 /** the response of ack error*/

#define ePH_RESPONE_INKING_COOR		0x16 /** the response of inking coopration*/

#define ePH_RESPONE_BUTTON_COOR		0x17 /** the response of button coopration*/

#define ePH_RESPONE_RECOG_ARRAY		0x18 /** the response of recognize array*/

#define ePH_RESPONE_EXIT_POWER_SAVING 0x33 /** the response of ack exit power saving*/

#define ePH_RESPONE_INIT_ON			0x42 /** the response of init is on*/



/********************* data structure list *****************/
/*
 * define the max stuff length,include checksum
 */
#define STUFF_LEN			22

/*
 * define the max packet length
 */
#define MAX_TPCKT_LEN		25

/*
 * define the length of packet header
 */
#define TPCKT_HDR_LEN		3

/*
 * define the header byte offset
 */
#define HDR_BYTE_OFFSET		0x0

/*
 * define the parameter length byte offset
 */
#define PARAM_LENGTH_OFFSET	0x2

/* 
 * define the header character
 */
#define HEADER_BYTE			0x50

/*
 * define the touch screen input packet format
 */
typedef struct{
	T_U8 hdr;				//header byte ,0x50
	T_U8 cmd;				//host command or ePH response
	T_U8 len;				//parameter length
	T_U8 data[STUFF_LEN];	//stuffed with data and checksum
}TSCR_PCKT;


#endif //__H_TSCR_COMMAND_



///@endcond


/*@}*/

