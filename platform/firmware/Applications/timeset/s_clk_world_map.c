
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Lib_event.h"
#include "gbl_global.h"
#include "ctl_msgBox.h"
#include  "fwl_keyhandler.h"
#include "eng_gblstring.h"
#include "eng_topbar.h"
#include  "eng_font.h"
#include  "lib_image_api.h"
#include  "eng_akbmp.h"
#include  "eng_dataconvert.h"
#include  "Fwl_rtc.h"
#include  "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "Fwl_tscrcom.h"


#define     TIMEZONE_NUM         33
#define     MAX_WORLDTIMEZONE   100
#define     LINE_LENTH            30  


#define     UI_REFRESH_NONE         0
#define     UI_REFRESH_STATBAR     1
#define     UI_REFRESH_CONTENT      2
#define     UI_REFRESH_BUTTON       4
#define     UI_REFRESH_ALL          7

typedef  enum{
    BT_OK,
    BT_CANCEL,

    BT_MAX
}T_BT_ID;

typedef enum{
    STATE_FCS,
    STATE_UNFCS,

    STATE_MAX
}T_BT_FCS_STAT;

typedef struct{

    T_pCDATA      pBtImgData[BT_MAX][STATE_MAX];


    T_RECT      rctBt[BT_MAX];    //position
    T_S32       nBtFcsState[BT_MAX]; //state
    T_U32       nBtIDFcsed;           

}T_SoftKey_Com;


typedef struct{
    
    T_SoftKey_Com    tSkCom;    //button
    
    T_pCDATA         pBGMapImg; 
    T_RECT           rctMapImg;   

    T_pCDATA        pTitleImg;
    T_RECT          rctTitleImg;

    T_pCDATA        pBottomImg;
    T_RECT          rctBottomImg;

    T_S32           nTscrCityId;
    T_RECT		    pTouchScope[TIMEZONE_NUM + 1];

}T_APP_RES;

typedef struct {
    T_APP_RES       appRes;

    T_S32           index; //current timezone index in worldtimezone list (sort from small to large)   

    T_MSGBOX        msgbox;

    T_S32           nRefreshFlg;

}T_WORLD_TIME;


extern T_U16 gb_CityCounts;
extern T_S16 gb_TimeZone[];
extern T_S16 gb_TimeZoneMinutes[];
extern T_U8 CURRENT_FONT_SIZE;

T_S32       gb_worldTimeZoneNumb = 0;
T_WORLDZONEMAP gb_worldMapZone[MAX_WORLDTIMEZONE];


static T_WORLD_TIME *pCLKWorldTime = AK_NULL;

#if (LCD_CONFIG_WIDTH==800)
static T_S16   city_coordinate[][2] =
{
    {453,302}, //0      
    {462,199}, //1      
    {492,215}, //2      
    {510, 84}, //3         
    {587,153}, //4        
    {605,142}, //5        
    {660,139}, //6        
    {680,144}, //7    
    {693,260}, //8         
    {707,278},//9         
    {733,278},//10       
    {767,197}, //11      
    {793,144}, //12      
    {37, 116},//13     
    {63, 113}, //14    
    {100, 151},//15    
    {123, 113},//16
    {133, 144}, //17   
    {167, 171}, //18 
    {197, 153}, //19 
    {203, 142}, //20 
    {230, 171}, //21 
    {250, 192}, //22
    {258, 201},//23 
    {268, 208}, //24
    {300, 142}, //25  
    {353,146},//26   
    {357,202},//27 
    {377,273},//28 
    {400,204},//29 
    {407,257},//30 
    {430,294},//31 
    {440,225}//32
};

#elif (LCD_CONFIG_WIDTH==480)
static T_S16   city_coordinate[][2] =
{
    {272,172}, //0      
    {277,113}, //1      
    {295,122}, //2      
    {306, 48}, //3         
    {352,87}, //4        
    {363,81}, //5        
    {396,79}, //6        
    {408,82}, //7    
    {416,148}, //8         
    {424,158},//9         
    {440,158},//10       
    {460,112}, //11      
    {476,82}, //12      
    {22, 66},//13     
    {38, 64}, //14    
    {60, 86},//15    
    {74, 64},//16
    {80, 82}, //17   
    {100, 97}, //18 
    {118, 87}, //19 
    {122, 81}, //20 
    {138, 97}, //21 
    {150, 109}, //22
    {155, 114},//23 
    {161, 118}, //24
    {180, 81}, //25  
    {212,83},//26   
    {214,115},//27 
    {226,155},//28 
    {240,116},//29 
    {244,146},//30 
    {258,167},//31 
    {264,128}//32
};

#elif (LCD_CONFIG_WIDTH==320)
static T_S16   city_coordinate[][2] =
{
    {181,147}, //0      
    {185,97}, //1      
    {197,105}, //2      
    {204, 41}, //3         
    {235,75}, //4        
    {242,69}, //5        
    {264,68}, //6        
    {272,70}, //7    
    {277,127}, //8         
    {283,135},//9         
    {293,135},//10       
    {307,96}, //11      
    {317,70}, //12      
    {15, 57},//13     
    {25, 55}, //14    
    {40, 74},//15    
    {49, 55},//16
    {53, 70}, //17   
    {67, 83}, //18 
    {79, 75}, //19 
    {81, 69}, //20 
    {92, 83}, //21 
    {100, 91}, //22
    {103, 98},//23 
    {107, 101}, //24
    {120, 69}, //25  
    {141,71},//26   
    {143,99},//27 
    {151,133},//28 
    {160,99},//29 
    {163,125},//30 
    {172,143},//31 
    {176,110}//32
};

