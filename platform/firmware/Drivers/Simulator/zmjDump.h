/************************************************************************************
* Copyright(c) 2005
* All rights reserved.
*
* File	:	debugwnd.h
* Brief :	
* 
* 
* Version : 1.0
* Author  : ZhangMuJun
* Modify  : 
* Data    : 2006-12-4
*************************************************************************************/
#ifdef OS_WIN32
#ifndef __DEBUG_WND_H__
#define __DEBUG_WND_H__

#ifdef __cplusplus
extern "C" {
#endif

	
// ----------- load dump tool
int     LoadDump();
int     FreeDump();
int		KillHistroyDump(int bKill);


// ------------ printer
int		InitConsolePrinter(int LLD, int bStreamFile);
void	ExitConsolePrinter(int LLD);
int		ConsolePrint(int LLD, const char *format, ...);


// ------------ color printer
typedef enum tagFRONTCOLOR
{
	FRONT_BLACK = 0,
	FRONT_BLUE  = 0x1,
	FRONT_GREEN = 0x2,
	FRONT_RED   = 0x4,
	FRONT_WHITE = 0x7,
}FRONTCOLOR;

typedef enum tagBACKCOLOR
{
	BACK_BLACK = 0,
	BACK_BLUE  = 0x10,
	BACK_GREEN = 0x20,
	BACK_RED   = 0x40,
	BACK_WHITE = 0x70,
}BACKCOLOR;

int		ConsolePrint_Color(int LLD, FRONTCOLOR frontColor, BACKCOLOR bkColor, const char *format, ...);


#ifdef __cplusplus
}
#endif


#endif
#endif