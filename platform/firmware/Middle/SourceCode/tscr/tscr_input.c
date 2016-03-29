/************************************************************************
* Copyright (c) 2001, Anyka Co., Ltd. 
* All rights reserved.	
*  
* File Name：tscr_input.c
* Function：handle write control ! such  as  mode control ,  recongize charachter control !
*
* Author：LuoXiaoQing
* Date：2005-10-10
* Version：1.0		  
*
* Reversion: 
* Author: 
* Date: 
**************************************************************************/
#ifdef OS_ANYKA


#include "tscr_input.h"
#include "eng_string.h"
#include "eng_debug.h"
#include "eng_screensave.h"
#include <stdarg.h>
#include "fwl_pfdisplay.h"
#include "Lib_state.h"
#include "Fwl_osMalloc.h"

#ifdef  wt_test
#include "data.h"
#endif
#include "akos_api.h"
#include "Hal_ts.h"
#include "Eng_DataConvert.h"
#include "gpio_config.h"
#include "Hal_gpio.h"
#include "drv_gpio.h"
#include "fwl_display.h"


#define			TSCR_LINE_MAX	    16
#define 		TSCR_BUFFER_LEN		40
#define 		TSCR_LINE_COLOR		0x3C0262//0x451013 
#define         GETABS(var) ((var)<0?(0-(var)):(var))


typedef struct
{
	T_fTSCRGET_CALLBACK	callback_func;	/* callback function only for Get_words called*/
} T_TSCRGET_DATA;


#define TS_SAVE_POINT_MAX			512
#define TS_RECONGIZE_BUFFER_SIZE    1024


/*
 * packet context
 */
typedef struct{
	T_POINT	pt;						//the inking point
	T_RECT	TextRect;				//Text Show Rect     ****when the user paint a char , the first point must begin in the text rect
	T_U8	words[STUFF_LEN-2];		//the recognized result, max length is 20
	T_U8	wordCnt;				//the words count

	volatile	T_S8	recogModeOff;			//recognise mode off   really work mode 
	T_BOOL	recogModeOffUser;		//when the first point is not in the TextRect  We must cancle the recognize!   this is the user  want work mode 
	T_RECT	WriteAreaRect;				//Text Show Rect     ****when the user paint a char , the first point must begin in the text rect

    T_TSPOINT	ts_point[TS_SAVE_POINT_MAX];
    T_U32       ts_pointhead;
    T_U32       ts_pointtail;

    T_U16	ts_recongize[TS_RECONGIZE_BUFFER_SIZE];
    T_U32   ts_recongizeCount;
    T_BOOL  ts_recongizing;
	T_S8	gb_hw_words[TSCR_BUFFER_LEN];
	T_U16	Down_Begin_TextRect;   // 1 down begin from text rect
}TSCR_PCKT_CONTEXT;

TSCR_PCKT_CONTEXT *pckt_context = AK_NULL;
T_TSCRGET_DATA	tscr_data;


T_POINT	pre_pt;


extern T_GLOBAL_S  gs;    
extern T_BOOL	   isChageHWState;
extern T_BOOL	   isShowHW;

#ifdef SUPPORT_OFFICEVIEWER
extern unsigned int converter_exit;
#endif

/////////////// static function define ////////////////////

static T_BOOL Set5Pixel(HLAYER hLayer, T_POS x, T_POS y, T_COLOR color);
static T_VOID DrawWidthLine(HLAYER hLayer, T_POS x1, T_POS y1, T_POS x2, T_POS y2, T_COLOR color);
//static T_BOOL tscr_SBCtoDBC(T_pSTR  dest ,T_U32 destLen, T_U16 * unicode , T_U16 unicodeLen);
static T_VOID tscr_showline(T_POINT *  firstPt , T_POINT * endPt);

static T_BOOL tscr_SetModeOnly(T_U8 mode);

static T_S16 tscr_CheckPt(T_TSPOINT * pt);
static T_U8 tscr_getLCDPoint(T_pTSPOINT displayPtr, const T_pTSPOINT screenPtr) ;
static T_VOID  tscr_handle_point(T_TSPOINT * tspt );
static T_VOID  tscr_callback(T_VOID);
///////////////end  static function define ////////////////////


extern T_U8 getDisplayPoint(T_pTSPOINT displayPtr, const T_pTSPOINT screenPtr, 
						       const T_MATRIX *matrixPtr);


/**
 * @brief   return the recognize points and point number 
 *         
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Luo_Xiaoqing
 * @date   2006-03-07
 * @version 1.0
 */
T_U16 *TsAdcGetPointList(T_U32 *pointLen)
{
	*pointLen = pckt_context->ts_recongizeCount / 2;
	return pckt_context->ts_recongize;
} 


T_VOID TSCR_ResetPointRecord(T_VOID)
{
	pckt_context->ts_recongizeCount = 0;
    pckt_context->ts_pointhead = 0;
    pckt_context->ts_pointtail = 0;
}

