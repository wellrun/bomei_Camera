/**
* @FILENAME   Lib_geshade.c
* @BRIEF        Graphic Effects
* Copyright (C) 2008 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR 
* @DATE         2008-11-20
* @VERSION    1.0
* @REF            Transplanting from Jupiter, which is a cell phone platform
*/

#include "lib_geapi.h"
#include "lib_image_api.h"
#include "AKSubthread.h"
#include "AKAppMgr.h"
#include "Lib_geshade.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "eng_math.h"
#include "fwl_power.h"
#include "Fwl_osCom.h"
#include "Eng_String.h"
#include "Lib_state.h"
#include "fwl_display.h"
#include "lib_state_api.h"
#include "fwl_graphic.h"

#ifdef SUPPORT_GE_SHADE

#define SHADE_SIZE           3
#define MAX_STEP_SIZE        255
#define STEP_FACTOR          3
#define STEP_INTERVAL_TIME   60        //60ms
#define SLEEP_TIME           2

typedef struct 
{
    T_BOOL          bShadeShow;
    T_BOOL          bSetMode;

    GE_SHADE_SELECT s_shade; 
    
    T_U32           s_cntTimes;  
    T_U32           s_normal_mode;

    T_SHADEMODE     s_effect_mode;
    T_MENUSHOW      *pMenuShow;
}T_GESHADE_INFO;

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
static T_U8 *pEffectshade_rgb888buf = AK_NULL;
static T_U8 *GE_GetRGB888Buf(T_VOID);
#endif

static T_GESHADE_INFO   GeShade;
static T_BOOL           bGeShadeValInit = AK_FALSE;

#define GE_LCD_WIDTH    Fwl_GetLcdWidth()
#define GE_LCD_HEIGHT   Fwl_GetLcdHeight()

static T_VOID GE_ShadeValInit(T_VOID);
static T_VOID GE_NormalShadeShow(T_VOID);
static T_U32  GE_GetShadeSize(T_VOID);
static T_U32  GE_GetSMAttr(T_VOID);

static T_VOID ShadeShow1(T_VOID);
static T_VOID ShadeShow2(T_VOID);
static T_VOID ShadeShow3(T_VOID);
static T_VOID ShadeShow4(T_VOID);
static T_VOID ShadeShow5(T_VOID);
static T_VOID ShadeShow6(T_VOID);

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))

static T_U8 *GE_GetRGB888Buf(T_VOID)
{
	T_U8 *p = AK_NULL;
	T_U8 *q = AK_NULL;
	T_U8 *pRGB565Buf = Fwl_GetDispMemory();
	T_U32 k, counts = Fwl_GetLcdWidth()*Fwl_GetLcdHeight();
	T_U16 temp;

	p = pRGB565Buf;
	q = pEffectshade_rgb888buf;

	for(k=0; k<counts; k++)
	{
		temp = (*p) | (*(p+1)<<8);
		p+=2;

		// 565 to 888
		*q++ = ((temp>>11)<<3);
		*q++ = ((temp>>5)<<2);
		*q++ = (temp<<3);
	}

	return pEffectshade_rgb888buf;

}
#endif

#ifdef OS_WIN32
T_VOID* GM_Malloc(unsigned int size)
#else
T_VOID* GM_Malloc(T_U32 size)
#endif
{
    return Fwl_Malloc(size);
}

T_VOID GM_Free(T_VOID *memblock)
{
    Fwl_Free(memblock);
}

static T_S32 GE_Print(T_pCSTR s, ...)
{
    va_list     args;
    T_S32       len;
    
    va_start(args, s);
    len = Fwl_VPrint(C3, M_GE_LIB, s, args);
    va_end(args); 
    
    return len;
}


/**
 * @brief    GE's effect area is different in different SM,
 * @brief    judge it at this function
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_U32         
 * @retval  0 : refresh the whole screen  
 */
static T_U32  GE_GetSMAttr(T_VOID)
{
    M_STATES    curState;
    T_U32       ret;

    curState = SM_GetCurrentSM();

    switch (curState)
    {
        default:
            ret = 0;
            break;
    }

    return ret;
}


