#ifdef OS_WIN32

#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <commctrl.h>
#include <TCHAR.h>
#include <richedit.h> 
#include <winvme.h>
#include "res/resource.h"
#include "anyka_types.h"
#include "gbl_global.h"
#include "eng_debug.h"


#define	KEY_BACKSPACE		0x08
#define	STRING_LEN			4096  

#define	EDITID				1
#define	MAX_EDIT_BUF_SIZE	(0x7FFE)
#define	EDIT_BUF_SIZE		(0x6000)   
#define	EDIT_BUF_DEC_SIZE	(0x2000)

#define NTXFONTHEIGHT(PointSize, hDC) (-MulDiv((PointSize), GetDeviceCaps(hDC, LOGPIXELSY), 72)) //add by ygm 

#define	printFilePath	".\\TraceInfo\\"
#define	PrintFileNameSuffix 	".txt" 


static COLORREF customcolors[16];
////////////////
///////////////////////
static HWND		  m_hWnd_DebugWnd;
static HWND             hWnd_PrintfWnd;
volatile static BOOL 	isUsed= FALSE;
static WinDebug  configInfo;

//new stuff
static WinDebug winDebugMessage;

static LOGFONT logfont;

//extern volatile printSelectOrNot printSelect;
static T_BOOL isLoadDefaultConfiguration; //此变量主要是防止当配置文件破坏或有人删除或移动了文件而去读省缺的配置信息。
static CHARFORMAT cf; //add by ygm
static LOGBRUSH logbrush;
static DebugOutoutType print_type;
static FILE *tracefile = AK_NULL;



static void SetupChooseFont(HWND hwnd,HFONT *cfont,LOGFONT* lfont);
static void winvme_InitDebugWindow(HWND hwnd);
static void SetupChooseColor(CHOOSECOLOR *cc,COLORREF *customclrs,HWND hwnd,COLORREF *init);
static T_VOID MenuFwlPrintf(HWND hWnd );
static T_VOID MenuDegTrace(HWND	hWnd);
static T_VOID MenuVcom0(HWND	hWnd);
static T_VOID MenuVcom1(HWND	hWnd);
static T_VOID MenuVcom2(HWND	hWnd);
static T_VOID MenuVatc(HWND	hWnd);
static T_VOID MenuFwlSetting(HWND	hWnd);
static T_VOID MenuDegSetting(HWND	hWnd);
static T_VOID MenuVcom0Setting(HWND	hWnd);
static T_VOID MenuVcom1Setting(HWND	hWnd);
static T_VOID MenuVcom2Setting(HWND	hWnd);
static T_VOID MenuVatcSetting(HWND	hWnd);
static T_VOID MenuFwlFont(HWND hwnd);
static T_VOID MenuDegFont(HWND hwnd);
static T_VOID MenuVcom0Font(HWND hwnd);
static T_VOID MenuVcom1Font(HWND hwnd);
static T_VOID MenuVcom2Font(HWND hwnd);
static T_VOID MenuVatcFont(HWND hwnd);

static T_VOID MenuHelpAnyka(HWND	hWnd);
static T_VOID MenuAbout(HWND	hWnd);

static T_pSTR winvme_use_time_get_log_filename(unsigned char* filename);
static T_VOID winvme_read_configuration(T_VOID);