/////////////// static function Begin  ////////////////////
static T_BOOL Set5Pixel(HLAYER hLayer, T_POS x, T_POS y, T_COLOR color)
{
	Fwl_SetPixel(hLayer, x, y, color);
	if ( y > 0 )
	{
		Fwl_SetPixel(hLayer, x, y-1, color);
	}
	Fwl_SetPixel(hLayer, x, y+1, color);	
	
	if (x > 0 )
	{
		Fwl_SetPixel(hLayer, x-1, y, color);
	}

	Fwl_SetPixel(hLayer, x+1, y, color);
	return AK_TRUE;
}


/* graphics operation */
/**
 * @brief Draw a line.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_POS x1  X coordination of start point.
 * @param  T_POS y1 Y coordination of start point.
 * @param  T_POS x2 X coordination of end point.
 * @param  T_POS y2 Y coordination of end point.
 * @param  T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
static T_VOID DrawWidthLine(HLAYER hLayer, T_POS x1, T_POS y1, T_POS x2, T_POS y2, T_COLOR color)
{
	T_POS	x			= 0;
	T_POS	y			= 0;
	T_POS	p			= 0;
	T_POS	n			= 0;
	T_POS	tn			= 0;
	T_U16	lcdBottom	= 0;
	T_U16	lcdRight		= 0;
	const T_U16 lcdTop	= 0;
	const T_U16 lcdLeft	= 0;

	
	lcdBottom =  Fwl_GetLcdHeight() -1; 
	lcdRight = Fwl_GetLcdWidth()  -1;

	
	if(x1==x2 && y1==y2)
	{
		Set5Pixel(hLayer, x1, y1, color);
		return;
	}

	if(y1 == y2)
	{
		//The horizontal line exceed the bottom of lcdrect
		if(y1>lcdBottom || y1<lcdTop)
		{ 
			return;
		}
		if(x1 > x2)
		{
			x=x2;x2=x1;x1=x;
		}
		if(x1 < lcdLeft)
		{
			x1 = lcdLeft;
		}
		if(x2 > lcdRight)
		{
			x2 = lcdRight;
		}
		if(x1 == x2)
		{
			Set5Pixel(hLayer, x1, y1, color);
		}
		else
		{
			for(x=x1; x<=x2; x++)
			{
				Set5Pixel(hLayer, x, y1, color);
			}
		}
		return;
	}

	if(x1 == x2)
	{
		if(x1>lcdRight || x2<lcdLeft)
		{
			return;
		}
		if(y1 > y2)
		{
			y=y2;y2=y1;y1=y;
		}
		if(y1 < lcdTop)
		{
			y1 = lcdTop;
		}
		if(y2 > lcdBottom)
		{
			y2 = lcdBottom;
		}
		if(y1 == y2)
		{
			Set5Pixel(hLayer, x1, y1, color);
		}
		else
		{
			for(y=y1; y<=y2; y++)
			{
				Set5Pixel(hLayer, x1, y, color);
			}
		}
		return;
	}

	if(GETABS(y2-y1) <= GETABS(x2-x1))
	{
		if((y2<y1&&x2<x1) || (y1<=y2&&x1>x2))
		{
			x=x2;y=y2;x2=x1;y2=y1;x1=x;y1=y;
		}
		if(y2>=y1 && x2>=x1)
		{
			x=x2-x1; y=y2-y1;
			p=2*y; n=2*x-2*y; tn=x;
			while(x1<=x2)
			{
				if(tn>=0)
				{
					tn-=p;
				}
				else
				{
					tn+=n;
					y1++;
				}
				if(	x1>=lcdLeft 
					&& x1<=lcdRight
					&& y1>=lcdTop
					&& y1<=lcdBottom)
				{
					Set5Pixel(hLayer, x1, y1, color);
				}
				++x1;
			}
		}
		else
		{
			x=x2-x1;y=y2-y1;
			p=-2*y;n=2*x+2*y;tn=x;
			while(x1<=x2)
			{
				if(tn>=0)
				{
					tn-=p;
				}
				else
				{
					tn+=n;
					y1--;
				}
				if(	x1>=lcdLeft 
					&& x1<=lcdRight
					&& y1>=lcdTop
					&& y1<=lcdBottom)
				{
					Set5Pixel(hLayer, x1, y1, color);
				}
				++x1;
			}
		}
	}
	else
	{
		x=x1;x1=y2;y2=x;y=y1;y1=x2;x2=y;
		if((y2<y1&&x2<x1) || (y1<=y2&&x1>x2))
		{
			x=x2;y=y2;x2=x1;y2=y1;x1=x;y1=y;
		}
		if(y2>=y1 && x2>=x1)
		{
			x=x2-x1;y=y2-y1;p=2*y;n=2*x-2*y;tn=x;
			while(x1 <= x2)
			{
				if(tn>=0)
				{
					tn-=p;
				}
				else
				{
					tn+=n;
					y1++;
				}
				if(	y1>=lcdLeft 
					&& y1<=lcdRight
					&& x1>=lcdTop
					&& x1<=lcdBottom)
				{
					Set5Pixel(hLayer, y1, x1, color);
				}
				++x1;
			}
		}
		else
		{
			x=x2-x1;y=y2-y1;p=-2*y;n=2*x+2*y;tn=x;
			while(x1 <= x2)
			{
				if(tn>=0)
				{
					tn-=p;
				}
				else
				{
					tn+=n;
					y1--;
				}
				if(	y1>=lcdLeft 
					&& y1<=lcdRight
					&& x1>=lcdTop
					&& x1<=lcdBottom)
				{
					Set5Pixel(hLayer, y1, x1, color);
				}
				++x1;
			}
		}
	}
}


static T_VOID tscr_showline(T_POINT *  firstPt , T_POINT * endPt)
{
	DrawWidthLine(HRGB_LAYER,firstPt->x,firstPt->y ,endPt->x, endPt->y ,TSCR_LINE_COLOR);
}

/**
 * @brief set char 0xff39  to 0x39  Just use for English and number 
 can not be used in character
 * @author Luo XiaoQing
 * @date	2005-10-10
 * @version 1.0
 */
 #if 0
