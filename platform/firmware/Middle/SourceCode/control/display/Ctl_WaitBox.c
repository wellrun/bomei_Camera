
#include "anyka_types.h"
#include "Fwl_public.h"
#include "hal_timer.h"
#include "Eng_dataconvert.h"
#include "Eng_dynamicfont.h"
#include "Eng_font.h"
#include "ctl_waitbox.h"
#include "fwl_oscom.h"
#include "Eng_AkBmp.h"
#include "Eng_String_UC.h"
#include "fwl_pfdisplay.h"
#include "AKAppMgr.h"
#include "fwl_display.h"



#define PROGRESS_BLOCK_NUM_MAX      16
#define PROGRESS_BLOCK_WIDTH        7
#define PROGRESS_BLOCK_HEIGHT       9
#define PROGRESS_BLOCK_INTERVAL     2

#define WAIT_PAINT_INTERVAL         500     // millisecond

#if (LCD_CONFIG_WIDTH == 800)
#define WAIT_TITLE_OFFSET            17
#define WAIT_TOP_OFFSET              65
#define WAIT_LEFT_OFFSET             15
#define WAIT_CLOCK_OFFSET            21
#else
#if (LCD_CONFIG_WIDTH == 480)
#define WAIT_TITLE_OFFSET            10
#define WAIT_TOP_OFFSET              37
#define WAIT_LEFT_OFFSET             9
#define WAIT_CLOCK_OFFSET            12
#else
#if (LCD_CONFIG_WIDTH == 320)
#define WAIT_TITLE_OFFSET            10
#define WAIT_TOP_OFFSET              37
#define WAIT_LEFT_OFFSET             9
#define WAIT_CLOCK_OFFSET            12

#else
#error "LCD no match!"
#endif
#endif
#endif


static T_VOID WaitBox_Timer_Callback(T_TIMER timer_id, T_U32 delay);
static T_VOID WaitBox_Clock_Show(T_VOID);
static T_VOID WaitBox_Progress_Show(T_VOID);
static T_VOID WaitBox_Rainbow_Show(T_VOID);
static T_VOID WaitBox_GetRes(T_eWAITBOX_MODE mode);
static T_VOID WaitBox_FreeRes(T_VOID);
static T_VOID WaitBox_Rainbow_SetTitle(T_pWSTR title);
static T_VOID WaitBox_Progress_SetTitle(T_pWSTR title);
static T_VOID WaitBox_Clock_SetTitle(T_pWSTR title);
static T_VOID WaitBox_SetTitle(T_pWSTR title);


typedef struct{
    T_TIMER         timer;
    T_eWAITBOX_MODE mode;
    
    T_U32           prg_percent;       // finished percent in progress mode
    T_RECT          prg_range;
    T_pWSTR         pPrgTitle;      
    T_pDATA			pPrgBackground;
    T_pDATA			pPrgBlock;
    
    T_RECT          clock_range;
    T_pDATA			pClockBackground;
    T_pDATA			pClock[8];
    
    T_pCWSTR        pRainbowTitle;
    T_RECT          rainbow_range;
    T_pDATA			pRainbowBackground;
    T_pDATA			pRainbow[6];

    T_hSemaphore	semaphore;
}T_WAITBOX;

static T_WAITBOX *pWaitbox = AK_NULL;

