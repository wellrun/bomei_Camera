/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.    
 *  
 * File Name：vinvme.c
 * Function： The entry of MMI PC version 
 *
 * Author：Miao Baoli
 * Date：2004-04-06
 * Version：1.0          
 *
 * Reversion: ZhuSizhe
 * Author: 2006-08-29
 * Date: 
**************************************************************************/
#ifdef OS_WIN32

#include <stdio.h>
#include <windows.h>
#include <WINDOWSX.H>
#include <direct.h>
#include <assert.h>
#include <string.h>
#include "winvme.h"
#include "anyka_types.h"
#include "fwl_pfdisplay.h"
#include "comport.h"
#include "vme_engine.h"
#include "res/resource.h"
#include "Gbl_Global.h"

/*****************************************************************************
 * defines
 *****************************************************************************
*/

#define MAX_MSG_BUFFER  1024

#define PCDEMO_WIDTH    266
#define PCDEMO_HEIGHT    658

#define MAX_LCD_NUM    1

#define    DEBUG_WINDOW_NAME    "Debug Info"

static int m_WindowWidth = 0;
static int m_WindowHeight = 0;


CHAR *szTitle[MAX_LCD_NUM] = 
{
    "PC SIM MMI",
};

CHAR *szWindowClass[MAX_LCD_NUM] = 
{
    "PC_SIM",
};



// *** windows user messages
#define WM_USER_SCHEDULE_VME        WM_USER+1
#define WM_USER_INIT_VME            WM_USER+2

extern T_BOOL keypad_interrupt_handler_WIN32(T_U8 MouseState, T_U16 x, T_U16 y);
extern T_BOOL vtimer_interrupt_handler_WIN32(T_S32 timer_id);


/*****************************************************************************
 * globals
 *****************************************************************************
*/
HINSTANCE    hApplInstance = NULL; 

// *** LCD
HWND        hwndLCD[MAX_LCD_NUM];
HDC            hdcLCD[MAX_LCD_NUM];
HBITMAP        hbitmapSkinBmp;


BYTE LCD0Memory[LCD0_WIDTH*LCD0_HEIGHT*3];

BYTE LCD1Memory[LCD0_WIDTH*LCD0_HEIGHT*3];


RECT  LCD0Rect = {0};

BOOL  bLCD0_On = FALSE;
// BOOL  bLCD1_On = FALSE;


// *** keypad 
typedef struct {
    RECT    field;
    WORD    ScanCode;    
    
} Key_t;

#define MAX_KEY_NUM         25

#define SCAN_0_3            (0x0018)
#define SCAN_0_2            (0x0014)
#define SCAN_0_1            (0x0012)          
#define SCAN_0_0            (0x0011)

#define SCAN_1_3            (0x0028)
#define SCAN_1_2            (0x0024)
#define SCAN_1_1            (0x0022)
#define SCAN_1_0            (0x0021)

#define SCAN_2_3            (0x0048)
#define SCAN_2_2            (0x0044)
#define SCAN_2_1            (0x0042)
#define SCAN_2_0            (0x0041)

#define SCAN_3_3            (0x0088)
#define SCAN_3_2            (0x0084)
#define SCAN_3_1            (0x0082)
#define SCAN_3_0            (0x0081)

#define SCAN_4_3            (0x0108)
#define SCAN_4_2            (0x0104)
#define SCAN_4_1            (0x0102)
#define SCAN_4_0            (0x0101)

#define SCAN_5_3            (0x0208)
#define SCAN_5_2            (0x0204)
#define SCAN_5_1            (0x0202)
#define SCAN_5_0            (0x0201)


Key_t rectKeyRect[MAX_KEY_NUM] = {
{  82+27, 630-21,  45,  23, SCAN_4_1},    //Key 0
{  31+27, 540-21,  45,  23, SCAN_1_2},    //Key 1
{  82+27, 540-21,  45,  23, SCAN_1_1},    //Key 2
{ 133+27, 540-21,  45,  23, SCAN_1_0},    //Key 3
{  31+27, 570-21,  45,  23, SCAN_2_2},    //Key 4
{  82+27, 570-21,  45,  23, SCAN_2_1},    //Key 5
{ 133+27, 570-21,  45,  23, SCAN_2_0},    //Key 6
{  31+27, 600-21,  45,  23, SCAN_3_2},    //Key 7
{  82+27, 600-21,  45,  23, SCAN_3_1},    //Key 8
{ 133+27, 600-21,  45,  23, SCAN_3_0},    //Key 9
{  31+27, 630-21,  45,  23, SCAN_4_2},    //Key Star *
{ 133+27, 630-21,  45,  23, SCAN_4_0},    //Key Pond #
{  80+27, 422-21,  48,  27, SCAN_0_3},    //Up Arrow
{  80+27, 479-21,  48,  27, SCAN_1_3},    //Down Arrow
{  54+27, 444-21,  25,  40, SCAN_3_3},    //Left Arrow
{ 129+27, 444-21,  25,  40, SCAN_2_3},    //Right Arrow
{  82+27, 451-21,  45,  26, SCAN_4_3},    //Menu M
{  29+27, 498-21,  41,  32, SCAN_5_3},    //Call 
{ 136+27, 498-21,  41,  32, SCAN_5_0},    //HangUp, Switch on/off
{  29+27, 411-21,  40,  30, SCAN_5_1},    //Soft Key 1 -> left,  Cancel
{ 141+27, 411-21,  40,  30, SCAN_5_2},    //Soft Key 2 -> right, Menu
{  84+27, 515-21,  40,  20, SCAN_0_1},    //clr -> OK
{  14+27, 415-21,   8,  16, SCAN_0_2},    //Side Key 2
{  14+27, 435-21,   8,  16, SCAN_0_0},            //Side Key 3
{   0,   0,   0,   0, }                //Side Key 4
};


/*****************************************************************************
 * functions
 *****************************************************************************
*/


