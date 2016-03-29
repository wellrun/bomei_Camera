/************************************************************************
* Copyright (c) 2001, Anyka Co., Ltd. 
* All rights reserved.    
*  
* File Name：s_tscr_calibrate.c
* Function：calibrate Touch screen
*
* Author：
* Date：2008-08-26
* Version：2.0  
*
* Reversion: 
* Author: 
* Date: 
**************************************************************************/
#include "Fwl_pfDisplay.h"
#include "Ctl_MsgBox.h"
#include "Eng_Debug.h"
#include "Fwl_osMalloc.h"
#include "Fwl_Initialize.h"
#include "Eng_GblString.h"
#include "Eng_Graph.h"
#include "Eng_font.h"
#include "Fwl_public.h"
#include "Eng_ScreenSave.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "Fwl_calibrate.h"
#include "Fwl_TScrcom.h"

#define TS_CALIB_REC_BORD                   11
/*the max number of the point actually used for calibrate*/
#define CALIB_POINT_MAX                     5

/*the threshold based on the ratio between LCD and Touch Screen*/
#define  ERROR_POINT_ABSOLUTE              60

/*the threshod that when the pen touch the touch screen , if slide ,valide distance is 400*/
#define  ERROR_TOUCH_SLIDE               80//  20

/*the threshold based on the calibrate Matrix*/
#define  ERROR_POINT_ABSOLUTE_LCD         50 //12//8

/* the coordinate availability judge     */
#define diffU8(i, j) ((i) > (j) ? ((i) - (j)) : ((j) - (i)))   // get the absolute difference
// |i-j|<n && |k-m|<n 
#define NorPoint(i, j, k, m, n) \
    ((diffU8(i, j) < (n)) && (diffU8(k, m) < (n)))

#define S_TSCR_CALIBRATE_RECT_COLOR        (COLOR_GREEN)
#ifdef SUPPORT_STDB_BOTTOMBAR    
#define ERR_Y_MAX_VALUE                    (LCD0_HEIGHT+20-1)
#else
#define ERR_Y_MAX_VALUE                    (LCD0_HEIGHT-1)
#endif

#define ADC_MAX_VALUE                       1024    
/*the max times that the third point can repeat */
#define MAX_REPEAT_TIME                     5

//the point in ADC  for calibrate
typedef struct{
    T_POINT     pt;
    T_BOOL      bHasTouched;
}T_ADCPOINT;

typedef struct {
    T_MSGBOX         msgbox;
    T_U8*            pLockstate;
    T_BOOL           bRefresh;
    T_U8             CurCalibratePt ;
    T_POINT          tsSamplePt[CALIB_POINT_MAX];  // the coordinate in ADC 
    T_BOOL           bFromInit;
    T_BOOL           calibSucc;    

    T_BOOL           bSetACDCoordSucc;
    T_U32            nACDCoordModeBk;
    T_ADCPOINT       ptADCFixed[CALIB_POINT_MAX];      
} T_TSCRcalibrate;

/*the fixed point in LCD for calibrating*/
#if (LCD0_WIDTH == 480)
const T_POINT displaySample[CALIB_POINT_MAX] =   
        {{ 30, 30}, 
        {450, 30},
        {450, 242}, 
        {30, 242},
        {240, 136}}; 
#else 
const T_POINT displaySample[CALIB_POINT_MAX] =   
        {{ 30, 30}, 
        {290, 30},
        {290, 210}, 
        {30, 210},
        {160, 120}}; 
#endif


static T_TSCRcalibrate  *pTSCRcalibrate = AK_NULL;

/*the location of point  in LCD coordinate converted from ADC corrdinate  */
T_POINT ADCSampleHope;

T_POINT ptADCModified[CALIB_POINT_MAX];

/*the temp matrix to restore the calibrate matrix*/
volatile    T_MATRIX    gs_matrixPtr ;

extern T_GLOBAL_S       gs;

static T_BOOL           tscr_is_pressed = AK_FALSE;

extern T_VOID tsCalculateCoordinate_point(T_U32 *x, T_U32 *y, T_POINT tsPoint);
#ifdef TOUCH_SCR
static T_POINT        tscr_down_point;
static T_POINT        tscr_last_move_point;
static T_U32            nRepeatTime = 0;
static T_BOOL 			bValidADPoint   = AK_FALSE;