/**
 * @brief   Initialize GeShade's value
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return T_VOID
 */
static T_VOID GE_ShadeValInit(T_VOID)
{
    GeShade.bShadeShow = AK_FALSE;
    GeShade.bSetMode   = AK_FALSE;
    GeShade.s_shade    = EFFECT_SHADE;
    GeShade.s_cntTimes = 0;
    GeShade.s_normal_mode = 0;
    GeShade.pMenuShow = AK_NULL;

    Utl_MemSet(&GeShade.s_effect_mode, 0, sizeof(GeShade.s_effect_mode));

    bGeShadeValInit = AK_TRUE;
}


/**
 * @brief   上下关窗效果
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_VOID
 * @retval  
 */
static T_VOID ShadeShow1(T_VOID)
{
    T_U32 yPos = 0;
    T_U32 time;
    T_U32 vertical;

    while(yPos < (T_U32)GE_LCD_HEIGHT/2)
    {
        time = Fwl_GetTickCount();
        vertical = GE_GetShadeSize() + GeShade.s_cntTimes*STEP_FACTOR;

        // before refresh, must make sure the rect is valid
        if (yPos + vertical > (T_U32)GE_LCD_HEIGHT)
        {
            break;
        }
        
        Fwl_InvalidateRect( 0, (T_POS)yPos, GE_LCD_WIDTH, (T_LEN)vertical);
        Fwl_InvalidateRect( 0, (T_POS)(GE_LCD_HEIGHT - yPos - vertical),
                           GE_LCD_WIDTH,(T_LEN)vertical);

        yPos += vertical;
        GeShade.s_cntTimes++;
        
        while(Fwl_GetTickCount()-time <= STEP_INTERVAL_TIME)
        {
            AK_Sleep(SLEEP_TIME);
        }
    }
    GeShade.bShadeShow = AK_FALSE;
}

/**
 * @brief   上下开窗效果
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_VOID
 * @retval  
 */
static T_VOID ShadeShow2(T_VOID)
{
    T_U32 yPos = 0;
    T_U32 time;
    T_U32 vertical;

    while(yPos < (T_U32)GE_LCD_HEIGHT/2)
    {

        time = Fwl_GetTickCount();

        vertical = GE_GetShadeSize() + GeShade.s_cntTimes*STEP_FACTOR;

        if (vertical + GE_LCD_HEIGHT/2 + yPos > (T_U32)GE_LCD_HEIGHT)
        {            
            break;
        }
        
        Fwl_InvalidateRect( 0, (T_POS)(GE_LCD_HEIGHT/2+yPos), GE_LCD_WIDTH, (T_LEN)vertical);
        Fwl_InvalidateRect( 0, (T_POS)(GE_LCD_HEIGHT/2-yPos), GE_LCD_WIDTH, (T_LEN)vertical);
        
        yPos += vertical;
        GeShade.s_cntTimes++;
        
        while(Fwl_GetTickCount()-time <= STEP_INTERVAL_TIME)
        {
            AK_Sleep(SLEEP_TIME);
        }
    }
    GeShade.bShadeShow = AK_FALSE;
}

/**
 * @brief  左右关窗效果
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_VOID
 * @retval  
 */
static T_VOID ShadeShow3(T_VOID)
{
    T_U32 yPos = 0;
    T_U32 time;    
    T_U32 vertical;

    while(yPos < (T_U32)GE_LCD_WIDTH/2)
    {
        time = Fwl_GetTickCount();
        vertical = GE_GetShadeSize() + GeShade.s_cntTimes*STEP_FACTOR;

        if (yPos + vertical > (T_U32)GE_LCD_WIDTH)
        {
            break;
        }
        
        Fwl_InvalidateRect( (T_POS)yPos, 0, (T_LEN)vertical,GE_LCD_HEIGHT);
        Fwl_InvalidateRect( (T_POS)(GE_LCD_WIDTH - yPos - vertical),
                           0, (T_LEN)vertical, GE_LCD_HEIGHT);
        
        yPos += vertical;
        GeShade.s_cntTimes++;
        
        while(Fwl_GetTickCount()-time <= STEP_INTERVAL_TIME)
        {
            AK_Sleep(SLEEP_TIME);
        }
    }
    GeShade.bShadeShow     = AK_FALSE;
}