static T_BOOL tscr_SBCtoDBC(T_pSTR  dest ,T_U32 destLen, T_U16 * unicode , T_U16 unicodeLen)
{
	T_U16	i,destPos=0;
	T_S8	key=' ';
	T_S32 	GbkRet;	

	for ( i = 0 ; ( i< unicodeLen ) && ( unicode[i]  != 0 ) ; i ++ )
	{
		if( ( unicode[i] & 0xff00 ) == 0xff00 )   
		{
			dest[destPos++] =  (T_S8) ( ( unicode[i] & 0x00ff ) + 0x20 ) ;
		}
	}
	
	//if unicodes has  0xff**! return directly!
	if ( destPos > 0 )
	{
		return AK_TRUE;
	}
	else
	{

		GbkRet = Eng_WideCharToMultiByte(eRES_LANG_ENGLISH, unicode, unicodeLen, AK_NULL, dest, destLen , &key);
	} 
	return  AK_TRUE;
}
#endif

#if 0
//////////////////////////////////////////////////////////////////
//////////////////集成汉王手写识别//////////////////////
//////////////////////// begin ////////////////////////////////////
//===========================================================================
//handwriting data of  "三"
/*
THWPoint aPenData[] = {
	0x10, 0x10, 0x20, 0x10, STROKEENDMARK, 0, 
	0x10, 0x20, 0x20, 0x20, STROKEENDMARK, 0, 
	0x10, 0x30, 0x20, 0x30, STROKEENDMARK, 0, 
	STROKEENDMARK, STROKEENDMARK	
};
*/
#define MAX_POINT_LEN   1000
static T_U32       pRam[HWRERAMSIZE/4]; // acquire RAM for recognition engine, in Four-byte alignment 
static THWPoint    aPenData[MAX_POINT_LEN+1] = {0};
static T_U8        *hDict = AK_NULL;
static T_U32       range = 0;
static T_U16       refCounts = 0;

//===========================================================================


THWPoint * HWRE_GetPenData()
{
	T_U32 PointLen;
	T_U16 i = 0;
	T_S16 *ptList = AK_NULL;
	
	ptList = TsAdcGetPointList(&PointLen);
    PointLen = PointLen < MAX_POINT_LEN ? PointLen : MAX_POINT_LEN;
    
	while(i<PointLen)
	{
		aPenData[i].x = ptList[2*i];
		aPenData[i].y = ptList[2*i+1];	
		i++;
	}

	return aPenData;
}


T_S16 HWRE_LoadDict(T_VOID)
{
	T_hFILE in;
	long nLen;

    if(hDict != AK_NULL)
    {
        Fwl_Print(C3, M_FWL, "#####hDict != AK_NULL#####\n");
        refCounts++;
        return 0;
    }
    
	if(gb.Lang == GetRussianIndex() 
        || gb.Lang ==  GetGreekIndex()
        || gb.Lang == GetSpanishIndex())
	{
		Fwl_Print(C3, M_FWL, "Load dic MTL\n");
		in = Fwl_FileOpen( _T(DRI_A"MTL_Unicode_LittleEndian_NorFlash.dic"), 
                          _FMODE_READ, 
                          _FMODE_READ);
	}
	else if(gb.Lang == GetThailandIndex())
	{
		Fwl_Print(C3, M_FWL, "Load dic TH\n");
		in = Fwl_FileOpen( _T(DRI_A"HW_Thailand_Unicode.bin"), 
                          _FMODE_READ, 
                          _FMODE_READ);
	
	}
	else
	{
		Fwl_Print(C3, M_FWL, "Load dic CHS\n");		
		in = Fwl_FileOpen( _T(DRI_A"CHMUNIDIC_LittleEndian.dic"), 
                          _FMODE_READ, 
                          _FMODE_READ);
	}
	Fwl_Print(C3, M_FWL, "\nFS_FileOpen dic: %d\n", in);
	if( in < 0 )
    {
        return -1;
    }
	nLen = Fwl_FileSeek( in, 0, _FSEEK_END);
	Fwl_FileSeek( in, 0, _FSEEK_SET);
	hDict = (T_U8*)Fwl_Malloc(nLen);
	if( hDict == AK_NULL )
	{
		Fwl_FileClose(in);
		return -2;
	}
    
    refCounts++;
	Fwl_FileRead( in, hDict, nLen );
	Fwl_FileClose(in);

	return 0;
}