/**
 * @brief Create waiting-box control and initialize 
 *
 * @author Zhengwenbo
 * @date 2008-01-24
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID WaitBox_Init(T_VOID)
{
    T_LEN width = 0;
    T_LEN height = 0;
    T_U8  deep = 0;
    
    pWaitbox = (T_WAITBOX*)Fwl_Malloc(sizeof(T_WAITBOX));
    AK_ASSERT_PTR_VOID(pWaitbox, "WaitBox_Init(): pWaitbox malloc error");
    memset(pWaitbox, 0, sizeof(T_WAITBOX));

    pWaitbox->timer = ERROR_TIMER;
    pWaitbox->mode = WAITBOX_CLOCK;
    pWaitbox->semaphore = AK_Create_Semaphore(1, AK_PRIORITY);

    WaitBox_GetRes(WAITBOX_ALL);

    // progress
    AKBmpGetInfo(pWaitbox->pPrgBackground, &width, &height, &deep);
    pWaitbox->prg_range.width = width;
    pWaitbox->prg_range.height = height;    
    pWaitbox->prg_range.left = (Fwl_GetLcdWidth() - width) >> 1;
    pWaitbox->prg_range.top = (Fwl_GetLcdHeight() - height) >> 1;

    // clock
    AKBmpGetInfo(pWaitbox->pClockBackground, &width, &height, &deep);
    pWaitbox->clock_range.width = width;
    pWaitbox->clock_range.height = height;    
    pWaitbox->clock_range.left = (Fwl_GetLcdWidth() - width) >> 1;
    pWaitbox->clock_range.top = (Fwl_GetLcdHeight() - height) >> 1;

    // rainbow
    AKBmpGetInfo(pWaitbox->pRainbowBackground, &width, &height, &deep);
    pWaitbox->rainbow_range.width = width;
    pWaitbox->rainbow_range.height = height;
    pWaitbox->rainbow_range.left = (Fwl_GetLcdWidth() - width) >> 1;
    pWaitbox->rainbow_range.top = (Fwl_GetLcdHeight() - height) >> 1;

    WaitBox_FreeRes();
}


static T_VOID WaitBox_FreeRes(T_VOID)
{
    T_U32 i = 0;

    // progress 
    if (AK_NULL != pWaitbox->pPrgBackground)
    {
    	pWaitbox->pPrgBackground = Fwl_Free(pWaitbox->pPrgBackground);
    }

    if (AK_NULL != pWaitbox->pPrgBlock)
    {
    	pWaitbox->pPrgBlock = Fwl_Free(pWaitbox->pPrgBlock);
	}

    // clock
    if (AK_NULL != pWaitbox->pClockBackground)
    {
    	pWaitbox->pClockBackground = Fwl_Free(pWaitbox->pClockBackground);
    }
    
    for (i = 0; i<8; i++)
    {
    	if (AK_NULL != pWaitbox->pClock[i])
	    {
	        pWaitbox->pClock[i] = Fwl_Free(pWaitbox->pClock[i]);
        }
    }

    // rainbow
    if (AK_NULL != pWaitbox->pRainbowBackground)
    {
    	pWaitbox->pRainbowBackground = Fwl_Free(pWaitbox->pRainbowBackground);
    }
    
    for (i=0; i<6; i++)
    {
    	if (AK_NULL != pWaitbox->pRainbow[i])
	    {
        	pWaitbox->pRainbow[i] = Fwl_Free(pWaitbox->pRainbow[i]);
        }
    }
}

/**
 * @brief Get resource of waiting box 
 *
 * @author Zhengwenbo
 * @date 2008-01-24
 * @param T_eWAITBOX_MODE mode
 * @return T_VOID
 * @retval
 */
static T_VOID WaitBox_GetRes(T_eWAITBOX_MODE mode)
{
    T_U32 len = 0;
    T_U32 i = 0;

    // progress 
    if ((WAITBOX_PROGRESS == mode) || (WAITBOX_ALL == mode))
    {
	    if (AK_NULL == pWaitbox->pPrgBackground)
	    {
	    	pWaitbox->pPrgBackground = Res_StaticLoad(AK_NULL, eRES_BMP_WAIT_PROGRESS_BACKGROUND, &len);
	    }

	    if (AK_NULL == pWaitbox->pPrgBlock)
	    {
	    	pWaitbox->pPrgBlock = Res_StaticLoad(AK_NULL, eRES_BMP_WAIT_PROGRESS_BLOCK, &len);
		}
	}

    // clock
    if ((WAITBOX_CLOCK == mode) || (WAITBOX_ALL == mode))
    {
	    if (AK_NULL == pWaitbox->pClockBackground)
	    {
	    	pWaitbox->pClockBackground = Res_StaticLoad(AK_NULL, eRES_BMP_WAIT_CLOCK_BACKGROUND, &len);
	    }
	    
	    for (i = 0; i<8; i++)
	    {
	    	if (AK_NULL == pWaitbox->pClock[i])
		    {
		        pWaitbox->pClock[i] = Res_StaticLoad(AK_NULL, eRES_BMP_WAIT_CLOCK0 + i, &len);
	        }
	    }
    }

    // rainbow
    if ((WAITBOX_RAINBOW == mode) || (WAITBOX_ALL == mode))
    {
	    if (AK_NULL == pWaitbox->pRainbowBackground)
	    {
	    	pWaitbox->pRainbowBackground = Res_StaticLoad(AK_NULL, eRES_BMP_WAIT_RAINBOW_BACKGROUND, &len);
	    }
	    
	    for (i=0; i<6; i++)
	    {
	    	if (AK_NULL == pWaitbox->pRainbow[i])
		    {
	        	pWaitbox->pRainbow[i] = Res_StaticLoad(AK_NULL, eRES_BMP_WAIT_RAINBOW0 + i, &len);
	        }
	    }
    }
}