static T_U32 get_distance(const T_POINT* p1, const T_POINT* p2 );
//static T_BOOL  PointInFixedArea(T_TSPOINT *ADCpt, T_ADCPOINT * ptADCFixed, T_U32 nFixPtNum);

static T_U32 SetADCCoordSys(T_POINT * ptADCArray);
static T_VOID ReSetPtCoord(T_POINT *ptADCModified,T_POINT * ptADCArray,T_U32 nPtNum);
#endif



/** 
* @brief   Draw a rectangle and Latin cross on lcd
* @author  Li Chenjie
* @date    2006-03-09
* @version 1.0
*/ 
static T_VOID calibDrawPoint(HLAYER hLayer, T_POINT disPoint, T_S16 border, T_COLOR color)
{
    // draw rectangle
    Fwl_DrawRect(hLayer, (T_POS)(disPoint.x - (border - 1)/2 ), (T_POS)(disPoint.y - (border - 1)/2), 
        border, border, color);
    // draw latin cross             
    Fwl_DrawLine(hLayer, (T_POS)(disPoint.x - border), disPoint.y, 
        (T_POS)(disPoint.x + border), disPoint.y, color);
    Fwl_DrawLine(hLayer, disPoint.x, (T_POS)(disPoint.y - border), 
        disPoint.x, (T_POS)(disPoint.y + border), color);
}

//Typho00002123   fix calling in refresh
static T_VOID resumeProc( T_VOID )
{

}

static T_VOID suspendProc( T_VOID)
{

}


#ifdef TOUCH_SCR
/************************************************************************
 * @BRIEF:    get the point location in LCD coordinate  converted from ADC coordinate by  calibrate matrix 
 * @AUTHOR:   
 * @DATE:     2008-11-21
 * @Modified:   
 * @PARAM:    T_pTSPOINT displayPtr: the destination LCD coordinate array
 * @PARAM:    const T_pTSPOINT screenPtr: the source ADC coordinate array
 * @PARAM:    T_MATRIX *matrixPtr: the calibrate matrix
 * @RETURN:   T_U8: be true if converting succeed ,else false
 * @RETVAL:
 **************************************************************************/

static T_U8 getLCDPointByMatrix(T_pPOINT displayPtr, const T_pPOINT screenPtr ,T_MATRIX *matrixPtr )
{
    T_POINT          wtpt;
    
    wtpt.x  = screenPtr->x;
    wtpt.y  = screenPtr->y;    
    
#ifdef OS_ANYKA
    getDisplayPoint_point(displayPtr,&wtpt,matrixPtr);
#endif
    
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
    return AK_TRUE;
}
#endif


/************************************************************************
 * @BRIEF:   test whether the calibrate operation is the first time to excute 
 * @AUTHOR:   
 * @DATE:     2008-11-21
 * @Modified:   
 * @PARAM:    
 * @RETURN:   T_U8: be true if it is the first time to calibrate ,else false
 * @RETVAL:
 **************************************************************************/
/*
static T_BOOL Test_IsFirstCalibarte()
{
    T_U8 iZeroCnt = 0 ;
#if 0 //TOUCH_SCR
    
    for(i=0; i<4; i++)
    {
        if (gs.matrixPtr.X[i] == 0 && gs.matrixPtr.Y[i] == 0)
        {
            iZeroCnt++;
        }        
    }
#endif
    
    if ( iZeroCnt > 1 )
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }    
    
}
*/

#ifdef TOUCH_SCR

T_BOOL TscrIsCalibrated(T_VOID)
{
	if (0 == gs.matrixPtr.X[0] && 0 == gs.matrixPtr.X[1] && 0 == gs.matrixPtr.X[2] )
	{
		return AK_FALSE;
	}
	else
	{
		return AK_TRUE;
	}
}

#endif

/************************************************************************
* @BRIEF:   initialize the variable 
* @AUTHOR:   
* @DATE:     2008-11-21
* @Modified:   
* @PARAM:    
* @RETURN:   T_U8: be true if it is the first time to calibrate ,else false
* @RETVAL:
**************************************************************************/