/*----------------------------------------------------------------    
 * brief :     com port and display initialization
 * author:    
 * param:    
 * retval:    return 0
 * history:    
 -----------------------------------------------------------------*/
int winvme_InitGlobals (void)
{
//     comport_Init ();
//     _mkdir(".\\TraceInfo");
    /**
    * init display
    */
    LCD0Rect.left = PC_LCD_LEFT + (320-MAIN_LCD_WIDTH)/2;
    LCD0Rect.top = PC_LCD_TOP + (240-MAIN_LCD_HEIGHT)/2;
    //LCD0Rect.left = PC_LCD_LEFT + (480-MAIN_LCD_WIDTH)/2;
    //LCD0Rect.top = PC_LCD_TOP + (272-MAIN_LCD_HEIGHT)/2;
    LCD0Rect.right = LCD0Rect.left + LCD0_WIDTH-1;
    LCD0Rect.bottom = LCD0Rect.top + LCD0_HEIGHT-1;
    
    memset (LCD0Memory, 0xFF, sizeof (LCD0Memory));
    winvme_DisplayOff (DISPLAY_LCD_0);


    return 0;
}


/*----------------------------------------------------------------    
 * brief :     set window shape
 * author:    
 * param:       the handle of a window    
 * param:       the handle to graphics object
 * retval:    return 0
 * history:    
 -----------------------------------------------------------------*/
static int winvme_SetWindowShape (HWND  hWnd,HGDIOBJ hBitMap)
{
  
    HDC         hDC;
    HDC         hBmpDC;
    RECT        stRect;
    BITMAP      stBmp;
    DWORD       dwX,dwY,dwStartX;
    HRGN        hRgn;
    COLORREF    rgbBack;
    COLORREF    rgbTmp;
    DWORD       Vecx;
    HRGN        Heax;
    DWORD       Veax;

    GetObject(hBitMap,sizeof(BITMAP),&stBmp);
    GetWindowRect(hWnd,&stRect);
    ShowWindow(hWnd,SW_HIDE);
    MoveWindow(hWnd,stRect.left,stRect.top,stBmp.bmWidth,stBmp.bmHeight,FALSE);

    hDC = GetDC(hWnd);
    hBmpDC = CreateCompatibleDC(hDC);
    SelectObject(hBmpDC,hBitMap);
    
//*************** calculate window shape ***************************************
    rgbBack = GetPixel(hBmpDC,0,0);
    hRgn = CreateRectRgn(0,0,0,0);

    dwY = 0;
    while  (1)
    {
        dwX = 0;
        dwStartX = -1;
        if (dwY == 250)
            dwY = 250;
        while (1)
        {
            rgbTmp = GetPixel(hBmpDC, dwX, dwY);
            if  (dwStartX == -1)
            {
                if  (rgbTmp !=  rgbBack)
                    dwStartX = dwX;
                }
            else if  (rgbTmp ==  rgbBack)
            {
                Vecx =  dwY;
                Vecx ++;
                Heax = CreateRectRgn(dwStartX, dwY, dwX,Vecx);
                CombineRgn(hRgn,hRgn,Heax,RGN_OR);
                DeleteObject(Heax);
                dwStartX = -1;
            }
            else if (dwX >=  (DWORD)stBmp.bmWidth - 1)
            {
                  Vecx =  dwY;
                  Vecx ++;
                  Heax = CreateRectRgn(dwStartX,dwY,dwX,Vecx);
                  CombineRgn(hRgn,hRgn,Heax,RGN_OR);
                  DeleteObject(Heax);
                  dwStartX = -1;
             }
            dwX ++;
            Veax = dwX;
            if (Veax >=  (DWORD)stBmp.bmWidth)
                break;
        }
        dwY ++;
        Veax = dwY;
        if (Veax >=  (DWORD)stBmp.bmHeight)
            break;
    }

    SetWindowRgn(hWnd, hRgn,TRUE);
//********************************************************************
    BitBlt(hDC,0,0, stBmp.bmWidth, stBmp.bmHeight,hBmpDC,0,0,SRCCOPY);
    DeleteDC(hBmpDC);
    ReleaseDC(hWnd, hDC);
    InvalidateRect(hWnd,NULL,-1);

    return 0; 
}