/**
 * @brief free waiting-box control
 *
 * @author Zhengwenbo
 * @date 2008-01-24
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID WaitBox_Free(T_VOID)
{
    if (AK_NULL != pWaitbox)
    {
    	AK_Delete_Semaphore(pWaitbox->semaphore);
        pWaitbox = Fwl_Free(pWaitbox);
        pWaitbox = AK_NULL;
    }
}

/**
 * @brief start wait control
 *
 * @author Zhengwenbo
 * @date 2008-01-24
 * @param T_eWAITBOX_MODE mode  T_pWSTR title
 * @return T_VOID
 * @retval
 */
T_VOID WaitBox_Start(T_eWAITBOX_MODE mode, T_pWSTR title)
{
    if (AK_NULL == pWaitbox)
    {
        return;
    }

    AK_Obtain_Semaphore(pWaitbox->semaphore, AK_SUSPEND);
    WaitBox_GetRes(mode);

    pWaitbox->mode = mode;

    WaitBox_SetTitle(title);

    if (ERROR_TIMER != pWaitbox->timer)
    {
        pWaitbox->timer = Fwl_StopTimer(pWaitbox->timer);
    }
    
    pWaitbox->timer = Fwl_SetMSTimerWithCallback(WAIT_PAINT_INTERVAL, AK_TRUE, WaitBox_Timer_Callback);
    
    switch (pWaitbox->mode)
    {
        case WAITBOX_CLOCK:
            WaitBox_Clock_Show();
            break;
            
        case WAITBOX_PROGRESS:            
            pWaitbox->prg_percent = 0;
            WaitBox_Progress_Show();  
            break;
            
        case WAITBOX_RAINBOW:
            WaitBox_Rainbow_Show();
            break;
            
        default:
            Fwl_Print(C3, M_CTRL, "WaitBox_Start():bad mode\n");
            break;
    }
    AK_Release_Semaphore(pWaitbox->semaphore);
}

/**
 * @brief stop wait box timer to kill waiting box
 *
 * @author Zhengwenbo
 * @date 2008-01-24
 * @param T_VOID
 * @return T_BOOL
 * @retval 1:stop successly  0:stop fail
 */
T_BOOL WaitBox_Stop(T_VOID)
{
	if (AK_NULL == pWaitbox)
    {
        return AK_FALSE;
    }
    
	AK_Obtain_Semaphore(pWaitbox->semaphore, AK_SUSPEND);
	
    if (ERROR_TIMER != pWaitbox->timer)
    {
        pWaitbox->timer = Fwl_StopTimer(pWaitbox->timer);
        pWaitbox->timer = ERROR_TIMER;

        VME_ReTriggerEvent((vT_EvtSubCode)VME_EVT_PAINT, (vUINT32)AK_NULL);

        WaitBox_FreeRes();
    	AK_Release_Semaphore(pWaitbox->semaphore);
        return AK_TRUE;
    }

    WaitBox_FreeRes();
	AK_Release_Semaphore(pWaitbox->semaphore);
    return AK_FALSE;
}

/**
 * @brief set the title of clock
 *
 * @author Zhengwenbo
 * @date 2008-05-08
 * @param T_pWSTR title: unicode string
 * @return T_VOID
 * @retval
 */
static T_VOID WaitBox_Clock_SetTitle(T_pWSTR title)
{
    // set value of the title member
}