void InitializeVal(void)
{
#ifdef TOUCH_SCR

    pTSCRcalibrate->CurCalibratePt = 0 ;
    pTSCRcalibrate->bRefresh = AK_TRUE ;
    pTSCRcalibrate->calibSucc = AK_FALSE;
    pTSCRcalibrate->bFromInit = AK_FALSE;//add 2006.8.11

    pTSCRcalibrate->nACDCoordModeBk = gs.nADCCoordMode;

    Fwl_Print(C3, M_SETTING, "adc coordinate mode is %d",gs.nADCCoordMode);
    gs.nADCCoordMode = 0;
    pTSCRcalibrate->bSetACDCoordSucc = AK_FALSE;
    {
        T_U8    j;
        for (j = 0 ; j < CALIB_POINT_MAX  ; j ++ )
        {
            pTSCRcalibrate->tsSamplePt[j].x = 0 ;
            pTSCRcalibrate->tsSamplePt[j].y = 0 ;

            pTSCRcalibrate->ptADCFixed[j].bHasTouched = AK_FALSE;
            pTSCRcalibrate->ptADCFixed[j].pt.x = displaySample[j].x * ADC_MAX_VALUE / LCD0_WIDTH;
            pTSCRcalibrate->ptADCFixed[j].pt.y = displaySample[j].y * ADC_MAX_VALUE / LCD0_HEIGHT;
        }    
    }

    Fwl_Print(C3, M_SETTING,"ADCfixed point in ratio: (%d %d)(%d %d) (%d %d) (%d %d)",
        pTSCRcalibrate->ptADCFixed[0].pt.x ,pTSCRcalibrate->ptADCFixed[0].pt.y,
        pTSCRcalibrate->ptADCFixed[1].pt.x ,pTSCRcalibrate->ptADCFixed[1].pt.y,
        pTSCRcalibrate->ptADCFixed[2].pt.x ,pTSCRcalibrate->ptADCFixed[2].pt.y,
        pTSCRcalibrate->ptADCFixed[3].pt.x ,pTSCRcalibrate->ptADCFixed[3].pt.y);

#endif

}


/*---------------------- BEGIN OF STATE s_tscr_calibrate ------------------------*/
void inittscr_calibrate(void)
{
    ScreenSaverDisable();

    pTSCRcalibrate = (T_TSCRcalibrate *)Fwl_Malloc(sizeof(T_TSCRcalibrate));
    AK_ASSERT_PTR_VOID(pTSCRcalibrate, "initstdb_missed_call_box(): malloc error");

    InitializeVal();


    m_regResumeFunc( resumeProc );
    m_regSuspendFunc( suspendProc);

    tscr_is_pressed = AK_FALSE;

    TopBar_DisableShow();

}

void exittscr_calibrate(void)
{
    ScreenSaverEnable();

    AK_DEBUG_OUTPUT(" ^^^^^^^^^^^ exittscr_calibrate ^^^^^^^^^^^^");

#ifdef TOUCH_SCR
	if (AK_FALSE == pTSCRcalibrate->calibSucc)
	{
		gs.nADCCoordMode = pTSCRcalibrate->nACDCoordModeBk;
	}
#endif

    Fwl_Free(pTSCRcalibrate);
    pTSCRcalibrate = AK_NULL;

}

