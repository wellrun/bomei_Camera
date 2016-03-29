/*
 * @FILENAME lib_GBA.h
 * @BRIEF This file declare the structure/type/function provided by GBA library
 * Copyright (C) 2008 Anyka (Guangzhou) Software Technology Co., LTD
 * @AUTHOR liu zhuyuan
 * @DATE 2008-12-08
 * @VERSION 1.0.0
 */


#ifndef	__GBA_H__
#define __GBA_H__

#include "lib_gba_callback.h"

#define GBA_LIB_VERSION "GBALib V1.0.1"

// result code
typedef enum {
	GBA_RET_OK = 0,
	GBA_RET_INPUT_ERROR,
	GBA_RET_FILE_ERROR,
	GBA_RET_ROM_UNSUPPORT_ERROR,
	GBA_RET_OUT_OF_MEM__ERROR,
	GBA_RET_MACHINE_INIT_ERROR,
	GBA_RET_PARAMETER_ERROR
}GBA_RET;

// button code
typedef enum {
	GBA_BT_A = 0,
	GBA_BT_B,
	GBA_BT_L,
	GBA_BT_R,
	GBA_BT_SELECT,
	GBA_BT_START,
	GBA_BT_UP,
	GBA_BT_DOWN,
	GBA_BT_LEFT,
	GBA_BT_RIGHT,
	GBA_BT_NUM
}GBA_BT_ID;

typedef void* GBAFILEHANDLE;

/** 
 * @brief Load rom data from GBA file and initialize the GBA Emulator 
 * @param[in] 	file name
 * @return GBA_RET
 */
extern GBA_RET GBA_Init(GBA_CALLBACK_FUNCS *cb_funcs, GBAFILEHANDLE fh);

/** 
 * @brief excute one frame instruction and display one frame
 * this function should be called 60 times every second
 * @return void
 */
extern void GBA_Run_Step(void);

/** 
 * @brief exit GBA
 * @return void
 */
extern void GBA_Exit(void);

/** 
 * @brief pause GBA
 * @return void
 */
extern void GBA_Pause (void);

/** 
 * @brief resume GBA
 * @return void
 */
extern void GBA_Resume (void);


/** 
 * @brief set skip frame number while GBA runing. With more skipnum, GBA will 
 * @display less frame and run faster. default is 5.
 * @param[in] 	frame number to skip between two display frame, should be (1-10)
 * @return GBA_RET GBA_RET_OK or GBA_RET_PARAMETER_ERROR
 */
extern GBA_RET GBA_SetFrameSkip(int skipnum);

/** 
 * @brief turn on/off GBA audio. default is on. 
 * @NOTE: DO NOT TURN ON/OFF AUDIO WHILE PLAYING GAME!
 * @param[in] 0: off; 1: on
 * @return GBA_RET GBA_RET_OK or GBA_RET_PARAMETER_ERROR
 */
extern GBA_RET GBA_AudioOn(int flag);

/** 
 * @brief set one key state(up or down)
 * @param[in] button the key id to be set
 * @param[in] state the button state(0 means unpressed, 1 means held)
 * @return void
 */
extern void GBA_SetButton(GBA_BT_ID button, int state);

/** 
 * @brief save or load immdiate date of the game 
 * @param[in] save file name
 * @return GBA_RET GBA_RET_OK or GBA_RET_FILE_ERROR
 */
extern GBA_RET GBA_SaveImmdiateData(GBAFILEHANDLE fh);

/** 
 * @brief save or load immdiate date of the game 
 * @param[in]  load file name
 * @return GBA_RET GBA_RET_OK or GBA_RET_FILE_ERROR
 */
extern GBA_RET GBA_LoadImmdiateData(GBAFILEHANDLE fh);

/** 
 * @brief get version information of gba library 
 * @return string about gba version information
 */
extern const char *GBA_GetVersionInfo(void);

#endif