/**
 * @brief  左右开窗效果
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_VOID
 * @retval  
 */
static T_VOID ShadeShow4(T_VOID)
{
    T_U32 yPos = 0;
    T_U32 time;    
    T_U32 vertical;

    while(yPos < (T_U32)GE_LCD_WIDTH/2)
    {
        time = Fwl_GetTickCount();
        vertical = GE_GetShadeSize() + GeShade.s_cntTimes*STEP_FACTOR;

        if (GE_LCD_WIDTH/2 + yPos + vertical > (T_U32)GE_LCD_WIDTH)
        {
            break;
        }

        Fwl_InvalidateRect( (T_POS)(GE_LCD_WIDTH/2+yPos), 0,
            (T_LEN)vertical, GE_LCD_HEIGHT);
        Fwl_InvalidateRect( (T_POS)(GE_LCD_WIDTH/2-yPos), 0, 
            (T_LEN)vertical, GE_LCD_HEIGHT);
        
        yPos += vertical;
        GeShade.s_cntTimes++;
        
        while(Fwl_GetTickCount()-time <= STEP_INTERVAL_TIME)
        {
            AK_Sleep(SLEEP_TIME);
        }
    }
    GeShade.bShadeShow = AK_FALSE;
}


/**
 * @brief  从左往右的效果
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_VOID
 * @retval  
 */
static T_VOID ShadeShow5(T_VOID)
{
    T_U32 yPos = GE_GetShadeSize();
    T_U32 time;    

    while(yPos < (T_U32)GE_LCD_WIDTH)
    {
        time = Fwl_GetTickCount();
        
        Fwl_InvalidateRect( 0, 0, (T_LEN)yPos, GE_LCD_HEIGHT);
        yPos += (GE_GetShadeSize() + GeShade.s_cntTimes*STEP_FACTOR);
        GeShade.s_cntTimes++;
        while(Fwl_GetTickCount()-time <= STEP_INTERVAL_TIME)
        {
            AK_Sleep(SLEEP_TIME);
        }
    }
    GeShade.bShadeShow = AK_FALSE;
}

/**
 * @brief  从右往左的效果
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_VOID
 * @retval  
 */
static T_VOID ShadeShow6(T_VOID)
{
    T_U32 yPos = GE_GetShadeSize();
    T_U32 time;
    
    while(yPos < (T_U32)GE_LCD_WIDTH)
    {
        time = Fwl_GetTickCount();
        
        Fwl_InvalidateRect( (T_POS)(GE_LCD_WIDTH-yPos), 0, (T_LEN)yPos, GE_LCD_HEIGHT);
        
        yPos += (GE_GetShadeSize() + GeShade.s_cntTimes*STEP_FACTOR);
        GeShade.s_cntTimes++;
        
        while(Fwl_GetTickCount()-time <= STEP_INTERVAL_TIME)
        {
            AK_Sleep(SLEEP_TIME);
        }

    }
    
    GeShade.bShadeShow = AK_FALSE;
}

