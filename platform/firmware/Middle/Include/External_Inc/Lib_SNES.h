/*
 * @FILENAME lib_SNES.h
 * @BRIEF This file declare the structure/type/function provided by SNES library
 * Copyright (C) 2009 Anyka (Guangzhou) Software Technology Co., LTD
 * @AUTHOR Wang Yanfei
 * @DATE 2009-12-9
 * @VERSION 0.3.3
 */
#ifndef	__SNES_H__
#define __SNES_H__

#define SNES_LIB_VERSION	"SNES_lib V1.1.00"

typedef	void* FILEHANDLE;
typedef	void* SNDHANDLE;

#define SNES_SEEK_SET 0
#define SNES_SEEK_CUR 1
#define SNES_SEEK_END 2

// Callback Function
/** 
 * @brief file system Management. read data from file 
 * @param[in] 	file descriptor
 * @param[in] 	buffer which the data write to
 * @param[in] 	data size to be read from file (in byte)
 * @return int	the data write to the buffer
 */
typedef int (*SNES_CALLBACK_FUNC_FREAD)(FILEHANDLE fp, void* buf, int length);
/** 
 * @brief file system Management. write data to file 
 * @param[in] 	file descriptor
 * @param[in] 	buffer which the data write to
 * @param[in] 	data size to be write to file (in byte)
 * @return int	the data write to the file
 */
typedef int (*SNES_CALLBACK_FUNC_FWRITE)(FILEHANDLE fp, void* buf, int length);
/** 
 * @brief file system Management. move the file pointer to a specified position
 * @param[in] 	file descriptor
 * @param[in] 	number of bytes from whence
 * @param[in] 	initial position
 * @return 0 if ok, otherwise return nonzero value
 */
typedef int (*SNES_CALLBACK_FUNC_FSEEK)(FILEHANDLE fp, long offset, int whence);
/** 
 * @brief file system Management. gets the current position of a file pointer
 * @param[in] 	file descriptor
 * @return int	current position
 */
typedef int (*SNES_CALLBACK_FUNC_FTELL)(FILEHANDLE fp);

/** 
 * @brief Memory Management. allocate a memory block with specified size (in byte) 
 * @param[in] size	size of the memory block
 * @return null(zero) if failed; otherwise, return a pointer to the memory
 */
typedef void* (*SNES_CALLBACK_FUNC_MALLOC)(unsigned int size);
/** 
 * @brief Memory Management. free a memory block 
 * @param[in] 	pointer to the memory block to be released
 * @return void
 */
typedef void (*SNES_CALLBACK_FUNC_FREE)(void* mem);

/** 
 * @brief display RGB565 image data on the screen
 * @param[in] 	RGB565 image data
 * @param[in] 	image width in pixel
 * @param[in] 	image height in pixel
 * @return NULL
 */
typedef void (*SNES_CALLBACK_FUNC_DISP)(void* imgBuf, int imgWidth, int imgHeight);

/** 
 * @brief open wave out port
 * @return wave out port handle, used in wave output interface
 */
typedef SNDHANDLE (*SNES_CALLBACK_FUNC_SND_OPEN)(void);

/** 
 * @brief close wave out port
 * @param[in] wave out port handle
 */
typedef void (*SNES_CALLBACK_FUNC_SND_CLOSE)(SNDHANDLE sh);

/** 
 * @brief sent pcm data to wave out port
 * @param[in] 	wave out port handle
 * @param[in] 	pcm data. (8K sample rate/mono/16bits little-endian)
 * @param[in] 	pcm data size in byte
 */
typedef void (*SNES_CALLBACK_FUNC_SND_OUTPUT)(SNDHANDLE sh, void *sample, int size);

/** 
 * @brief sent pcm data to wave out port
 * @param[in] 	wave out port handle
 * @return Streamed Data in bytes (Mono, 16-bits)
 */
typedef int (*SNES_CALLBACK_FUNC_SND_GETSTREAMEDSIZE)(SNDHANDLE sh);

/** 
 * @brief load srm data of current game. every game should have a srm data file
 * @param[in] 	srm data buffer
 * @param[in] 	size of srm data to be read from file(in byte)
 * @return int 		actual data size read from file
 */
typedef int (*SNES_CALLBACK_FUNC_LOAD_SRM_DATA)(void *data, int size);

/** 
 * @brief save srm data of current game. every game should have a srm data file
 * @param[in] 	srm data buffer
 * @param[in] 	size of srm data to be writen to file(in byte)
 * @return int 		actual data size writen to file
 */
typedef int (*SNES_CALLBACK_FUNC_SAVE_SRM_DATA)(void *data, int size);