T_VOID HWRE_FreeDict( T_VOID )
{
    refCounts--;
    
    if( refCounts == 0 && hDict != AK_NULL )
	{
		Fwl_Free(hDict);
		hDict = AK_NULL;
	}

	return;
}


static T_BOOL tscr_WTRecongize()
{
	THWAttribute        attr        = {0};
	E_THWAttribute      e_attr      = {0};
	THAI_THWAttribute   thai_attr   = {0};
	T_U16               Result[MAXCANDNUM+1] = {0};
	T_U32               rang        = 0;    
	T_S32               iCodeNum = 1;
	T_U8                *pRom;
	T_U16               language = 0;
	T_U16               i;
	
	if (!isChageHWState)
	{
		isChageHWState = AK_TRUE;
	}
    
	isShowHW = AK_TRUE;

	if(gb.Lang == GetRussianIndex() 
        || gb.Lang ==  GetGreekIndex()
        || gb.Lang == GetSpanishIndex()
        || gb.Lang == GetFrenchIndex()
        || gb.Lang == GetFarsiIndex())
	{
		if(gb.Lang == GetRussianIndex())
		{
			language = HWLANG_Russian;
		}
		else if(gb.Lang == GetSpanishIndex())
		{
			language = HWLANG_Spanish;		
		}
		else if(gb.Lang == GetGreekIndex())
		{
			language = HWLANG_Greek;		
		}
		else if (gb.Lang == GetFrenchIndex())
		{
			language = HWLANG_French;
		}
			
		Fwl_Print(C3, M_FWL, "\n______Multi Language!_______\n");
        if((e_attr.pRom = hDict) == AK_NULL)
		{
			return AK_FALSE;
		}
		e_attr.pRam = ( T_U8* )pRam;
		e_attr.iCandidateNum = MAXCANDNUM; //set the number of candidate you want to be returned
		e_attr.dwRange = (T_U32)range | 0x00000000;
		e_attr.wMode = MODE_SINGLECHAR;	// 单字识别
		e_attr.wLanguage = language;
		e_attr.pFrame = AK_NULL;
		e_attr.pCharSet = AK_NULL;
		
		Fwl_Print(C3, M_FWL, "\ne_attr.dwRange = %x\n", e_attr.dwRange);
		// iCodeNum = MTL_HWRecognize(HWRE_GetPenData(), &e_attr, Result);

	}
	else if(gb.Lang == GetThailandIndex())
	{
		Fwl_Print(C3, M_FWL, "\n______Thailand!_______\n");
        if((thai_attr.pRom = hDict) == AK_NULL)
		{
			return AK_FALSE;
		}
        
		thai_attr.pRam = ( T_U8* )pRam;
		thai_attr.iCandidateNum = MAXCANDNUM; //set the number of candidate you want to be returned
		thai_attr.dwRange = (T_U32)range | 0x00000000;
		thai_attr.iBoxWidth = 0;
		thai_attr.iBoxHeight = 0;
		
		Fwl_Print(C3, M_FWL, "\nattr.dwRange = %x\n", thai_attr.dwRange);
		//iCodeNum = THAI_HWRecognize(HWRE_GetPenData(), &thai_attr, Result);
	}
	else
	{
		Fwl_Print(C3, M_FWL, "\n______Chinese!_______\n");
        if((attr.pRom = hDict) == AK_NULL)
		{  
			return AK_FALSE;
		}

		attr.pRam = ( T_U8* )pRam;
		attr.iCandidateNum = MAXCANDNUM; //set the number of candidate you want to be returned
		attr.dwRange = (T_U32)range | 0x00000000;

		Fwl_Print(C3, M_FWL, "\nattr.dwRange = %x\n", attr.dwRange);
		iCodeNum = HWRecognize(HWRE_GetPenData(), &attr, Result);

	}
	
	if( iCodeNum <= 0 )
	{
	    pckt_context->gb_hw_words[0] = L'\0';
		return AK_FALSE;
	}
    else
    {
        Utl_TStrCpy(pckt_context->gb_hw_words, Result);
        return AK_TRUE;
    }
    
}


//////////////////////////////////////////////////////////////////
//////////////////集成汉王手写识别//////////////////////
//////////////////////// end //////////////////////////////////////






