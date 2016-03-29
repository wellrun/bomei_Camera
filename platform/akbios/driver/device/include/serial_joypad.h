/**
 * @file joypad.h
 * @brief joypad driver, provide serial port joypad APIs
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author wangyf
 * @date 2007-05-16
 * @version 1.0
 */
#ifndef __JOYPAD_H__
#define __JOYPAD_H__

typedef unsigned short JOYPAD_KEY_STATUS;

/*
 *	joypad key mask list
 */
#define JOYPAD_KEY_NULL		0x0000
#define JOYPAD_KEY_UP		0x0001
#define JOYPAD_KEY_DOWN		0x0002
#define JOYPAD_KEY_LEFT		0x0004
#define JOYPAD_KEY_RIGHT	0x0008
#define JOYPAD_KEY_LKEY		0x0100
#define JOYPAD_KEY_RKEY		0x0020
#define JOYPAD_KEY_SELECT	0x0040
#define JOYPAD_KEY_PLAY		0x0080
#define JOYPAD_KEY_TURBO	0x0010
#define JOYPAD_KEY_A		0x0200
#define JOYPAD_KEY_B		0x0400
#define JOYPAD_KEY_C		0x1000
#define JOYPAD_KEY_D		0x0800

#define	JOYPAD_KEY_UNCHANGE	0xffff

/**
 * @brief reset joypad
 * should be called before joypad start to work
 * @author wangyf
 * @date 2007-05-16
 * @param
 * @return T_VOID
 */
void Joypad_Reset();

/**
 * @brief handle serial character for joypad
 * @author wangyf
 * @date 2007-05-16
 * @param	unsigned char c	character from serial
 * @return JOYPAD_KEY_STATUS:	JOYPAD_KEY_UNCHANGE or current KEY mask
 */
JOYPAD_KEY_STATUS Joypad_Serial_Single_Event(unsigned char c);

/**
 * @brief handle several serial characters for joypad
 * @author wangyf
 * @date 2007-05-16
 * @param	unsigned char *c	character buffer from serial
 * @param	int eventNum		character number in the buffer
 * @return JOYPAD_KEY_STATUS:	JOYPAD_KEY_UNCHANGE or current KEY mask
 */
JOYPAD_KEY_STATUS Joypad_Serial_Multi_Event(unsigned char *c, int eventNum);

#endif