#endif


T_VOID Init_WorldTimeZone(T_VOID)
{
	T_U16	i, j;
	T_S16	hour, minute;

    Fwl_Print(C3,M_CLOCK,"Init_WorldTimeZone");

	//gb_CityCounts = 33;
	gb_worldTimeZoneNumb = 1;
	gb_worldMapZone[0].hour = 0;
	gb_worldMapZone[0].minute = 0;
	gb_worldMapZone[0].index = 0;

	for( i=0; i<gb_CityCounts; i++ )
	{
		hour = gb_TimeZone[i];
		minute = gb_TimeZoneMinutes[i];

		for( j=0; j<gb_worldTimeZoneNumb; j++ )
		{
			if( hour == gb_worldMapZone[j].hour && minute == gb_worldMapZone[j].minute )
			{
				break;
			}
		}

		if( j == gb_worldTimeZoneNumb )
		{
			gb_worldMapZone[ j ].hour = (T_S8)hour;
			gb_worldMapZone[ j ].minute = (T_S8)minute;
			if( hour >= 0 )
			{
				gb_worldMapZone[ j ].index = (T_U8)hour;
			}
			else
			{
				gb_worldMapZone[ j ].index = 24 + hour;
			}
			gb_worldTimeZoneNumb++;
		}
	}

	for( i=0; i<gb_worldTimeZoneNumb-1; i++ )
	{
		for( j=i+1; j<gb_worldTimeZoneNumb; j++ )
		{
			if( (gb_worldMapZone[i].hour > gb_worldMapZone[j].hour) ||
				( gb_worldMapZone[i].hour == gb_worldMapZone[j].hour && gb_worldMapZone[i].minute > gb_worldMapZone[j].minute) )
			{
				T_WORLDZONEMAP temp;

				temp = gb_worldMapZone[i];
				gb_worldMapZone[i] = gb_worldMapZone[j];
				gb_worldMapZone[j] = temp;
			}
		}
	}
}




/*---------------------- BEGIN OF STATE s_clk_world_map ------------------------*/

T_VOID    TZ_InitRes(T_APP_RES * appRes)
{
    //T_S32   nTitle = 0;

    T_LEN    nTmpWidth = 0;
    T_LEN    nTmpHeight = 0;

    //title
    appRes->pTitleImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_TITLE, AK_NULL);
    AKBmpGetInfo(appRes->pTitleImg, &appRes->rctTitleImg.width, &appRes->rctTitleImg.height, AK_NULL);
    appRes->rctTitleImg.top = 0;
    appRes->rctTitleImg.left = 0;

    //state bar
    appRes->pBottomImg = appRes->pTitleImg;
    appRes->rctBottomImg.width = appRes->rctTitleImg.width;
    appRes->rctBottomImg.height = appRes->rctTitleImg.height;
    appRes->rctBottomImg.top = Fwl_GetLcdHeight() -  appRes->rctBottomImg.height ;
    appRes->rctBottomImg.left = 0; 
      
    //map image
    appRes->pBGMapImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_WORLDMAP, AK_NULL);
    AKBmpGetInfo(appRes->pBGMapImg, &appRes->rctMapImg.width, &appRes->rctMapImg.height, AK_NULL);
    appRes->rctMapImg.top = appRes->rctTitleImg.height + appRes->rctTitleImg.top;
    appRes->rctMapImg.left = 0;

//    AK_DEBUG_OUTPUT("appRes->pBGMapImg  = %d, top =%d, left =%d, h = %d, w=%d", appRes->pBGMapImg, appRes->rctMapImg.top, appRes->rctMapImg.left, appRes->rctMapImg.height, appRes->rctMapImg.width);

    // button resource
    appRes->tSkCom.pBtImgData[BT_OK][STATE_FCS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_OK_FCS, AK_NULL);
	AKBmpGetInfo(appRes->tSkCom.pBtImgData[BT_OK][STATE_FCS], &nTmpWidth, &nTmpHeight, AK_NULL);
    appRes->tSkCom.pBtImgData[BT_OK][STATE_UNFCS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_OK_UNFCS, AK_NULL);
	AKBmpGetInfo(appRes->tSkCom.pBtImgData[BT_OK][STATE_UNFCS], &nTmpWidth, &nTmpHeight, AK_NULL);

    appRes->tSkCom.rctBt[BT_OK].width = nTmpWidth;
    appRes->tSkCom.rctBt[BT_OK].height = nTmpHeight;
    appRes->tSkCom.rctBt[BT_OK].left = 0;
    appRes->tSkCom.rctBt[BT_OK].top = Fwl_GetLcdHeight() - appRes->tSkCom.rctBt[BT_OK].height;


    appRes->tSkCom.pBtImgData[BT_CANCEL][STATE_FCS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_CANCEL_FCS, AK_NULL);
	AKBmpGetInfo(appRes->tSkCom.pBtImgData[BT_CANCEL][STATE_FCS], &nTmpWidth, &nTmpHeight, AK_NULL);
    appRes->tSkCom.pBtImgData[BT_CANCEL][STATE_UNFCS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_CANCEL_UNFCS, AK_NULL);
	AKBmpGetInfo(appRes->tSkCom.pBtImgData[BT_CANCEL][STATE_UNFCS], &nTmpWidth, &nTmpHeight, AK_NULL);

    appRes->tSkCom.rctBt[BT_CANCEL].width = nTmpWidth;
    appRes->tSkCom.rctBt[BT_CANCEL].height = nTmpHeight;     
    appRes->tSkCom.rctBt[BT_CANCEL].left = Fwl_GetLcdWidth()- appRes->tSkCom.rctBt[BT_CANCEL].width;
    appRes->tSkCom.rctBt[BT_CANCEL].top = Fwl_GetLcdHeight() - appRes->tSkCom.rctBt[BT_CANCEL].height; 
}