/*----------------------------------------------------------------    
 * brief :     Refresh the LCD screen
 * author:          
 * param:    uiLCDNum: num of the lcd(DISPLAY_LCD_0: The main LCD, which is color screen; 
                          LCD_1: The second LCD, which is black and white screen)
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
extern int LCD_REFRESH_RECT[4];
//struct ________winvme_RefreshLCD{ int a ;  };
static void ReverseColor()
{
    int i=0;
    BYTE temp;
    for(i=0;i<LCD0_WIDTH*LCD0_HEIGHT*3;i=i+3)
    {
        temp=LCD0Memory[i+2];
        LCD0Memory[i+2]=LCD0Memory[i+0];
        LCD0Memory[i+0]=temp;
    }
    
}
static void winvme_RefreshLCD (unsigned int uiLCDNum)
{
//     int mod =   1    ;
//     
//     if( mod )
//     {
//         static BITMAPINFO bi;
//         if (bi.bmiHeader.biSize == 0) {
//             bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
//             bi.bmiHeader.biPlanes = 1;
//             bi.bmiHeader.biBitCount = 24;
//             bi.bmiHeader.biCompression = BI_RGB;
//             bi.bmiHeader.biClrUsed = 0;
//             bi.bmiHeader.biSizeImage = 0;
//             bi.bmiHeader.biXPelsPerMeter = 72;
//             bi.bmiHeader.biYPelsPerMeter = 72;
//             bi.bmiHeader.biClrUsed = 0;
//             bi.bmiHeader.biClrImportant = 0;
//             bi.bmiHeader.biWidth = 320;
//             bi.bmiHeader.biHeight= -240;
//         }
//         
//         ReverseColor();
//         SetDIBitsToDevice( hdcLCD[uiLCDNum] , LCD0Rect.left,0/*LCD0Rect.top*/, LCD0Rect.right - LCD0Rect.left, LCD0Rect.bottom - LCD0Rect.top,     0,     0,     0,     320,LCD0Memory,     &bi,     DIB_RGB_COLORS);
//         return ;
//     }
//     else
//     {    
// 
//         int T,L;
//         int Left= LCD_REFRESH_RECT[0];
//         int Top = LCD_REFRESH_RECT[1];
//         int Width= LCD_REFRESH_RECT[2];
//         int Height= LCD_REFRESH_RECT[3];
//         
//         for( T = Top ; T<= Top+Height  ; T++ )
//         {
//         
//             for( L=Left ; L<= Left+Width ; L++ )
//             {    
//                 char* p = LCD0Memory+ T*720 + L*3;
//                 SetPixel(hdcLCD[uiLCDNum], L+12 , T , RGB( p[0] , p[1] , p[2] ) );     
//             }
//         }
//     }
// 
//     return;
        
    WORD    i,j;
    BYTE    *pPixelBase;
    BYTE    r,g,b;
    
    // in the first step: only one LCD is supported
    //assert (uiLCDNum == DISPLAY_LCD_0);
    if (uiLCDNum == DISPLAY_LCD_0)
    {
//         if (1)
//         {
//             ReverseColor();
            pPixelBase  = LCD0Memory;//lcd_get_disp_buf_addr(DISPLAY_LCD_0);
            
            for (j = 0; j < Fwl_GetLcdHeight(); j++)
            {
                for (i = 0; i < Fwl_GetLcdWidth(); i++)
                {
                    r = *pPixelBase++;
                    g = *pPixelBase++;
                    b = *pPixelBase++;
                    
                    SetPixel(hdcLCD[uiLCDNum], LCD0Rect.left + i, LCD0Rect.top + j, RGB(r, g, b));
                }
            }
//         }
//         else
//         {
//             for (j = 0; j < Fwl_GetLcdHeight(); j++)
//             {
//                 for (i = 0; i < Fwl_GetLcdWidth(); i++)
//                 {
//                     SetPixel(hdcLCD[uiLCDNum], LCD0Left + i, LCD0Top + j, RGB(0, 0, 0));
//                 }
//             }
//         }
    }
}

/*----------------------------------------------------------------    
 * brief :     Is the point in the rectangle
 * author:      
 * param:    RECT *rect: The position of the rectangle; 
 * param:    LONG x: The x coordinate of a point; 
 * param:    LONG y: The y coordinate of a point; 
 * retval:    The point is in the rectangle,return TRUE; The point 
                is not in the rectangle, return FALSE;
 * history:    
 -----------------------------------------------------------------*/
BOOL winvme_PointInRect(RECT *rect, LONG x, LONG y)
{
    if(x >= rect->left && x <= rect->right &&
       y >= rect->top  && y <= rect->bottom)
       return TRUE;

    return FALSE;
}

/*----------------------------------------------------------------    
 * brief :     initialize the size of the key
 * author:    
 * param:    
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
// static void winvme_InitKeyRect(void)
// {
//     int iKeypadIndex;
// 
//     // correct RECT Structure
//     for (iKeypadIndex = 0; iKeypadIndex < MAX_KEY_NUM; iKeypadIndex++)
//     {
//         rectKeyRect[iKeypadIndex].field.right   +=   rectKeyRect[iKeypadIndex].field.left;
//         rectKeyRect[iKeypadIndex].field.bottom  +=   rectKeyRect[iKeypadIndex].field.top;
//     }
//     return;
// }

// /*----------------------------------------------------------------    
//  * brief :     return the key value of the point 
//  * author:   
//  * param:    int x: The x coordinate of the point; 
//  * param:    int y: The y coordinate of the point;   
//  * param:    DWORD *pdwKeyMatrixCode: return pointer of the key value 
//  * retval:    TRUE: The point is in the rectangle of a key; FALSE: The point 
//                 is not in the rectangle of any key; 
//  * history:    
//  -----------------------------------------------------------------*/
// static BOOL winvme_GetKeyCode (int x, int y, DWORD *pdwKeyMatrixCode)
// {
//     int iKeypadIndex;
// 
//     for (iKeypadIndex = 0; iKeypadIndex < MAX_KEY_NUM; iKeypadIndex++)
//     {
//         if (winvme_PointInRect(&rectKeyRect[iKeypadIndex].field,x,y))
//         {
//             *pdwKeyMatrixCode   = rectKeyRect[iKeypadIndex].ScanCode;
//             return TRUE;
//         }
//     }
//     return FALSE;
// }

/*----------------------------------------------------------------    
 * brief :     return the key value of the point 
 * author:   
 * param:    int x: The x coordinate of the point; 
 * param:    int y: The y coordinate of the point;   
 * param:    DWORD *pdwKeyMatrixCode: return pointer of the key value 
 * retval:    TRUE: The point is in the rectangle of a key; FALSE: The point 
                is not in the rectangle of any key; 
 * history:    
 -----------------------------------------------------------------*/