T_VOID GE_EffectShadeShow(T_VOID)
{
    T_S32 step;
    T_U32 lcd_RGB;
    GE_PBITMAP dismap = AK_NULL;
    T_U32 mode; 
    T_U32 extra;
    T_U32 direction = 1;
    T_U32 time;
    T_RECT rect;
    T_U32 rand;

    Fwl_Print(C2, M_GE, "Enter GE_EffectShadeShow");


    if(GE_GetSMAttr() == 0)
    {
        RectInit(&rect,0,0,GE_LCD_WIDTH, GE_LCD_HEIGHT);
    }

    lcd_RGB = GeShade.pMenuShow->pBitmapPre->dwWidth * GeShade.pMenuShow->pBitmapPre->dwHeight* 3;

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
    GE_CreateBitmapFromBuffer24(
                                rect.width,
                                rect.height,
                                GE_GetRGB888Buf() +(rect.top*GE_LCD_WIDTH+rect.left)*3,
                                GeShade.pMenuShow->pBitmapOut);
#else
	GE_CreateBitmapFromBuffer24(
                                rect.width,
                                rect.height,
                                Fwl_GetDispMemory()+(rect.top*GE_LCD_WIDTH+rect.left)*3,
                                GeShade.pMenuShow->pBitmapOut);
#endif

    if(GE_SUCCESS != GE_CreateBitmap(rect.width, rect.height, AK_FALSE, &dismap))
    {
        Fwl_Print(C2, M_GE, "dismap create error");
        goto g_quit;
    }


    if (GeShade.bSetMode)
    {
        mode = GeShade.s_effect_mode.mode;
        extra = GeShade.s_effect_mode.extra;
        direction = GeShade.s_effect_mode.direction;
    }
    else
    {
        Fwl_RandSeed();
        rand = Fwl_GetRand(7);
    
        switch(rand)
        {
            case 0:
                mode = GE_EFFECT_MODE_ALPHA;
                extra = GE_EFFECT_MODE_EXTRA_NONE;
                break;
            case 1:
                mode = GE_EFFECT_MODE_SLIDE_COVER;
                extra = Fwl_GetRand(4) + 1;
                break;
            case 2:
                mode = GE_EFFECT_MODE_WINDOWBLIND;
                extra = Fwl_GetRand(2) + 5;
                break;
            case 3:
                //mode = GE_EFFECT_MODE_DIAMOND;棱形扩展图像库没做
                //extra = GE_EFFECT_MODE_EXTRA_NONE;棱形扩展图像库没做
                mode = GE_EFFECT_MODE_SLIDE_PULL;
                extra = Fwl_GetRand(4) + 1;
                break;
            case 4:
                mode = GE_EFFECT_MODE_POPUP_SCALE;
                extra = Fwl_GetRand(2) + 7;
                break;
            case 5:
                mode = GE_EFFECT_MODE_PLANE_TURN;
                extra = Fwl_GetRand(2) + 5;
                break;
            case 6:
                mode = GE_EFFECT_MODE_CUBIC_TURN;
                extra = Fwl_GetRand(2) + 5;
                direction = Fwl_GetRand(2);
                break;
            default:
                mode = GE_EFFECT_MODE_ALPHA;
                extra = GE_EFFECT_MODE_EXTRA_NONE;
                break;
        }
    }//  end    if (GeShade.bSetMode)
    Fwl_Print(C3, M_GE, "mode = %d,extra = %d, direction = %d",mode,extra,direction);

    if(1 == direction)
    {
        step = GE_GetShadeSize();
        while(step <= MAX_STEP_SIZE)
        {    
            time = Fwl_GetTickCount();
            if (GE_SUCCESS == 
            GE_BrowseEffect(GeShade.pMenuShow->pBitmapPre, GeShade.pMenuShow->pBitmapOut, dismap, step, mode, extra))
            {   
            #if (defined (LCD_MODE_565) && defined (OS_ANYKA))
            	Fwl_RGB888toRGB565(Fwl_GetDispMemory()+(rect.top*GE_LCD_WIDTH+rect.left)*2, dismap->pData, GeShade.pMenuShow->pBitmapPre->dwWidth, GeShade.pMenuShow->pBitmapPre->dwHeight);
			#else				
				Utl_MemCpy(Fwl_GetDispMemory()+(rect.top*GE_LCD_WIDTH+rect.left)*3, dismap->pData, lcd_RGB);
			#endif
                Fwl_RefreshDisplay();
            }
            else
            {
                
                Fwl_Print(C2, M_GE, "GE_BrowseEffect error 1!");
                goto g_quit;
            }

            step += (GE_GetShadeSize() + GeShade.s_cntTimes*STEP_FACTOR);
            GeShade.s_cntTimes++;

            while(Fwl_GetTickCount()-time <= STEP_INTERVAL_TIME)
            {
                AK_Sleep(SLEEP_TIME);
            }
            
        }
    }
    else
    {
        step = MAX_STEP_SIZE;
        
        while(step > 0)
        {
            time = Fwl_GetTickCount();
            if (GE_SUCCESS == 
            GE_BrowseEffect(GeShade.pMenuShow->pBitmapOut, GeShade.pMenuShow->pBitmapPre, dismap, step, mode, extra))
            {                
            #if (defined (LCD_MODE_565) && defined (OS_ANYKA))
				Fwl_RGB888toRGB565(Fwl_GetDispMemory()+(rect.top*GE_LCD_WIDTH+rect.left)*2, dismap->pData, GeShade.pMenuShow->pBitmapPre->dwWidth, GeShade.pMenuShow->pBitmapPre->dwHeight);
			#else
				Utl_MemCpy(Fwl_GetDispMemory()+(rect.top*GE_LCD_WIDTH+rect.left)*3, dismap->pData, lcd_RGB);
			#endif
                Fwl_RefreshDisplay();
            }
            else
            {
                Fwl_Print(C2, M_GE, "GE_BrowseEffect error 2!");
                goto g_quit;
            }

            step -= (GE_GetShadeSize() + GeShade.s_cntTimes*STEP_FACTOR);
            GeShade.s_cntTimes++;
            while(Fwl_GetTickCount()-time <= STEP_INTERVAL_TIME)
            {
                AK_Sleep(SLEEP_TIME);
            }
        }
    }

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
	Fwl_RGB888toRGB565(Fwl_GetDispMemory()+(rect.top*GE_LCD_WIDTH+rect.left)*2, GeShade.pMenuShow->pBitmapOut->pData, GeShade.pMenuShow->pBitmapPre->dwWidth, GeShade.pMenuShow->pBitmapPre->dwHeight);
#else
	Utl_MemCpy(Fwl_GetDispMemory()+(rect.top*GE_LCD_WIDTH+rect.left)*3, GeShade.pMenuShow->pBitmapOut->pData, lcd_RGB);
#endif
    Fwl_InvalidateRect(0,0,0,0);
    
g_quit:
    GE_DestroyBitmap(&dismap);
	
    if (GeShade.pMenuShow != AK_NULL)
	{
	    GE_DestroyBitmap(&GeShade.pMenuShow->pBitmapPre);
	    GE_DestroyBitmap(&GeShade.pMenuShow->pBitmapOut);

        GeShade.pMenuShow = Fwl_Free(GeShade.pMenuShow);
    }
    
    Fwl_Print(C2, M_GE, "Leave GE_ShadeShow, the mode=%d, the extra=%d", mode, extra);

    GeShade.bShadeShow = AK_FALSE;
    GeShade.bSetMode   = AK_FALSE;
}