T_VOID TZ_SetRefreshFlag(T_S32 nFlag)
{    
    pCLKWorldTime->nRefreshFlg |= nFlag;
}


static T_VOID TZ_Suspend(T_VOID)
{

}

static T_VOID TZ_Resume(T_VOID)
{
    TZ_InitRes(&(pCLKWorldTime->appRes));
    TZ_SetRefreshFlag(UI_REFRESH_ALL);
}



T_VOID TZ_ClearRefreshFlag(T_VOID)
{
    pCLKWorldTime->nRefreshFlg = 0;
}

T_S32 TZ_GetRefreshFlag(T_VOID)
{
    return pCLKWorldTime->nRefreshFlg;
}
#endif
void initclk_world_map(void)
{
#ifdef SUPPORT_SYS_SET

    T_S32     i = 0;
    
	pCLKWorldTime = ( T_WORLD_TIME * )Fwl_Malloc( sizeof( T_WORLD_TIME ) );
    AK_ASSERT_PTR_VOID(pCLKWorldTime,"pCLKWorldTime malloc error");


    m_regResumeFunc(TZ_Resume);
    m_regSuspendFunc(TZ_Suspend);    

    //get current timezone 
	for( i=0; i<gb_worldTimeZoneNumb; i++ )
	{
		if( gb_worldMapZone[i].hour == gs.curTimeZone.hour && gb_worldMapZone[i].minute == gs.curTimeZone.minute)
		{
			pCLKWorldTime->index = i;
			break;
		}
	}     

    TopBar_SetTitle(Res_GetStringByID(eRES_STR_TZ_TOPTEXT));
    TopBar_Show(TB_REFRESH_TITLE);

    //get resource
    TZ_InitRes(&(pCLKWorldTime->appRes));
    
    pCLKWorldTime->appRes.tSkCom.nBtIDFcsed = BT_MAX;
    pCLKWorldTime->appRes.tSkCom.nBtFcsState[BT_OK] = STATE_UNFCS;
    pCLKWorldTime->appRes.tSkCom.nBtFcsState[BT_CANCEL] = STATE_UNFCS;  
    pCLKWorldTime->appRes.nTscrCityId = TIMEZONE_NUM;

    TZ_ClearRefreshFlag();

    TZ_SetRefreshFlag(UI_REFRESH_ALL);

//    AK_DEBUG_OUTPUT("pCLKWorldTime->nRefreshFlg = %d", pCLKWorldTime->nRefreshFlg);

	for(i = 0; i < TIMEZONE_NUM; i++)
	{
		pCLKWorldTime->appRes.pTouchScope[i].left = city_coordinate[i][0];		
		pCLKWorldTime->appRes.pTouchScope[i].top = pCLKWorldTime->appRes.rctTitleImg.height;
		pCLKWorldTime->appRes.pTouchScope[i].height = pCLKWorldTime->appRes.rctMapImg.height;
		if (i == 12)
		{            
			pCLKWorldTime->appRes.pTouchScope[i].width = Fwl_GetLcdWidth() - city_coordinate[i][0];	
		}
		else if (i == 32)
		{
			pCLKWorldTime->appRes.pTouchScope[i].width = city_coordinate[0][0] - city_coordinate[i][0];
		}
		else
			pCLKWorldTime->appRes.pTouchScope[i].width = city_coordinate[i+1][0] - city_coordinate[i][0];
	}    
#endif
}

void exitclk_world_map(void)
{
#ifdef SUPPORT_SYS_SET
    if(pCLKWorldTime != AK_NULL)
    {
        Fwl_Free(pCLKWorldTime);
        pCLKWorldTime = AK_NULL;
    }
#endif
}


#ifdef SUPPORT_SYS_SET

