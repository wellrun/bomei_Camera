/*
 * @FILENAME lib_gba_callback.h
 * @BRIEF This file declare the structure/type/function provided by GBA library
 * Copyright (C) 2008 Anyka (Guangzhou) Software Technology Co., LTD
 * @AUTHOR wang_yanfei
 * @DATE 2008-12-08
 * @VERSION 1.0.1
 */
 
#ifndef _GBA_CALLBACK_H
#define _GBA_CALLBACK_H

#define GBA_SEEK_SET 0
#define GBA_SEEK_CUR 1
#define GBA_SEEK_END 2

typedef void*  (*GBA_CALLBACK_FUNC_MALLOC)(unsigned int size);
typedef void   (*GBA_CALLBACK_FUNC_FREE)(void *memblock);
typedef size_t (*GBA_CALLBACK_FUNC_FREAD)(void *buf, size_t size, size_t count, void *fp);
typedef size_t (*GBA_CALLBACK_FUNC_FWRITE)(const void *buf, size_t size, size_t count, void *fp);
typedef int    (*GBA_CALLBACK_FUNC_FSEEK)(void *fp, long offset, int whence);
typedef long   (*GBA_CALLBACK_FUNC_FTELL)(void *fp);
typedef int    (*GBA_CALLBACK_FUNC_PRINTF)(const char *format, ...);
typedef void   (*GBA_CALLBACK_FUNC_REFRESH_SCREEN)(void* imgBuf, int imgWidth, int imgHeight);
typedef void (*GBA_CALLBACK_FUNC_SND_START)(void);
typedef void (*GBA_CALLBACK_FUNC_SND_STOP)(void);
typedef int (*GBA_CALLBACK_FUNC_SND_OUTPUT)(signed short *sample, size_t size);
typedef long (*GBA_CALLBACK_FUNC_LOAD_SRM_DATA)(void *data, signed long size);
typedef long (*GBA_CALLBACK_FUNC_SAVE_SRM_DATA)(void *data, signed long size);

typedef struct _GBA_CALLBACK_FUNCS
{
	//	For standard memory support
	GBA_CALLBACK_FUNC_MALLOC	malloc;
	GBA_CALLBACK_FUNC_FREE		free;

	GBA_CALLBACK_FUNC_FREAD  fread;
	GBA_CALLBACK_FUNC_FWRITE fwrite;
	GBA_CALLBACK_FUNC_FSEEK  fseek;
	GBA_CALLBACK_FUNC_FTELL  ftell;

	GBA_CALLBACK_FUNC_PRINTF	printf;
	GBA_CALLBACK_FUNC_REFRESH_SCREEN  pf_refresh_screen;

	GBA_CALLBACK_FUNC_SND_START snd_start;
	GBA_CALLBACK_FUNC_SND_STOP snd_stop;
	GBA_CALLBACK_FUNC_SND_OUTPUT snd_output;

	GBA_CALLBACK_FUNC_LOAD_SRM_DATA LoadSrmData;
	GBA_CALLBACK_FUNC_SAVE_SRM_DATA SaveSrmData;
	
} GBA_CALLBACK_FUNCS;

#endif