/**
 * @brief   The effect show,  which don't use GE lib
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_VOID
 * @retval  
 */
static T_VOID GE_NormalShadeShow(T_VOID)
{
    T_U32 mode;

    if (GeShade.bSetMode)
    {
        mode = GeShade.s_normal_mode;
    }
    else
    {
        Fwl_RandSeed();
        mode = Fwl_GetRand(6) + 1;
    }
    
    switch(mode)
    {
        case 1:
            ShadeShow1();
            break;
        case 2:
            ShadeShow2();
            break;
        case 3:
            ShadeShow3();
            break;
        case 4:
            ShadeShow4();
            break;
        case 5:
            ShadeShow5();
            break;
        case 6:
            ShadeShow6();
            break;
        default:
            ShadeShow1();
            break;
    }

    GE_ShadeFree();
}

/**
 * @brief   The subthread's callback function to 
 * @          show the effect
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_VOID
 * @retval  
 */
T_VOID GE_ShadeShow(T_VOID *param)
{
    GE_SHADE_SELECT mode;	
    
    Fwl_Print(C2, M_GE, "Enter GE_ShadeShow");


    IThread_Suspend(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MMI));

    GeShade.s_cntTimes = 0;

    if (GeShade.bSetMode)
    {
        mode = GeShade.s_shade;
    }
    else
    {
	    Fwl_RandSeed();
	    if (Fwl_GetRand(5) == 0)
        {
            mode = NORMAL_SHADE;
        }   
        else
        {
            mode = EFFECT_SHADE;
        }
    }
    
	if (mode == NORMAL_SHADE)
	{
		GE_NormalShadeShow();
	}
	else
	{
		GE_EffectShadeShow();
	}
    
    IThread_Run(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MMI));
    
}