static BOOL winvme_InTouchScrRect (int x, int y)
{
    return winvme_PointInRect(&LCD0Rect,x,y);
}
#if 00
/*----------------------------------------------------------------    
 * brief :     print the error message which is creat by vme_engine_Init ()
 * author:    
 * param:    enum_vmeeng_ret_t ErrCode: error value
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
static void PrintErrMsg (enum_vmeeng_ret_t ErrCode)
{
    BYTE    MsgBuffer [MAX_MSG_BUFFER];

    switch (ErrCode)
    {
        case VMEENG_ERR_CANNOT_OPEN_V24:
            LoadString (hApplInstance, MSG_VMEENG_ERR_CANNOT_OPEN_V24, MsgBuffer, sizeof (MsgBuffer));
            break;
        case VMEENG_ERR_CANNOT_OPEN_ATC1:
            LoadString (hApplInstance, MSG_VMEENG_ERR_CANNOT_OPEN_ATC1, MsgBuffer, sizeof (MsgBuffer));
            break;
        case VMEENG_ERR_CANNOT_OPEN_ATC2:
            LoadString (hApplInstance, MSG_VMEENG_ERR_CANNOT_OPEN_ATC2, MsgBuffer, sizeof (MsgBuffer));
            break;
        case VMEENG_ERR_CANNOT_OPEN_ATC3:
            LoadString (hApplInstance, MSG_VMEENG_ERR_CANNOT_OPEN_ATC3, MsgBuffer, sizeof (MsgBuffer));
            break;
        default:
            LoadString (hApplInstance, MSG_ERR_UNKNONW, MsgBuffer, sizeof (MsgBuffer));
            break;
    }

//    MessageBox (NULL, MsgBuffer, "Error", MB_OK | MB_ICONERROR);

    strcat(MsgBuffer, "\n\nContinue or not?");
    if (MessageBox (NULL, MsgBuffer, "Continue?", MB_YESNO | MB_ICONQUESTION) == IDNO)
    {
        vme_engine_Term ();
    }
    return;
}
#endif

/*----------------------------------------------------------------    
 * brief :     Post message WM_USER_INIT_VME, the function winvme_MainWndProc 
                response this message and enter the circle of the MMI
 * author:    
 * param:    HWND hWnd: the handle of the window
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
LRESULT MainHandle_WM_CREATE(
    HWND    hWnd, 
    UINT    message, 
    WPARAM    wParam, 
    LPARAM    lParam
)
{
    // start and init vme engine
    PostMessage (hWnd, WM_USER_INIT_VME, 0, 0);
    return 0;    
}

/*----------------------------------------------------------------    
 * brief :     trancate the program when the button down
 * author:    
 * param:       HWND    hWnd: the handle of the window
 * param:       LPARAM    lParam: the parameter of the window, store the orientation menu
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
LRESULT MainHandle_WM_RBUTTONDOWN(
    HWND    hWnd, 
    UINT    message, 
    WPARAM    wParam, 
    LPARAM    lParam
)
{
    HMENU   MainMenu;
    HMENU   PopupMenuSys;
    RECT    WndRect;

    MainMenu        =   LoadMenu (hApplInstance, MAKEINTRESOURCE (IDR_MENU_RBUTTON_POPUP));
    if (NULL != MainMenu)
    {

        PopupMenuSys    =   GetSubMenu (MainMenu, 0);
        if (NULL != PopupMenuSys)
        {

            GetWindowRect (hWnd, &WndRect);

            TrackPopupMenu (
                PopupMenuSys,
                TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                WndRect.left + GET_X_LPARAM(lParam), 
                WndRect.top + GET_Y_LPARAM(lParam), 
                0,
                hWnd,
                NULL
            );
        }
    }
    return 0;
}

/*----------------------------------------------------------------    
 * brief :     run command 
 * author:    
 * param:       HWND    hWnd: the handle of the window
 * param:       WPARAM    wParam: the parameter of the window, store the command
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
LRESULT MainHandle_WM_COMMAND(
    HWND    hWnd, 
    UINT    message, 
    WPARAM    wParam, 
    LPARAM    lParam
)
{
    
    switch (LOWORD (wParam))
    {
        case IDM_CLOSE:
            vme_engine_Term ();
            break;
    }

    return 0;
}

/*----------------------------------------------------------------    
 * brief :     LCD1 message manage
 * author:    
 * param:    HWND    hWnd:  the handle of the window          
 * param:       UINT    message: message ID
 * param:       WPARAM    wParam:  the parameter of the window, store the command
 * param:       LPARAM    lParam:  the parameter of the window, store the orientation menu
 * retval:    return 0
 * history:    
 -----------------------------------------------------------------*/
#include "vme_event.h"

extern vBOOL SendUniqueVMEEvent (vT_EvtCode EvtCode, vT_EvtParam* pEvtParam);