T_VOID TZ_ShowButton(T_WORLD_TIME *wt )
{
    T_POS  x = 0;
    T_POS  y = 0;
    T_S32   nStatId = 0;

    T_S32   i = 0;

    AK_ASSERT_PTR_VOID(wt, "pointer wt is invalid");

    for(i = 0; i < BT_MAX; i++)
    {
        x = wt->appRes.tSkCom.rctBt[i].left;
        y = wt->appRes.tSkCom.rctBt[i].top;
        nStatId = wt->appRes.tSkCom.nBtFcsState[i];
        
        Fwl_AkBmpDrawFromString(HRGB_LAYER, x, y,wt->appRes.tSkCom.pBtImgData[i][nStatId], &g_Graph.TransColor,AK_FALSE);
    }    

}

T_VOID TZ_ShowText(T_WORLD_TIME *wt )
{
#if 0    
        T_S16   zone;
        T_TCHR  strTemp[30], strTemp1[30], strZone[4];
        T_S8    hour, minute;
        T_S16   textLeft;
    
        hour = gb_worldMapZone[ wt->index ].hour;
        minute = gb_worldMapZone[ wt->index ].minute;
    /*
        Utl_TStrCpy(CntryName, time_zone_city[wt->index][gb.LanguageIndex[gb.Lang]]);
    */
        Utl_TStrCpy(CntryName, GetString(etTimeZoneCity, wt->index));
    
        Utl_TStrCpy(strTemp, _T("(GMT"));
    
        if( hour == 0 )
        {
            Utl_TStrCat( strTemp, _T(")") );
        }
        else if( hour < 0 )
        {
            Utl_TStrCat( strTemp, _T("-") );
            zone = 0 - hour;
    
            Utl_TItoa( zone, strZone, 10 );
            if( zone < 10 )
            {
                Utl_TStrCat( strTemp, _T("0") );
            }
            Utl_TStrCat( strTemp, strZone );
            Utl_TStrCat( strTemp, _T(":") );
            minute = 0 - minute;
            Utl_TItoa( minute, strZone, 10 );
            if( minute < 10 )
            {
                Utl_TStrCat( strTemp, _T("0") );
            }
            Utl_TStrCat( strTemp, strZone );
            Utl_TStrCat( strTemp, _T(")") );
        }
        else
        {
            Utl_TStrCat( strTemp, _T("+") );
            
            Utl_TItoa( hour, strZone, 10 );
            if( hour < 10 )
            {
                Utl_TStrCat( strTemp, _T("0") );
            }
            Utl_TStrCat( strTemp, strZone );
            Utl_TStrCat( strTemp, _T(":") );
            Utl_TItoa( minute, strZone, 10 );
            if( minute < 10 )
            {
                Utl_TStrCat( strTemp, _T("0") );
            }
            Utl_TStrCat( strTemp, strZone );
            Utl_TStrCat( strTemp, _T(")") );
        }
    
        /* display city name and time and zone according to time zone */
        Fwl_FillSolidRect( DISPLAY_LCD_0, 0, 220, Fwl_GetLcdWidth(), 60, S_CLK_WORLD_MAP_RECT_COLOR );
        textLeft = (Fwl_GetLcdWidth() - Eng_GetTextWidth_T(CURRENT_FONT_SIZE, CntryName))/2;   
        if( textLeft < 0 )
        {
            textLeft = 0;
            Utl_TStrMid(strTemp1, CntryName, 0, (T_S16)(gb_stdb.LCDCOL[DISPLAY_LCD_0]+3));
            DispSpeciString(DISPLAY_LCD_0, textLeft, 226, strTemp1, S_CLK_WORLD_MAP_TEXT_COLOR, CURRENT_FONT_SIZE, Utl_TStrLen(strTemp1));
        }
        else
        {
            DispSpeciString(DISPLAY_LCD_0, textLeft, 226, CntryName, S_CLK_WORLD_MAP_TEXT_COLOR, CURRENT_FONT_SIZE, Utl_TStrLen(CntryName));
        }
        textLeft = (Fwl_GetLcdWidth() - Eng_GetTextWidth_T(CURRENT_FONT_SIZE, strTemp))/2; 
        DispSpeciString(DISPLAY_LCD_0, textLeft, 256, strTemp, S_CLK_WORLD_MAP_TEXT_COLOR, CURRENT_FONT_SIZE, Utl_TStrLen(strTemp));
#endif

}

