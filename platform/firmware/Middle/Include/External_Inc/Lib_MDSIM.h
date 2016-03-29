/*
 * @FILENAME lib_MDSIM.h
 * @BRIEF This file declare the structure/type/function provided by MDSIM library
 * Copyright (C) 2009 Anyka (Guangzhou) Software Technology Co., LTD
 * @AUTHOR Wang Yanfei
 * @DATE 2010-5-26
 * @VERSION 0.1.6
 */


#ifndef	__MDSIM_H__
#define __MDSIM_H__

#define MDSIM_LIB_VERSION "MDSIM_lib V0.1.6"

#ifdef __cplusplus
extern "C" {
#endif

typedef	void* FILEHANDLE;
typedef	void* SNDHANDLE;

#define MDSIM_SEEK_SET 0
#define MDSIM_SEEK_CUR 1
#define MDSIM_SEEK_END 2

// Callback Function
/** 
 * @brief file system Management. read data from file 
 * @param[in] 	file descriptor
 * @param[in] 	buffer which the data write to
 * @param[in] 	data size to be read from file (in byte)
 * @return int	the data write to the buffer
 */
typedef int (*MDSIM_CALLBACK_FUNC_FREAD)(FILEHANDLE fp, void* buf, int length);
/** 
 * @brief file system Management. write data to file 
 * @param[in] 	file descriptor
 * @param[in] 	buffer which the data write to
 * @param[in] 	data size to be write to file (in byte)
 * @return int	the data write to the file
 */
typedef int (*MDSIM_CALLBACK_FUNC_FWRITE)(FILEHANDLE fp, void* buf, int length);
/** 
 * @brief file system Management. move the file pointer to a specified position
 * @param[in] 	file descriptor
 * @param[in] 	number of bytes from whence
 * @param[in] 	initial position
 * @return 0 if ok, otherwise return nonzero value
 */
typedef int (*MDSIM_CALLBACK_FUNC_FSEEK)(FILEHANDLE fp, long offset, int whence);
/** 
 * @brief file system Management. gets the current position of a file pointer
 * @param[in] 	file descriptor
 * @return int	current position
 */
typedef int (*MDSIM_CALLBACK_FUNC_FTELL)(FILEHANDLE fp);

/** 
 * @brief Memory Management. allocate a memory block with specified size (in byte) 
 * @param[in] size	size of the memory block
 * @return null(zero) if failed; otherwise, return a pointer to the memory
 */
typedef void* (*MDSIM_CALLBACK_FUNC_MALLOC)(unsigned int size);
/** 
 * @brief Memory Management. free a memory block 
 * @param[in] 	pointer to the memory block to be released
 * @return void
 */
typedef void (*MDSIM_CALLBACK_FUNC_FREE)(void* mem);

/** 
 * @brief display RGB565 image data on the screen
 * @param[in] 	RGB565 image data
 * @param[in] 	image width in pixel
 * @param[in] 	image height in pixel
 * @return NULL
 */
typedef void (*MDSIM_CALLBACK_FUNC_DISP)(void* imgBuf, int imgWidth, int imgHeight);

/** 
 * @brief open wave out port
 * @return wave out port handle, used in wave output interface
 */
typedef SNDHANDLE (*MDSIM_CALLBACK_FUNC_SND_OPEN)(void);
/** 
 * @brief close wave out port
 * @param[in] wave out port handle
 */
typedef void (*MDSIM_CALLBACK_FUNC_SND_CLOSE)(SNDHANDLE sh);
/** 
 * @brief sent pcm data to wave out port
 * @param[in] 	wave out port handle
 * @param[in] 	pcm data. (11025 sample rate/stero/16bits little-endian)
 * @param[in] 	pcm data size in byte
 */
typedef void (*MDSIM_CALLBACK_FUNC_SND_OUTPUT)(SNDHANDLE sh, void *sample, int size);

/** 
 * @brief load srm data of current game. every game should have a srm data file
 * @param[in] 	srm data buffer
 * @param[in] 	size of srm data to be read from file(in byte)
 * @return int 		actual data size read from file
 */
typedef int (*MDSIM_CALLBACK_FUNC_LOAD_SRM_DATA)(void *data, int size);
/** 
 * @brief save srm data of current game. every game should have a srm data file
 * @param[in] 	srm data buffer
 * @param[in] 	size of srm data to be writen to file(in byte)
 * @return int 		actual data size writen to file
 */
typedef int (*MDSIM_CALLBACK_FUNC_SAVE_SRM_DATA)(void *data, int size);

typedef struct _tagMDSIMCallbackInfo
{
	//memory management
	MDSIM_CALLBACK_FUNC_MALLOC		malloc;
	MDSIM_CALLBACK_FUNC_FREE		free;

	//file operation
	MDSIM_CALLBACK_FUNC_FREAD		fread;
	MDSIM_CALLBACK_FUNC_FWRITE         fwrite;
	MDSIM_CALLBACK_FUNC_FSEEK		fseek;
	MDSIM_CALLBACK_FUNC_FTELL		ftell;

	//display interface
	MDSIM_CALLBACK_FUNC_DISP  		display;

	//sound interface
	MDSIM_CALLBACK_FUNC_SND_OPEN	sndOpen;
	MDSIM_CALLBACK_FUNC_SND_CLOSE 	sndClose;
	MDSIM_CALLBACK_FUNC_SND_OUTPUT sndOutput;

	//game srm data management
	MDSIM_CALLBACK_FUNC_LOAD_SRM_DATA loadSrmData;
	MDSIM_CALLBACK_FUNC_SAVE_SRM_DATA saveSrmData;
} MDSIM_CALLBACK_INFO, *LPMDSIM_CALLBACK_INFO;

// result code
typedef enum {
	MDSIM_RET_OK = 0,
	MDSIM_RET_INPUT_ERROR,
	MDSIM_RET_FILE_ERROR,
	MDSIM_RET_ROM_UNSUPPORT_ERROR,
	MDSIM_RET_OUT_OF_MEM__ERROR,
	MDSIM_RET_MMC_UNSUPPORT_ERROR,
	MDSIM_RET_MACHINE_INIT_ERROR,
	MDSIM_RET_PARAMETER_ERROR
}MDSIM_RET;

// button code
typedef enum {
	MDSIM_BT_P1_A = 0,
	MDSIM_BT_P1_B,
	MDSIM_BT_P1_C,
	MDSIM_BT_P1_X,
	MDSIM_BT_P1_Y,	
	MDSIM_BT_P1_Z,
	MDSIM_BT_P1_MODE,
	MDSIM_BT_P1_START,
	MDSIM_BT_P1_UP,
	MDSIM_BT_P1_DOWN,
	MDSIM_BT_P1_LEFT,
	MDSIM_BT_P1_RIGHT,
	MDSIM_BT_P2_A,
	MDSIM_BT_P2_B,
	MDSIM_BT_P2_C,
	MDSIM_BT_P2_X,
	MDSIM_BT_P2_Y,
	MDSIM_BT_P2_Z,
	MDSIM_BT_P2_MODE,
	MDSIM_BT_P2_START,
	MDSIM_BT_P2_UP,
	MDSIM_BT_P2_DOWN,
	MDSIM_BT_P2_LEFT,
	MDSIM_BT_P2_RIGHT
}MDSIM_BT_ID;

/** 
 * @brief Load rom data from MD file and initialize the MD Emulator 
 * @param[in] 	file name
 * @return MDSIM_RET
 */
MDSIM_RET MDSIM_Init(MDSIM_CALLBACK_INFO *cb, FILEHANDLE fh);

/** 
 * @brief excute one frame instruction and display one frame
 * this function should be called 60 times every second
 * @return void
 */
void MDSIM_Run(void);

/** 
 * @brief exit MDSIM
 * @return void
 */
void MDSIM_Exit(void);

/** 
 * @brief pause MDSIM
 * @return void
 */
void MDSIM_Pause(void);

/** 
 * @brief resume MDSIM
 * @return void
 */
void MDSIM_Resume(void);


/** 
 * @brief set skip frame number while MDSIM runing. With more skipnum, MDSIM will 
 * @display less frame and run faster. default is 5.
 * @param[in] 	frame number to skip between two display frame, should be (1-10)
 * @return MDSIM_RET MDSIM_RET_OK or MDSIM_RET_PARAMETER_ERROR
 */
MDSIM_RET MDSIM_SetFrameSkip(int skipnum);

/** 
 * @brief turn on/off snes audio. default is on. 
 * @NOTE: DO NOT TURN ON/OFF AUDIO WHILE PLAYING GAME!
 * @param[in] 0: off; 1: on
 * @return MDSIM_RET MDSIM_RET_OK or MDSIM_RET_PARAMETER_ERROR
 */
MDSIM_RET MDSIM_AudioOn(int flag);

/** 
 * @brief set one key state(up or down)
 * @param[in] button the key id to be set
 * @param[in] state the button state(0 means unpressed, 1 means held)
 * @return void
 */
void MDSIM_SetButton(MDSIM_BT_ID button, int state);

/** 
 * @brief save or load immdiate date of the game 
 * @param[in] save file name
 * @return MDSIM_RET MDSIM_RET_OK or MDSIM_RET_FILE_ERROR
 */
MDSIM_RET MDSIM_SaveImmdiateData(FILEHANDLE fh);

/** 
 * @brief save or load immdiate date of the game 
 * @param[in]  load file name
 * @return MDSIM_RET MDSIM_RET_OK or MDSIM_RET_FILE_ERROR
 */
MDSIM_RET MDSIM_LoadImmdiateData(FILEHANDLE fh);

/** 
 * @brief get version information of mdsim library 
 * @return string about mdsim version information
 */
const char *MDSIM_GetVersionInfo(void);

#ifdef __cplusplus
} // End of extern "C"
#endif

#endif
