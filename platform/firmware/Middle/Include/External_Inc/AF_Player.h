#ifndef __AF_PLAYER_H__
#define __AF_PLAYER_H__


/**
* @FILENAME AF_Player.h
* @BRIEF Flash player lib api  
* Copyright (C) 2012 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR Wang Yanfei
* @DATE 2012-03-20
* @UPDATE 2012-07-07
* @VERSION 1.0.05
* @REF 
*/

/**
 flash库使用说明：
 >> 初始化
    在使用图像库的函数接口之前，必须要按照如下方法进行初始化：
    (1)使用AF_Player_SetChipID()函数设置所使用的芯片类型(参见本文件中的定义)
    (2)使用AF_Player_Init)()函数初始化flash
    示例代码：
	AF_PLAYER_PARAM p;
	int ver;
	unsigned char signature[4] = {0};
	p.vmem = vmem;
	p.w = LCD_WIDTH;
	p.h = LCD_HEIGHT;
	p.bpp = 16;
	p.quality = AF_HIGH_QUALITY;
	p.screen_mode = AF_DISP_FIXEDRATIO;

	p.malloc = AF_Malloc;
	p.realloc = AF_Realloc;
	p.free = AF_Free;

	p.fopen = AF_fopen;
	p.fread=AF_fread;
	p.fwrite = AF_fwrite;
	p.fseek = AF_fseek;
	p.ftell = AF_ftell;
	p.fclose = AF_fclose;
	p.fstat = AF_fstat;

	p.sndOpen = AF_sndOpen;
	p.sndClose = AF_sndClose;
	p.sndOutput = AF_sndOutput;
	p.mp3Open = mp3codec_open;
	p.mp3Close = mp3codec_close;
	p.mp3Fill = mp3codec_fill;
	p.mp3Decode = mp3codec_decode;

	p.sleep = AF_Sleep;
	p.getTicks = AF_GetTickCount;
	p.printf = printf;
	p.paint = AF_paint;

  	p.get_Hashdata = GetHashData;

	p.fsc = fscommand;
	strcpy(p.fontPath, "C:/WINDOWS/Fonts");
	strcpy(p.fontFilename, "msyh.TTF");
	AF_Player_SetChipID(ANYKA_CHIP_980x);
	AF_Player_Init(&p);

 >> 获取库版本号
    使用AF_Player_GetVersionInfo()函数可以获取flash库的版本号，返回值是一个字符串。

 >> 运行过程调用示例
	AF_Player_Start(filename);
	ver = AF_FlashVersion(signature);
	quit_flag = 0;
	while(quit_flag==0)
	{
		AF_Player_Run();
		if (catch_mouse_event)
		{
			switch(event.type)
			{
			case down:
				AF_Player_Mouse_Event(AF_MOUSE_PRESS, 0, 0);
			case up:
				AF_Player_Mouse_Event(AF_MOUSE_RELEASE, 0, 0);
			case move:
				AF_Player_Mouse_Event(AF_MOUSE_MOVE, event.mouse.posx, event.mouse.posy);
			}
		}
		if (catch_key_event)
		{
			AF_Player_Kbd_Event(AF_KBD_EVENT, down/up);
		}
		...
	}
 >> 关闭库
    使用AF_Player_Close()函数关闭flash库,释放内部占用的资源
**/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define AK_FLASH_PLAYER_VERSION		"AF Player V2.0.08(UT)_SVN239"

typedef enum 
{
    	AF_RDONLY	= 0x01,
    	AF_WRONLY 	= 0x02,
    	AF_RDWR		= 0x04,
    	AF_CREAT 	= 0x08,
    	AF_TRUNC	= 0x10,
    	AF_APPEND	= 0x20,
    	AF_NONBLOCK = 0x40
}AF_FILE_OPEN_TYPE;

typedef enum 
{
    	AF_SEEK_SET	= 0x0,
    	AF_SEEK_CUR	= 0x1,
    	AF_SEEK_END	= 0x2
}AF_SEEK_TYPE;

typedef int	AF_FILE_HANDLE;
#define INVALID_FILE_HANDLE	-1;