/**
 * @brief   The subthread's callback function of abnormity quit 
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_VOID
 * @retval  
 */
T_VOID GE_ShadeShowAbortEntry(T_VOID *param)
{
    Fwl_Print(C2, M_GE, "Enter GE_ShadeShowAbortEntry");


	if (GeShade.pMenuShow != AK_NULL)
	{
	    GE_DestroyBitmap(&GeShade.pMenuShow->pBitmapPre);
	    GE_DestroyBitmap(&GeShade.pMenuShow->pBitmapOut);

        GeShade.pMenuShow = Fwl_Free(GeShade.pMenuShow);
    }

    IThread_Run(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MMI));
}

/**
 * @brief    the speed of geshade 
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_U32
 * @retval  
 */
static T_U32 GE_GetShadeSize(T_VOID)
{
    T_U32    shadeSize = SHADE_SIZE;

    switch(gs.AniSwitchLevel)
    {
        case eFastLevel:
            shadeSize = 70;
            break;
        case eMiddleLevel:
            shadeSize = 35;
            break;
        default:
            break;
    }
    
    return shadeSize;
}
#endif

/**
 * @brief   Initialize GE_SHADE
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return T_U32
 * @retval  GE_SUCCESS: Initialize OK
 * @retval  1: GE_SetCallback failed
 * @retval  2: GE_Initialize failed
 * @retval  3: get pre pic failed
 * @retval  4: system close animate
 */