T_VOID TZ_ShowContent(T_WORLD_TIME *wt )
{
    //T_S32   len;

    T_S16   x = 0;
    T_S16   y = 0;
    T_S32   nStrId = 0;
    T_U16  * ustrText = AK_NULL;
    T_S32   nHeight = 0;
    T_S32   nWidth = 0;

    T_U16   uStrTimeDiff[512], uStrTmp[512];
    //T_U8    strTmp[512];
    T_S32   nHour = 0;
    T_S32   nMinute = 0;
    T_S32   nTemp =0;
    
    

    AK_ASSERT_PTR_VOID(wt, "pointer wt is invalid");

    Fwl_AkBmpDrawFromString(HRGB_LAYER, wt->appRes.rctMapImg.left, wt->appRes.rctMapImg.top,wt->appRes.pBGMapImg, &g_Graph.TransColor,AK_FALSE);

    x = city_coordinate[wt->index][0];
    y = city_coordinate[wt->index][1]  + wt->appRes.rctTitleImg.height;  

    if(x < LINE_LENTH)
    {
        Fwl_DrawLine(HRGB_LAYER, 0, y, (T_POS)(x + LINE_LENTH), y, RGB2AkColor(0, 0, 0, LCD0_COLOR));
    }
    else if(x > Fwl_GetLcdWidth() -  LINE_LENTH)
    {
        Fwl_DrawLine(HRGB_LAYER, (T_POS)(x - LINE_LENTH), y, Fwl_GetLcdWidth(), y, RGB2AkColor(0, 0, 0, LCD0_COLOR));
    }
    else
    {
        Fwl_DrawLine(HRGB_LAYER, (T_POS)(x - LINE_LENTH), y, (T_POS)(x + LINE_LENTH), y, RGB2AkColor(0,0,0,LCD0_COLOR));
    }

    Fwl_DrawLine(HRGB_LAYER, x, (T_POS)(y-10), x, (T_POS)(y+10),RGB2AkColor(0,0,0,LCD0_COLOR));

    //show text 

    //   (133 191)

    nStrId = eRES_STR_TZ_ENIWETOK + wt->index;
    ustrText = (T_U16 *)Res_GetStringByID(nStrId);
    nHeight = GetFontHeight(CURRENT_FONT_SIZE);
    nWidth = UGetSpeciStringWidth(ustrText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(ustrText));

    x = (Fwl_GetLcdWidth() - nWidth) / 2;
    y = (T_S16)(wt->appRes.rctTitleImg.height + wt->appRes.rctMapImg.height - nHeight * 2.5 - 2);

//    Fwl_FillSolidRect(HRGB_LAYER , 0 , );    

    Fwl_UDispSpeciString(HRGB_LAYER, x, y, ustrText , COLOR_BLACK, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(ustrText));  

	nHour = gb_worldMapZone[ wt->index ].hour;
	nMinute = gb_worldMapZone[ wt->index ].minute;

//    AK_DEBUG_OUTPUT("  wt->index =%d nHour =%d, nMinute=%d", wt->index, nHour, nMinute);    

    Utl_UStrCpy(uStrTimeDiff, _T("(GMT"));

    if(0 == nHour)
    {
        Utl_UStrCat(uStrTimeDiff, _T(")"));

    }
    else if(nHour < 0)
    {
        Utl_UStrCat(uStrTimeDiff, _T("-"));
        nTemp = 0 - nHour ;
        Utl_UItoa(nTemp, uStrTmp, 10);
        if(nTemp < 10)
        {
            Utl_UStrCat(uStrTimeDiff, _T("0"));
        }

        Utl_UStrCat(uStrTimeDiff, uStrTmp);
        Utl_UStrCat(uStrTimeDiff, _T(":"));
        nTemp = 0 - nMinute;
        

        Utl_UItoa(nTemp, uStrTmp, 10);
        if(nTemp < 10)
        {
            Utl_UStrCat(uStrTimeDiff, _T("0"));
        }

        Utl_UStrCat(uStrTimeDiff, uStrTmp);        
        Utl_UStrCat(uStrTimeDiff, _T(")"));      
    }
    else
    {
//        AK_DEBUG_OUTPUT("nHour =%d, nMinute=%d", nHour, nMinute);
        nTemp  = nHour;
        Utl_UStrCat(uStrTimeDiff, _T("+"));        
        Utl_UItoa(nTemp, uStrTmp, 10);
        if(nTemp < 10)
        {
            Utl_UStrCat(uStrTimeDiff, _T("0"));
        }

        Utl_UStrCat(uStrTimeDiff, uStrTmp);
        Utl_UStrCat(uStrTimeDiff, _T(":"));       
        nTemp  = nMinute;
        
        Utl_UItoa(nTemp, uStrTmp, 10);
        if(nTemp < 10)
        {
            Utl_UStrCat(uStrTimeDiff, _T("0"));
        }

        Utl_UStrCat(uStrTimeDiff, uStrTmp);        
        Utl_UStrCat(uStrTimeDiff, _T(")"));          

    }

    nHeight = GetFontHeight(CURRENT_FONT_SIZE);
    nWidth = UGetSpeciStringWidth(uStrTimeDiff, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uStrTimeDiff));

    x = (Fwl_GetLcdWidth() - nWidth) / 2;
    y = wt->appRes.rctTitleImg.height + wt->appRes.rctMapImg.height - nHeight  - 2;
    Fwl_UDispSpeciString(HRGB_LAYER, x, y, uStrTimeDiff , COLOR_BLACK, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uStrTimeDiff));  
    
   
}

T_VOID TZ_ShowStatBar(T_WORLD_TIME *wt )
{
    AK_ASSERT_PTR_VOID(wt, "pointer wt is invalid");

//    AK_DEBUG_OUTPUT("TZ_ShowStatBar wt->appRes.rctBottomImg=%d", wt->appRes.rctBottomImg);
    Fwl_AkBmpDrawFromString(HRGB_LAYER, wt->appRes.rctBottomImg.left, wt->appRes.rctBottomImg.top,wt->appRes.pBottomImg, &g_Graph.TransColor,AK_FALSE);

}