typedef enum 
{
		AF_KEY_LEFT   		,
		AF_KEY_RIGHT      	,
		AF_KEY_HOME        	,
		AF_KEY_END         		,
		AF_KEY_INSERT      	,
		AF_KEY_DELETE      	,
		AF_KEY_BACKSPACE   	, 
		AF_KEY_ENTER		, 
		AF_KEY_UP		    	,
		AF_KEY_DOWN		,
		AF_KEY_PAGE_UP	   	,
		AF_KEY_PAGE_DOWN 	,
		AF_KEY_TAB			,
		AF_KEY_ESCAPE	 	, 
		AF_KEY_F1			,
		AF_KEY_F2			,
		AF_KEY_F3			,
		AF_KEY_F4			,
		AF_KEY_F5			,
		AF_KEY_F6			,
		AF_KEY_F7			,
		AF_KEY_F8			,
		AF_KEY_F9			,
		AF_KEY_F10			,
		AF_KEY_F11			,
		AF_KEY_F12			,
		AF_KEY_F13			,
		AF_KEY_F14			,
		AF_KEY_F15			,
		AF_KEY_COUNT
}AF_KBD_EVENT;

typedef enum
{
	AF_MOUSE_MOVE,
	AF_MOUSE_PRESS,
	AF_MOUSE_RELEASE
}AF_MOUSE_EVENT;

typedef enum
{
	AF_LOW_QUALITY = 0,
	AF_MEDIUM_QUALITY = 1,
	AF_HIGH_QUALITY = 2,
	AF_QUALITY_MASK = 3
} AF_QUALITY;

typedef enum
{
	AF_DISP_FIXEDRATIO = 0,
	AF_DISP_EXACTFIT,
} AF_SCREENMODE;

typedef enum
{
	ANYKA_CHIP_980x,
	ANYKA_CHIP_37xx,
	ANYKA_CHIP_37xx_L,
	ANYKA_CHIP_ALL
} AF_ChipID;

//memory management interface
typedef void*  (*AF_CBFUNC_MALLOC)(unsigned int size);
typedef void   (*AF_CBFUNC_FREE)(void *ptr);
typedef void*  (*AF_CBFUNC_REALLOC)(void *ptr, unsigned int size);

//File system interface
typedef AF_FILE_HANDLE   (*AF_CBFUNC_FOPEN)(const char *pathname, AF_FILE_OPEN_TYPE flags);
typedef int	   (*AF_CBFUNC_FREAD)(AF_FILE_HANDLE fh, void *buf, unsigned int size);
typedef int	   (*AF_CBFUNC_FWRITE)(AF_FILE_HANDLE fh, void *buf, unsigned int size);
typedef int    (*AF_CBFUNC_FSEEK)(AF_FILE_HANDLE fh, long offset, AF_SEEK_TYPE whence);
typedef long   (*AF_CBFUNC_FTELL)(AF_FILE_HANDLE fh);
typedef int    (*AF_CBFUNC_FCLOSE)(AF_FILE_HANDLE fh);
typedef int (*AF_CBFUNC_FSTAT)(const char * file_name, short int *st_dev,unsigned short int *st_ino, unsigned short int *st_mode,short int *st_nlink, unsigned short int *st_uid, unsigned short int *st_gid, short int *st_rdev, long int *st_size, long int *st_atime_1, long int *st_mtime_1, long int *st_ctime_1);

typedef int    (*AF_CBFUNC_SND_OPEN)(int freq, int channel, int bits);
typedef void (*AF_CBFUNC_SND_CLOSE)(void);
typedef int (*AF_CBFUNC_SND_OUTPUT)(void *sample, int size);

typedef int    (*AF_CBFUNC_SLEEP)(int ms);
typedef long    (*AF_CBFUNC_GETTICKS)(void);

typedef int    (*AF_CBFUNC_PRINTF)(const char *format, ...);
typedef void   (*AF_CBFUNC_PAINT)(void* imgBuf, int x, int y, int w, int h);

typedef void    (*AF_CBFUNC_FSC)(const char *format);