void painttscr_calibrate(void)
{

    if(!pTSCRcalibrate->calibSucc)
    {
#ifdef OS_ANYKA
        T_POINT pt;
        T_U32   str_x;
        T_U16     strlen;
        T_TCHR  ShowHintStr[50];
        //Typho00002123   fix calling in refresh
        
        if (AK_TRUE == pTSCRcalibrate->bRefresh )
        {
            
            //show black screen  gb_stdb.FocusBkCL)
            Fwl_FillSolidRect(HRGB_LAYER, 
                0,
                0, 
                Fwl_GetLcdWidth() ,
                Fwl_GetLcdHeight(),
                S_TSCR_CALIBRATE_RECT_COLOR);

            //show calibrate point
            pt.x = (T_S16)displaySample[pTSCRcalibrate->CurCalibratePt].x;
            pt.y = (T_S16)displaySample[pTSCRcalibrate->CurCalibratePt].y;
            
            calibDrawPoint(HRGB_LAYER, pt, TS_CALIB_REC_BORD, COLOR_BLACK);

            Utl_TStrCpy(ShowHintStr, Res_GetStringByID(eRES_STR_CHECK_CALIBARATE_NOTE)); // "请点击十字校准!", "Touch 十 calibrate!"
            
            /* 获取居中的位置，by Dingliwei, for T300_00009170*/
            str_x = MAIN_LCD_WIDTH>UGetSpeciStringWidth(ShowHintStr, CURRENT_FONT_SIZE, Utl_UStrLen(ShowHintStr))?
                (MAIN_LCD_WIDTH-UGetSpeciStringWidth(ShowHintStr, CURRENT_FONT_SIZE, Utl_UStrLen(ShowHintStr)))/2:0;
            
            if (0 == str_x)
            {
                /* cut string*/
                strlen = Fwl_GetUStringDispNum(ShowHintStr, Utl_UStrLen(ShowHintStr), 
                    Fwl_GetLcdWidth(), CURRENT_FONT_SIZE);                
            }
            else
            {
                strlen = Utl_TStrLen(ShowHintStr);
            }
            
            Fwl_UDispSpeciString(HRGB_LAYER, (T_POS)str_x, ((Fwl_GetLcdHeight() - g_Font.CHEIGHT) / 2), ShowHintStr, COLOR_BLACK, 
                CURRENT_FONT_SIZE, strlen);         
                        
            Fwl_InvalidateRect( 0, 0, 0, 0 );
            pTSCRcalibrate->bRefresh = AK_FALSE ;
        }
#endif //#ifdef OS_ANYKA    
    }
    else
    {
        MsgBox_Show(&pTSCRcalibrate->msgbox);
        Fwl_InvalidateRect( 0, 0, 0, 0 );
        
    }
    

}

#ifdef TOUCH_SCR
/************************************************************************
 * @BRIEF:    get the distance of  two point
 * @AUTHOR:   
 * @DATE:     
 * @Modified:   
 * @PARAM:    const T_POINT* p1:point one
 * @PARAM:    const T_POINT* p2:point two
 * @RETURN:   T_U32:the distance
 * @RETVAL:
 **************************************************************************/

static T_U32 get_distance(const T_POINT* p1, const T_POINT* p2 )
{
    return (p1->x - p2->x)*(p1->x - p2->x) + (p1->y - p2->y)*(p1->y - p2->y);
}

/*
static T_BOOL  PointInFixedArea(T_TSPOINT *ADCpt, T_ADCPOINT * ptADCFixed, T_U32 nFixPtNum)
{
    T_BOOL nRtn = AK_FALSE;
    T_U32 i = 0;
    for(i = 0; i < nFixPtNum; i++){
        if(AK_FALSE == ptADCFixed[i].bHasTouched){
            if(get_distance(ADCpt,&(ptADCFixed[i].pt)) <= ERROR_POINT_ABSOLUTE * ERROR_POINT_ABSOLUTE ){ //has touch the right area
                ptADCFixed[i].bHasTouched = AK_TRUE;
                nRtn = AK_TRUE;
            }
        }
    }
    return nRtn;

}*/


static T_U32 SetADCCoordSys(T_POINT * ptADCArray)
{
    //T_U32   i = 0;
    T_U32   nCoordMode = 0;

    // similar with draw a line in the screen from left to right
    // to set the x coordinate 
    if( diffU8(ptADCArray[0].x,ptADCArray[1].x) > diffU8(ptADCArray[0].y,ptADCArray[1].y)){
        //Y coordinate  is constance
        if(ptADCArray[0].x < ptADCArray[1].x){ // x coordinate  increase
            nCoordMode = nCoordMode | 0x00; 
        }
        else{ // x coordinate decrease
            nCoordMode = nCoordMode | 0x04;
        }
    }
    else{ //X coordinate is constance
        if(ptADCArray[0].y < ptADCArray[1].y){// y coordinate increase
            nCoordMode = nCoordMode | 0x08;
        }
        else{// y coordinate decrease
            nCoordMode = nCoordMode | 0x0c;
        }
    }

    // similar with draw a line in the screen from top to bottom
    // to set the Y coordinate 
    if(diffU8(ptADCArray[1].x,ptADCArray[2].x) < diffU8(ptADCArray[1].y,ptADCArray[2].y)){
        //X coordinate is constance
        if(ptADCArray[1].y < ptADCArray[2].y){// y coordinate increase
            nCoordMode = nCoordMode | 0x0; 
        }
        else{ // y coordinate decrease
            nCoordMode = nCoordMode | 0x01;   
        }
    }
    else{
        //Y coordinate is constance
        if(ptADCArray[1].x < ptADCArray[2].x){
            //x coordinate increase
            nCoordMode = nCoordMode | 0x02;
        }
        else{// x coordinate decrease
            nCoordMode = nCoordMode | 0x03;
        }
    }

    return nCoordMode;

}