/**
 * @brief show waiting box of clock mode
 *
 * @author Zhengwenbo
 * @date 2008-01-24
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
static T_VOID WaitBox_Clock_Show(T_VOID)
{
    static T_U32 index = 0;
    T_POS x = 0;
    T_POS y = 0;

    // draw background
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pWaitbox->clock_range.left, pWaitbox->clock_range.top, \
                        pWaitbox->pClockBackground, &g_Graph.TransColor, AK_FALSE);

    // draw clock
    x = pWaitbox->clock_range.left + WAIT_LEFT_OFFSET;
    y = pWaitbox->clock_range.top + WAIT_CLOCK_OFFSET;
    Fwl_AkBmpDrawFromString(HRGB_LAYER, x, y, pWaitbox->pClock[(index++) % 8], &g_Graph.TransColor, AK_FALSE);

    Fwl_InvalidateRect( pWaitbox->clock_range.left, pWaitbox->clock_range.top, \
                        pWaitbox->clock_range.width, pWaitbox->clock_range.height);
}

/**
 * @brief show waiting box of progress mode
 *
 * @author Zhengwenbo
 * @date 2008-01-24
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
static T_VOID WaitBox_Progress_Show(T_VOID)
{
    T_POS x, y;
    T_U32 width = 0;
    T_U32 i = 0;
    T_U32 block_num = 0;

    /**draw background*/
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pWaitbox->prg_range.left, pWaitbox->prg_range.top, pWaitbox->pPrgBackground, &g_Graph.TransColor, AK_FALSE);

    /**print title*/
    if (AK_NULL != pWaitbox->pPrgTitle)
    {
        width = UGetSpeciStringWidth((T_U16 *)pWaitbox->pPrgTitle, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pWaitbox->pPrgTitle));
        x = pWaitbox->prg_range.left + (T_LEN)((pWaitbox->prg_range.width - width) >> 1);
        y = pWaitbox->prg_range.top + WAIT_TITLE_OFFSET;

        Fwl_UDispSpeciString(HRGB_LAYER, x, y, (T_U16 *)pWaitbox->pPrgTitle, COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pWaitbox->pPrgTitle));
	}

    /**draw progress block*/
    if (pWaitbox->prg_percent > 0)
    {
        Fwl_Print(C3, M_CTRL, "debug token: WaitBox_Progress_DrawBar():percent = %d", pWaitbox->prg_percent);
        block_num = (T_U32)(PROGRESS_BLOCK_NUM_MAX * ((float)pWaitbox->prg_percent / 100));
        Fwl_Print(C3, M_CTRL, "debug token: WaitBox_Progress_DrawBar():block_num = %d", block_num);
        
        y = pWaitbox->prg_range.top + WAIT_TOP_OFFSET;
        for (i=0; i<block_num; i++)
        {
            x = pWaitbox->prg_range.left + WAIT_LEFT_OFFSET + (T_LEN)((PROGRESS_BLOCK_WIDTH + PROGRESS_BLOCK_INTERVAL) * i);
        
            Fwl_AkBmpDrawFromString(HRGB_LAYER, x, y, pWaitbox->pPrgBlock, &g_Graph.TransColor, AK_FALSE);
        }
    }


    Fwl_InvalidateRect( pWaitbox->prg_range.left, pWaitbox->prg_range.top, pWaitbox->prg_range.width, pWaitbox->prg_range.height);
}

/**
 * @brief set the title of progress
 *
 * @author Zhengwenbo
 * @date 2008-01-24
 * @param T_pWSTR title: unicode string
 * @return T_VOID
 * @retval
 */
static T_VOID WaitBox_Progress_SetTitle(T_pWSTR title)
{
    pWaitbox->pPrgTitle = title; 
}

/**
 * @brief set the percent of progress
 *
 * @author Zhengwenbo
 * @date 2008-01-24
 * @param T_U32 percent: finished percent, total is 100, its value shout be between 0 and 100
 * @return T_VOID
 * @retval
 */
T_VOID WaitBox_Progress_SetPercent(T_U32 percent)
{
    pWaitbox->prg_percent = percent;
}

/**
 * @brief set the title of rainbow
 *
 * @author Zhengwenbo
 * @date 2008-05-05
 * @param T_pWSTR title: unicode string
 * @return T_VOID
 * @retval
 */
static T_VOID WaitBox_Rainbow_SetTitle(T_pWSTR title)
{
    pWaitbox->pRainbowTitle = title; 
}