// static T_VOID MenuFwlPrintf(HWND hWnd )
// {
// 
// 	HMENU hMenu;     
// 	UINT state;
// 	
// 	hMenu=GetMenu(hWnd);
// 	state = GetMenuState(hMenu,CM_FWL_PRINTF,MF_BYCOMMAND);
// 	if (state & MF_CHECKED)
// 	{
// 		CheckMenuItem(hMenu,CM_FWL_PRINTF,MF_BYCOMMAND|MF_UNCHECKED);
// 		winDebugMessage.printSelect.isFwlPrintf=FALSE;
// 	}
// 	else
// 	{
// 		CheckMenuItem(hMenu,CM_FWL_PRINTF,MF_BYCOMMAND|MF_CHECKED);
// 		winDebugMessage.printSelect.isFwlPrintf=TRUE;
// 	}
// 	
// }
// #if 0
// static T_VOID MenuDegTrace(HWND	hWnd)
// {
// 	HMENU hMenu;     
// 	UINT state;	
// 	hMenu=GetMenu(hWnd);
// 	state = GetMenuState(hMenu,CM_DEG_TRACE,MF_BYCOMMAND);
// 	if (state & MF_CHECKED)
// 	{
// 		CheckMenuItem(hMenu,CM_DEG_TRACE,MF_BYCOMMAND|MF_UNCHECKED);
// 		winDebugMessage.printSelect.isDegTrace=FALSE;
// 	}
// 	else
// 	{
// 		CheckMenuItem(hMenu,CM_DEG_TRACE,MF_BYCOMMAND|MF_CHECKED);
// 		winDebugMessage.printSelect.isDegTrace=TRUE;
// 	}
// 	
// }
// static T_VOID MenuVcom0(HWND	hWnd)
// {
// 	HMENU hMenu;     
// 	UINT state;	
// 	hMenu=GetMenu(hWnd);
// 	state = GetMenuState(hMenu,CM_VCOM0,MF_BYCOMMAND);
// 	if (state & MF_CHECKED)
// 	{
// 		CheckMenuItem(hMenu,CM_VCOM0,MF_BYCOMMAND|MF_UNCHECKED);
// 		winDebugMessage.printSelect.isVom0=FALSE;
// 	}
// 	else
// 	{
// 		CheckMenuItem(hMenu,CM_VCOM0,MF_BYCOMMAND|MF_CHECKED);
// 		winDebugMessage.printSelect.isVom0=TRUE;
// 	}
// 	
// }
// 
// static T_VOID MenuVcom1(HWND	hWnd)
// {
// 	HMENU hMenu;     
// 	UINT state;	
// 	hMenu=GetMenu(hWnd);
// 	state = GetMenuState(hMenu,CM_VCOM1,MF_BYCOMMAND);
// 	if (state & MF_CHECKED)
// 	{
// 		CheckMenuItem(hMenu,CM_VCOM1,MF_BYCOMMAND|MF_UNCHECKED);
// 		winDebugMessage.printSelect.isVcom1=FALSE;
// 	}
// 	else
// 	{
// 		CheckMenuItem(hMenu,CM_VCOM1,MF_BYCOMMAND|MF_CHECKED);
// 		winDebugMessage.printSelect.isVcom1=TRUE;
// 	}
// 	
// }
// static T_VOID MenuVcom2(HWND	hWnd)
// {
// 	HMENU hMenu;     
// 	UINT state;	
// 	hMenu=GetMenu(hWnd);
// 	state = GetMenuState(hMenu,CM_VCOM2,MF_BYCOMMAND);
// 	if (state & MF_CHECKED)
// 	{
// 		CheckMenuItem(hMenu,CM_VCOM2,MF_BYCOMMAND|MF_UNCHECKED);
// 		winDebugMessage.printSelect.isVcom2=FALSE;
// 	}
// 	else
// 	{
// 		CheckMenuItem(hMenu,CM_VCOM2,MF_BYCOMMAND|MF_CHECKED);
// 		winDebugMessage.printSelect.isVcom2=TRUE;
// 	}
// 	
// }
// 
// static T_VOID MenuVatc(HWND	hWnd)
// {
// 	HMENU hMenu;     
// 	UINT state;	
// 	hMenu=GetMenu(hWnd);
// 	state = GetMenuState(hMenu,CM_VATC,MF_BYCOMMAND);
// 	if (state & MF_CHECKED)
// 	{
// 		CheckMenuItem(hMenu,CM_VATC,MF_BYCOMMAND|MF_UNCHECKED);
// 		winDebugMessage.printSelect.isVatc=FALSE;
// 	}
// 	else
// 	{
// 		CheckMenuItem(hMenu,CM_VATC,MF_BYCOMMAND|MF_CHECKED);
// 		winDebugMessage.printSelect.isVatc=TRUE;
// 	}
// 	
// }
// #endif

// static T_VOID MenuFwlSetting(HWND	hWnd)
// {
// 	// winDebugMessage.Fwl_printf_choosecolor= choosecolor; 
// 	if(ChooseColor(&(winDebugMessage.Fwl_printf_choosecolor)))   
// 	{
// 		;
// 	}	
// 	SendMessage(hWnd,WM_PAINT,0,0);
// 	
// 		
// }
// 
// static T_VOID MenuDegSetting(HWND	hWnd)
// {
// //	winDebugMessage.Deg_trace_choosecolor= choosecolor;
// 	if(ChooseColor(&(winDebugMessage.Deg_trace_choosecolor)))
// 	{
// 		;
// 	}			
//      SendMessage(hWnd,WM_PAINT,0,0);
// }
// static T_VOID MenuVcom0Setting(HWND	hWnd)
// {
// //	winDebugMessage.vcom0_choosecolor= choosecolor;
// 	if(ChooseColor(&(winDebugMessage.vcom0_choosecolor)))
// 	{
// 		;
// 	}
// 	SendMessage(hWnd,WM_PAINT,0,0);		
// }
// static T_VOID MenuVcom1Setting(HWND	hWnd)
// {
// //	winDebugMessage.vcom1_choosecolor= choosecolor;
// 	if(ChooseColor(&(winDebugMessage.vcom1_choosecolor)))
// 	{
// 		;
// 	}
// 	SendMessage(hWnd,WM_PAINT,0,0);		
// }
// static T_VOID MenuVcom2Setting(HWND	hWnd)
// {
// //	winDebugMessage.vcom2_choosecolor= choosecolor;
// 	if(ChooseColor(&(winDebugMessage.vcom2_choosecolor)))
// 	{
// 		;
// 	}
// 	SendMessage(hWnd,WM_PAINT,0,0);	
// }
// static T_VOID MenuVatcSetting(HWND	hWnd)
// {
// //	winDebugMessage.vatc_choosecolor= choosecolor;
// 	if(ChooseColor(&(winDebugMessage.vatc_choosecolor)))
// 	{
// 		;
// 	}
// 	SendMessage(hWnd,WM_PAINT,0,0);	
// }
// static T_VOID MenuFwlFont(HWND hwnd)
// {
// //	char testFwlPrintfFont[20];
// 	//winDebugMessage.Fwl_printf_choosefont = choosefont;
// 	if(ChooseFont(&(winDebugMessage.Fwl_printf_choosefont)))//call the dialog for fonts
// 	{
// 	  ;
// 	}
//     //_itoa(winDebugMessage.Fwl_printf_choosefont.iPointSize,testFwlPrintfFont,10);
// 	//MessageBox(hwnd,testFwlPrintfFont,"yeNow",MB_OK);
// 
// } 
// static T_VOID MenuDegFont(HWND hwnd)
// {
// 	//winDebugMessage.Deg_trace_choosefont = choosefont;
// 	if(ChooseFont(&(winDebugMessage.Deg_trace_choosefont)))//call the dialog for fonts
// 	{
// 	   ;
// 	}
// }
// static T_VOID MenuVcom0Font(HWND hwnd)
// {
// 	//winDebugMessage.vcom0_choosefont = choosefont;
// 	if(ChooseFont(&(winDebugMessage.vcom0_choosefont)))//call the dialog for fonts
// 	{
// 	  ;
// 	}
// }
// static T_VOID MenuVcom1Font(HWND hwnd)
// {
// //	winDebugMessage.vcom1_choosefont = choosefont;
// 	if(ChooseFont(&(winDebugMessage.vcom1_choosefont)))//call the dialog for fonts
// 	{
// 	  ;
// 	}
// }
// static T_VOID MenuVcom2Font(HWND hwnd)
// {
// 	//winDebugMessage.vcom2_choosefont = choosefont;
// 	if(ChooseFont(&(winDebugMessage.vcom2_choosefont)))//call the dialog for fonts
// 	{
// 	   ;
// 	}
// }
// static T_VOID MenuVatcFont(HWND hwnd)
// {
// 	//winDebugMessage.vatc_choosefont = choosefont;
// 	if(ChooseFont(&(winDebugMessage.vatc_choosefont)))//call the dialog for fonts
// 	{
// 	   ;		
// 	}
// }
// ///font setting end 
// static T_VOID MenuHelpAnyka(HWND	hWnd)
// {
// 	MessageBox(hWnd,TEXT("the program is designed for ANYKA debug tool\nmy email: ye_guangming@anyka.com\nmy telephone: 358\n"),
// 		
// 		"debug tool for WIN32",MB_OK);
// 	
// 	return;
// }
// static T_VOID MenuAbout(HWND	hWnd)
// {
// 	MessageBox(hWnd,TEXT("the program is designed for ANYKA debug tool\nmy email: ye_guangming@anyka.com\nmy telephone: 358\n"),
// 		
// 		"debug tool for WIN32",MB_OK);
// 
// }

