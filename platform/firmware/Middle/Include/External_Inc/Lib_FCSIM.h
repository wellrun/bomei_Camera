/*
 * @FILENAME lib_FCSIM.h
 * @BRIEF This file declare the structure/type/function provided by FCSIM library
 * Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
 * @AUTHOR Wang Yanfei
 * @DATE 2009-10-23
 * @VERSION 0.3.3
 */

/*
 * This file declare the functions called in the FCSIM library. Any  
 * system wish to using FCSIM library should define those function at
 * first. 
 */

/*	example:
 *
 *	if(FCSIM_Init(fh)!=FCSIM_RET_OK)
 *		return;
 *
 *	while(exit_flag)
 *	{
 *		sleep(17);
 *		FCSIM_Run();	//excute FCSIM_Run 60 times every second
 *	}
 *
 *	FCSIM_Exit();
 *
 */

#ifndef	__FCSIM_H__
#define __FCSIM_H__

#define FCSIM_LIB_VERSION	"FCSIM mini Lib V0.2.2"

// result code
typedef enum {
	FCSIM_RET_OK = 0,
	FCSIM_RET_INPUT_ERROR,
	FCSIM_RET_FILE_ERROR,
	FCSIM_RET_ROM_UNSUPPORT_ERROR,
	FCSIM_RET_OUT_OF_MEM__ERROR,
	FCSIM_RET_MMC_UNSUPPORT_ERROR,
	FCSIM_RET_MACHINE_INIT_ERROR,
	FCSIM_RET_PARAMETER_ERROR
}FCSIM_RET;

// button code
typedef enum {
	FCSIM_BT_P1_A = 0,
	FCSIM_BT_P1_B,
	FCSIM_BT_P1_SELECT,
	FCSIM_BT_P1_START,
	FCSIM_BT_P1_UP,
	FCSIM_BT_P1_DOWN,
	FCSIM_BT_P1_LEFT,
	FCSIM_BT_P1_RIGHT,
	FCSIM_BT_P2_A,
	FCSIM_BT_P2_B,
	FCSIM_BT_P2_SELECT,
	FCSIM_BT_P2_START,
	FCSIM_BT_P2_UP,
	FCSIM_BT_P2_DOWN,
	FCSIM_BT_P2_LEFT,
	FCSIM_BT_P2_RIGHT
}FCSIM_BT_ID;

typedef void*	FCFILEHANDLE;
typedef	void*	SNDHANDLE;

#define FC_SEEK_SET 0
#define FC_SEEK_CUR 1
#define FC_SEEK_END 2

// Callback Function
/** 
 * @brief file system Management. read data from file 
 * @param[in] 	file descriptor
 * @param[in] 	buffer which the data write to
 * @param[in] 	data size to be read from file (in byte)
 * @return int	the data write to the buffer
 */
typedef int (*FC_CALLBACK_FUNC_FREAD)(FCFILEHANDLE fp, void* buf, int length);
/** 
 * @brief file system Management. write data to file 
 * @param[in] 	file descriptor
 * @param[in] 	buffer which the data write to
 * @param[in] 	data size to be write to file (in byte)
 * @return int	the data write to the file
 */
typedef int (*FC_CALLBACK_FUNC_FWRITE)(FCFILEHANDLE fp, void* buf, int length);
/** 
 * @brief file system Management. move the file pointer to a specified position
 * @param[in] 	file descriptor
 * @param[in] 	number of bytes from whence
 * @param[in] 	initial position
 * @return 0 if ok, otherwise return nonzero value
 */
typedef int (*FC_CALLBACK_FUNC_FSEEK)(FCFILEHANDLE fp, long offset, int whence);
/** 
 * @brief file system Management. gets the current position of a file pointer
 * @param[in] 	file descriptor
 * @return int	current position
 */
typedef int (*FC_CALLBACK_FUNC_FTELL)(FCFILEHANDLE fp);

/** 
 * @brief Memory Management. allocate a memory block with specified size (in byte) 
 * @param[in] size	size of the memory block
 * @return null(zero) if failed; otherwise, return a pointer to the memory
 */
typedef void* (*FC_CALLBACK_FUNC_MALLOC)(unsigned int size);
/** 
 * @brief Memory Management. free a memory block 
 * @param[in] 	pointer to the memory block to be released
 * @return void
 */