/**
 * @brief show rainbow
 *
 * @author Zhengwenbo
 * @date 2008-05-05
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
static T_VOID WaitBox_Rainbow_Show(T_VOID)
{
    T_POS x = 0;
    T_POS y = 0;
    static T_U32 index = 0;
    T_U32 width = 0;

    // draw background
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pWaitbox->rainbow_range.left, pWaitbox->rainbow_range.top, \
                        pWaitbox->pRainbowBackground, &g_Graph.TransColor, AK_FALSE);

    //print title
    if (AK_NULL != pWaitbox->pRainbowTitle)
    {
        width = UGetSpeciStringWidth((T_U16 *)pWaitbox->pRainbowTitle, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pWaitbox->pRainbowTitle));
        x = pWaitbox->rainbow_range.left + (T_LEN)((pWaitbox->rainbow_range.width - width) >> 1);
        y = pWaitbox->rainbow_range.top + WAIT_TITLE_OFFSET;

        Fwl_UDispSpeciString(HRGB_LAYER, x, y, (T_U16 *)pWaitbox->pRainbowTitle, COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pWaitbox->pRainbowTitle));

	}
    
    // draw rainbow
    x = pWaitbox->rainbow_range.left + WAIT_LEFT_OFFSET;
    y = pWaitbox->rainbow_range.top + WAIT_TOP_OFFSET;
    Fwl_AkBmpDrawFromString(HRGB_LAYER, x, y, pWaitbox->pRainbow[(index++) % 6], &g_Graph.TransColor, AK_FALSE);

    Fwl_InvalidateRect( pWaitbox->rainbow_range.left, pWaitbox->rainbow_range.top, \
                        pWaitbox->rainbow_range.width, pWaitbox->rainbow_range.height);
}


/**
 * @brief the callback function of waiting-box timer
 *
 * @author Zhengwenbo
 * @date 2008-01-24
 * @param T_TIMER timer_id, T_U32 delay
 * @return T_VOID
 * @retval
 */
static T_VOID WaitBox_Timer_Callback(T_TIMER timer_id, T_U32 delay)
{
	T_SYS_MAILBOX mailbox;
    
    mailbox.event = PUBLIC_EVT_WAITBOX_SHOW;
    mailbox.param.w.Param1 = pWaitbox->mode;

    IAppMgr_PostEvent(AK_GetAppMgr(), AKAPP_CLSID_PUBLIC, &mailbox);
}

/**
 * @brief show waitbox by mode
 *
 * @author Zhengwenbo
 * @date 2008-05-29
 * @param T_eWAITBOX_MODE mode
 * @return T_VOID
 * @retval
 */
T_VOID WaitBox_Show(T_eWAITBOX_MODE mode)
{
#ifdef SUPPORT_VFONT
	T_BOOL		bUseVFont = AK_FALSE;
#endif

	if (AK_NULL == pWaitbox)
    {
        return;
    }
    
    AK_Obtain_Semaphore(pWaitbox->semaphore, AK_SUSPEND);
    
	if (ERROR_TIMER == pWaitbox->timer)
    {
    	AK_Release_Semaphore(pWaitbox->semaphore);
		return;
    }

#ifdef SUPPORT_VFONT
	bUseVFont = gb.bIsUseVFont;
	gb.bIsUseVFont = AK_FALSE;
#endif

    switch (mode)  
    {
        case WAITBOX_CLOCK:
            WaitBox_Clock_Show();
            break;
            
        case WAITBOX_PROGRESS:
            WaitBox_Progress_Show();
            break;

        case WAITBOX_RAINBOW:
            WaitBox_Rainbow_Show();
            break;

        default:
            Fwl_Print(C3, M_CTRL, "WaitBox_Timer_Callback(): bad mode\n");
            break;
    }
	
#ifdef SUPPORT_VFONT
	gb.bIsUseVFont = bUseVFont;
#endif

    AK_Release_Semaphore(pWaitbox->semaphore);
}

/**
 * @brief set the title of waitbox
 *
 * @author Zhengwenbo
 *      
 * @date 2008-05-08
 * @param T_pWSTR title: unicode string
 * @return T_VOID
 * @retval
 */
static T_VOID WaitBox_SetTitle(T_pWSTR title)
{
    switch (pWaitbox->mode)
    {
        case WAITBOX_PROGRESS:
            WaitBox_Progress_SetTitle(title);
            break;

        case WAITBOX_CLOCK:  
            WaitBox_Clock_SetTitle(title);
            break;

        case WAITBOX_RAINBOW:
            WaitBox_Rainbow_SetTitle(title);
            break;

        default:
            Fwl_Print(C3, M_CTRL, "WaitBox_SetTitle(): bad mode\n");
            break;
    }
    pWaitbox->pPrgTitle = title; 
}