struct ________winvme_MainWndProc{ int a ;  };
LRESULT CALLBACK winvme_MainWndProc(
    HWND    hWnd, 
    UINT    message, 
    WPARAM    wParam, 
    LPARAM    lParam
)
{
    PAINTSTRUCT ps;
    HDC            hDC, hBmpDC;
    HBITMAP        hBitmap;
    WORD        x, y;
    WORD        winx, winy;
    switch(message)
    {
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
        case WM_MOUSEMOVE:
        case WM_LBUTTONUP:
            winx = LOWORD( lParam );
            winy = HIWORD( lParam );
            x = (WORD)(winx - LCD0Rect.left);
            y = (WORD)(winy - LCD0Rect.top);

            break;
        default:
            break;
    }

    switch (message) 
    {
//             case WM_KEYDOWN:
//                 {
//                 vT_EvtParam EvtParam;
//                 EvtParam.c.Param1 = wParam;
//                 switch(wParam)
//                 {
//                     case 38 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 412) ,  kbUP );
//                         break;
//                     case 40 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 412) ,  kbDOWN );
//                         break;
//                     case 37 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 412) ,  kbLEFT );
//                         break;
//                     case 39 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 412) ,  kbRIGHT );
//                         break;
// //                     case 90 :
// //                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 412) ,  kbCALL );
// //                         break;
//                     case 88 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 412) ,  kbSTART_MODULE );
//                         break;
//                     case 67 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 412) ,  kbMENU );
//                         break;
// //                     case 86 :
// //                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 412) ,  kbCANCEL );
// //                         break;
// //                     case 66 :
// //                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 412) ,  kbDONE );
// //                         break;
//                     case 78 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 412) ,  kbCLEAR );
//                         break;
//     
//                 }
//     
//             }
//             break;
//     
//     
//         case WM_KEYUP:
//             {
//                 vT_EvtParam EvtParam;
//                 EvtParam.c.Param1 = wParam;
//                 
//                 switch(wParam)
//                 {
//                     case 38 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 413) ,  kbUP );break;
//                     case 40 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 413) ,  kbDOWN );break;
//                     case 37 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 413) ,  kbLEFT );break;
//                     case 39 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 413) ,  kbRIGHT );break;
// //                     case 90 :
// //                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 413) ,  kbCALL );break;
//                     case 88 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 413) ,  kbSTART_MODULE );break;
//                     case 67 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 413) ,  kbMENU );break;
// //                     case 86 :
// //                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 413) ,  kbCANCEL );break;
// //                     case 66 :
// //                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 413) ,  kbDONE );break;
//                     case 78 :
//                         vme_event_Send_VME_EVT_KEY( (VME_EVT_USER + 413) ,  kbCLEAR );break;
//                 }
//                 
//             }
//             break;
        case WM_CREATE:
            MainHandle_WM_CREATE (hWnd, message, wParam, lParam);
            break;

        case WM_COMMAND:
            MainHandle_WM_COMMAND (hWnd, message, wParam, lParam);
            break;

        case WM_SIZE:
        case WM_MOVE:
        case WM_PAINT:

            hDC = BeginPaint(hWnd, &ps);
            hBmpDC = CreateCompatibleDC( hDC );
            hBitmap = (HBITMAP)SelectObject( hBmpDC, hbitmapSkinBmp );
            BitBlt( hDC, 0, 0, m_WindowWidth, m_WindowHeight, hBmpDC, 0, 0, SRCCOPY );
            DeleteDC( hBmpDC );
            EndPaint(hWnd, &ps);

            winvme_RefreshLCD (DISPLAY_LCD_0);
//             winvme_RefreshLCD (LCD_1);

            break;

        case WM_RBUTTONDOWN:
            {
                MainHandle_WM_RBUTTONDOWN (hWnd, message, wParam, lParam);
            }
            break;

        case WM_LBUTTONDOWN:
            // calculte keymatrix value, dependent on mouse position
            {
//                vUINT32 KeyMatrixCode;
                if(winvme_InTouchScrRect(winx, winy))
                {
                    vme_event_Send_VME_EVT_TOUCHSCR_ACTION(eTOUCHSCR_DOWN, x, y, (vUINT32)GetTickCount());
                }
                else if (keypad_interrupt_handler_WIN32(0, LOWORD(lParam), HIWORD(lParam)))
                {
                    break;
                }
                else
                {
                    /* for dragging the window */
                    SendMessage(hwndLCD[DISPLAY_LCD_0], WM_NCLBUTTONDOWN,HTCAPTION,0);
                }
            }

            break;
        case WM_LBUTTONUP:
            {
                //                vUINT32 KeyMatrixCode;
                if(winvme_InTouchScrRect(winx, winy))
                {
                    vme_event_Send_VME_EVT_TOUCHSCR_ACTION(eTOUCHSCR_UP, x, y, (vUINT32)GetTickCount());
                }
                else if (keypad_interrupt_handler_WIN32(2, LOWORD(lParam), HIWORD(lParam)))
                {
                    break;
                }
            }
            
            break;
        case WM_MOUSEMOVE:
            {
//                vUINT32 KeyMatrixCode;
                if(winvme_InTouchScrRect(winx, winy))
                {
                    if((wParam & MK_LBUTTON))
                    {
                        vme_event_Send_VME_EVT_TOUCHSCR_ACTION(eTOUCHSCR_MOVE, x, y, (vUINT32)GetTickCount());
                    }
                }
/*
                else if (winvme_GetKeyCode (LOWORD(lParam), HIWORD(lParam), &KeyMatrixCode))
                {
                    if((wParam & MK_LBUTTON))
                        break;

                    if(FALSE == bKeyPressed) 
                        break;                  
                    // save state of virtual keypad
                    bKeyPressed = FALSE;
                }
*/
            }
            break;



        case WM_TIMER:
            vtimer_interrupt_handler_WIN32(wParam);
            break;

        case WM_USER_INIT_VME:
            {
                enum_vmeeng_ret_t   Ret;

                if (VMEENG_OK != (Ret = vme_engine_Init ()))
                {
                    //PrintErrMsg (Ret);
                    vme_engine_Term ();
                }
            }
            break;

        case WM_USER_SCHEDULE_VME:
            vme_engine_HandleEventQueue ();
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}
// #if 0
// /*----------------------------------------------------------------    
//  * brief :     LCD2 message manage
//  * author:    
//  * param:    HWND    hWnd:  the handle of the window          
//  * param:       UINT    message: message ID
//  * param:       WPARAM    wParam:  the parameter of the window, store the command
//  * param:       LPARAM    lParam:  the parameter of the window, store the orientation menu
//  * retval:    return 0
//  * history:    
//  -----------------------------------------------------------------*/
// LRESULT CALLBACK winvme_WndProcLCD2(
//     HWND    hWnd, 
//     UINT    message, 
//     WPARAM    wParam, 
//     LPARAM    lParam
// )
// {
//     switch (message) 
//     {
//         case WM_CREATE:
//             break;
// 
//         case WM_LBUTTONDOWN:
//             SendMessage(hwndLCD[LCD_1], WM_NCLBUTTONDOWN,HTCAPTION,0);
//             break;
// 
//         case WM_DESTROY:
//             PostQuitMessage(0);
//             break;
//         //case WM_PAINT:
//             //winvme_RefreshLCD (LCD_1);
// 
//             //break;
//         default:
//             return DefWindowProc(hWnd, message, wParam, lParam);
//    }
//    return 0;
// }
// #endif

/*----------------------------------------------------------------    
 * brief :     Register Class
 * author:    
 * param:    HINSTANCE hInstance: the handle of a application  function
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
static ATOM winvme_RegisterClass(
    HINSTANCE    hInstance
)
{
    WNDCLASSEX    wcex;
    ATOM        aTom;


    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)winvme_MainWndProc;
    wcex.cbClsExtra        = 0;
    wcex.cbWndExtra        = 0;
    wcex.hInstance        = hInstance;
    wcex.hIcon            = NULL; // LoadIcon(hInstance, (LPCTSTR)PCDEMO_ICON);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName    = NULL;
    wcex.lpszClassName    = szWindowClass[DISPLAY_LCD_0];
    wcex.hIconSm        = NULL; // LoadIcon(wcex.hInstance, (LPCTSTR)PCDEMO_ICON);

    aTom =  RegisterClassEx(&wcex);


//     wcex.style            = CS_HREDRAW | CS_VREDRAW;
//     wcex.lpfnWndProc    = (WNDPROC)winvme_WndProcLCD2;
//     wcex.cbClsExtra        = 0;
//     wcex.cbWndExtra        = 0;
//     wcex.hInstance        = hInstance;
//     wcex.hIcon            = NULL; // LoadIcon(hInstance, (LPCTSTR)PCDEMO_ICON);
//     wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
//     wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
//     wcex.lpszMenuName    = NULL;
//     wcex.lpszClassName    = szWindowClass[LCD_1];
//     wcex.hIconSm        = NULL; // LoadIcon(wcex.hInstance, (LPCTSTR)PCDEMO_ICON);

    //aTom =  RegisterClassEx(&wcex);   //modified by ygm 20060910

    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)winvme_DebugWindow;
    wcex.cbClsExtra        = 0;
    wcex.cbWndExtra        = 0;
    wcex.hInstance        = hInstance;
    wcex.hIcon            = NULL; // LoadIcon(hInstance, (LPCTSTR)PCDEMO_ICON);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName    = MAKEINTRESOURCE(IDR_MENU_WINDEBUG);//NULL;
    wcex.lpszClassName    = DEBUG_WINDOW_NAME;
    wcex.hIconSm        = NULL; // LoadIcon(wcex.hInstance, (LPCTSTR)PCDEMO_ICON);

    aTom =  RegisterClassEx(&wcex);

    return aTom;
}

/*----------------------------------------------------------------    
 * brief :     the application function initialization, create the DISPLAY_LCD_0 and LCD_1 window
 * author:    
 * param:       HINSTANCE hInstance: the handle of the application function
 * param:       int nCmdShow:
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
static BOOL winvme_InitInstance(
    HINSTANCE    hInstance, 
    int            nCmdShow
)
{
    HWND        hWnd[MAX_LCD_NUM];
    RECT        clientRect;
    BITMAP      stBmp;
    HWND        hDebugWnd;
/* Create Debug Window */
    hDebugWnd = CreateWindow(DEBUG_WINDOW_NAME, DEBUG_WINDOW_NAME,
                  WS_OVERLAPPEDWINDOW,
                  CW_USEDEFAULT, CW_USEDEFAULT,
                  WINDOW_XSIZE,WINDOW_YSIZE,
                  NULL, NULL, hInstance, NULL);
    
    if (hDebugWnd == NULL)
        return FALSE;

    ShowWindow(hDebugWnd, nCmdShow);
    UpdateWindow(hDebugWnd);
/* Create Debug Window end */
    
    hWnd[DISPLAY_LCD_0] =    CreateWindow(
                        szWindowClass[DISPLAY_LCD_0], 
                        szTitle[DISPLAY_LCD_0],  
                        WS_POPUP,  //| WS_BORDER,
                        CW_USEDEFAULT, 
                        0, 
                        CW_USEDEFAULT, 
                        0, 
                        NULL, 
                        NULL, 
                        hInstance, 
                        NULL
                    );
    if (!hWnd[DISPLAY_LCD_0])
    {
        return FALSE;
    }

    clientRect.left        = ( GetSystemMetrics( SM_CXSCREEN ) - PCDEMO_WIDTH ) / 2;
    clientRect.top        = ( GetSystemMetrics( SM_CYSCREEN ) - PCDEMO_HEIGHT ) / 2;
    clientRect.right    = clientRect.left + PCDEMO_WIDTH;
    clientRect.bottom    = clientRect.top + PCDEMO_HEIGHT;

    SetWindowPos(
        hWnd[DISPLAY_LCD_0], 
        HWND_TOP, 
        clientRect.left, 
        clientRect.top,
        clientRect.right - clientRect.left, 
        clientRect.bottom - clientRect.top, 
        SWP_NOZORDER|SWP_NOACTIVATE 
    );

    hwndLCD[DISPLAY_LCD_0]    = hWnd[DISPLAY_LCD_0];
    hdcLCD[DISPLAY_LCD_0]    = GetDC( hWnd[DISPLAY_LCD_0] );

    hbitmapSkinBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP_VOICEPHONE)); 
    GetObject(hbitmapSkinBmp,sizeof(BITMAP),&stBmp);
    m_WindowWidth = stBmp.bmWidth;
    m_WindowHeight = stBmp.bmHeight;
    
    clientRect.left     = ( GetSystemMetrics( SM_CXSCREEN ) - m_WindowWidth ) / 2;
    clientRect.top      = ( GetSystemMetrics( SM_CYSCREEN ) - m_WindowHeight ) / 2;
    clientRect.right    = clientRect.left + m_WindowWidth;
    clientRect.bottom   = clientRect.top + m_WindowHeight;


    winvme_SetWindowShape(hWnd[DISPLAY_LCD_0],(HGDIOBJ)hbitmapSkinBmp);

    ShowWindow(hWnd[DISPLAY_LCD_0], nCmdShow);
    UpdateWindow(hWnd[DISPLAY_LCD_0]);