typedef struct _tagSNESCallbackInfo
{
	//memory management
	SNES_CALLBACK_FUNC_MALLOC		malloc;
	SNES_CALLBACK_FUNC_FREE		free;

	//file operation
	SNES_CALLBACK_FUNC_FREAD		fread;
	SNES_CALLBACK_FUNC_FWRITE         fwrite;
	SNES_CALLBACK_FUNC_FSEEK		fseek;
	SNES_CALLBACK_FUNC_FTELL		ftell;

	//display interface
	SNES_CALLBACK_FUNC_DISP  		display;

	//sound interface
	SNES_CALLBACK_FUNC_SND_OPEN	sndOpen;
	SNES_CALLBACK_FUNC_SND_CLOSE 	sndClose;
	SNES_CALLBACK_FUNC_SND_OUTPUT sndOutput;
	SNES_CALLBACK_FUNC_SND_GETSTREAMEDSIZE sndGetStreamedSize;
	

	//game srm data management
	SNES_CALLBACK_FUNC_LOAD_SRM_DATA loadSrmData;
	SNES_CALLBACK_FUNC_SAVE_SRM_DATA saveSrmData;
} SNES_CALLBACK_INFO, *LPSNES_CALLBACK_INFO;

// result code
typedef enum {
	SNES_RET_OK = 0,
	SNES_RET_INPUT_ERROR,
	SNES_RET_FILE_ERROR,
	SNES_RET_ROM_UNSUPPORT_ERROR,
	SNES_RET_OUT_OF_MEM__ERROR,
	SNES_RET_MMC_UNSUPPORT_ERROR,
	SNES_RET_MACHINE_INIT_ERROR,
	SNES_RET_PARAMETER_ERROR
}SNES_RET;

// button code
typedef enum {
	SNES_BT_P1_A = 0,
	SNES_BT_P1_B,
	SNES_BT_P1_X,
	SNES_BT_P1_Y,
	SNES_BT_P1_L,
	SNES_BT_P1_R,
	SNES_BT_P1_SELECT,
	SNES_BT_P1_START,
	SNES_BT_P1_UP,
	SNES_BT_P1_DOWN,
	SNES_BT_P1_LEFT,
	SNES_BT_P1_RIGHT,
	SNES_BT_P2_A,
	SNES_BT_P2_B,
	SNES_BT_P2_X,
	SNES_BT_P2_Y,
	SNES_BT_P2_L,
	SNES_BT_P2_R,
	SNES_BT_P2_SELECT,
	SNES_BT_P2_START,
	SNES_BT_P2_UP,
	SNES_BT_P2_DOWN,
	SNES_BT_P2_LEFT,
	SNES_BT_P2_RIGHT
}SNES_BT_ID;

/** 
 * @brief Load rom data from SNES file and initialize the SNES Emulator 
 * @param[in] 	file name
 * @return SNES_RET
 */
SNES_RET SNES_Init(SNES_CALLBACK_INFO *cb, FILEHANDLE fh);

/** 
 * @brief excute one frame instruction and display one frame
 * this function should be called 60 times every second
 * @return void
 */
void SNES_Run(void);

/** 
 * @brief exit SNES
 * @return void
 */
void SNES_Exit(void);

/** 
 * @brief pause SNES
 * @return void
 */
void SNES_Pause(void);

/** 
 * @brief resume SNES
 * @return void
 */
void SNES_Resume(void);


/** 
 * @brief set skip frame number while SNES runing. With more skipnum, SNES will 
 * @display less frame and run faster. default is 5.
 * @param[in] 	frame number to skip between two display frame, should be (1-10)
 * @return SNES_RET SNES_RET_OK or SNES_RET_PARAMETER_ERROR
 */
SNES_RET SNES_SetFrameSkip(int skipnum);

/** 
 * @brief turn on/off snes audio. default is on. 
 * @NOTE: DO NOT TURN ON/OFF AUDIO WHILE PLAYING GAME!
 * @param[in] 0: off; 1: on
 * @return SNES_RET SNES_RET_OK or SNES_RET_PARAMETER_ERROR
 */
SNES_RET SNES_AudioOn(int flag);

/** 
 * @brief set one key state(up or down)
 * @param[in] button the key id to be set
 * @param[in] state the button state(0 means unpressed, 1 means held)
 * @return void
 */
void SNES_SetButton(SNES_BT_ID button, int state);

/** 
 * @brief save or load immdiate date of the game 
 * @param[in] save file name
 * @return SNES_RET SNES_RET_OK or SNES_RET_FILE_ERROR
 */
SNES_RET SNES_SaveImmdiateData(FILEHANDLE fh);

/** 
 * @brief save or load immdiate date of the game 
 * @param[in]  load file name
 * @return SNES_RET SNES_RET_OK or SNES_RET_FILE_ERROR
 */
SNES_RET SNES_LoadImmdiateData(FILEHANDLE fh);

/** 
 * @brief get version information of SNES library 
 * @return string about SNES version information
 */
const char* SNES_GetVersionInfo(T_VOID);

#endif