static T_VOID ReSetPtCoord(T_POINT *ptADCModified,T_POINT * ptADCArray,T_U32 nPtNum)
{
    T_U32   i = 0;
    T_S32   x = 0;
    T_S32   y = 0;
    for(i = 0; i < nPtNum; i++){
        tsCalculateCoordinate_point(&x,&y,ptADCArray[i]);
        ptADCModified[i].x = x;
        ptADCModified[i].y = y;
    }
}
#endif

unsigned char handletscr_calibrate(vT_EvtCode event, vT_EvtParam* pEventParm)
{    

#ifdef TOUCH_SCR

    T_U8    ret=AK_TRUE;
    T_U32   nCount = 0;   

    T_U32   nTmpCount = 0;

    if(IsPostProcessEvent(event))
    {
        if ( M_EVT_Z99COM_POWEROFF == event )
        {
            MsgBox_InitAfx(&pTSCRcalibrate->msgbox, 1, ctHINT, csFAILURE, MSGBOX_INFORMATION );

            MsgBox_Show(&pTSCRcalibrate->msgbox);
            Fwl_InvalidateRect( 0, 0, 0, 0 );
        }
        else
        {
            //show the failed message and calibrate again
            pTSCRcalibrate->CurCalibratePt  =  0 ;
        }
        pTSCRcalibrate->bRefresh = AK_TRUE;
        return 1;
    }

    /*----------------add on Aug 11,2006-----------------*/
    if (event == M_EVT_TSCR_CALIBRATE)//if init Calibrate
    {
        pTSCRcalibrate->bFromInit = (T_BOOL)pEventParm->c.Param8;
        Fwl_Print(C3, M_SETTING, "pTSCRcalibrate->bFromInit = %d", pTSCRcalibrate->bFromInit);
    }
    /*----------------add on Aug 11,2006-----------------*/
    
    if (M_EVT_TOUCH_SCREEN == event)
    {
        switch (pEventParm->s.Param1) 
        {
        case eTOUCHSCR_MOVE:
#ifdef OS_ANYKA
			if (bValidADPoint)
			{
            	bValidADPoint = Fwl_tscr_getCurADPt_point(&tscr_last_move_point);
            }
#endif
            break;
            
        case eTOUCHSCR_DOWN:
#ifdef OS_ANYKA            
            tscr_is_pressed = AK_TRUE;
            bValidADPoint = Fwl_tscr_getCurADPt_point(&tscr_down_point);
            tscr_last_move_point = tscr_down_point;
#endif
            break;            
            
        case eTOUCHSCR_UP:
            if(!tscr_is_pressed || !bValidADPoint) {
                break;
            }
            tscr_is_pressed = AK_FALSE;

            pTSCRcalibrate->bRefresh = AK_TRUE;

            Fwl_Print(C3,M_SETTING,"the %dth point , coordinate mode is %d",pTSCRcalibrate->CurCalibratePt,gs.nADCCoordMode);

            Fwl_Print(C3,M_SETTING,"down point (%d %d) , last moved point (%d %d)",tscr_down_point.x,tscr_down_point.y,tscr_last_move_point.x,tscr_last_move_point.y );

            //the point touched on can not move (ERROR_POINT_ABSOLUTE * 6 = 360)  unit far            
            if(pTSCRcalibrate->CurCalibratePt < CALIB_POINT_MAX - 2 && get_distance(&tscr_down_point, &tscr_last_move_point) >= ERROR_TOUCH_SLIDE * ERROR_TOUCH_SLIDE)
            {
                Fwl_Print(C3,M_SETTING,"moved too far , touch again");
                break;
            }

            pTSCRcalibrate->tsSamplePt[pTSCRcalibrate->CurCalibratePt] = tscr_down_point;

#ifdef OS_ANYKA

#if 0
            ret = PointInFixedArea(&tscr_down_point,pTSCRcalibrate->ptADCFixed,CALIB_POINT_MAX - 1);
            if(AK_FALSE == ret){ // point touched do not in the right area(one of four)
                Fwl_Print(C3, M_SETTING, " point do not in the right area ");
                break; 
            }
#endif 

            nTmpCount = pTSCRcalibrate->CurCalibratePt;
            
              //the point in one of the four point area in the Touch Screen          
            if(CALIB_POINT_MAX - 2 > nTmpCount){
                pTSCRcalibrate->CurCalibratePt ++ ;            
            }           
            else if(CALIB_POINT_MAX - 2 == nTmpCount){
                T_U32 tmpCount = nRepeatTime;
                //T_U32 ResetFlag = 0;
                T_U32   tmpDist = 0;

                if(tmpCount < 1 /*MAX_REPEAT_TIME*/){

                    T_U32  nPtToReset = 0;

                    nRepeatTime++; 
                    Fwl_Print(C3, M_SETTING,"nReatTime is %d",nRepeatTime);

                    if( 1 == nRepeatTime ){
                        gs.nADCCoordMode = SetADCCoordSys(pTSCRcalibrate->tsSamplePt);

                        Fwl_Print(C3, M_SETTING, "gs.nADCcoordinate is %d" , gs.nADCCoordMode);
                        
                        //reset the coordinate of  4 sample point in new coordinate system     
                        nPtToReset = CALIB_POINT_MAX - 1; //ptADCModified                    
                    }               
                    else{
                        nPtToReset = CALIB_POINT_MAX - 2; //ptADCModified                   
                        ptADCModified[3] = pTSCRcalibrate->tsSamplePt[3];
                    }

                    Fwl_Print(C3, M_SETTING,"the raw four points are (%d,%d),(%d,%d) ,(%d,%d),(%d,%d)",
                        pTSCRcalibrate->tsSamplePt[0].x,pTSCRcalibrate->tsSamplePt[0].y,
                        pTSCRcalibrate->tsSamplePt[1].x,pTSCRcalibrate->tsSamplePt[1].y,
                        pTSCRcalibrate->tsSamplePt[2].x,pTSCRcalibrate->tsSamplePt[2].y,
                        pTSCRcalibrate->tsSamplePt[3].x,pTSCRcalibrate->tsSamplePt[3].y);

             
                    //reset the coordinate of  4 sample point in new coordinate system     
                    ReSetPtCoord(ptADCModified,pTSCRcalibrate->tsSamplePt,nPtToReset); //ptADCModified                    

                    Fwl_Print(C3, M_SETTING,"the modified four points are (%d,%d),(%d,%d) ,(%d,%d),(%d,%d)",
                        ptADCModified[0].x,ptADCModified[0].y,
                        ptADCModified[1].x,ptADCModified[1].y,
                        ptADCModified[2].x,ptADCModified[2].y,
                        ptADCModified[3].x,ptADCModified[3].y);      

                    if(!((ptADCModified[1].x - ptADCModified[0].x) > diffU8(ptADCModified[0].y,ptADCModified[1].y)
                        && diffU8(ptADCModified[1].x,ptADCModified[2].x) < (ptADCModified[2].y - ptADCModified[1].y)
                        && (ptADCModified[2].x - ptADCModified[3].x) > diffU8(ptADCModified[3].y,ptADCModified[2].y)
                        && (ptADCModified[3].y - ptADCModified[0].y) > diffU8(ptADCModified[0].x,ptADCModified[3].x)                
                    )){
                    //do not obey the ruler that : point1.x > point0.x  && point2.y > point1.y && point2.x > point3.x && point3.y > point0.y
                        Fwl_Print(C3, M_SETTING,"the four points are not obey the order");  
                        {
                            //restart to calibrate
                            gs.nADCCoordMode = pTSCRcalibrate->nACDCoordModeBk;
                            InitializeVal();
                            nRepeatTime = 0;  

                        }
                        
                        break;
                    
                    }


                    ret = setCalibrationMatrix (displaySample,ptADCModified ,&gs_matrixPtr);

                    if(AK_FALSE == ret){
                        Fwl_Print(C3, M_SETTING, " fail to set the calibrate Matrix ");
                        gs.nADCCoordMode = pTSCRcalibrate->nACDCoordModeBk;
                        nRepeatTime = 0;

                        {
                            //restart to calibrate
                            gs.nADCCoordMode = pTSCRcalibrate->nACDCoordModeBk;
                            InitializeVal();
                            nRepeatTime = 0;                              
                        }
                        
                        return 0;                     
                    }    

                    getLCDPointByMatrix( &ADCSampleHope,
                        &ptADCModified[pTSCRcalibrate->CurCalibratePt],
                        &gs_matrixPtr );   
                    
                    Fwl_Print(C3,M_SETTING, "the 4th sample is (%d %d) , map to (%d %d)",
                        ptADCModified[pTSCRcalibrate->CurCalibratePt].x,ptADCModified[pTSCRcalibrate->CurCalibratePt].y,
                        ADCSampleHope.x,ADCSampleHope.y);

                    tmpDist = get_distance(&displaySample[pTSCRcalibrate->CurCalibratePt],&ADCSampleHope);
                    Fwl_Print(C3,M_SETTING, "the distance error is %d",tmpDist);


                    
                    if(get_distance(&displaySample[pTSCRcalibrate->CurCalibratePt],&ADCSampleHope) \
                        > ERROR_POINT_ABSOLUTE_LCD * ERROR_POINT_ABSOLUTE_LCD ){
                        //failed to check the matrix
                        Fwl_Print(C3, M_SETTING, "failed to check, resample the 4th point, nRepeatTime = %d",nRepeatTime);
                        gs.nADCCoordMode = pTSCRcalibrate->nACDCoordModeBk; 

                        {
                            //restart to calibrate
                            gs.nADCCoordMode = pTSCRcalibrate->nACDCoordModeBk;
                            InitializeVal();
                            nRepeatTime = 0;
                        }
                    }
                    else {// succeed to check the Matrix

                            Fwl_Print(C3, M_SETTING, " succeed to check the calibrate matrix");  

                            for(nCount = 0; nCount < CALIB_POINT_MAX - 1; nCount++){
                                gs.matrixPtr.X[nCount] = gs_matrixPtr.X[nCount];
                                gs.matrixPtr.Y[nCount] = gs_matrixPtr.Y[nCount];                            
                            }                                

                            Fwl_Print(C3, M_SETTING, " get ret  oK Result is  %d, %d %d %d %d %d %d  %",
                                gs.matrixPtr.X[0],
                                gs.matrixPtr.X[1],
                                gs.matrixPtr.X[2],
                                gs.matrixPtr.X[3],
                                gs.matrixPtr.Y[1],
                                gs.matrixPtr.Y[2],
                                gs.matrixPtr.Y[3],
                                gs.nADCCoordMode);

                            pTSCRcalibrate->calibSucc = AK_TRUE;
                            nRepeatTime = 0;
                            
                            MsgBox_InitAfx(&pTSCRcalibrate->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION );
                            MsgBox_SetDelay(&pTSCRcalibrate->msgbox, MSGBOX_DELAY_1);
                            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pTSCRcalibrate->msgbox);                    

                    }


                }
                else if(tmpCount >= 1 /*MAX_REPEAT_TIME*/){
                    // restart to calibrate 
                    Fwl_Print(C3,M_SETTING,"restart to calibrate");
                    gs.nADCCoordMode = pTSCRcalibrate->nACDCoordModeBk;
                    InitializeVal();
                    nRepeatTime = 0;                    
                }


            }
           


#endif
            break;
        default:
            //other event  return;
            return eStay;
        }             
    }
    
    if (event == M_EVT_USER_KEY)
    {      
        /*----------------add on Aug 11,2006-----------------*/
        if (kbCLEAR == (T_eKEY_ID)pEventParm->c.Param1)
        {    
            if(AK_FALSE == pTSCRcalibrate->bFromInit)
            {    
                AK_DEBUG_OUTPUT("exit 1");
                gs.nADCCoordMode = pTSCRcalibrate->nACDCoordModeBk;
                m_triggerEvent(M_EVT_CALIBRATE_END, pEventParm);
                return 0;
            }
        }
        if (kbSTART_MODULE == (T_eKEY_ID)pEventParm->c.Param1)
        {
            if(AK_TRUE == pTSCRcalibrate->bFromInit)
            {
                AK_DEBUG_OUTPUT("exit 2");
                m_triggerEvent(M_EVT_CALIBRATE_END, pEventParm);
                return 0;
            }
        }
        /*----------------add on Aug 11,2006-----------------*/
    }             

    

#endif
    
    return 0;
}