/*    if (CUR_LCD_NUM >= 2)
    {
        hWnd[LCD_1]    =    CreateWindow(
                            szWindowClass[LCD_1], 
                            szTitle[LCD_1], 
                            WS_POPUP | WS_BORDER ,
                            CW_USEDEFAULT, 
                            0, 
                            CW_USEDEFAULT, 
                            0, 
                            hwndLCD[DISPLAY_LCD_0], 
                            NULL, 
                            hInstance, 
                            NULL
                        );

        if (!hWnd[LCD_1])
        {
            return FALSE;
        }

        clientRect.left        = ( GetSystemMetrics( SM_CXSCREEN ) - LCD1_WIDTH ) / 2;
        clientRect.top        = clientRect.top+10;
        clientRect.right    = clientRect.left + LCD1_WIDTH;
        clientRect.bottom    = clientRect.top + LCD1_HEIGHT;

        SetWindowPos(
            hWnd[LCD_1], 
            HWND_TOP, 
            clientRect.left, 
            clientRect.top,
            clientRect.right - clientRect.left,
            clientRect.bottom - clientRect.top,
            SWP_NOZORDER|SWP_NOACTIVATE 
        );

        hwndLCD[LCD_1]    = hWnd[LCD_1];
        hdcLCD[LCD_1]    = GetDC( hWnd[LCD_1] );

        ShowWindow(hWnd[LCD_1], nCmdShow);
        UpdateWindow(hWnd[LCD_1]);
    }
*/
    return TRUE;
}