typedef void (*FC_CALLBACK_FUNC_FREE)(void* mem);

/** 
 * @brief display RGB565 image data on the screen
 * @param[in] 	RGB565 image data
 * @param[in] 	image width in pixel
 * @param[in] 	image line
 * @return NULL
 */
typedef void (*FC_CALLBACK_FUNC_DISP)(void* imgBuf, unsigned short imgWidth, unsigned short lineNum);

/** 
 * @brief open wave out port
 * @return wave out port handle, used in wave output interface
 */
typedef SNDHANDLE (*FC_CALLBACK_FUNC_SND_OPEN)(void);
/** 
 * @brief close wave out port
 * @param[in] wave out port handle
 */
typedef void (*FC_CALLBACK_FUNC_SND_CLOSE)(SNDHANDLE sh);
/** 
 * @brief sent pcm data to wave out port
 * @param[in] 	wave out port handle
 * @param[in] 	pcm data. 
 * @param[in] 	pcm data size in byte
 */
typedef void (*FC_CALLBACK_FUNC_SND_OUTPUT)(SNDHANDLE sh, void *sample, int size);

/** 
 * @brief load srm data of current game. every game should have a srm data file
 * @param[in] 	srm data buffer
 * @param[in] 	size of srm data to be read from file(in byte)
 * @return int 		actual data size read from file
 */
typedef int (*FC_CALLBACK_FUNC_LOAD_SRM_DATA)(void *data, int size);
/** 
 * @brief save srm data of current game. every game should have a srm data file
 * @param[in] 	srm data buffer
 * @param[in] 	size of srm data to be writen to file(in byte)
 * @return int 		actual data size writen to file
 */
typedef int (*FC_CALLBACK_FUNC_SAVE_SRM_DATA)(void *data, int size);

typedef struct _tagFCCallbackInfo
{
	//memory management
	FC_CALLBACK_FUNC_MALLOC		malloc;
	FC_CALLBACK_FUNC_FREE		free;

	//file operation
	FC_CALLBACK_FUNC_FREAD		fread;
	FC_CALLBACK_FUNC_FWRITE		fwrite;
	FC_CALLBACK_FUNC_FSEEK		fseek;
	FC_CALLBACK_FUNC_FTELL		ftell;

	//display interface
	FC_CALLBACK_FUNC_DISP  		display;

	//sound interface
	FC_CALLBACK_FUNC_SND_OPEN	sndOpen;
	FC_CALLBACK_FUNC_SND_CLOSE	sndClose;
	FC_CALLBACK_FUNC_SND_OUTPUT	sndOutput;

	//game srm data management
	FC_CALLBACK_FUNC_LOAD_SRM_DATA loadSrmData;
	FC_CALLBACK_FUNC_SAVE_SRM_DATA saveSrmData;
} FC_CALLBACK_INFO, *LPFC_CALLBACK_INFO;

/** 
 * @brief Load rom data from nes file and initialize the FSSIM 
 * @param[in] 	callback function
 * @param[in] 	file descriptor
 * @return FCSIM_RET
 */
FCSIM_RET FCSIM_Init(FC_CALLBACK_INFO *cb, FCFILEHANDLE fh);

/** 
 * @brief set skip frame number while FCSIM runing. With more skipnum, FCSIM will 
 * @display less frame and run faster. default is 5.
 * @param[in] 	frame number to skip between two display frame, should be (1-10)
 * @return FCSIM_RET FCSIM_RET_OK or FCSIM_RET_PARAMETER_ERROR
 */
FCSIM_RET FCSIM_SetFrameSkip(int skipnum);

/** 
 * @brief excute one frame instruction and display one frame
 *			this function should be called 60 times every second
 * @return void
 */
void FCSIM_Run (void);

/** 
 * @brief set one key state(up or down)
 * @param[in] button the key id to be set
 * @param[in] state the button state(0 means unpressed, 1 means held)
 * @return void
 */
void FCSIM_SetButton(FCSIM_BT_ID button, int state);

/** 
 * @brief exit FCSIM
 * @return void
 */
void FCSIM_Exit (void);

#endif