// the program load message from file 
// author :ye guangming
// date:   2006.09.04

// get log file name 

static T_pSTR winvme_use_time_get_log_filename(unsigned char* filename)
{
	SYSTEMTIME st;
	char time_log[21];// for example:20060902101012 // year month day hour minute second
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	
	char buffer_month[2];
	char buffer_day[2];
	char buffer_hour[2];
	char buffer_minute[2];
	char buffer_second[2];
	
	
	GetLocalTime(&st);
	year =st.wYear;
	month =st.wMonth;
	day =st.wDay;
	hour =st.wHour;
	minute =st.wMinute;
	second =st.wSecond;
	_itoa(year,time_log,10);
	_itoa(month,buffer_month,10);
	_itoa(day,buffer_day,10);
	_itoa(hour,buffer_hour,10);
	_itoa(minute,buffer_minute,10);
	_itoa(second,buffer_second,10);
	if(month>=1 && month<10)
	{
		strcat(time_log,"0");
		strcat(time_log,buffer_month);
	}
	else
		strcat(time_log,buffer_month);
	
	
	if(day>=1 && day<10)
	{
		strcat(time_log,"0");
		strcat(time_log,buffer_day);
	}
	else
		strcat(time_log,buffer_day);
	
	if(hour>=0 && hour<10)
	{
		strcat(time_log,"0");
		strcat(time_log,buffer_hour);		
	}
	else
		strcat(time_log,buffer_hour);
	
	if(minute>=0 &&minute<10)
	{
		strcat(time_log,"0");
		strcat(time_log,buffer_minute);
	}
	else
		strcat(time_log,buffer_minute);
	
	if(second>=0 && second<10)	
	{
		strcat(time_log,"0");
		strcat(time_log,buffer_second);
	}
	else
		strcat(time_log,buffer_second);

	strcat(time_log,PrintFileNameSuffix);
	strcpy(filename,printFilePath); // output file name to debug message (time).txt
	strcat(filename,time_log); // output file name to debug message (time).txt
	
	return filename;
}


// read the corresponding configuration from file ,to setting some parameters
static T_VOID winvme_read_configuration(T_VOID)
{
	char Filename[] = "confingInfo.txt";
	FILE *fp;
	
	
	fp = fopen( Filename,"r+t");
	if (NULL == fp )
	{
		isLoadDefaultConfiguration = TRUE;
		return ;
		
	}
	fseek(fp,0L,SEEK_END);
	if ( ftell( fp ) != sizeof( configInfo ))
	{
	
		isLoadDefaultConfiguration = TRUE;//*winvme_get_default_configuration(); // if no configuration or configuration is not correct 
		fclose(fp);
		return;
	}
	fseek(fp,0L,SEEK_SET);
	fread (&configInfo,sizeof(configInfo),1,fp);

	fclose(fp);
	isLoadDefaultConfiguration = FALSE;
	return;
}