//#ifdef WT_TEST
//static T_S32  wenTongTest(T_VOID)
//{
//	T_S32 i;    
//  	T_U8 RamAddress[2048],*LibStartAddress;
//	//FILE * WTPENLibFile;    ?
//	//T_S32 TotalFileLength;  ?
//	T_U16 CandidateResult[11];
//
//	//LibStartAddress = (T_U8 *)WTPENPDAGB2312SetUnicode;
//	//LibStartAddress = gb_LargeBuffer;	
//	/*LibStartAddress = ( T_U8 * )Fwl_Malloc(819200);
//	AK_ASSERT_PTR_VOID(LibStartAddress,"malloc failed \r\n");
//	for (i=0;i<794868;i++)
//		LibStartAddress[i] = gb_HandSignCheck[i];*/
//	LibStartAddress = gb_HandSignCheck ;	
//
//	Fwl_Print(C3, M_FWL, " 11111 ");
//		Fwl_Print(C3, M_FWL, "The Lib First 32 byte is r\n");
//		{
//			T_U16  i;
//			T_U8 * p;
//			p = LibStartAddress;
//			for (i=0;i<32;i++)
//				Fwl_Print(C3, M_FWL, "%02x  ",p[i]);
//		   
//
//		Fwl_Print(C3, M_FWL, "\nThe Lib last 36 byte is r\n");
//		 
//			p = LibStartAddress;
//			p+=0xc20d0;
//			for (i=0;i<36;i++)
//				Fwl_Print(C3, M_FWL, "%02x  ",p[i]);
//		}
//
//// HanWang
////	if(WTRecognizeInit(RamAddress,2048, LibStartAddress))
////	//if(WTRecognizeInit(RamAddress,2048,LibStartAddress))
////	{
////		Fwl_Print(C3, M_FWL, "\nRecognizeInit Error!");
////		return(1);
////	}
//
//
//	
//	Fwl_Print(C3, M_FWL, "22222  ");
//	//设置识别范围，实际识别范围与识别库有关
//	//WTSetRange(0x07ff,RamAddress);
//	//WTSetRange(strTSConfigTab.recognitionRange);     // for 2006.02.28 v3 
//
//	// HanWang
//	// WTSetRange(0x0001);
//
//
//	//WTSetSpeed(strTSConfigTab.recognitionSpd);
//	Fwl_Print(C3, M_FWL, " 333333 ");	
//    //timer_dis_int();
//
//	// HanWang
//	// WTRecognize(character3, 94, CandidateResult); 
//
//	//timer_ena_int();
//	Fwl_Print(C3, M_FWL, " 4444444 ");  
//	Fwl_Print(C3, M_FWL, "\ncharacter3:\n");
//	for (i = 0; i < 10; i++)
//    {
//        Fwl_Print(C3, M_FWL, "\t%04x\n", CandidateResult[i]);
//    }
//
// 
//
//	// 显示识别结果 	
//	CandidateResult[10]=0;	// 为了正确显示，字符串最后必须是0 
//    	Fwl_Print(C3, M_FWL, "Recognize result:\n%s\n",(char *)CandidateResult);
//	Fwl_Print(C3, M_FWL, "The demo program is over,thanks!\n");
//	
//	 //	退出
//	//WTRecognizeEnd(RamAddress);
//	//WTRecognizeEnd();      // for 2006.02.28 v3  
//
//}
//#endif

#endif
/*
 *brief: Default is Recognition mode. Graphic mode transmission coordinates
		are similar to Recognition mode except recognition is not processed
		under Graphic mode.
 *param: mode, 0x00 for recogniton mode, 0x01 for graphic mode
 *command: 0x49
 */
static T_BOOL tscr_SetModeOnly(T_U8 mode)
{

	if (mode == 0x01)
	{
		pckt_context->recogModeOff = 1;
	}
	else
	{
		pckt_context->recogModeOff = 0;
	}
		return AK_TRUE;
}



static T_U8 tscr_getLCDPoint(T_pTSPOINT displayPtr, const T_pTSPOINT screenPtr)
{
/*
matrix.An =    91080  matrix.Bn =     4140  matrix.Cn = -11558880
matrix.Dn =    11880  matrix.En =   102390  matrix.Fn = -11547180
matrix.Divider =   294492
*/
#ifdef TOUCH_SCR
	T_U8  			ret;   
	T_TSPOINT  		wtpt;

	wtpt.x  = screenPtr->x;
	wtpt.y  = screenPtr->y;	
    

	ret = getDisplayPoint(displayPtr,&wtpt,&gs.matrixPtr);

    //AK_DEBUG_OUTPUT("!!!!!!! 1. displayPtr->x = %d !!!!!!!!", displayPtr->x);
	//AK_DEBUG_OUTPUT("!!!!!!! 1. displayPtr->y = %d !!!!!!!!", displayPtr->y);

	if ( displayPtr->x  <=  0 ) 
	{
		displayPtr->x = 0;
	}
	else if (displayPtr->x  >= (LCD0_WIDTH-1))
	{
		displayPtr->x = LCD0_WIDTH-1;	
	}

	if ( displayPtr->y  <=  0 ) 
	{
		displayPtr->y = 0; 
	}
	#ifdef SUPPORT_STDB_BOTTOMBAR	
	else if (displayPtr->y  >= (LCD0_HEIGHT+20-1))
	{
		displayPtr->y = (LCD0_HEIGHT+20-1);	
	}
	#else
	else if (displayPtr->y  >= (LCD0_HEIGHT-1))
	{
		displayPtr->y = (LCD0_HEIGHT - 1);	
	}
	#endif

#endif
    return AK_TRUE;
}