T_VOID TZ_ShowUI(T_WORLD_TIME *wt )
{
    T_S32   nRefreshFlag = 0;

    nRefreshFlag = TZ_GetRefreshFlag();

//    AK_DEBUG_OUTPUT("TZ_ShowUI, nRefreshFlag=%d",nRefreshFlag);

    if(UI_REFRESH_NONE == nRefreshFlag)
    {   
        return;
    }
    
    if( nRefreshFlag && UI_REFRESH_CONTENT)
    {
        TZ_ShowContent(wt);        
    }   

    if(nRefreshFlag && UI_REFRESH_STATBAR)
    {
        TZ_ShowStatBar(wt);
    }

    if(nRefreshFlag && UI_REFRESH_BUTTON)
    {
        TZ_ShowButton(wt);
    }

    TZ_ClearRefreshFlag();    
    Fwl_RefreshDisplay();

}

#endif
void paintclk_world_map(void)
{
#ifdef SUPPORT_SYS_SET
//    AK_DEBUG_OUTPUT("paintclk_world_map");
    TZ_ShowUI(pCLKWorldTime);   
#endif
}

#ifdef SUPPORT_SYS_SET

T_VOID TZ_Decrease(T_WORLD_TIME *wt )
{
	if( wt->index == 0 )
	{
		wt->index = gb_worldTimeZoneNumb-1;
	}
	else
	{
		wt->index--;
	}
}


void TZ_Increase( T_WORLD_TIME *wt )
{
	if( wt->index == gb_worldTimeZoneNumb-1 )
	{
		wt->index = 0;
	}
	else
	{
		wt->index++;
	}
}

void TZ_AdjustLocalTimeZone( T_WORLD_TIME *wt )
{
    T_U32   nRtcCount = 0;
    T_S8   nNewHour = 0;
    T_S8   nNewMinute = 0;

    T_S32   nDiffHour = 0;    
    T_S32   nDiffMinute = 0;
    T_S32   nTmp = 0;
    T_U32   uVar = 0;

    nRtcCount = Fwl_RTCGetCount();

	nNewHour = gb_worldMapZone[ wt->index ].hour;
	nNewMinute = gb_worldMapZone[ wt->index ].minute;  

    nDiffHour = nNewHour - gs.curTimeZone.hour;
    nDiffMinute = nNewMinute - gs.curTimeZone.minute;   

    nTmp = (nDiffHour * 60 + nDiffMinute) * 60;

//    AK_DEBUG_OUTPUT("nTmp  = %d, nDiffHour = %d, nDiffMinute=%d, nRtcCount=%d", nTmp , nDiffHour, nDiffMinute, nRtcCount);

    if(0 == nTmp)
    {
         
    }
    else if(nTmp < 0)
    {      
        uVar = (T_U32)(0 - nTmp);
        if(nRtcCount > uVar)
        {
            nRtcCount = nRtcCount - (T_U32)(0 - nTmp);
        }
        
    }
    else // nTmp > 0
    {           
        nRtcCount = nRtcCount + (T_U32)(nTmp);   
    }

//     AK_DEBUG_OUTPUT("1nTmp  = %d, nDiffHour = %d, nDiffMinute=%d, nRtcCount=%d", nTmp , nDiffHour, nDiffMinute, nRtcCount);
    
     Fwl_RTCSetCount(nRtcCount);

//     AK_DEBUG_OUTPUT("rtccount =%d", Fwl_RTCGetCount());
     
     gs.curTimeZone.hour = nNewHour;
     gs.curTimeZone.minute = nNewMinute;

}

T_eBACK_STATE TZ_UsrKeyHandle(T_MMI_KEYPAD phyKey)
{
    T_eBACK_STATE enmRet = eStay;

    switch(phyKey.keyID)
    {
        case kbLEFT:
            TZ_Decrease(pCLKWorldTime);
            TZ_SetRefreshFlag(UI_REFRESH_CONTENT);            
            break;

        case kbRIGHT:
            TZ_Increase(pCLKWorldTime);
            TZ_SetRefreshFlag(UI_REFRESH_CONTENT);            
            break;

        case kbOK:
            TZ_AdjustLocalTimeZone(pCLKWorldTime);
            enmRet = eNext;
            break;
        case kbCLEAR:
            if(PRESS_SHORT == phyKey.pressType)
            {
                enmRet = eReturn;
            }
            else if(phyKey.pressType == PRESS_LONG)
            {
                enmRet = eHome;
            }
            break;

         default:
            break;
    }

    return enmRet;  
}

#if 0
T_VOID TZ_SetBtFcsStat(T_S32 nOldSkId,T_S32  enmBtStat, T_SoftKey_Com * tSkCom)
{
    if(nOldSkId < BT_MAX && nOldSkId >= BT_OK)
    {
        tSkCom->nBtFcsState[nOldSkId] = enmBtStat;    
    }

}
#endif


T_VOID TZ_SetBtUnfcs(T_S32 nSkId,T_SoftKey_Com * tSkCom)
{
    if(tSkCom->nBtIDFcsed != BT_MAX && ((T_U32)nSkId) == tSkCom->nBtIDFcsed)
    {
        tSkCom->nBtFcsState[nSkId] =  STATE_UNFCS;
        tSkCom->nBtIDFcsed = BT_MAX;                
    }    
}