/*----------------------------------------------------------------    
 * brief :     the main function of the programme
 * author:    
 * param:
 * param:
 * param:
 * param:    
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
int APIENTRY WinMain(
    HINSTANCE    hInstance, 
    HINSTANCE    hPrevInstance,
    LPSTR        lpCmdLine,
    int            nCmdShow
)
{
    MSG msg;


    hApplInstance   =   hInstance;

    winvme_InitGlobals ();

//     winvme_InitKeyRect ();

    // Initialize global strings
    winvme_RegisterClass (hInstance);

    // Perform application initialization:
    if (!winvme_InitInstance (hInstance, nCmdShow)) 
    {
        return FALSE;
    }

    while( GetMessage( &msg, NULL, 0, 0 ) )
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
//     winvme_save_configuration();
    ReleaseDC( hwndLCD[DISPLAY_LCD_0], hdcLCD[DISPLAY_LCD_0] );
//     Winvme_CloseTraceInfoFile();
    

/*    if (CUR_LCD_NUM >= 2)
        ReleaseDC( hwndLCD[LCD_1], hdcLCD[LCD_1] );
*/
    DeleteObject(hbitmapSkinBmp);

    return msg.wParam;
}


/*****************************************************************************
 * interface
 *****************************************************************************
*/
/***
* application
*/
/*----------------------------------------------------------------    
 * brief :     post a message WM_USER_SCHEDULE_VME, the winvme_MainWndProc function will call deal with a event
 * author:    
 * param:    
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
T_VOID winvme_ScheduleVMEEngine (T_VOID)
{
//    PostMessage (hwndLCD[DISPLAY_LCD_0], WM_USER_SCHEDULE_VME, 0, 0);
    return;
}

/*----------------------------------------------------------------    
 * brief :     post a message WM_CLOSE
 * author:    
 * param:    
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
T_VOID winvme_CloesAppl (T_VOID)
{
    PostMessage (hwndLCD[DISPLAY_LCD_0], WM_CLOSE, 0, 0);
}

/***
* timer
*/
/*----------------------------------------------------------------    
 * brief :     set a windows timer
 * author:    
 * param:    unsigned int uiTimeOut: time
 * param:       unsigned int uiTimerId: timer ID
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
T_U32 winvme_StartTimer (T_U32 uiTimeOut, T_U32 uiTimerId)
{

    return SetTimer (hwndLCD[DISPLAY_LCD_0], uiTimerId, uiTimeOut, (TIMERPROC) NULL);
    //if (0 == SetTimer (hwndLCD[DISPLAY_LCD_0], uiTimerId, uiTimeOut, (TIMERPROC) NULL))
    //{
    //    dwError = GetLastError ();
    //}


    //return;
}


typedef T_VOID (*T_fVTIMER_CALLBACK)(T_TIMER timer_id, T_U32 delay);




/* ***************************************************************************

   Note by ZMJ
   MFC下面多线程调试时，很容易出现界面线程竞争设备导致断点调试时长时间机器僵死
   所以建议在main_loop函数前调试CMMB.
   所以不能使用系统的Timer函数, 需使用半仿真硬中断的Timer函数，不依赖平台分发
   

*************************************************************************** */
#include <process.h>

unsigned __stdcall osWin_TimerThreader(void *lparam)
{
	MSG  msg;
	unsigned long *hParam = (unsigned long*)lparam;
	TIMERPROC cb = (TIMERPROC)hParam[0];
	T_U32 elapse = hParam[3];
	volatile unsigned long *lflag = (volatile unsigned long*)&hParam[1];

	UINT id = SetTimer(0, 0, elapse, cb);
	
	while((0==*lflag) && GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, 0, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}	

	KillTimer(0, id);
	*lflag = 2;
	_endthreadex(0);
	return 0;
}

unsigned long	 osWin_SetTimer(int elapse, unsigned long _fTimerCallback)
{
	unsigned long *handler = (unsigned long *)malloc(4*sizeof(unsigned long));
	handler[0] = _fTimerCallback;
	handler[1] = 0;
	handler[2] = 0;
	handler[3] = elapse;
	handler[2] = _beginthreadex (0, 0, osWin_TimerThreader, handler, 0, 0);
	return (unsigned long)handler;
}

void osWin_KillTimer(unsigned long handler)
{
	unsigned long *hParam = (unsigned long*)handler;
	volatile unsigned long *lflag = (volatile unsigned long*)&hParam[1];
	
	*lflag = 1;
	while(1 == *lflag)
	{
		Sleep(10);
	}

	free(hParam);
}

//================================================================================================
T_U32  WINOS_SetTimer(T_U32 milli_sec, T_BOOL loop, T_fVTIMER_CALLBACK callback_func)
{
	return osWin_SetTimer(milli_sec, (unsigned long)callback_func);
}

T_BOOL WINOS_KillTimer(T_U32 id)
{
	osWin_KillTimer(id);
	return AK_TRUE;
}
//================================================================================================




