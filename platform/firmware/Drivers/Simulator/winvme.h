/*****************************************************************************
 *   Project: 
 *****************************************************************************
 * $Workfile: $
 * $Revision: $
 *     $Date: $
 *****************************************************************************
 * Description:
 *
 *****************************************************************************
*/
#ifdef OS_WIN32

#ifndef _WINVME_H
#define _WINVME_H

//#include "gbl_global.h"
#include "anyka_types.h"
#include <windows.h>  //add by ygm
#include "stdio.h"

#define PC_LCD_LEFT     151
#define PC_LCD_TOP      37

#define	EDIT_SCREEN_X		320
#define	EDIT_SCREEN_Y		240
#define	SCREEN_X			(EDIT_SCREEN_X + 2 + 16 + 2)
#define	SCREEN_Y			(EDIT_SCREEN_Y + 2 + 16 + 2)
#define	WINDOW_XSIZE		(SCREEN_X + 8)
#define	WINDOW_YSIZE		(SCREEN_Y + 8 + 38) 
/**
* application
*/

// addd by ye guangming to setting print or not somw message
typedef  struct {
	T_BOOL isFwlPrintf;
// 	T_BOOL isDegTrace;
// 	T_BOOL isVom0;
// 	T_BOOL isVcom1;
// 	T_BOOL isVcom2;
// 	T_BOOL isVatc;

}printSelectOrNot;

 
//this message is setting for print realtime ,to ensure what message to print
typedef struct _WinDebug
{	
	CHOOSECOLOR    Fwl_printf_choosecolor;  //setting for Fwl_printf , to make clear this message
	CHOOSEFONT     Fwl_printf_choosefont;

	
	CHOOSECOLOR    Deg_trace_choosecolor;  //setting for Deg_trace , to make clear this message
	CHOOSEFONT     Deg_trace_choosefont;

	
	CHOOSECOLOR    vcom0_choosecolor;  //setting for vcom0 , to make clear this message
	CHOOSEFONT     vcom0_choosefont;

	
	CHOOSECOLOR    vcom1_choosecolor;  //setting for vcom1 , to make clear this message
	CHOOSEFONT     vcom1_choosefont;

	
	CHOOSECOLOR    vcom2_choosecolor;  //setting for vcom2 , to make clear this message
	CHOOSEFONT     vcom2_choosefont;

	
	CHOOSECOLOR    vatc_choosecolor;  //setting for vatc , to make clear this message
	CHOOSEFONT     vatc_choosefont;

	volatile    printSelectOrNot   printSelect;
	volatile    T_BOOL             print_keydown;
}WinDebug;

/* debug message ,want to print message */
// design for debug window program yeguangming
typedef enum{
	ePrintf,
	eMaxType
}DebugOutoutType;
//end for debug window program yeguangming
T_VOID winvme_ScheduleVMEEngine (T_VOID);
T_VOID winvme_CloesAppl (T_VOID);


// T_VOID Winvme_WriteTraceInfoFile(const char * data);
// T_VOID Winvme_CloseTraceInfoFile(void);
// T_VOID Winvme_OpenTraceInfoFile(void);


/**
* timer
*/
T_U32 winvme_StartTimer (T_U32 uiTimeOut, T_U32 uiTimerId);
T_VOID winvme_StopTimer (T_U32 uiTimerId);

/**
* display
*/
T_VOID winvme_InitDisplay (T_U32 uiLCDNum);
T_VOID winvme_SetPixel (T_U32 uiLCDNum, T_U32 xPos, T_U32 yPos, T_U32 refColor);
T_VOID winvme_GetPixel (T_U32 uiLCDNum, T_U32 xPos, T_U32 yPos, T_U32 *prefColor);
T_VOID winvme_DisplayOff (T_U32 uiLCDNum);
T_VOID winvme_DisplayOn (T_U32 uiLCDNum);
T_VOID winvme_UpdateDisplay (T_U32 uiLCDNum);


// this function is added by ygm 20060910
/* initial and load some relatively message for setting and configuration*/
T_VOID winvme_InitialConfiguration(T_VOID);

/* save and load relative configuration ,to ensure next time get previous message*/
// T_VOID winvme_save_configuration(T_VOID);
T_VOID winvme_read_configuration(T_VOID);
/* to get default configuration ,to ensure initial this configuration*/

T_BOOL Winvme_GetPrintDegStatus(T_VOID);
T_BOOL Winvme_GetPrintFwlStatus(T_VOID);
T_BOOL Winvme_GetPrintVcom0Status(T_VOID);
T_BOOL Winvme_GetPrintVcom1Status(T_VOID);
T_BOOL Winvme_GetPrintVcom2Status(T_VOID);
T_BOOL Winvme_GetPrintVatcStatus(T_VOID);

WinDebug* winvme_get_default_configuration(T_VOID);
T_VOID Winvme_SetCurrentPrintType(DebugOutoutType printtype);
LRESULT CALLBACK winvme_DebugWindow(
	HWND	hWnd, 
	UINT	message, 
	WPARAM	wParam, 
	LPARAM	lParam
);
/*  windows message main loop, to execute corresponding program */
int EB_Printf(const char *fmt, ...);
T_VOID winvme_save_configuration(T_VOID);

//T_VOID PutToWindow(DebugOutoutType type, T_pSTR logstr);
#endif // _WINVME_H
#endif