static T_S16 tscr_CheckPt(T_TSPOINT * pt)
{
	T_S16 		ret=0;
	T_S32 		x,y;
	T_TSPOINT  	TSLCDpt;
	T_POINT  	LCDpt;
	static T_S32  	s_ret=1;
	
	//parse command
	
	x = pt->x;
	y = pt->y;
	//Fwl_Print(C3, M_FWL, "check ADC pt (%d,%d)  pckt_context->recogModeOff  %d   s_ret  %d \r\n",
	//	x,y,pckt_context->recogModeOff,s_ret);
	if ((x==TS_HAND_WR_INVALID_COORD && y==0))
	{
	
		//stroke over, means pen up
		if (pckt_context->recogModeOff==1)
		{
			s_ret = 1;
			//Fwl_Print(C3, M_FWL, "Up 222\r\n\n");
			ret = ePH_RET_PEN_UP;
		}
		else
		{
			//Fwl_Print(C3, M_FWL, "STROKE_OVER  333\r\n\n");
			ret = ePH_RET_STROKE_OVER;
			s_ret = 1;
		}
	}
	else if (x==TS_HAND_WR_INVALID_COORD && y==TS_HAND_WR_INVALID_COORD)	
	{
	//word over
        if(pckt_context->recogModeOff==0)
		{
			Fwl_Print(C3, M_FWL, "recognise\r\n\n");

	/*		bWtRecogzie = tscr_WTRecongize();
			//wenTongTest();
			if (AK_TRUE == bWtRecogzie)
			{
				ret = ePH_RET_RECOG_ARRAY;
			}
          */  pckt_context->ts_recongizeCount = 0;

			//Get a char set the    LXQ Change Mode
			tscr_SetModeOnly(0x01);
		}
		s_ret = 1;		
	}
	else if (((pckt_context->recogModeOff==1) && (s_ret==1)))
	{
 		tscr_getLCDPoint(&TSLCDpt,pt);
		
		LCDpt.x = TSLCDpt.x;
		LCDpt.y = TSLCDpt.y;
		
		//tscr_Touch2LCDPt(&LCDpt);

		pckt_context->pt.x = LCDpt.x;
		pckt_context->pt.y = LCDpt.y;

//		Fwl_Print(C3, M_FWL, "check1(%d,%d)",pckt_context->pt.x,pckt_context->pt.y);
		
		pckt_context->Down_Begin_TextRect=1;

		Fwl_Print(C3, M_FWL, "DOWN 1111 \r\n\n");
		ret = ePH_RET_PEN_DOWN;
		s_ret = 0;
	} 
	else	
	{
        tscr_getLCDPoint(&TSLCDpt,pt);
		
		LCDpt.x = TSLCDpt.x;
		LCDpt.y = TSLCDpt.y;
		
		//tscr_Touch2LCDPt(&LCDpt);
		
		//if in graphic mode, return pen move event
		if (pckt_context->recogModeOff==1)
		{
			pckt_context->pt.x = LCDpt.x;
			pckt_context->pt.y = LCDpt.y;
			//Fwl_Print(C3, M_FWL, "check2(%d,%d)",pckt_context->pt.x,pckt_context->pt.y);
			
            ret = ePH_RET_PEN_MOVE;	
			//user intaface is Reogmod  ! and down begin from text rect ! Set mode back  LXQ Change Mode
			if(pckt_context->recogModeOffUser==0&&pckt_context->Down_Begin_TextRect==1)  
			{				
				//Fwl_Print(C3, M_FWL, "current LCD Point  x :%d  y:%d\r\n",curpt.x,curpt.y);
				//Fwl_Print(C3, M_FWL, "change back to recongize mode !\r\n\n");
				tscr_SetModeOnly(0x00);
				ret = ePH_RET_INKING_COOR;
			}						
		}
		else
		{
			//Inking coor must be in write area
			pckt_context->pt.x = LCDpt.x;
			pckt_context->pt.y = LCDpt.y;		
			ret = ePH_RET_INKING_COOR;
		}					
	}

	return ret;
}


T_VOID tscr_showCurLine(T_POINT endpt)
{
	tscr_showline(&pre_pt , &endpt);
}