LRESULT CALLBACK winvme_DebugWindow(
	HWND	hWnd, 
	UINT	message, 
	WPARAM	wParam, 
	LPARAM	lParam
)
{
    switch (message)
    {
    case WM_CREATE:
        // Create the edit control child window
        m_hWnd_DebugWnd = CreateWindow (TEXT("edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL |
            WS_BORDER | ES_LEFT | ES_MULTILINE | ES_READONLY |
            ES_AUTOVSCROLL,
            0, 0, SCREEN_X, SCREEN_Y,
            hWnd, (HMENU)EDITID, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        //          SetFont(hWnd_DebugWnd);
        SendMessage(m_hWnd_DebugWnd, EM_SETLIMITTEXT, MAX_EDIT_BUF_SIZE, 0L);
        return 0;
    case WM_LBUTTONDOWN:
        //          SendMessage(hwndLCD[LCD_S], WM_NCLBUTTONDOWN,HTCAPTION,0);
        break;
    case WM_DESTROY:
        //          PostQuitMessage(0);
        break;
    case WM_SIZE :
        MoveWindow(m_hWnd_DebugWnd, 0, 0, LOWORD (lParam), HIWORD (lParam), TRUE);
        return 0 ;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
   return 0;
}

// 
// void EB_Printf(char *fmt, ...)
// {
// //    va_list ap;
//     int i, slen, lineIdx;
//     int txtRepStart, txtRepEnd, txtSelEnd;
//     int str2Pt = 0;
//     static int	wasCr = 0; //should be static type.
//     static char string[STRING_LEN+4096]; //margin for '\b'
//     static char string2[STRING_LEN+4096]; //margin for '\n'->'\r\n'
//     static int	prevBSCnt = 0;
// //   char testPrintMessage[20]; // add by ygm
// 
// 
// //    while (isUsed); //EB_Printf can be called multiplely  //KIW
// 
// 	isUsed = TRUE;
// 
//     txtRepStart = SendMessage(m_hWnd_DebugWnd, WM_GETTEXTLENGTH, 0x0, 0x0);
//     txtRepEnd = txtRepStart - 1;
//     
//     //va_start(ap, fmt);
//     // _vsnprintf(string2, STRING_LEN-1, fmt, ap);
//     //va_end(ap);
//     lstrcpy(string2, fmt);
// 
//     string2[STRING_LEN - 1] = '\0';
//     
//     //for better look of BS(backspace) char.,
//     //the BS in the end of the string will be processed next time.
//     for (i = 0; i < prevBSCnt; i++) //process the previous BS char.
// 	{
// 		string[i]='\b';
// 	}
//     string[prevBSCnt] = '\0';
//     lstrcat(string, string2);
//     string2[0] = '\0';
// 
//     slen = lstrlen(string);
//     for (i = 0; i < slen; i++)
// 	{
//     	if (string[slen - i - 1] != '\b')
// 			break;
// 	}
//     
//     prevBSCnt = i; // These BSs will be processed next time.
//     slen = slen - prevBSCnt;
//     
//     if (slen == 0)
//     {
// 		isUsed = FALSE;
// 		return;
//     }
// 
//     for (i = 0; i < slen; i++)
//     {
// 		if ((string[i] == KEY_BACKSPACE))
// 		{
// 			/*
// 			string2[str2Pt++] = KEY_BACKSPACE;txtRepEnd++;
// 			string2[str2Pt++] = ' ';txtRepEnd++;
// 			string2[str2Pt++] = KEY_BACKSPACE;txtRepEnd++;
// 			wasCr = 0;
// 			continue;
// 			*/
// 			if (str2Pt > 0)
// 			{
// 				string2[str2Pt--] = KEY_BACKSPACE;txtRepEnd--;
// 				//string2[str2Pt++] = ' ';txtRepEnd++;
// 				//string2[str2Pt++] = KEY_BACKSPACE;txtRepEnd++;
// 				wasCr = 0;
// 				continue;
// 			}
// 		}
// 
// 		if ((string[i] == '\n'))
// 		{
// 			string2[str2Pt++] = '\r';
// 			txtRepEnd++;
// 			string2[str2Pt++] = '\n';
// 			txtRepEnd++;
// 			wasCr = 0;
// 			continue;
// 		}
// 		if ((string[i] != '\n') && (wasCr == 1))
// 		{
// 			string2[str2Pt++] = '\r';
// 			txtRepEnd++;
// 			string2[str2Pt++] = '\n';
// 			txtRepEnd++;
// 			wasCr=0;
// 		}
// 		if (string[i] == '\r')
// 		{
// 			wasCr = 1;
// 			continue;
// 		}
// 
// 		if (string[i] == '\b')
// 		{
// 			//flush string2
// 			if (str2Pt > 0)
// 			{
// 				string2[--str2Pt] = '\0';
// 				txtRepEnd--;
// 				continue;
// 			}
// 			//str2Pt==0;	    
// 			if(txtRepStart > 0)
// 			{
// 				txtRepStart--;		
// 			}
// 			continue;
// 		}
// 		string2[str2Pt++] = string[i];
// 		txtRepEnd++;
// 		// if (str2Pt > 256-3)	break; //why needed? 2001.1.26
//     }
//     
//     string2[str2Pt] = '\0';    
//     if (str2Pt > 0)
//     {
// 		SendMessage(m_hWnd_DebugWnd,EM_SETSEL,txtRepStart,txtRepEnd+1);
// 	    // add color message to ensure font color is changeing
// 		SendMessage(m_hWnd_DebugWnd, EM_GETCHARFORMAT, 1, (LPARAM)&cf);
// 		cf.cbSize = sizeof(cf);
// 		switch(print_type)
// 		{
// 		case ePrintf:
// 			cf.crTextColor= winDebugMessage.Fwl_printf_choosecolor.rgbResult;//winDebugMessage.Fwl_printf_currentcolor;//RGB(255,0,0);
// 			cf.yHeight=winDebugMessage.Fwl_printf_choosefont.iPointSize*2;
// 			cf.dwEffects = 0;	
// 			strcpy (cf.szFaceName, "Times New Roman");
// 			//cf.dwEffects &= CFE_BOLD;
// 			//cf.dwEffects &=CFE_ITALIC;
// 			
// 			break;
// 
// // 		case eDebugTrace:
// // 			cf.crTextColor= winDebugMessage.Deg_trace_choosecolor.rgbResult;//RGB(0,255,0);
// // 		    cf.yHeight=winDebugMessage.Deg_trace_choosefont.iPointSize*2;
// // 			cf.dwEffects &= CFE_BOLD;
// // 			cf.dwEffects &=CFE_ITALIC;
// // 			break;
// // 
// // 		case eVCom0:
// // 			cf.crTextColor= winDebugMessage.vcom0_choosecolor.rgbResult;//RGB(0,0,255);
// // 		    cf.yHeight=winDebugMessage.vcom0_choosefont.iPointSize*2;
// // 			cf.dwEffects &= CFE_BOLD;
// // 			cf.dwEffects &=CFE_ITALIC;
// // 			break;
// // 
// // 		case eVCom1:
// // 			cf.crTextColor= winDebugMessage.vcom1_choosecolor.rgbResult;//RGB(128,128,0);
// // 		    cf.yHeight=winDebugMessage.vcom1_choosefont.iPointSize*2;
// // 			cf.dwEffects &= CFE_BOLD;
// // 			cf.dwEffects &=CFE_ITALIC;
// // 			break;
// // 
// // 		case eVCom2:
// // 			cf.crTextColor= winDebugMessage.vcom2_choosecolor.rgbResult;//RGB(0,128,128);
// // 		    cf.yHeight=winDebugMessage.vcom2_choosefont.iPointSize*2;
// // 			cf.dwEffects &= CFE_BOLD;
// // 			cf.dwEffects &=CFE_ITALIC;
// // 			break;
// // 
// // 		case eVATC:
// // 			cf.crTextColor= winDebugMessage.vatc_choosecolor.rgbResult;//RGB(128,0,128);
// // 		    cf.yHeight=winDebugMessage.vatc_choosefont.iPointSize*2;
// // 			cf.dwEffects &= CFE_BOLD;
// // 			cf.dwEffects &=CFE_ITALIC;
// // 			break;
// 
// 		default:
//             cf.crTextColor= RGB(192,192,192);
// 			cf.yHeight=140*2;
//             
// 			cf.dwEffects = 0;
// 			//cf.dwEffects &= CFE_BOLD;
// 			//cf.dwEffects &=CFE_ITALIC;
// 			break;
// 			
// 
// 		}	
// 		cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE |
//         CFM_PROTECTED | CFM_STRIKEOUT | CFM_FACE | CFM_SIZE|CFM_COLOR;
// 		
// 		SendMessage(m_hWnd_DebugWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
// 
// 		if (winDebugMessage.print_keydown)
// 		{  
// 			
//            SendMessage(m_hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)string2); 
// 		}
// 		        
//     }
//     else
//     {
// 		if (txtRepStart <= txtRepEnd)
// 		{
// 			SendMessage(m_hWnd_DebugWnd,EM_SETSEL,txtRepStart,txtRepEnd+1);
// 			SendMessage(m_hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)"");         
// 		}
//     }
// 	
//     //If edit buffer is over EDIT_BUF_SIZE,
//     //the size of buffer must be decreased by EDIT_BUF_DEC_SIZE.
//     if (txtRepEnd > EDIT_BUF_SIZE)
//     {
// 		lineIdx=SendMessage(m_hWnd_DebugWnd,EM_LINEFROMCHAR,EDIT_BUF_DEC_SIZE,0x0);
// 		//lineIdx=SendMessage(hWnd_DebugWnd,EM_LINEFROMCHAR,txtRepEnd-txtRepStart+1,0x0); //for debug
// 		txtSelEnd=SendMessage(m_hWnd_DebugWnd,EM_LINEINDEX,lineIdx,0x0)-1;
// 		SendMessage(m_hWnd_DebugWnd,EM_SETSEL,0,txtSelEnd+1);
// 
// 		SendMessage(m_hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)"");
// 		//SendMessage(hWnd_DebugWnd,WM_CLEAR,0,0); //WM_CLEAR doesn't work? Why?
// 		//SendMessage(hWnd_DebugWnd,WM_CUT,0,0); //WM_CUT doesn't work? Why?
// 
// 		//make the end of the text shown.
// 		txtRepEnd=SendMessage(m_hWnd_DebugWnd,WM_GETTEXTLENGTH,0x0,0x0)-1;
// 		SendMessage(m_hWnd_DebugWnd,EM_SETSEL,txtRepEnd+1,txtRepEnd+2); 
// 		SendMessage(m_hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)" ");
// 		SendMessage(m_hWnd_DebugWnd,EM_SETSEL,txtRepEnd+1,txtRepEnd+2); 
// 		SendMessage(m_hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)"");
//     }
// 
//     isUsed = FALSE;
// }

///////////////////////////////////////////////
// add by ygm 20060911 ,to set up color ,to make color change 
static void SetupChooseColor(CHOOSECOLOR *cc,COLORREF *customclrs,HWND hwnd,COLORREF *init)
{
	int ca;
	
	
	for(ca = 0;ca<16;ca++){*(customclrs+ca) = RGB(255,255,255);}
	cc->lStructSize = sizeof(CHOOSECOLOR);
	cc->hwndOwner = hwnd;
	cc->rgbResult = *init;
	cc->lpCustColors = customclrs;
	cc->Flags = CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT;
}

void SetupChooseFont(HWND hwnd,HFONT *cfont,LOGFONT* lfont)
{
	winDebugMessage.Fwl_printf_choosefont.lStructSize = sizeof(CHOOSEFONT);
	winDebugMessage.Fwl_printf_choosefont.hwndOwner = hwnd;
	winDebugMessage.Fwl_printf_choosefont.lpLogFont = lfont;
	winDebugMessage.Fwl_printf_choosefont.rgbColors = RGB(0,0,0);
	winDebugMessage.Fwl_printf_choosefont.Flags = CF_FORCEFONTEXIST|CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT;
	*cfont = CreateFont(NTXFONTHEIGHT(10,GetDC(hwnd)),0,0,0,FW_REGULAR,0,0,0,ANSI_CHARSET,0,0,PROOF_QUALITY,DEFAULT_PITCH,"Tahoma");
    //////////////////////////////////////////
	winDebugMessage.Deg_trace_choosefont.lStructSize = sizeof(CHOOSEFONT);
	winDebugMessage.Deg_trace_choosefont.hwndOwner = hwnd;
	winDebugMessage.Deg_trace_choosefont.lpLogFont = lfont;
	winDebugMessage.Deg_trace_choosefont.rgbColors = RGB(0,0,0);
	winDebugMessage.Deg_trace_choosefont.Flags = CF_FORCEFONTEXIST|CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT;
	*cfont = CreateFont(NTXFONTHEIGHT(10,GetDC(hwnd)),0,0,0,FW_REGULAR,0,0,0,ANSI_CHARSET,0,0,PROOF_QUALITY,DEFAULT_PITCH,"Tahoma");
	//////////////////////////////
	winDebugMessage.vcom0_choosefont.lStructSize = sizeof(CHOOSEFONT);
	winDebugMessage.vcom0_choosefont.hwndOwner = hwnd;
	winDebugMessage.vcom0_choosefont.lpLogFont = lfont;
	winDebugMessage.vcom0_choosefont.rgbColors = RGB(0,0,0);
	winDebugMessage.vcom0_choosefont.Flags = CF_FORCEFONTEXIST|CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT;
	*cfont = CreateFont(NTXFONTHEIGHT(10,GetDC(hwnd)),0,0,0,FW_REGULAR,0,0,0,ANSI_CHARSET,0,0,PROOF_QUALITY,DEFAULT_PITCH,"Tahoma");
	//////////////////////////////
	winDebugMessage.vcom1_choosefont.lStructSize = sizeof(CHOOSEFONT);
	winDebugMessage.vcom1_choosefont.hwndOwner = hwnd;
	winDebugMessage.vcom1_choosefont.lpLogFont = lfont;
	winDebugMessage.vcom1_choosefont.rgbColors = RGB(0,0,0);
	winDebugMessage.vcom1_choosefont.Flags = CF_FORCEFONTEXIST|CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT;
	*cfont = CreateFont(NTXFONTHEIGHT(10,GetDC(hwnd)),0,0,0,FW_REGULAR,0,0,0,ANSI_CHARSET,0,0,PROOF_QUALITY,DEFAULT_PITCH,"Tahoma");
	//////////////////////////
	winDebugMessage.vcom2_choosefont.lStructSize = sizeof(CHOOSEFONT);
	winDebugMessage.vcom2_choosefont.hwndOwner = hwnd;
	winDebugMessage.vcom2_choosefont.lpLogFont = lfont;
	winDebugMessage.vcom2_choosefont.rgbColors = RGB(0,0,0);
	winDebugMessage.vcom2_choosefont.Flags = CF_FORCEFONTEXIST|CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT;
	*cfont = CreateFont(NTXFONTHEIGHT(10,GetDC(hwnd)),0,0,0,FW_REGULAR,0,0,0,ANSI_CHARSET,0,0,PROOF_QUALITY,DEFAULT_PITCH,"Tahoma");
	//////////////////////////////////////////
	winDebugMessage.vatc_choosefont.lStructSize = sizeof(CHOOSEFONT);
	winDebugMessage.vatc_choosefont.hwndOwner = hwnd;
	winDebugMessage.vatc_choosefont.lpLogFont = lfont;
	winDebugMessage.vatc_choosefont.rgbColors = RGB(0,0,0);
	winDebugMessage.vatc_choosefont.Flags = CF_FORCEFONTEXIST|CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT;
	*cfont = CreateFont(NTXFONTHEIGHT(10,GetDC(hwnd)),0,0,0,FW_REGULAR,0,0,0,ANSI_CHARSET,0,0,PROOF_QUALITY,DEFAULT_PITCH,"Tahoma");
	////////////////////////////////////////////////
	if ( isLoadDefaultConfiguration == TRUE )
	{
		winDebugMessage.Fwl_printf_choosefont.iPointSize     =140;
		winDebugMessage.Deg_trace_choosefont.iPointSize      =140;
		winDebugMessage.vcom0_choosefont.iPointSize          =140;
		winDebugMessage.vcom1_choosefont.iPointSize          =140;
		winDebugMessage.vcom2_choosefont.iPointSize          =140;
		winDebugMessage.vatc_choosefont.iPointSize           =140;
	}
	else
	{
	/*	FwlPointSize    =winDebugMessage.Fwl_printf_choosefont.iPointSize ;
		DegPointSize    =winDebugMessage.Deg_trace_choosefont.iPointSize  ;
		Vcom0PointSize  =winDebugMessage.vcom0_choosefont.iPointSize      ;
		Vcom1PointSize  =winDebugMessage.vcom1_choosefont.iPointSize      ;
		Vcom2PointSize  =winDebugMessage.vcom2_choosefont.iPointSize      ;
		VatcPointSize   =winDebugMessage.vatc_choosefont.iPointSize       ;
		*/
	}
}

void winvme_InitDebugWindow(HWND hwnd)
{
	HMENU hMenu; 
	unsigned char printFileName[50]= {0};    
	hMenu=GetMenu(hwnd);
	//////////////////////////////////
	if ( isLoadDefaultConfiguration == TRUE )
	{
		winDebugMessage.printSelect.isFwlPrintf=TRUE;
// 		winDebugMessage.printSelect.isDegTrace=TRUE;
// 		winDebugMessage.printSelect.isVom0=TRUE;
// 		winDebugMessage.printSelect.isVcom1=TRUE;
// 		winDebugMessage.printSelect.isVcom2=TRUE;
// 		winDebugMessage.printSelect.isVatc=TRUE;
		winDebugMessage.print_keydown =TRUE;

	}
	else
	{
		winvme_read_configuration();
		winDebugMessage = configInfo ;
	}
//	winDebugMessage.Fwl_printf_choosecolor.

///////////////////////////////////////////

// 	if (winDebugMessage.printSelect.isFwlPrintf == FALSE)
// 	CheckMenuItem(hMenu,CM_FWL_PRINTF,MF_BYCOMMAND|MF_UNCHECKED);
// 	else
// 	CheckMenuItem(hMenu,CM_FWL_PRINTF,MF_BYCOMMAND|MF_CHECKED);
// 	
// 	///////////////////////////////////////////
// 	if (winDebugMessage.printSelect.isDegTrace == FALSE)
// 	CheckMenuItem(hMenu,CM_DEG_TRACE,MF_BYCOMMAND|MF_UNCHECKED);
// 	else
// 	CheckMenuItem(hMenu,CM_DEG_TRACE,MF_BYCOMMAND|MF_CHECKED);
// 	
// 	/////////////////////////////////////////
// 	if (winDebugMessage.printSelect.isVom0 == FALSE)
// 	CheckMenuItem(hMenu,CM_VCOM0,MF_BYCOMMAND|MF_UNCHECKED);
// 	else
// 	CheckMenuItem(hMenu,CM_VCOM0,MF_BYCOMMAND|MF_CHECKED);
// 	
// 	//////////////////////////////////////////////
// 	if (winDebugMessage.printSelect.isVcom1 ==FALSE)
// 	CheckMenuItem(hMenu,CM_VCOM1,MF_BYCOMMAND|MF_UNCHECKED);
// 	else
// 	CheckMenuItem(hMenu,CM_VCOM1,MF_BYCOMMAND|MF_CHECKED);
// 	
// 	//////////////////////////////////////////////
// 	if (winDebugMessage.printSelect.isVcom2 ==FALSE )
// 	CheckMenuItem(hMenu,CM_VCOM2,MF_BYCOMMAND|MF_UNCHECKED);
// 	else
// 	CheckMenuItem(hMenu,CM_VCOM2,MF_BYCOMMAND|MF_CHECKED);
// 	
// 	////////////////////////////////////////////
// 	if ( winDebugMessage.printSelect.isVatc == FALSE)
// 	CheckMenuItem(hMenu,CM_VATC,MF_BYCOMMAND|MF_UNCHECKED);
// 	else
// 	CheckMenuItem(hMenu,CM_VATC,MF_BYCOMMAND|MF_CHECKED);

// 	Winvme_OpenTraceInfoFile();   
}

// save the program configuration ,to get next time know use it 
// T_VOID winvme_save_configuration(T_VOID)
// {
// 	char Filename[] = "confingInfo.txt";
// 	FILE *fp;
// 	
// 	fp =fopen( Filename,"w+t");
// 	if(NULL == fp)
// 	{
// 		MessageBox(NULL,"can't save configuration message!","save warning!!",MB_OK);
// 		return;		
// 	}
// 	configInfo= winDebugMessage;
// 	fwrite(&configInfo,sizeof(configInfo),1,fp);  //read content to file
// 	fclose(fp);
// 	return;
// }
// 

// T_BOOL Winvme_GetPrintDegStatus(T_VOID)
// {
// 	return winDebugMessage.printSelect.isDegTrace;
// }
T_BOOL Winvme_GetPrintFwlStatus(T_VOID)
{
	return AK_TRUE/*winDebugMessage.printSelect.isFwlPrintf*/;
}
// T_BOOL Winvme_GetPrintVcom0Status(T_VOID)
// {
// 	return winDebugMessage.printSelect.isVom0;
// }
// T_BOOL Winvme_GetPrintVcom1Status(T_VOID)
// {
// 	return winDebugMessage.printSelect.isVcom1;
// }
// T_BOOL Winvme_GetPrintVcom2Status(T_VOID)
// {
// 	return winDebugMessage.printSelect.isVcom2;
// }
// T_BOOL Winvme_GetPrintVatcStatus(T_VOID)
// {
// 	return winDebugMessage.printSelect.isVatc;
// }
T_VOID Winvme_SetCurrentPrintType(DebugOutoutType printtype)
{
	print_type = ePrintf/*printtype*/;
}

// T_VOID Winvme_WriteTraceInfoFile(const char * data)
// {
// 	if ( tracefile != AK_NULL )
// 		fputs(data, tracefile);	
// }
// 
// T_VOID Winvme_CloseTraceInfoFile(void)
// {
// 	if ( tracefile != AK_NULL )
// 		fclose(tracefile);
// 	tracefile = AK_NULL;
// }
// 
// T_VOID Winvme_OpenTraceInfoFile(void)
// {
// 	unsigned char printFileName[50] = {0};
// 	winvme_use_time_get_log_filename(printFileName);
// 
// 	if ( tracefile != AK_NULL )
// 		Winvme_CloseTraceInfoFile();
// 	tracefile = fopen(printFileName, "ab+");
// }
FILE* pDebugFile=NULL;

int EB_Printf(const char *fmt, ...)
{
    va_list ap;
    int i, slen, lineIdx;
    int txtRepStart, txtRepEnd, txtSelEnd;
    int str2Pt = 0;
    static int  wasCr = 0; //should be static type.
    static char string[2*STRING_LEN]; //margin for '\b'
    static char string2[2*STRING_LEN]; //margin for '\n'->'\r\n'
    static int  prevBSCnt = 0;

	ConsolePrint(0, fmt);//always can use for multi-thread

    while (isUsed); //EB_Printf can be called multiplely  //KIW

    isUsed = TRUE;

    txtRepStart = SendMessage(m_hWnd_DebugWnd, WM_GETTEXTLENGTH, 0x0, 0x0);
    txtRepEnd = txtRepStart - 1;

    va_start(ap, fmt);
    _vsntprintf(string2, STRING_LEN-1, fmt, ap);
    va_end(ap);

    string2[STRING_LEN - 1] = '\0';

    //for better look of BS(backspace) char.,
    //the BS in the end of the string will be processed next time.
    for (i = 0; i < prevBSCnt; i++) //process the previous BS char.
    {
        string[i]='\b';
    }
    string[prevBSCnt] = '\0';
    lstrcat(string, string2);
    string2[0] = '\0';

    slen = lstrlen(string);
    for (i = 0; i < slen; i++)
    {
        if (string[slen - i - 1] != '\b')
            break;
    }

    prevBSCnt = i; // These BSs will be processed next time.
    slen = slen - prevBSCnt;

    if (slen == 0)
    {
        isUsed = FALSE;
        return 0;
    }

    for (i = 0; i < slen; i++)
    {
        if ((string[i] == KEY_BACKSPACE))
        {
            /*
            string2[str2Pt++] = KEY_BACKSPACE;txtRepEnd++;
            string2[str2Pt++] = ' ';txtRepEnd++;
            string2[str2Pt++] = KEY_BACKSPACE;txtRepEnd++;
            wasCr = 0;
            continue;
            */
            if (str2Pt > 0)
            {
                string2[str2Pt--] = KEY_BACKSPACE;txtRepEnd--;
                //string2[str2Pt++] = ' ';txtRepEnd++;
                //string2[str2Pt++] = KEY_BACKSPACE;txtRepEnd++;
                wasCr = 0;
                continue;
            }
        }

        if ((string[i] == '\n'))
        {
            string2[str2Pt++] = '\r';
            txtRepEnd++;
            string2[str2Pt++] = '\n';
            txtRepEnd++;
            wasCr = 0;
            continue;
        }
        if ((string[i] != '\n') && (wasCr == 1))
        {
            string2[str2Pt++] = '\r';
            txtRepEnd++;
            string2[str2Pt++] = '\n';
            txtRepEnd++;
            wasCr=0;
        }
        if (string[i] == '\r')
        {
            wasCr = 1;
            continue;
        }

        if (string[i] == '\b')
        {
            //flush string2
            if (str2Pt > 0)
            {
                string2[--str2Pt] = '\0';
                txtRepEnd--;
                continue;
            }
            //str2Pt==0;
            if(txtRepStart > 0)
            {
                txtRepStart--;
            }
            continue;
        }
        string2[str2Pt++] = string[i];
        txtRepEnd++;
        // if (str2Pt > 256-3)  break; //why needed? 2001.1.26
    }

    string2[str2Pt] = '\0';
    if (str2Pt > 0)
    {
        SendMessage(m_hWnd_DebugWnd,EM_SETSEL,txtRepStart,txtRepEnd+1);
        SendMessage(m_hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)string2);
        if (NULL==pDebugFile)
        {
            pDebugFile = fopen("../debug.txt","w");
        }
        fprintf(pDebugFile,string2);
        fflush(pDebugFile);
    }
    else
    {
        if (txtRepStart <= txtRepEnd)
        {
            SendMessage(m_hWnd_DebugWnd,EM_SETSEL,txtRepStart,txtRepEnd+1);
            SendMessage(m_hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)"");
        }
    }

    //If edit buffer is over EDIT_BUF_SIZE,
    //the size of buffer must be decreased by EDIT_BUF_DEC_SIZE.
    if (txtRepEnd > EDIT_BUF_SIZE)
    {
        lineIdx=SendMessage(m_hWnd_DebugWnd,EM_LINEFROMCHAR,EDIT_BUF_DEC_SIZE,0x0);
        //lineIdx=SendMessage(hWnd_DebugWnd,EM_LINEFROMCHAR,txtRepEnd-txtRepStart+1,0x0); //for debug
        txtSelEnd=SendMessage(m_hWnd_DebugWnd,EM_LINEINDEX,lineIdx,0x0)-1;
        SendMessage(m_hWnd_DebugWnd,EM_SETSEL,0,txtSelEnd+1);

        SendMessage(m_hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)"");
        //SendMessage(hWnd_DebugWnd,WM_CLEAR,0,0); //WM_CLEAR doesn't work? Why?
        //SendMessage(hWnd_DebugWnd,WM_CUT,0,0); //WM_CUT doesn't work? Why?

        //make the end of the text shown.
        txtRepEnd=SendMessage(m_hWnd_DebugWnd,WM_GETTEXTLENGTH,0x0,0x0)-1;
        SendMessage(m_hWnd_DebugWnd,EM_SETSEL,txtRepEnd+1,txtRepEnd+2);
        SendMessage(m_hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)" ");
        SendMessage(m_hWnd_DebugWnd,EM_SETSEL,txtRepEnd+1,txtRepEnd+2);
        SendMessage(m_hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)"");
    }

    isUsed = FALSE;

    return 0;
}
#endif