typedef int    (*AF_CBFUNC_MP3DEC_OPEN)(int freq, int channel, int bits);
typedef void (*AF_CBFUNC_MP3DEC_CLOSE)(void);
typedef int (*AF_CBFUNC_MP3DEC_FILL)(void *src_data, int size);
typedef int (*AF_CBFUNC_MP3DEC_DECODE)(void *pcm, int size);

typedef int (*AF_CBFUNC_GET_HASHDATA)(unsigned char *data, unsigned char *hashData,const unsigned char keyid); 

typedef struct _AF_PLAYER_PARAM
{
	void *vmem;
	int w;
	int h;
	int bpp;
	AF_QUALITY  quality;
	AF_SCREENMODE screen_mode;
	
	AF_CBFUNC_MALLOC malloc;
	AF_CBFUNC_FREE free;
	AF_CBFUNC_REALLOC realloc;

	//File system interface
	AF_CBFUNC_FOPEN		fopen;
	AF_CBFUNC_FREAD		fread;
	AF_CBFUNC_FWRITE	fwrite;
	AF_CBFUNC_FSEEK		fseek;
	AF_CBFUNC_FTELL		ftell;
	AF_CBFUNC_FCLOSE	fclose;
	AF_CBFUNC_FSTAT		fstat;

	AF_CBFUNC_SND_OPEN	sndOpen;
	AF_CBFUNC_SND_CLOSE	sndClose;
	AF_CBFUNC_SND_OUTPUT	sndOutput;

	AF_CBFUNC_MP3DEC_OPEN	mp3Open;
	AF_CBFUNC_MP3DEC_CLOSE	mp3Close;
	AF_CBFUNC_MP3DEC_FILL	mp3Fill;
	AF_CBFUNC_MP3DEC_DECODE	mp3Decode;

	AF_CBFUNC_SLEEP		sleep;
	AF_CBFUNC_GETTICKS	getTicks;
	AF_CBFUNC_PRINTF	printf;
	AF_CBFUNC_PAINT		paint;

	AF_CBFUNC_GET_HASHDATA get_Hashdata;

	AF_CBFUNC_FSC	fsc;
	char fontPath[64];
	char fontFilename[64];
}AF_PLAYER_PARAM;


/*API*/
/** 
 * @brief get the version information of Flash Player 
 * @param[in] 	void
 * @return const char*: version information string
 */
const char* AF_Player_GetVersionInfo(void);

/** 
 * @brief initialize the flash player 
 * @param[in] 	AF_PLAYER_PARAM
 * @return 1: success; 0: fail
 */
int AF_Player_Init(AF_PLAYER_PARAM *param);

/** 
 * @brief check the platform chip id
 * @param[in] 	id
 * @return 1: success; 0: fail
 */
int AF_Player_SetChipID(AF_ChipID id);

/** 
 * @brief close the flash player 
 * @param[in] 	void
 * @return 1: success; 0: fail
 */
int AF_Player_Close(void);

/** 
 * @brief start to play the flash
 * @param[in] Url: flash file path and name
 * @return 1: success; 0: fail
 */
int AF_Player_Start(char *Url);

/** 
 * @brief get flash file's signature and  Vervion
 * @param[in] signature: CWS or FWS, size always 3
 * @return >0: FlashVervion; 0: fail
 */
int AF_FlashVersion(unsigned char*signature);


/** 
 * @brief player run funcation, should be called periodically
 * @param[in] void
 * @return 1: success; 0: fail
 */
int AF_Player_Run(void);

/** 
 * @brief send the keybroad evnet to the player
 * @param[in] AF_KBD_EVENT
 * @param[in] isKeyDown: 1=key down		0=key up
 * @return 1: success; 0: fail
 */
int AF_Player_Kbd_Event(AF_KBD_EVENT key, int isKeyDown);

/** 
 * @brief send mouse evnet to the player
 * @param[in] mouse_event: AF_MOUSE_MOVE/AF_MOUSE_PRESS/AF_MOUSE_RELEASE
 * @param[in] xpos: x position of cursor
 * @param[in] ypos: y position of cursor
 * @return 1: success; 0: fail
 */
int AF_Player_Mouse_Event(AF_MOUSE_EVENT mouse_event,int xpos,int ypos);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif //__AF_PLAYER_H__