/*----------------------------------------------------------------    
 * brief :     stop a windows timer
 * author:    
 * param:    unsigned int uiTimerId: a timer ID
 * param:       
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
T_VOID winvme_StopTimer (T_U32 uiTimerId)
{
    KillTimer (hwndLCD[DISPLAY_LCD_0], uiTimerId);
    return;
}

/***
* display
*/
/*----------------------------------------------------------------    
 * brief :     Display initialization
 * author:    
 * param:    unsigned int uiLCDNum: num of the LCD(DISPLAY_LCD_0, LCD_1)
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
T_VOID winvme_InitDisplay (T_U32 uiLCDNum)
{
    // in the first step: only one LCD is supported
    //assert (uiLCDNum == DISPLAY_LCD_0);

    if (uiLCDNum == DISPLAY_LCD_0)
    {
        memset (LCD0Memory, 0xFF, sizeof (LCD0Memory));
        winvme_UpdateDisplay (uiLCDNum);
    }
//     else if( uiLCDNum == LCD_1)
//     {
//         memset (LCD1Memory, 0xFF, sizeof (LCD1Memory));
//         winvme_UpdateDisplay (uiLCDNum);
//     }
    return;   
}

/*----------------------------------------------------------------    
 * brief :     Set the color of a point in the LCD
 * author:    
 * param:       unsigned int uiLCDNum: num of the LCD
 * param:       unsigned int xPos: The x coordinate of a point
 * param:       unsigned int yPos: The y coordinate of a point
 * param:    unsigned int refColor: The value of the color
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
T_VOID winvme_SetPixel (T_U32 uiLCDNum, T_U32 xPos, T_U32 yPos, T_U32 refColor)
{
    BYTE    *pPixelBase;

    assert (xPos < LCD0_WIDTH);
    assert (yPos < LCD0_HEIGHT);

    // in the first step: only one LCD is supported
    //assert (uiLCDNum == DISPLAY_LCD_0);

    if (uiLCDNum == DISPLAY_LCD_0)
    {
        // set RGB - Information
        pPixelBase          = &LCD0Memory[(yPos * LCD0_WIDTH + xPos) * 3];
        *pPixelBase         = GetRValue (refColor);
        *(pPixelBase + 1)   = GetGValue (refColor);
        *(pPixelBase + 2)   = GetBValue (refColor);
    }
//     else if( uiLCDNum == LCD_1 )
//     {
//         pPixelBase          = &LCD1Memory[(yPos * LCD1_WIDTH + xPos) * 3];
//         *pPixelBase         = GetRValue (refColor);
//         *(pPixelBase + 1)   = GetGValue (refColor);
//         *(pPixelBase + 2)   = GetBValue (refColor);
//     }
    return;
}

/*----------------------------------------------------------------    
 * brief :     Get the color value of a point in the LCD
 * author:    
 * param:       unsigned int uiLCDNum: num of the LCD
 * param:       unsigned int xPos: The x coordinate of a point
 * param:       unsigned int yPos: The y coordinate of a point
 * param:    unsigned int *prefColor: The return pointer of the color value
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
T_VOID winvme_GetPixel (T_U32 uiLCDNum, T_U32 xPos, T_U32 yPos, T_U32 *prefColor)
{
    BYTE    *pPixelBase;

    assert (xPos < LCD0_WIDTH);
    assert (yPos < LCD0_HEIGHT);

    // in the first step: only one LCD is supported
    //assert (uiLCDNum == DISPLAY_LCD_0);

    if (uiLCDNum == DISPLAY_LCD_0)
    {
        // set RGB - Information
        pPixelBase          = &LCD0Memory[(yPos * LCD0_WIDTH + xPos) * 3];
        *prefColor = RGB (*pPixelBase, *(pPixelBase + 1), *(pPixelBase + 2));
    }
//     else if( uiLCDNum == LCD_1 )
//     {
//         pPixelBase          = &LCD1Memory[(yPos * LCD1_WIDTH + xPos) * 3];
//         *prefColor = RGB (*pPixelBase, *(pPixelBase + 1), *(pPixelBase + 2));
//     }

    return;
}

/*----------------------------------------------------------------    
 * brief :     Shut down the LCD
 * author:    
 * param:    unsigned int uiLCDNum: The num of the LCD
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
T_VOID winvme_DisplayOff (T_U32 uiLCDNum)
{
    // in the first step: only one LCD is supported
    //assert (uiLCDNum == DISPLAY_LCD_0);

    if (uiLCDNum == DISPLAY_LCD_0)
    {

        bLCD0_On    =   FALSE;    

        winvme_UpdateDisplay (uiLCDNum);
    }
//     else if( uiLCDNum == LCD_1)
//     {
//         bLCD1_On    =   FALSE;    
//         winvme_UpdateDisplay (uiLCDNum);
//     }
    return;
}

/*----------------------------------------------------------------    
 * brief :     Light up the LCD
 * author:    
 * param:    unsigned int uiLCDNum: The num of the LCD
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
T_VOID winvme_DisplayOn (T_U32 uiLCDNum)
{
    // in the first step: only one LCD is supported
    //assert (uiLCDNum == DISPLAY_LCD_0);

    if (uiLCDNum == DISPLAY_LCD_0)
    {
        bLCD0_On    =   TRUE;    

        winvme_UpdateDisplay (uiLCDNum);
    }
//     else if( uiLCDNum == LCD_1)
//     {
//         bLCD1_On    =   TRUE;    
//         winvme_UpdateDisplay (uiLCDNum);
//     }
    return;
}

/*----------------------------------------------------------------    
 * brief :     Update the LCD display
 * author:    
 * param:    unsigned int uiLCDNum: The num of the LCD
 * retval:    
 * history:    
 -----------------------------------------------------------------*/
T_VOID winvme_UpdateDisplay (T_U32 uiLCDNum)
{
    // in the first step: only one LCD is supported
    //assert (uiLCDNum == DISPLAY_LCD_0);

    if (uiLCDNum == DISPLAY_LCD_0)
    {
        SendMessage(hwndLCD[uiLCDNum], WM_PAINT, 0, 0);
    }
    return;
}

T_VOID print_d( T_U8 l )
{
}

vVOID VME_SwitchOffMobile()
{
}
#endif