static T_VOID  tscr_callback(T_VOID)
{
    T_BOOL GetTsRet;
    T_TSPOINT pt;

    do 
    {
        GetTsRet = ts_get_value(&pt);
        
#ifdef TSCR_DEBUG        
        AK_DEBUG_OUTPUT("Driver data: pt.x == %d, pt.y == %d", pt.x, pt.y);
#endif
       
        if (AK_TRUE == GetTsRet)
        {
#if 0        
			if(pckt_context->recogModeOff == 0)//recongize
	        {
	            if(pckt_context->ts_recongizeCount < TS_RECONGIZE_BUFFER_SIZE - 16)
	            {

	                if(pt.x == TS_HAND_WR_INVALID_COORD && pt.y == 0)//pen up
	                {                   
	                    pckt_context->ts_recongize[pckt_context->ts_recongizeCount++] = STROKEENDMARK;
	                    pckt_context->ts_recongize[pckt_context->ts_recongizeCount++] = 0;
	                
	                }
	                else if(pt.x == TS_HAND_WR_INVALID_COORD && pt.y == TS_HAND_WR_INVALID_COORD)//font end
	                {
	                    pckt_context->ts_recongize[pckt_context->ts_recongizeCount++] = STROKEENDMARK;
	                    pckt_context->ts_recongize[pckt_context->ts_recongizeCount++] = STROKEENDMARK;
	                }
	                else
	                {
	                    pckt_context->ts_recongize[pckt_context->ts_recongizeCount++] = pt.x;
	                    pckt_context->ts_recongize[pckt_context->ts_recongizeCount++] = pt.y;
	                }

	            }
	            else if(pckt_context->ts_recongizeCount < TS_RECONGIZE_BUFFER_SIZE)
	            {
	                Fwl_Print(C3, M_FWL, "recongize buffer full\n");
	                
	                if(pckt_context->ts_recongize[pckt_context->ts_recongizeCount - 2] != STROKEENDMARK)
	                {
	                    pckt_context->ts_recongize[pckt_context->ts_recongizeCount++] = STROKEENDMARK;
						pckt_context->ts_recongize[pckt_context->ts_recongizeCount++] = 0;
						pckt_context->ts_recongize[pckt_context->ts_recongizeCount++] = STROKEENDMARK;
						pckt_context->ts_recongize[pckt_context->ts_recongizeCount++] = STROKEENDMARK;
	                }
	                else if(pckt_context->ts_recongize[pckt_context->ts_recongizeCount - 1] == 0)
	                {
	                    pckt_context->ts_recongize[pckt_context->ts_recongizeCount++] = STROKEENDMARK;
						pckt_context->ts_recongize[pckt_context->ts_recongizeCount++] = STROKEENDMARK;
	                }
	            }
	        }
#endif
	        tscr_handle_point(&pt);
        }
    } while(AK_TRUE == GetTsRet);
}




static T_VOID  tscr_handle_point(T_TSPOINT * tspt )
{
	T_S16 ret;
	T_POINT pt;
	T_POINT clickpt;
	T_TOUCHSCR_ACTION act;

	if (Fwl_TSCR_IsNeedRespond() == AK_FALSE)
	{
		TSCR_ResetPointRecord();
	}
	
	ret = tscr_CheckPt(tspt);

	tscr_GetPoint(&pt);		

	//interrupt process for OfficeViewer
#ifdef SUPPORT_OFFICEVIEWER
	if(WOC_IsActive())
	{
		//if touch the area of exit softkey
		if( pt.x>=160 && pt.x <240 && pt.y>=300 && pt.x<=320)
		{
			if(SM_GetCurrentSM() == eM_s_offln_wml_view)
			{ 
				converter_exit = AK_TRUE;
			}
		}
	}
#endif

	switch(ret)
	{
	case ePH_RET_INKING_COOR:
		tscr_GetPoint(&pt);	
		Fwl_TSCR_SetTouchStatus(AK_TRUE);
		//paint the point at screen
		if((pre_pt.x==0)&&(pre_pt.y==0))
		{
			tscr_GetPoint(&pre_pt);
		}
	
		act = eTOUCHSCR_REFRESH_LINE;
		tscr_data.callback_func(act,pt);

		tscr_GetPoint(&pre_pt);
		break;
	case ePH_RET_COOR_OVER_WRITEAREA:
		pre_pt.x=0;
		pre_pt.y=0;
		Fwl_TSCR_SetTouchStatus(AK_TRUE);
		break;
	case ePH_RET_PEN_MOVE:
		tscr_GetPoint(&clickpt);
		act=eTOUCHSCR_MOVE;
		Fwl_TSCR_SetTouchStatus(AK_TRUE);
		tscr_data.callback_func(act,clickpt);
		break;
	case ePH_RET_PEN_DOWN:
		Fwl_TSCR_SetTouchStatus(AK_TRUE);
		tscr_GetPoint(&clickpt);
		act=eTOUCHSCR_DOWN;
		pre_pt.x=0;
		pre_pt.y=0;
		tscr_data.callback_func(act,clickpt);
		break;
	case ePH_RET_STROKE_OVER:
		pre_pt.x=0;
		pre_pt.y=0;
		Fwl_TSCR_SetTouchStatus(AK_FALSE);
		break;
	case ePH_RET_RECOG_ARRAY:
		pre_pt.x=0;
		pre_pt.y=0;
		//send the Get Words message to EEDIT
		tscr_GetPoint(&clickpt);
		act=eTOUCHSCR_GETWORDS;
		tscr_data.callback_func(act,clickpt);
		break;
	case ePH_RET_PEN_UP:
		tscr_GetPoint(&clickpt);
		//Fwl_Print(C3, M_FWL, "Get Up Event by TSCR\r\n");
		act=eTOUCHSCR_UP;
		Fwl_TSCR_SetTouchStatus(AK_FALSE);
		tscr_data.callback_func(act,clickpt);
		break;
	default:
		break;
	}
   
}