GE_RESULT GE_ShadeInit(T_VOID)
{
#ifdef SUPPORT_GE_SHADE
    T_RECT  rect;
    GE_CALLBACK_FUNCS pcbf;
    
    if(eClose == gs.AniSwitchLevel) 
    {
        return 4;  
    } 
    else if ((eClose != gs.AniSwitchLevel) && (GE_GetSMAttr() == 0))
    {
        RectInit(&rect,0,0,GE_LCD_WIDTH, GE_LCD_HEIGHT);
    }

    if (!bGeShadeValInit)
    {
        GE_ShadeValInit();    
    }

    if (GeShade.pMenuShow == AK_NULL)
    {
        GeShade.pMenuShow = (T_MENUSHOW *)Fwl_Malloc(sizeof(T_MENUSHOW));
		
		if (GeShade.pMenuShow == AK_NULL)
		{
			return 5;
		}
    }
    else
    {
        GE_DestroyBitmap(&GeShade.pMenuShow->pBitmapPre);
		GE_DestroyBitmap(&GeShade.pMenuShow->pBitmapOut);
    }

    Utl_MemSet(GeShade.pMenuShow, 0, sizeof(T_MENUSHOW));
	
    GeShade.bShadeShow = AK_TRUE;

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
	if (AK_NULL == pEffectshade_rgb888buf)
	{
		pEffectshade_rgb888buf = (T_U8 *)Fwl_Malloc(Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*3);

		if (AK_NULL == pEffectshade_rgb888buf)
		{
			Fwl_Print(C2, M_GE, "pEffectshade_rgb888buf malloc fail!");
			return 5;
		}

		memset(pEffectshade_rgb888buf, 0, Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*3);
	}
#endif

    pcbf.malloc = GM_Malloc;
    pcbf.free   = GM_Free;
    pcbf.memcpy = memcpy;
    pcbf.memset = memset;
    pcbf.printf = GE_Print;
    pcbf.Img_ImageToBmp = Img_ImageToBmp;
    pcbf.Img_ImageType  = Img_ImageType;
	pcbf.Img_WBMPInfo = Img_WBMPInfo;
	pcbf.Img_JpegInfo = Img_JpegInfo;
	pcbf.Img_PNGInfo = Img_PNGInfo;
	pcbf.Img_GIFInfo = Img_GIFInfo;

    if (GE_SUCCESS != GE_SetCallback(&pcbf))
    {
        return 1;
    }

    Fwl_Print(C3, M_GE, "Geshade init....");
    
    if (GE_SUCCESS == GE_Initialize())
    {
    	T_S32 ret = GE_S_FALSE;

		if(GE_SUCCESS != GE_CreateBitmap(rect.width, rect.height, AK_FALSE, &(GeShade.pMenuShow->pBitmapPre)))
	    {
	    	GeShade.pMenuShow = Fwl_Free(GeShade.pMenuShow);
	        Fwl_Print(C2, M_GE, "pBitmapPre create error");
	        return 6;
	    }

		if(GE_SUCCESS != GE_CreateBitmap(rect.width, rect.height, AK_FALSE, &(GeShade.pMenuShow->pBitmapOut)))
	    {
	    	GE_DestroyBitmap(&GeShade.pMenuShow->pBitmapPre);
	    	GeShade.pMenuShow = Fwl_Free(GeShade.pMenuShow);
	        Fwl_Print(C2, M_GE, "pBitmapOut create error");
	        return 7;
	    }
		
        // get content from lcd buffer
    #if (defined (LCD_MODE_565) && defined (OS_ANYKA))
        ret = GE_CreateBitmapFromBuffer24(
                                rect.width,
                                rect.height,
                                GE_GetRGB888Buf() +(rect.top*Fwl_GetLcdWidth()+rect.left)*3,
                                GeShade.pMenuShow->pBitmapPre);
    #else
		ret = GE_CreateBitmapFromBuffer24(
                                rect.width,
                                rect.height,
                                Fwl_GetDispMemory() +(rect.top*Fwl_GetLcdWidth()+rect.left)*3,
                                GeShade.pMenuShow->pBitmapPre);
	#endif


		if (GE_SUCCESS == ret)
		{
			Fwl_Print(C3, M_GE, "get pre pic");
		}
		else
		{
			Fwl_Print(C2, M_GE, "error create bitmap ret = 0x%x", ret);
            return 3;
		}
    }
    else
    {
        Fwl_Print(C2, M_GE, "GE init error!");
        
        return 2;
    }
#endif
//    Fwl_Print(C3, M_GE, "GE_ShadeInit,GeShade.bShadeShow= %d,GeShade.pMenuShow=%d",GeShade.bShadeShow,GeShade.pMenuShow);
    return GE_SUCCESS;
}


/**
 * @brief   Starting the shade
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_VOID
 * @retval  
 */
T_VOID GE_StartShade(T_VOID)
{
#ifdef SUPPORT_GE_SHADE
    ISubThread    *pShowSubThread = AK_NULL;
    
    if(eClose == gs.AniSwitchLevel)
    {
        return;
    }
//    Fwl_Print(C3, M_GE, "GeShade.bShadeShow= %d,GeShade.pMenuShow=%d",GeShade.bShadeShow,GeShade.pMenuShow);
    
    if (GeShade.bShadeShow && (GeShade.pMenuShow != AK_NULL))
    {
        T_SUBTHREAD_INITPARAM   param;

//        FreqMgr_StateCheckIn(FREQ_MOST_NEEDED, FREQ_PRIOR_HIGH);
        Fwl_Print(C2, M_GE, "Enter Create paint sub thread");
        
        param.pcszName        = "Show";
        param.byPriority      = 90;
        param.ulTimeSlice     = 1;
        param.ulStackSize     = 5*1024 ;
        param.wMainThreadCls  = AKAPP_CLSID_MMI ;
        param.pUserData       = GeShade.pMenuShow;
        param.fnEntry         = GE_ShadeShow;
        param.fnAbort         = GE_ShadeShowAbortEntry; 

        if(AK_SUCCESS != CSubThread_New(&pShowSubThread, &param, AK_TRUE))
        {
           Fwl_Print(C2, M_GE, "Create paint sub thread fail");
        }

       AK_RELEASEIF(pShowSubThread);
//       FreqMgr_StateCheckOut(FREQ_MOST_NEEDED);
    }
#endif    
}