T_VOID TZ_SetBtFcs(T_S32 nSkId,T_SoftKey_Com * tSkCom)
{
    T_S32   nOldSkId = 0;
    nOldSkId = tSkCom->nBtIDFcsed;

    if(nSkId != nOldSkId || nSkId != BT_MAX)
    {
        TZ_SetBtUnfcs(nOldSkId, tSkCom);
        tSkCom->nBtIDFcsed = nSkId;     
        tSkCom->nBtFcsState[tSkCom->nBtIDFcsed] = STATE_FCS;
    }

}


T_eBACK_STATE TZ_TscrHandle(T_S32  nTscrAction, T_S32    nPosx, T_S32    nPosy, T_WORLD_TIME *wt)
{
    T_eBACK_STATE enmRet = eStay;

    T_U32   i = 0;
    T_BOOL  bIsSk = AK_FALSE;
    //T_S32   nSkId = BT_MAX;
    //T_S32   nOldSkId = BT_MAX;

    //T_BOOL  bSkDown = AK_FALSE;

    T_RECT    rect;
    T_MMI_KEYPAD phyKey;

//    AK_DEBUG_OUTPUT("TZ_TscrHandle nTscrAction =%d, nPosx=%d, nPosy = %d, wt->appRes.nTscrCityId=%d",nTscrAction ,nPosx, nPosy,wt->appRes.nTscrCityId);
    switch(nTscrAction)
    {
        case eTOUCHSCR_UP:

            rect = TopBar_GetRectofCancelButton();
            
             //hit cancel button
            if (PointInRect(&rect, (T_POS)nPosx, (T_POS)nPosy))
            {
                phyKey.keyID = kbCLEAR;
                phyKey.pressType = PRESS_SHORT;
                enmRet = TZ_UsrKeyHandle(phyKey);
                return enmRet;                
            }            

            bIsSk = AK_FALSE;
            // check the soft key
            for(i = 0; i < BT_MAX; i++)
            {
                if(PointInRect(&(wt->appRes.tSkCom.rctBt[i]), (T_POS)nPosx, (T_POS)nPosy))
                {
                    bIsSk = AK_TRUE;

                    if(i == wt->appRes.tSkCom.nBtIDFcsed && i != BT_MAX)
                    {      
                        
                        phyKey.keyID = kbNULL;
                    
                        phyKey.pressType = PRESS_SHORT;                    

                        if(BT_OK == i)
                        {
                            phyKey.keyID = kbOK;
                        }
                        else
                        {
                            phyKey.keyID = kbCLEAR;                            
                        }
                        enmRet = TZ_UsrKeyHandle(phyKey);
                        
                    }
                    TZ_SetBtUnfcs(i, &(wt->appRes.tSkCom));                                    
                    TZ_SetRefreshFlag(UI_REFRESH_BUTTON);
                    break;
                }               
            }

//            AK_DEBUG_OUTPUT("eTOUCHSCR_UP 1 bIsSk =%d,top =%d,bottom =%d, wt->appRes.nTscrCityId=%d", bIsSk,wt->appRes.rctTitleImg.height,wt->appRes.rctTitleImg.height + wt->appRes.rctMapImg.height,wt->appRes.nTscrCityId);
            if(bIsSk)
            {
             
            }
            else 
            {
                if(wt->appRes.tSkCom.nBtIDFcsed != BT_MAX)
                {
                    TZ_SetBtUnfcs(wt->appRes.tSkCom.nBtIDFcsed, &(wt->appRes.tSkCom));                                    
                    TZ_SetRefreshFlag(UI_REFRESH_BUTTON);
                }                 
                
                if(nPosy > wt->appRes.rctTitleImg.height && nPosy <= wt->appRes.rctTitleImg.height + wt->appRes.rctMapImg.height )//press on map
                {
//                    AK_DEBUG_OUTPUT("eTOUCHSCR_UP111  left =%d, top =%d, height =%d, width =%d", wt->appRes.pTouchScope[wt->appRes.nTscrCityId].left, wt->appRes.pTouchScope[wt->appRes.nTscrCityId].top, wt->appRes.pTouchScope[wt->appRes.nTscrCityId].height,wt->appRes.pTouchScope[wt->appRes.nTscrCityId].width);
                    if (wt->appRes.nTscrCityId < TIMEZONE_NUM && PointInRect(&wt->appRes.pTouchScope[wt->appRes.nTscrCityId], (T_POS)nPosx, (T_POS)nPosy))
                    {                    
                        wt->index = wt->appRes.nTscrCityId;     
                        TZ_SetRefreshFlag(UI_REFRESH_CONTENT);                    
//                        AK_DEBUG_OUTPUT("eTOUCHSCR_UP2 pCLKWorldTime->nRefreshFlg=%d ",pCLKWorldTime->nRefreshFlg);
                        break;
                    }             
                }
            }


            
            break;
        case eTOUCHSCR_DOWN:

//            AK_DEBUG_OUTPUT("eTOUCHSCR_DOWN1 wt->appRes.nTscrCityId =%d",wt->appRes.nTscrCityId );

            // check the soft key
            for(i = 0; i < BT_MAX; i++)
            {
                if(PointInRect(&(wt->appRes.tSkCom.rctBt[i]), (T_POS)nPosx, (T_POS)nPosy))
                {
                    bIsSk = AK_TRUE;

                    TZ_SetBtFcs(i, &(wt->appRes.tSkCom));                                    
                    TZ_SetRefreshFlag(UI_REFRESH_BUTTON);
                    break;
                }               
            }    

            if(bIsSk)
            { 
            }
            else 
            {
                if(wt->appRes.tSkCom.nBtIDFcsed != BT_MAX)
                {
                    TZ_SetBtUnfcs(wt->appRes.tSkCom.nBtIDFcsed, &(wt->appRes.tSkCom));                                    
                    TZ_SetRefreshFlag(UI_REFRESH_BUTTON);
                }            
                
                if(nPosy > wt->appRes.rctTitleImg.height && nPosy <= wt->appRes.rctTitleImg.height + wt->appRes.rctMapImg.height )//press on map
                {
                    for (i = 0; i < TIMEZONE_NUM; i++)
                    {                    
                        if (PointInRect(&wt->appRes.pTouchScope[i], (T_POS)nPosx, (T_POS)nPosy))
                        {
                            wt->appRes.nTscrCityId = i;
                            break;
                        }   
                    }                             
                }            
            }


//            AK_DEBUG_OUTPUT("eTOUCHSCR_DOWN2 wt->appRes.nTscrCityId =%d",wt->appRes.nTscrCityId );           
            break;

        case eTOUCHSCR_MOVE:

//            AK_DEBUG_OUTPUT("eTOUCHSCR_MOVE1 wt->appRes.nTscrCityId =%d",wt->appRes.nTscrCityId );            
            // check the soft key
            for(i = 0; i < BT_MAX; i++)
            {
                if(PointInRect(&(wt->appRes.tSkCom.rctBt[i]), (T_POS)nPosx, (T_POS)nPosy))
                {
                    bIsSk = AK_TRUE;

                    TZ_SetBtFcs(i, &(wt->appRes.tSkCom));    
//                    AK_DEBUG_OUTPUT("eTOUCHSCR_MOVE88 fcsId = %d ",bIsSk );
                    TZ_SetRefreshFlag(UI_REFRESH_BUTTON);
                    break;
                }               
            }  


            if(bIsSk)
            {
             
            }
            else
            {
                if(wt->appRes.tSkCom.nBtIDFcsed != BT_MAX)
                {
                    TZ_SetBtUnfcs(wt->appRes.tSkCom.nBtIDFcsed, &(wt->appRes.tSkCom));                                    
                    TZ_SetRefreshFlag(UI_REFRESH_BUTTON);
                } 
                
                if(nPosy > wt->appRes.rctTitleImg.height && nPosy <= wt->appRes.rctTitleImg.height + wt->appRes.rctMapImg.height )//press on map
                {
                    for (i = 0; i < TIMEZONE_NUM; i++)
                    {                    
                        if (PointInRect(&wt->appRes.pTouchScope[i], (T_POS)nPosx, (T_POS)nPosy))
                        {
                            wt->appRes.nTscrCityId = i;
                            break;
                        }   
                    }                             
                } 
            }

                

//            AK_DEBUG_OUTPUT("eTOUCHSCR_MOVE2 wt->appRes.nTscrCityId =%d",wt->appRes.nTscrCityId );                        
            break;

        default:
            break;


    }

//    AK_DEBUG_OUTPUT("2TZ_TscrHandle nTscrAction =%d, nPosx=%d, nPosy = %d, wt->appRes.nTscrCityId=%d, pCLKWorldTime->nRefreshFlg=%d",nTscrAction ,nPosx, nPosy,wt->appRes.nTscrCityId,pCLKWorldTime->nRefreshFlg);

    return enmRet;      

}

#endif
unsigned char handleclk_world_map(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE enmTzRet = eStay;
    T_MMI_KEYPAD phyKey;
        
    if (IsPostProcessEvent(event))
    {
        return 1;
    }

//    AK_DEBUG_OUTPUT("event =%d",event);

    switch(event)
    {
        case M_EVT_USER_KEY:                      
           
            phyKey.keyID = pEventParm->c.Param1;
            phyKey.pressType = pEventParm->c.Param2;

            enmTzRet = TZ_UsrKeyHandle(phyKey);
            
            break;

        case M_EVT_TOUCH_SCREEN:

            enmTzRet = TZ_TscrHandle(pEventParm->s.Param1, pEventParm->s.Param2, pEventParm->s.Param3, pCLKWorldTime);
            break;

        case VME_EVT_TIMER:

            break;

        default:
          break;      

    }


    switch(enmTzRet)
    {
        case eNext:

            MsgBox_InitAfx(&pCLKWorldTime->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pCLKWorldTime->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pCLKWorldTime->msgbox);            

            break;

        default:
            ReturnDefauleProc(enmTzRet, pEventParm);
            break;

    }    

//    m_triggerEvent(M_EVT_EXIT, pEventParm);
#endif
    return 0;
}