/////////////// static function End  ////////////////////

T_BOOL tscr_HW_init(T_VOID)
{
	T_BOOL ret = AK_TRUE;

	pckt_context = (TSCR_PCKT_CONTEXT *)Fwl_Malloc(sizeof(TSCR_PCKT_CONTEXT));
	AK_ASSERT_PTR(pckt_context, "tscr_HW_init(): malloc error", AK_FALSE);

	Utl_MemSet(pckt_context, 0, sizeof(TSCR_PCKT_CONTEXT));

	//ADC Init        
    gpio_set_pin_as_gpio(GPIO_TSCR_ADC);
    ts_init(E_TS_TYPE_RES,tscr_callback, GPIO_TSCR_ADC, 
		gpio_pin_get_ActiveLevel(GPIO_TSCR_ADC));	
	return ret;
}

T_VOID tscr_HW_free(T_VOID)
{
	pckt_context = Fwl_Free(pckt_context);
}


T_BOOL tscr_set_event_callback(T_fTSCRGET_CALLBACK callback_func)
{
	pre_pt.x=0;
	pre_pt.y=0;
	tscr_data.callback_func = callback_func;
	return AK_TRUE;
}



T_BOOL tscr_SetRecogMode(T_U32 rangeInput)
{
#ifdef HANDWRITE_HW
    range = rangeInput;
    return AK_TRUE;
#endif
#ifdef HANDWRITE_WT
    T_U16   range = 0;
    T_U16   ret = 0;
    range |= rangeInput;

    ret = WTSetRange(range,WT_RAM);
    return ret == 0 ? AK_TRUE : AK_FALSE;
#endif
	return AK_TRUE;
}


T_BOOL tscr_SetMode(T_U8 mode)
{
	if (mode == 0x01)
	{
		pckt_context->recogModeOff = 1;    //In user calling we always need  up down event
		pckt_context->recogModeOffUser=1;
	}
	else
	{
		pckt_context->recogModeOff = 1;	 //In user calling we always need  up down event
		pckt_context->recogModeOffUser=0;
	}

	return AK_TRUE;
}



T_BOOL tscr_SetWriteArea(const T_RECT *rect)
 {
 	pckt_context->WriteAreaRect.left		=	rect->left + 1;
	pckt_context->WriteAreaRect.top		=	rect->top + 1 ;
	pckt_context->WriteAreaRect.width	=	rect->width - 2 ;
	pckt_context->WriteAreaRect.height	=	rect->height - 2;
	return AK_TRUE;
}


 T_BOOL tscr_SetTextRect(const T_RECT *rect)
 {
 	pckt_context->TextRect.left		=	rect->left;
	pckt_context->TextRect.top		=	rect->top;   
	pckt_context->TextRect.width	=	rect->width;
	pckt_context->TextRect.height	=	rect->height ;
	return AK_TRUE;
 	 
 }

T_U8   tscr_getCurADPt(T_pTSPOINT   ADpt)
{
	return ts_get_cur_point(ADpt);
}

T_U8 tscr_getLastDownADPt(T_pTSPOINT   ADpt)
{
	//return ts_get_lastdown_point(ADpt);
	return 0;
}

T_BOOL tscr_GetPoint(T_POINT *pt)
{
	pt->x = pckt_context->pt.x;
	pt->y = pckt_context->pt.y;
	return AK_TRUE;
}

T_BOOL tscr_GETWORDS(T_pTSTR dest)
{
	Utl_MemCpy(dest, pckt_context->gb_hw_words, TSCR_BUFFER_LEN) ;
	return AK_TRUE;
}

T_U8 tscr_getUserMode(T_VOID)
{
	return pckt_context->recogModeOffUser;
}

T_VOID tscr_SetMatrixToDef(T_VOID)
{
#ifdef TOUCH_SCR
	gs.matrixPtr.X[0] =	0;
	gs.matrixPtr.X[1] =	18781;
	gs.matrixPtr.X[2] =	140;
	gs.matrixPtr.X[3] =	-1732578;
	
	gs.matrixPtr.Y[0] =	0;
	gs.matrixPtr.Y[1] =	78;
	gs.matrixPtr.Y[2] =	24553;
	gs.matrixPtr.Y[3] =	-2026813;
#endif
}


#endif