/**
 * @brief   
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return T_VOID
 * @retval  
 */
T_VOID GE_ShadeCancel(T_VOID)
{
#ifdef SUPPORT_GE_SHADE
    GeShade.bShadeShow = AK_FALSE;

    GE_ShadeFree();
#endif    
}

/**
 * @brief    setting the mode of normal shade 
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_U32
 * @retval  
 */
T_BOOL GE_SetNormalShade(T_U32 mode)
{
#ifdef SUPPORT_GE_SHADE
    if (!GeShade.bShadeShow)
    {
        return AK_FALSE;
    }

    if ((mode < 1) || (mode > 6))
    {
        return AK_FALSE;
    }
        
    GeShade.bSetMode = AK_TRUE;
    GeShade.s_shade  = NORMAL_SHADE;
    GeShade.s_normal_mode = mode;
#endif

    return AK_TRUE;
}

/**
 * @brief    setting the mode of effect shade
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_U32
 * @retval  
 */
T_BOOL GE_SetEffectShade(T_SHADEMODE mode)
{
#ifdef SUPPORT_GE_SHADE
    if (!GeShade.bShadeShow)
    {
        return AK_FALSE;
    }

    if ((mode.extra < GE_EFFECT_MODE_EXTRA_NONE) 
        || (mode.extra >= GE_EFFECT_MODE_EXTRA_EDGE_TO_CENTER))
    {
        return AK_FALSE;
    }


    if ((mode.mode < 0) 
        || (mode.mode >= GE_EFFECT_MODE_CUBIC_TURN))
    {
        return AK_FALSE;
    }
    
    GeShade.bSetMode = AK_TRUE;
    GeShade.s_shade  = EFFECT_SHADE;

    GeShade.s_effect_mode.mode      = mode.mode;
    GeShade.s_effect_mode.extra     = mode.extra;
    GeShade.s_effect_mode.direction = mode.direction;
#endif

    return AK_TRUE;
}

/**
 * @brief    Realeasing the buffer of GeShade and GeShade.pMenuShow
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return  T_VOID
 * @retval  
 */
T_VOID GE_ShadeFree(T_VOID)
{
#ifdef SUPPORT_GE_SHADE
    if (GeShade.pMenuShow != AK_NULL)
    {        
        GE_DestroyBitmap(&GeShade.pMenuShow->pBitmapPre);
		GE_DestroyBitmap(&GeShade.pMenuShow->pBitmapOut);
        GeShade.pMenuShow = Fwl_Free(GeShade.pMenuShow);
    }

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
	if (AK_NULL != pEffectshade_rgb888buf)
	{
		pEffectshade_rgb888buf = Fwl_Free(pEffectshade_rgb888buf);
	}
#endif
#endif    
}

T_BOOL GE_SetAniSwitchLevel(T_eAniMenuLevelType level)
{
#ifdef SUPPORT_GE_SHADE
    if (gs.AniSwitchLevel != level)
    {
        gs.AniSwitchLevel = level;

        if (level == eClose)
            GE_ShadeFree();
    }
 
#endif

    return AK_TRUE;
}

T_eAniMenuLevelType GE_GetAniSwitchLevel(T_VOID)
{
#ifdef SUPPORT_GE_SHADE
    return gs.AniSwitchLevel;
#else
    return eClose;
#endif
}
// end of file

T_BOOL GE_GetShadeShowFlag(T_VOID)
{
#ifdef SUPPORT_GE_SHADE
    return GeShade.bShadeShow;
#else
    return AK_FALSE;
#endif
}

