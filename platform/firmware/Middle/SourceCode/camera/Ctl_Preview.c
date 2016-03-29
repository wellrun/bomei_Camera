/*
MODELNAME: PREVIEW CONTROL
DISCRIPTION: BASE FOR CAPTURE CONTROL AND VIDEO RECORD CONTROL
AUTHOR:ZHENGWENBO
DATE:2006-6-29
*/
#include "eng_string.h"

#ifdef CAMERA_SUPPORT

#include "eng_gblstring.h"
#include "Fwl_pfCamera.h"
#include "ctl_msgbox.h"
#include "eng_graph.h"
#include "ctl_preview.h"
#include "eng_keymapping.h"
#include "eng_font.h"
#include "fwl_osmalloc.h"
#include "Fwl_Image.h"
#include "Ctl_IconExplorer.h"
#include "Eng_AkBmp.h"
#include "Fwl_pfDisplay.h"
#include "lib_image_api.h"
#include "Eng_ImgConvert.h"
#include "Eng_dynamicfont.h"
#include "eng_graph.h"
#include "Ctl_Capture.h"
#include "Eng_DataConvert.h"
#include "fwl_oscom.h"
#include "lib_media_struct.h"
#include "config.h"
#include "fwl_pfcamera.h"
#include "hal_camera.h"
#include "Fwl_display.h"
#include "arch_gui.h"
#include "Fwl_tscrcom.h"
#include "fwl_graphic.h"
#include "log_videoZoom.h"
#include "Fwl_DispOsd.h"

extern T_BOOL Is_04CHIP(T_VOID);

T_BOOL gbCameraPreviewOsdFreshFlag = AK_FALSE;


static T_BOOL Preview_StartTimer(T_TIMER* pTimer, T_U32 time, T_BOOL loop);
static T_VOID Preview_StopTimer(T_TIMER* pTimer);
static T_VOID Preview_DecCapDelayCount(T_PREVIEW *pPreview);
static T_BOOL Preview_GetDispWin(T_PREVIEW *pPreview , T_RECT *pRect);
static T_CAMERA_MODE Preview_GetCurFrameMode(T_PREVIEW* pPreview, T_U8 camMode);
static T_VOID Preview_GetWinSize(T_PREVIEW* pPreview, T_U8 camMode, T_CAMERA_MODE mode, T_U32 *pWidth, T_U32 *pHeight);
static T_BOOL Preview_Paint_Yuv(T_PREVIEW *pPreview, T_RECT *pRect);
static T_VOID Preview_showIcon(T_PREVIEW *pPreview, T_RECT *pRect);

#define VIDEO_RECORD_LCD_WIDTH          (Fwl_GetLcdWidth())
#define VIDEO_RECORD_LCD_HEIGHT         (Fwl_GetLcdHeight())


/**************************# start of camera toolbar defining.****************************/
#if (LCD_CONFIG_WIDTH == 800)

#define CAM_BUTTON_WIDTH                60
#define CAM_BUTTON_HEIGHT               42
#define CAM_BUTTON_INTERVAL             2

#else
#if (LCD_CONFIG_WIDTH == 480)

#define CAM_BUTTON_WIDTH                36
#define CAM_BUTTON_HEIGHT               25
#define CAM_BUTTON_INTERVAL             2

#else
#if (LCD_CONFIG_WIDTH == 320)

#define CAM_BUTTON_WIDTH                36
#define CAM_BUTTON_HEIGHT               25
#define CAM_BUTTON_INTERVAL             2

#endif
#endif
#endif
//#define CAM_ToolBar_FontColor           COLOR_ORANGE
//#define CAM_ToolBar_BACKGROUND_COLOR    0x00808000 //(RGB:128,128,0)
#define CAM_ToolBar_FontColor           0x00ffffff
#define CAM_ToolBar_BACKGROUND_COLOR    0x00010101 //(RGB:128,128,0)



typedef enum
{
    CAM_TB_TRANS_NONE     = 0,          //不透明
    CAM_TB_TRANS_QUARTER  = 3,         //四分之一透明
    CAM_TB_TRANS_THIRD    = 5,         //三分之一透明
    CAM_TB_TRANS_HALF     = 8,         //一半透明
    CAM_TB_TRANS_2PART    = 10,         //三分之二透明
    CAM_TB_TRANS_3QUARTER = 12,        //四分之三透明
    CAM_TB_TRANS_TOTALLY  = 15         //完全透明
}T_CAM_TOOLBAR_TRANS;

T_RECT PreView2DWin = {0};


static T_eBACK_STATE Preview_UserKey_Handle(T_PREVIEW* pPreview, T_MMI_KEYPAD phyKey);
static T_MMI_KEYPAD Preview_MapTSCR_To_Key(T_PREVIEW* pPreview, T_POS x, T_POS y);
static T_VOID Preview_ToolBarGetButtonRes(T_CAM_BUTON_ICON *pButtonIcon)
{
    T_U32   len;
    T_U32   id = 0;

    pButtonIcon->CapRecSwitch.Id = 10 * (id++);
    if (gs.CamMode == CAM_DC)
        Utl_UStrCpy(pButtonIcon->CapRecSwitch.Name, Res_GetStringByID(eRES_STR_CMR_SWITCH_TO_DV));
    else
        Utl_UStrCpy(pButtonIcon->CapRecSwitch.Name, Res_GetStringByID(eRES_STR_CMR_SWITCH_TO_DC));
    pButtonIcon->CapRecSwitch.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL,AK_TRUE, eRES_BMP_CAMERA_MODESWITCH_NORMAL, &len);
    pButtonIcon->CapRecSwitch.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_MODESWITCH_FOCUS, &len);
    pButtonIcon->CapRecSwitch.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_MODESWITCH_DOWN, &len);

    pButtonIcon->ModeSelect.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->ModeSelect.Name, Res_GetStringByID(eRES_STR_CMR_CAPTURE_PHOTO));
    pButtonIcon->ModeSelect.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_CAPMODE_NORMAL, &len);
    pButtonIcon->ModeSelect.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_CAPMODE_FOCUS, &len);
    pButtonIcon->ModeSelect.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_CAPMODE_DOWN, &len);

    pButtonIcon->PhotoSize.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->PhotoSize.Name, Res_GetStringByID(eRES_STR_CMP_SIZE));
    pButtonIcon->PhotoSize.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_SIZE_NORMAL, &len);
    pButtonIcon->PhotoSize.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_SIZE_FOCUS, &len);
    pButtonIcon->PhotoSize.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_SIZE_DOWN, &len);

    pButtonIcon->KacaMode.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->KacaMode.Name, Res_GetStringByID(eRES_STR_CMR_KACHAMODE));
    pButtonIcon->KacaMode.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_KACA_NORMAL, &len);
    pButtonIcon->KacaMode.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_KACA_FOCUS, &len);
    pButtonIcon->KacaMode.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_KACA_DOWN, &len);
    
    pButtonIcon->Brightness.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->Brightness.Name, Res_GetStringByID(eRES_STR_CMP_BRIGHTNESS));
    pButtonIcon->Brightness.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_BRIGHTNESS_NORMAL, &len);
    pButtonIcon->Brightness.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_BRIGHTNESS_FOCUS, &len);
    pButtonIcon->Brightness.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_BRIGHTNESS_DOWN, &len);

    pButtonIcon->Contrast.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->Contrast.Name, Res_GetStringByID(eRES_STR_CMP_CONTRAST));
    pButtonIcon->Contrast.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_CONTRAST_NORMAL, &len);
    pButtonIcon->Contrast.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_CONTRAST_FOCUS, &len);
    pButtonIcon->Contrast.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_CONTRAST_DOWN, &len);

    pButtonIcon->Saturation.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->Saturation.Name, Res_GetStringByID(eRES_STR_CMP_COLOR_SATURATION));
    pButtonIcon->Saturation.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_SATURATION_NORMAL, &len);
    pButtonIcon->Saturation.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_SATURATION_FOCUS, &len);
    pButtonIcon->Saturation.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_SATURATION_DOWN, &len);

    pButtonIcon->NightMode.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->NightMode.Name, Res_GetStringByID(eRES_STR_CMR_NIGHTMODE));
    pButtonIcon->NightMode.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_NIGHTMODE_NORMAL, &len);
    pButtonIcon->NightMode.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_NIGHTMODE_FOCUS, &len);
    pButtonIcon->NightMode.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_NIGHTMODE_DOWN, &len);

    pButtonIcon->DelayCap.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->DelayCap.Name, Res_GetStringByID(eRES_STR_CMR_TIME_CAPTURE));
    pButtonIcon->DelayCap.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_DELAYCAP_NORMAL, &len);
    pButtonIcon->DelayCap.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_DELAYCAP_FOCUS, &len);
    pButtonIcon->DelayCap.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_DELAYCAP_DOWN, &len);

    pButtonIcon->FlashLight.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->FlashLight.Name, Res_GetStringByID(eRES_STR_CMR_FLASHLIGHT_SETUP));
    pButtonIcon->FlashLight.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_FLASHLIGHT_NORMAL, &len);
    pButtonIcon->FlashLight.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_FLASHLIGHT_FOCUS, &len);
    pButtonIcon->FlashLight.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_FLASHLIGHT_DOWN, &len);

    pButtonIcon->PhotoQuality.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->PhotoQuality.Name, Res_GetStringByID(eRES_STR_CMR_PHOTO_QUALITY));
    pButtonIcon->PhotoQuality.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_PHOTOQLTY_NORMAL, &len);
    pButtonIcon->PhotoQuality.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_PHOTOQLTY_FOCUS, &len);
    pButtonIcon->PhotoQuality.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_PHOTOQLTY_DOWN, &len);

    pButtonIcon->SavePath.Id = 10 * (id++);
    if (gs.CamMode == CAM_DC)
        Utl_UStrCpy(pButtonIcon->SavePath.Name, Res_GetStringByID(eRES_STR_CMR_CAPTURE_PATH_SETUP));
    else
        Utl_UStrCpy(pButtonIcon->SavePath.Name, Res_GetStringByID(eRES_STR_CMR_VIDEOREC_PATH_SETUP));
    pButtonIcon->SavePath.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_SETPATH_NORMAL, &len);
    pButtonIcon->SavePath.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_SETPATH_FOCUS, &len);
    pButtonIcon->SavePath.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_SETPATH_DOWN, &len);

    pButtonIcon->RecFileType.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->RecFileType.Name, Res_GetStringByID(eRES_STR_CMR_REC_FILETYPE));
    pButtonIcon->RecFileType.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_RECFILETYPE_NORMAL, &len);
    pButtonIcon->RecFileType.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_RECFILETYPE_FOCUS, &len);
    pButtonIcon->RecFileType.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_RECFILETYPE_DOWN, &len);

    pButtonIcon->DetectMode.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->DetectMode.Name, Res_GetStringByID(eRES_STR_CMR_REC_DETECTMODE));
    pButtonIcon->DetectMode.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_DETECTMODE_NORMAL, &len);
    pButtonIcon->DetectMode.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_DETECTMODE_FOCUS, &len);
    pButtonIcon->DetectMode.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_DETECTMODE_DOWN, &len);

    pButtonIcon->CycleMode.Id = 10 * (id++);
    Utl_UStrCpy(pButtonIcon->CycleMode.Name, Res_GetStringByID(eRES_STR_CMR_REC_CYCLEMODE));
    pButtonIcon->CycleMode.stateIcon[BTN_STATE_NORMAL] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_CYCLEMODE_NORMAL, &len);
    pButtonIcon->CycleMode.stateIcon[BTN_STATE_FOCUS] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_CYCLEMODE_FOCUS, &len);
    pButtonIcon->CycleMode.stateIcon[BTN_STATE_DOWN] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_CYCLEMODE_DOWN, &len);
}

///////////////////////////////////////////////////////////////////////////////
static T_VOID Preview_IconInit(T_PREVIEW *pPreview)
{
    AK_ASSERT_PTR_VOID(pPreview, "Preview_IconInit(): input parameter is error\n");

    pPreview->pCamDCMode = AK_NULL;
    pPreview->pCamDVMode = AK_NULL;
    pPreview->pCamMenu = AK_NULL;
    pPreview->pCamOk = AK_NULL;
    pPreview->pCamRet = AK_NULL;
}

static T_VOID Preview_SetIconRect(T_PREVIEW *pPreview)
{
    AK_ASSERT_PTR_VOID(pPreview, "Preview_SetIconRect(): input parameter is error\n");
    
    //ok icon rect
    pPreview->CamOkRect.left =  CAMMODE_PIC_LEFT;
    pPreview->CamOkRect.top =  CAMOK_PIC_TOP;
    pPreview->CamOkRect.width =  CAMMENU_PIC_WIDTH;
    pPreview->CamOkRect.height =  CAMMENU_PIC_HEIGHT;

    pPreview->CamDCModeRect.left =  CAMMODE_PIC_LEFT;
    pPreview->CamDCModeRect.top =  CAMMODE_PIC_TOP;
    pPreview->CamDCModeRect.width =  CAMMODE_PIC_WIDTH;
    pPreview->CamDCModeRect.height =  CAMMODE_PIC_HEIGHT ;

    pPreview->CamDVModeRect.left =  CAMMODE_PIC_LEFT;
    pPreview->CamDVModeRect.top =  CAMMODE_PIC_TOP/* + CAMMODE_PIC_HEIGHT / 2*/;
    pPreview->CamDVModeRect.width =  CAMMODE_PIC_WIDTH;
    pPreview->CamDVModeRect.height =  CAMMODE_PIC_HEIGHT ;

    pPreview->CamMenuRect.left =  CAMMENU_PIC_LEFT;
    pPreview->CamMenuRect.top =  CAMMENU_PIC_TOP;
    pPreview->CamMenuRect.width =  CAMMENU_PIC_WIDTH;
    pPreview->CamMenuRect.height =  CAMMENU_PIC_HEIGHT;

    //return icon rect
    pPreview->CamRetRect.left =  CAMMODE_PIC_LEFT;
    pPreview->CamRetRect.top =  CAMRET_PIC_TOP;
    pPreview->CamRetRect.width =  CAMMENU_PIC_WIDTH;
    pPreview->CamRetRect.height =  CAMMENU_PIC_HEIGHT;

}

static T_VOID Preview_GetIconRes(T_PREVIEW *pPreview)
{
    T_U32        len;

    AK_ASSERT_PTR_VOID(pPreview, "Preview_GetIconRes(): input parameter is error\n");
    
    pPreview->pCamDCMode = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_CAMERA_MODE_CAPTURE, &len);

    pPreview->pCamDVMode = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_CAMERA_MODE_RECORD, &len);

    pPreview->pCamMenu = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_CAMERA_MENU, &len);

    pPreview->pCamOk = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_CAMERA_OK, &len);

    pPreview->pCamRet = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_CAMERA_RETURN, &len);
}
  
static T_VOID Preview_showIcon(T_PREVIEW *pPreview, T_RECT *pRect)
{
    T_STR_INFO tmpstr;
    T_U32 width,height;
    
    if (gs.CamMode == CAM_DC)
    {

//        if (!Fwl_AkBmpDrawFromString(HRGB_LAYER, pPreview->CamDCModeRect.left, pPreview->CamDCModeRect.top, pPreview->pCamDCMode, AK_NULL, AK_FALSE))
//			Fwl_Print(C3, M_CAMERA,  "Draw DC Icon Failure\n");
		Fwl_Osd_DrawStreamBmpByGray(&pPreview->CamDCModeRect , pPreview->pCamDCMode);
    }
    else
    {
//        if(!Fwl_AkBmpDrawFromString(HRGB_LAYER, pPreview->CamDVModeRect.left, pPreview->CamDVModeRect.top, pPreview->pCamDVMode, AK_NULL, AK_FALSE))
//			Fwl_Print(C3, M_CAMERA,  "Draw DV Icon Failure\n");
		Fwl_Osd_DrawStreamBmpByGray(&pPreview->CamDVModeRect , pPreview->pCamDVMode);
    }
    
//    if (!Fwl_AkBmpDrawFromString(HRGB_LAYER, pPreview->CamMenuRect.left, pPreview->CamMenuRect.top, pPreview->pCamMenu, AK_NULL, AK_FALSE))
//		Fwl_Print(C3, M_CAMERA,  "Draw Cam MENU Icon Failure\n");
	Fwl_Osd_DrawStreamBmpByGray(&pPreview->CamMenuRect , pPreview->pCamMenu);
					
//    if (!Fwl_AkBmpDrawFromString(HRGB_LAYER, pPreview->CamOkRect.left, pPreview->CamOkRect.top, pPreview->pCamOk, AK_NULL, AK_FALSE))
//		Fwl_Print(C3, M_CAMERA,  "Draw Cam OK Icon Failure\n");
	Fwl_Osd_DrawStreamBmpByGray(&pPreview->CamOkRect , pPreview->pCamOk);
		
//    if (!Fwl_AkBmpDrawFromString(HRGB_LAYER, pPreview->CamRetRect.left, pPreview->CamRetRect.top, pPreview->pCamRet, AK_NULL, AK_FALSE))
//		Fwl_Print(C3, M_CAMERA,  "Draw Cam RETURN Icon Failure\n");
	Fwl_Osd_DrawStreamBmpByGray(&pPreview->CamRetRect , pPreview->pCamRet);

    // zoom level
    if (ERROR_TIMER != pPreview->zoomlevel_shown_timer_id)
    {
        sprintf(tmpstr, "x%d.0", (pPreview->camFocusLevel+1));

//        Fwl_DispString(HRGB_LAYER, pRect->left,pRect->top,
//                    tmpstr, (T_U16)strlen(tmpstr), COLOR_YELLOW, CURRENT_FONT_SIZE);
		Fwl_Osd_DrawStringByGray( pRect->left, pRect->top 
			,tmpstr, (T_U16)strlen(tmpstr), COLOR_WHITE,CURRENT_FONT_SIZE);
    }

    //shot mode
    if (gs.CamMode == CAM_DC)
    {
        if (gs.DCShotMode == DC_NORMAL_SHOT)
            strcpy(tmpstr, "Normal");
        else
            strcpy(tmpstr, "Multi");

//        Fwl_DispString(HRGB_LAYER, pRect->left, (T_POS)(pRect->top + pRect->height - 40),
//                    tmpstr, (T_U16)strlen(tmpstr), COLOR_YELLOW, CURRENT_FONT_SIZE);
                    
		Fwl_Osd_DrawStringByGray( pRect->left, pRect->top + pRect->height - 40
			,tmpstr, strlen(tmpstr), COLOR_WHITE,CURRENT_FONT_SIZE);
       
    }
    else if (CAM_DV == gs.CamMode)
    {
        if (gs.CamRecFileType == eRECORD_MEDIA_3GP_MPEG4_AMR)
            strcpy(tmpstr, "3GP");
        else
            strcpy(tmpstr, "AVI");
//         Fwl_DispString(HRGB_LAYER, pRect->left, (T_POS)(pRect->top + pRect->height - 40),
//                    tmpstr, (T_U16)strlen(tmpstr), COLOR_YELLOW, CURRENT_FONT_SIZE);
   		Fwl_Osd_DrawStringByGray( pRect->left, pRect->top + pRect->height - 40
			,tmpstr, strlen(tmpstr), COLOR_WHITE,CURRENT_FONT_SIZE);
    }

    Fwl_GetRecFrameSize(Preview_GetCurFrameMode(pPreview, gs.CamMode),
                        &width, &height);
    sprintf(tmpstr, "%ld", width);
    
//    Fwl_DispString(HRGB_LAYER, pRect->left, (T_POS)(pRect->top + pRect->height - 20),
//               tmpstr, (T_U16)strlen(tmpstr), COLOR_YELLOW, CURRENT_FONT_SIZE);
    Fwl_Osd_DrawStringByGray(pRect->left, (T_POS)(pRect->top + pRect->height - 20),
               tmpstr, (T_U16)strlen(tmpstr), COLOR_WHITE, CURRENT_FONT_SIZE);

    //delay time
    if (ERROR_TIMER != pPreview->delay_shot_tm_id)
    {
        sprintf(tmpstr, "%d", pPreview->shot_delay_count);

//        Fwl_DispString(HRGB_LAYER, (T_POS)(pRect->left + (pRect->width>>1)),
//        (T_POS)(pRect->top + pRect->height - 20), tmpstr, (T_U16)strlen(tmpstr), COLOR_YELLOW, CURRENT_FONT_SIZE);
        Fwl_Osd_DrawStringByGray((T_POS)(pRect->left + (pRect->width>>1)),
        (T_POS)(pRect->top + pRect->height - 20), tmpstr, (T_U16)strlen(tmpstr), COLOR_YELLOW, CURRENT_FONT_SIZE);
    }
}

T_BOOL Preview_ToolBarInitRecFileType(T_PREVIEW *pPreview)
{
    T_USTR_INFO UstrTmp;
  
      
    ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, \
        CAMERA_MODE_VGA, Eng_StrMbcs2Ucs_2("640 X 480", UstrTmp));
        
    ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, \
        CAMERA_MODE_QVGA, Eng_StrMbcs2Ucs_2("320 X 240", UstrTmp));

    ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, \
        CAMERA_MODE_CIF, Eng_StrMbcs2Ucs_2("352 X 288", UstrTmp));
    
    Fwl_Print(C3, M_CAMERA,  "Camera Type: %d\n", Fwl_CameraGetType());
/*
    if (Is_04CHIP()&& (SDRAM_MODE >= 16))
    {
        //support real and fake 1280 x 720
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id,
            CAMERA_MODE_720P, Eng_StrMbcs2Ucs_2("1280 X 720", UstrTmp));
    }
    else 
    if ((CAMERA_2M <= Fwl_CameraGetType()) && (CAMERA_5M >= Fwl_CameraGetType()) && (SDRAM_MODE >= 16))
    {
        // Open 720P Rec ONLY Camera Pixels More Than 2M
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id,\
            CAMERA_MODE_720P, Eng_StrMbcs2Ucs_2("1280 X 720", UstrTmp));   
    }
*/
    return AK_TRUE;

}

T_BOOL Preview_ToolBarNew(T_PREVIEW *pPreview)
{
    T_USTR_INFO UstrTmp;

    AK_ASSERT_PTR(pPreview, "Preview_ToolBarNew()>>pPreview is null", AK_FALSE);

    if (AK_NULL != pPreview->pToolBar)    
    {
        Fwl_Print(C3, M_CAMERA,  "pPreview->pToolBar is exist already! free it first!");
        pPreview->pToolBar = Fwl_Free(pPreview->pToolBar);
    }

    pPreview->pToolBar = (T_pTOOLBAR)Fwl_Malloc(sizeof(T_TOOLBAR));
    AK_ASSERT_PTR(pPreview->pToolBar, \
                 "Preview_ToolBarNew()>>pPreview->pToolBar malloc failed! ", AK_FALSE);

    ToolBar_Init(pPreview->pToolBar, TB_eBOTTOM, MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT, CAM_BUTTON_INTERVAL, \
                 CAM_BUTTON_WIDTH, CAM_BUTTON_HEIGHT, TB_eMODE_SHOWN_NORMAL, \
                 CAM_ToolBar_FontColor, CAM_ToolBar_BACKGROUND_COLOR, CAM_TB_TRANS_HALF);

    //get icon resources
    Preview_ToolBarGetButtonRes(&pPreview->ButtonIcon);

    //add buttons CapRecSwitch
    ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.CapRecSwitch.Id, \
                      BTN_TYPE_SWITCH, pPreview->ButtonIcon.CapRecSwitch.Name, \
                      pPreview->ButtonIcon.CapRecSwitch.stateIcon);
    ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.CapRecSwitch.Id, \
                      CAM_DC, Res_GetStringByID(eRES_STR_CMR_SWITCH_TO_DV));
    ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.CapRecSwitch.Id, \
                      CAM_DV, Res_GetStringByID(eRES_STR_CMR_SWITCH_TO_DC));
    ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.CapRecSwitch.Id, gs.CamMode);


    //just when (gs.CamMode == CAM_DC), it's need to add the ModeSelect button
    //otherwise add the Record File Type select button.
    if (gs.CamMode == CAM_DC)
    {
        //add button capmode select
        ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.ModeSelect.Id, \
                          BTN_TYPE_SUBMENU, pPreview->ButtonIcon.ModeSelect.Name, \
                          pPreview->ButtonIcon.ModeSelect.stateIcon);
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.ModeSelect.Id, \
                          DC_NORMAL_SHOT, Res_GetStringByID(eRES_STR_CMR_CAPTURE_NORMAL));
//        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.ModeSelect.Id, 
//                          DC_MULTI_SHOT, Res_GetStringByID(eRES_STR_CMR_CAPTURE_SUCCESSIVE));
//        ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.ModeSelect.Id, gs.DCShotMode);
        ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.ModeSelect.Id, DC_NORMAL_SHOT);

    }
    else
    {
        //add button RecFileType
        ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.RecFileType.Id, \
                          BTN_TYPE_SUBMENU, pPreview->ButtonIcon.RecFileType.Name, \
                          pPreview->ButtonIcon.RecFileType.stateIcon);
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.RecFileType.Id, \
                          eRECORD_MEDIA_AVI_MPEG4_PCM, Eng_StrMbcs2Ucs_2("AVI(MPEG4+PCM)", UstrTmp));
        
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.RecFileType.Id, \
                              eRECORD_MEDIA_AVI_MJPEG_PCM, Eng_StrMbcs2Ucs_2("AVI(MJPEG+PCM)", UstrTmp));
                              
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.RecFileType.Id, \
                              eRECORD_MEDIA_3GP_MPEG4_AMR, Eng_StrMbcs2Ucs_2("3GP(MPEG4+AMR)", UstrTmp));
        
        ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.RecFileType.Id, gs.CamRecFileType);

        //-----------------add button of detect mode------------------ 
        ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.DetectMode.Id, \
                          BTN_TYPE_SUBMENU, pPreview->ButtonIcon.DetectMode.Name, \
                          pPreview->ButtonIcon.DetectMode.stateIcon);
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.DetectMode.Id, \
                          0, Res_GetStringByID(eRES_STR_CMR_CAPTURE_NORMAL));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.DetectMode.Id, \
                          1, Res_GetStringByID(eRES_STR_CMR_REC_DETECTMODE));
        ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.DetectMode.Id, gb.isDetectMode);

        //-----------------add button of cycle mode------------------ 
        ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.CycleMode.Id, \
                          BTN_TYPE_SUBMENU, pPreview->ButtonIcon.CycleMode.Name, \
                          pPreview->ButtonIcon.CycleMode.stateIcon);
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.CycleMode.Id, \
                          VIDEO_REC_INTERVAL_NONE, Res_GetStringByID(eRES_STR_PUB_CLOSE));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.CycleMode.Id, \
                          VIDEO_REC_INTERVAL_2, Res_GetStringByID(eRES_STR_CMR_REC_CYCLEMODE2));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.CycleMode.Id, \
                          VIDEO_REC_INTERVAL_5, Res_GetStringByID(eRES_STR_CMR_REC_CYCLEMODE5));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.CycleMode.Id, \
                          VIDEO_REC_INTERVAL_15, Res_GetStringByID(eRES_STR_CMR_REC_CYCLEMODE15));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.CycleMode.Id, \
                          VIDEO_REC_INTERVAL_30, Res_GetStringByID(eRES_STR_CMR_REC_CYCLEMODE30));
        ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.CycleMode.Id, gb.isCycMode);
    }

    //add button Size
    ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, \
                      BTN_TYPE_SUBMENU, pPreview->ButtonIcon.PhotoSize.Name, \
                      pPreview->ButtonIcon.PhotoSize.stateIcon);
    if (CAM_DC == gs.CamMode)
    {
/*    
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, \
                          CAMERA_MODE_QVGA, Eng_StrMbcs2Ucs_2("320 X 240", UstrTmp));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, \
                          CAMERA_MODE_CIF, Eng_StrMbcs2Ucs_2("352 X 288", UstrTmp));     
*/                          
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, \
                          CAMERA_MODE_VGA, Eng_StrMbcs2Ucs_2("640 X 480", UstrTmp));
/*		ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, \
                          CAMERA_MODE_720P, Eng_StrMbcs2Ucs_2("1280 X 720", UstrTmp));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, \
                          CAMERA_MODE_SXGA, Eng_StrMbcs2Ucs_2("1280 X 1024", UstrTmp));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, \
                          CAMERA_MODE_UXGA, Eng_StrMbcs2Ucs_2("1600 X 1200", UstrTmp));*/
        ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, gs.camPhotoMode);
        
        //add button KacaMode
        ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.KacaMode.Id, \
                          BTN_TYPE_SWITCH, pPreview->ButtonIcon.KacaMode.Name, \
                          pPreview->ButtonIcon.KacaMode.stateIcon);
    
        Utl_UStrCpy(UstrTmp, Res_GetStringByID(eRES_STR_PUB_CLOSE));
        Utl_UStrCat(UstrTmp, Res_GetStringByID(eRES_STR_CMR_KACHAMODE));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.KacaMode.Id, \
                          AK_TRUE, UstrTmp);
        Utl_UStrCpy(UstrTmp, Res_GetStringByID(eRES_STR_PUB_OPEN));
        Utl_UStrCat(UstrTmp, Res_GetStringByID(eRES_STR_CMR_KACHAMODE));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.KacaMode.Id, \
                          AK_FALSE, UstrTmp);
        ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.KacaMode.Id, gs.camSoundSw);
    }
    else
    {
        Preview_ToolBarInitRecFileType(pPreview);
        
        ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, gs.camRecMode[gs.CamRecFileType]);
    }

    //add button Brightness
    ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.Brightness.Id, \
                      BTN_TYPE_EDIT, pPreview->ButtonIcon.Brightness.Name, \
                      pPreview->ButtonIcon.Brightness.stateIcon);

    //add button Contrast
    ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.Contrast.Id, \
                      BTN_TYPE_EDIT, pPreview->ButtonIcon.Contrast.Name, \
                      pPreview->ButtonIcon.Contrast.stateIcon);

    //add button Saturation
    ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.Saturation.Id, \
                      BTN_TYPE_EDIT, pPreview->ButtonIcon.Saturation.Name, \
                      pPreview->ButtonIcon.Saturation.stateIcon);

    //add button NightMode
    ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.NightMode.Id, \
                      BTN_TYPE_SWITCH, pPreview->ButtonIcon.NightMode.Name, \
                      pPreview->ButtonIcon.NightMode.stateIcon);

    Utl_UStrCpy(UstrTmp, Res_GetStringByID(eRES_STR_PUB_CLOSE));
    Utl_UStrCat(UstrTmp, Res_GetStringByID(eRES_STR_CMR_NIGHTMODE));
    ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.NightMode.Id, \
                      CAMERA_NIGHT_MODE, UstrTmp);
    Utl_UStrCpy(UstrTmp, Res_GetStringByID(eRES_STR_PUB_OPEN));
    Utl_UStrCat(UstrTmp, Res_GetStringByID(eRES_STR_CMR_NIGHTMODE));
    ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.NightMode.Id, \
                      CAMERA_DAY_MODE, UstrTmp);
    ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.NightMode.Id, gs.camFlashMode);

    if (gs.CamMode == CAM_DC)
    {
        //add button DelayCap
        ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.DelayCap.Id, \
                          BTN_TYPE_SUBMENU, pPreview->ButtonIcon.DelayCap.Name, \
                          pPreview->ButtonIcon.DelayCap.stateIcon);
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.DelayCap.Id, \
                          0, Res_GetStringByID(eRES_STR_PUB_CLOSE));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.DelayCap.Id, \
                          5, Res_GetStringByID(eRES_STR_FIVE_SECONDS));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.DelayCap.Id, \
                          10, Res_GetStringByID(eRES_STR_TEN_SECONDS));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.DelayCap.Id, \
                          20, Res_GetStringByID(eRES_STR_TWENTY_SECONDS));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.DelayCap.Id, \
                          30, Res_GetStringByID(eRES_STR_THIRTY_SECONDS));
        ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.DelayCap.Id, gs.ShotDelayCount);

        //add button flashlight
        ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.FlashLight.Id, \
                          BTN_TYPE_SWITCH, pPreview->ButtonIcon.FlashLight.Name, \
                          pPreview->ButtonIcon.FlashLight.stateIcon);
        Utl_UStrCpy(UstrTmp, Res_GetStringByID(eRES_STR_PUB_CLOSE));
        Utl_UStrCat(UstrTmp, Res_GetStringByID(eRES_STR_CMR_FLASHLIGHT_SETUP));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.FlashLight.Id, \
                          AK_TRUE, UstrTmp);
        Utl_UStrCpy(UstrTmp, Res_GetStringByID(eRES_STR_PUB_OPEN));
        Utl_UStrCat(UstrTmp, Res_GetStringByID(eRES_STR_CMR_FLASHLIGHT_SETUP));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.FlashLight.Id, \
                          AK_FALSE, UstrTmp);
        ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.FlashLight.Id, gs.CamFlashlight);

        //add button CapQuality
        ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoQuality.Id, \
                          BTN_TYPE_SUBMENU, pPreview->ButtonIcon.PhotoQuality.Name, \
                          pPreview->ButtonIcon.PhotoQuality.stateIcon);
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoQuality.Id, \
                          CAM_QLTY_HIGH, Res_GetStringByID(eRES_STR_COM_HIGH));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoQuality.Id, \
                          CAM_QLTY_MIDDLE, Res_GetStringByID(eRES_STR_COM_MIDDLE));
        ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoQuality.Id, \
                          CAM_QLTY_LOW, Res_GetStringByID(eRES_STR_COM_LOW));
        ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.PhotoQuality.Id, gs.camPhotoQty);
    }

    //add button SetPath
    ToolBar_AddButton(pPreview->pToolBar, pPreview->ButtonIcon.SavePath.Id, \
                      BTN_TYPE_SWITCH, pPreview->ButtonIcon.SavePath.Name, \
                      pPreview->ButtonIcon.SavePath.stateIcon);
    ToolBar_AddOptionToButton(pPreview->pToolBar, pPreview->ButtonIcon.SavePath.Id, \
                      0, pPreview->ButtonIcon.SavePath.Name);

    return AK_TRUE;
}


T_VOID Preview_ToolBarFree( T_PREVIEW *pPreview)
{
    ToolBar_Free(pPreview->pToolBar);
    
    Fwl_Free(pPreview->pToolBar);
    pPreview->pToolBar = AK_NULL;
    
    Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
}

/**************************#end of camera toolbar defining.****************************/




/***********************************************************************************
FUNC NAME: Preview_StartTimer
DESCRIPTION: start timer
INPUT: time--millisecond   must can be divided by 10
OUTPUT:
RETURN:
AUTHOR:zhengwenbo
CREATE DATE:2006-6-27
MODIFY LOG:
***********************************************************************************/
static T_BOOL Preview_StartTimer(T_TIMER* pTimer, T_U32 time, T_BOOL loop)
{
    if (ERROR_TIMER != *pTimer)
    {
        Fwl_StopTimer(*pTimer);
        *pTimer = ERROR_TIMER;
    }
    *pTimer = Fwl_SetTimerMilliSecond(time, loop);

     if (ERROR_TIMER == *pTimer)
    {
        Fwl_Print(C3, M_CAMERA,  "Preview timer create fail!\n");
        return AK_FALSE;
    }

     return AK_TRUE;
}




/***********************************************************************************
FUNC NAME: Preview_StopTimer
DESCRIPTION: stop timer
INPUT:
OUTPUT:
RETURN:
AUTHOR:zhengwenbo
CREATE DATE:2006-6-27
MODIFY LOG:
***********************************************************************************/
static T_VOID Preview_StopTimer(T_TIMER* pTimer)
{
    if (ERROR_TIMER != *pTimer)
    {
         Fwl_StopTimer(*pTimer);
        *pTimer = ERROR_TIMER;
    }
}

T_VOID Preview_Catch_a_YUV(T_PREVIEW *pPreview)
{
#ifdef OS_ANYKA
        T_U8 *pY = AK_NULL;
        T_U8 *pU = AK_NULL;
        T_U8 *pV = AK_NULL;
        T_CAMERA_BUFFER *YUV;
    
        if (!camstream_ready())
        {
            return;
        }
        YUV = camstream_get();
        if (AK_NULL == YUV)
        {
            return;
        }
        pY = YUV->dY;
        pU = YUV->dU;
        pV = YUV->dV;
        if (AK_NULL == pY)
        {
            return;
        }
        if (pPreview->bufPtr == pPreview->pY)
        {
            pPreview->pY = pY;
            pPreview->pU = pU;
            pPreview->pV = pV;
        }
        else
        {
            pPreview->pY2 = pY;
            pPreview->pU2 = pU;
            pPreview->pV2 = pV;
        }

        pPreview->bufPtr = pY;
#endif
}

T_VOID Preview_Catch_a_YUV_NoWait(T_PREVIEW *pPreview)
{
#ifdef OS_ANYKA
    Preview_Catch_a_YUV(pPreview);
#endif
}

static T_BOOL Preview_GetDispWin(T_PREVIEW *pPreview , T_RECT *pRect)
{
    T_U32 srcW = 0, srcH=0;
    T_BOOL ret = AK_FALSE;

    srcW = pPreview->prev_window_width;
    srcH = pPreview->prev_window_height;
    if (pPreview->firstSrcWidth != srcW)
    {
        pPreview->firstSrcWidth = srcW;
        Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);    
        ret = AK_TRUE;
    }


   if (!Fwl_CameraGetClipWin(pRect, srcW, srcH, 
            MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT, eCAMCLIP_AUTO))
    {
       Fwl_Print(C3, M_CAMERA,  "Fwl_CameraGetClipWin Failure.\n"); 
        return AK_FALSE;
    }

    if (ret)
    {
        Fwl_Print(C3, M_CAMERA,  "Disp[%dx%d]:%dx%d=>(%d,%d)%dx%d\n",
        MAIN_LCD_WIDTH,MAIN_LCD_HEIGHT,
        srcW, srcH, pRect->left, pRect->top, pRect->width, pRect->height); 
    }

	return AK_TRUE;
}

static T_BOOL  Preview_Paint_Yuv(T_PREVIEW *pPreview, T_RECT *pRect)
{
    T_RECT foucsRect = {0};
    T_S32 lRet  = AK_EFAILED;
    T_BOOL ret = AK_FALSE;
    T_U8 *pY = AK_NULL;
    T_U8 *pU = AK_NULL;
    T_U8 *pV = AK_NULL;
    static T_U32 oldaddr = 0;

    if (pPreview->bufPtr == pPreview->pY2)
    {
        pY = pPreview->pY2;
        pU = pPreview->pU2;
        pV = pPreview->pV2;
    }
    else
    {
        pY = pPreview->pY;
        pU = pPreview->pU;
        pV = pPreview->pV;
    }

	if (AK_NULL == pY)
	{		
        return AK_FALSE;
    }
    
    //防止RGB屏闪屏
    if (oldaddr != (T_U32)pY)
    {
        oldaddr = (T_U32)pY;
    }
    else
    {
        return AK_FALSE;
    }
    
#if 0
    foucsRect.left = 0;
	foucsRect.top = 0;
	foucsRect.width = 1280;
	foucsRect.height = 720;
#else
	lRet = VideoZoom_GetFoucsWin(pPreview->hZoom,&foucsRect);

    if (AK_IS_FAILURE(lRet))
    {
        Fwl_Print(C3, M_CAMERA,  "VideoZoom_GetFoucsWin Failure.\n"); 
        return AK_FALSE;
    }
#endif   

	ret = Fwl_RefreshYUV1(pY, pU, pV, pPreview->prev_window_width
		, pPreview->prev_window_height, pRect->left,pRect->top,pRect->width,pRect->height);
    if (!ret)
    {
       Fwl_Print(C3, M_CAMERA,  "lcd_refresh_YUV1 Failure.src width=%d,height=%d; \
       	des left=%d,top=%d , width=%d,height=%d\n" ,pPreview->prev_window_width , pPreview->prev_window_height 
       	, foucsRect.left, foucsRect.top, foucsRect.width,foucsRect.height); 
    }
		
/*		
//    ret = Fwl_YuvZoom(pY, pU, pV, (T_LEN)pPreview->prev_window_width,
//        &foucsRect, Fwl_GetDispMemory565(), 
//        MAIN_LCD_WIDTH, RGB565, pRect);
        
    
    if (!ret)
    {
       Fwl_Print(C3, M_CAMERA,  "Preview_Invalidate_YUV Failure.\n"); 
    }
*/
    return AK_TRUE;
}

T_VOID Preview_Invalidate_YUV(T_PREVIEW *pPreview)
{
    T_U8 focusTips[16] = {0};
    T_U16 len = 0;
	static T_U32 tick = 0;
    
	Fwl_Osd_ClearDispBuf();
    Preview_GetDispWin(pPreview, &PreView2DWin);    

    if (AK_NULL != pPreview->pToolBar)
    {
        //ToolBar_SetBackYUV(pPreview->pToolBar, pY, pU, pV, CAM_PREV_WIN_W, CAM_PREV_WIN_H);
        ToolBar_Show(pPreview->pToolBar);
    }
    else
    {
        Preview_showIcon(pPreview, &PreView2DWin);
    }

    if (gb.nZoomInMultiple > 0)
    {
        Fwl_CameraGetFocusTips((T_U32)gb.nZoomInMultiple, focusTips);
        len = (T_U16)strlen(focusTips);
        if (len > 0)
        {
            Fwl_DispString(HRGB_LAYER, PreView2DWin.left, PreView2DWin.top, focusTips, (T_U16)len, COLOR_RED, 0);
        }
	}
    if (!Preview_Paint_Yuv(pPreview, &PreView2DWin))
    {
        return;
    }
    
	if (gbCameraPreviewOsdFreshFlag || (Fwl_GetTickCount() - tick > 500))
	{
		Fwl_Osd_RefreshDisplay();
		gbCameraPreviewOsdFreshFlag = AK_FALSE;		
	}
	
	tick = Fwl_GetTickCount();

	Fwl_RefreshDisplayByColor(COLOR_BLACK);
	
	Fwl_Refresh_Output();
}



/***********************************************************************************
FUNC NAME: Preview_SetFocusLevel
DESCRIPTION: set zoom value
INPUT:  zoom value
OUTPUT:
RETURN:
AUTHOR:lizhuobin
CREATE DATE:2006-6-27
MODIFY LOG:
***********************************************************************************/
T_VOID Preview_SetFocusLevel(T_PREVIEW *pPreview, T_U16 level)
{
    if (AK_NULL == pPreview)
    {
        Fwl_Print(C3, M_CAMERA,  "Preview_SetFocusLevel(): pPreview is NULL\n");
        return;
    }    
    
   	pPreview->camFocusLevel = level;

	gb.nZoomInMultiple = (T_S8)pPreview->camFocusLevel;
    
}


T_VOID Preview_SwitchCamMode(T_PREVIEW *pPreview, T_U8 cam_mode)
{
    T_U32 width;
    T_U32 height;
    
    Preview_GetCurWinSize(pPreview, cam_mode, &width, &height);

    switch (cam_mode)
    {
    case CAM_DC:
        pPreview->firstSrcWidth = 0;
        Preview_ResetWindow(pPreview, width, height);
        pPreview->camMode = cam_mode;
        Preview_SetFocusLevel(pPreview, CAMERA_FOCUS_0);  // mode change need reset focuslevel
        Preview_SetFocusWindow(pPreview);
        break;
    case CAM_DV:
        pPreview->firstSrcWidth = 0;
        /**cancel current delay capture's timer if change mode from capture to video recorder*/
        pPreview->shot_delay_count = 0;
        if (ERROR_TIMER != pPreview->delay_shot_tm_id)
        {
            Fwl_StopTimer(pPreview->delay_shot_tm_id);
            pPreview->delay_shot_tm_id = ERROR_TIMER;
        }
        Preview_ResetWindow(pPreview, width, height);
        pPreview->camMode = cam_mode;
        Preview_SetFocusLevel(pPreview, CAMERA_FOCUS_0);  // mode change need reset focuslevel
        Preview_SetFocusWindow(pPreview);
        break;
        
    default:
        Fwl_Print(C3, M_CAMERA,  "Camera mode set error");
        break;
    }
    
}

/***********************************************************************************
FUNC NAME: Preview_Init
DESCRIPTION: preview control init
INPUT:
OUTPUT:
RETURN:
AUTHOR:zhengwenbo
CREATE DATE:2006-7-04
MODIFY LOG:
***********************************************************************************/
T_BOOL Preview_Init(T_PREVIEW *pPreview)
{
    AK_ASSERT_PTR(pPreview, "Preview_Init(): pPreview is NULL\n",AK_FALSE);

    pPreview->prev_timer_id = ERROR_TIMER;
    pPreview->delay_shot_tm_id = ERROR_TIMER;
    pPreview->zoomlevel_shown_timer_id = ERROR_TIMER;
    pPreview->pY = AK_NULL;
    pPreview->pU = AK_NULL;
    pPreview->pV = AK_NULL;
    pPreview->pY2 = AK_NULL;
    pPreview->pU2 = AK_NULL;
    pPreview->pV2 = AK_NULL;
    pPreview->bufPtr = AK_NULL;
    pPreview->hZoom  = 0;

    pPreview->pToolBar = AK_NULL;
    pPreview->OldFocuBtnId = ERROR_BUTTON_ID;

    pPreview->bFlashLight = gs.CamFlashlight;
    pPreview->refreshFlag = CAMERA_PREVIEW_REFRESH_NONE;
    pPreview->shot_delay_count = 0;

    pPreview->brightness = gs.CamBrightness;

    pPreview->camFocusLevel         = CAMERA_FOCUS_0;
    pPreview->prev_window_width     = CAM_PRE_WINDOW_PIXEL_WIDTH;
    pPreview->prev_window_height    = CAM_PRE_WINDOW_PIXEL_HEIGHT;
    pPreview->camMode = gs.CamMode;
    pPreview->firstSrcWidth = 0;
    
    pPreview->shot_delay_count = gs.ShotDelayCount;
    pPreview->bCameraInit = AK_FALSE;

    Preview_IconInit(pPreview);
    Preview_SetIconRect(pPreview);
    Preview_GetIconRes(pPreview);
    /*Start preview timer*/
    Preview_StartTimer(&pPreview->prev_timer_id, PREVIEW_INTERVAL, AK_TRUE);

    return AK_TRUE;
}







/***********************************************************************************
FUNC NAME: Preview_CamStrmOpen
DESCRIPTION: open  camera
INPUT:
OUTPUT:
RETURN:
AUTHOR:
CREATE DATE:2007-12-18
MODIFY LOG:
***********************************************************************************/
T_BOOL Preview_CamStrmOpen(T_PREVIEW *pPreview)
{
    AK_ASSERT_PTR(pPreview, "Preview_CameraStart(): pPreview is NULL\n",AK_FALSE);

    Preview_GetCurWinSize(pPreview, gs.CamMode,
                &pPreview->prev_window_width, &pPreview->prev_window_height);

    pPreview->hZoom = VideoZoom_Open((T_U8)pPreview->camFocusLevel, 
                                     pPreview->prev_window_width, 
                                     pPreview->prev_window_height);
    if (0 == pPreview->hZoom) 
    {
        Fwl_Print(C3, M_CAMERA,  "VideoZoom_Open Er\n");
        return AK_FALSE;
    }

    return AK_TRUE;
}


/***********************************************************************************
FUNC NAME: Preview_CamStrmOpen
DESCRIPTION: open  camera
INPUT:
OUTPUT:
RETURN:
AUTHOR:
CREATE DATE:2007-12-18
MODIFY LOG:
***********************************************************************************/
T_BOOL Preview_CamStrmClose(T_PREVIEW *pPreview)
{
    AK_ASSERT_PTR(pPreview, "Preview_CameraStart(): pPreview is NULL\n",AK_FALSE);

    if ((T_HANDLE)0 != pPreview->hZoom)
    {
        VideoZoom_Close(pPreview->hZoom);
        pPreview->hZoom = (T_HANDLE)0;
    }
    

    return AK_TRUE;
}


/***********************************************************************************
FUNC NAME: Preview_CameraStart
DESCRIPTION: start camera
INPUT:
OUTPUT:
RETURN:
AUTHOR:zhengwenbo
CREATE DATE:2007-12-18
MODIFY LOG:
***********************************************************************************/
T_BOOL Preview_CameraStart(T_PREVIEW *pPreview)
{
    AK_ASSERT_PTR(pPreview, "Preview_CameraStart(): pPreview is NULL\n",AK_FALSE);

    VideoZoom_DetectSetInterval(pPreview->hZoom,PREVIEW_DETECT_INTERVAL);

    Preview_Catch_a_YUV(pPreview);
    Preview_Catch_a_YUV(pPreview);
    Preview_Catch_a_YUV(pPreview);
    return AK_TRUE;
}

T_VOID bomei_ChangeYData( T_PREVIEW* pPreview,
								T_U16 y_row,T_U16 y_col,T_U8 y_data)
{
	memset(pPreview->pY + y_row * 640 + y_col,y_data,1);
}

T_VOID bomei_ChangeUData( T_PREVIEW* pPreview,
								T_U16 u_row,T_U16 u_col,T_U8 u_data)
{
	AK_DEBUG_OUTPUT("width = %d,height = %d \r\n", pPreview->shotParm.width,pPreview->shotParm.height);
	memset(pPreview->pU + u_row * 640 + u_col + pPreview->shotParm.width * pPreview->shotParm.height,u_data,1);
}


T_VOID bomei_ChangeVData( T_PREVIEW* pPreview,
								T_U16 v_row,T_U16 v_col,T_U8 v_data)
{
	AK_DEBUG_OUTPUT("width = %d,height = %d \r\n", pPreview->shotParm.width,pPreview->shotParm.height);
	memset(pPreview->pV + v_row * 640 * v_col + (pPreview->shotParm.width * pPreview->shotParm.height)*3 /2 ,v_data,1);
}

/***********************************************************************************
FUNC NAME: Preview_handle
DESCRIPTION: change photo size
INPUT: bSize  0-size up     1-size down
OUTPUT:
RETURN:
AUTHOR:zhengwenbo
CREATE DATE:2006-6-27
MODIFY LOG:
***********************************************************************************/
T_eBACK_STATE Preview_handle(T_PREVIEW* pPreview, T_EVT_CODE Event, T_EVT_PARAM *pEventParm)
{
    T_eBACK_STATE retState = eStay;
    T_MMI_KEYPAD    phyKey;
	static T_U16 i = 0x00;
	T_U16 row = 0x00;
	T_U16 col = 0x00;

    AK_ASSERT_PTR(pPreview, "Preview_handle():pCapture\n", eStay);
    AK_ASSERT_PTR(pEventParm, "Preview_handle():pEventParm\n", eStay);
	
    switch (Event)
    {
    case M_EVT_USER_KEY:
        AK_ASSERT_PTR(pEventParm, "Menu_Handler(): pEventParm", eStay);
        phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_BOOL)pEventParm->c.Param2;

        //do corresponding process,according to the user key 
        retState = Preview_UserKey_Handle(pPreview, phyKey);
        break;
        
    case VME_EVT_TIMER:
        if ((pEventParm->w.Param1 == (T_U32)pPreview->prev_timer_id)\
            && (pEventParm->w.Param2 == PREVIEW_INTERVAL))//preview timer
        {
            Fwl_StopTimer(pPreview->prev_timer_id);
            pPreview->prev_timer_id = ERROR_TIMER;
            Preview_Catch_a_YUV_NoWait(pPreview);

			for(row = 0x00; row < 20;row++)
			{
				for(col = 0x00;col < 20; col++)
				{
					bomei_ChangeYData(pPreview,i + row,i + col,255);
				}
			}	

			if (i >= 255){
				i = 0x00;
			}
			i++;


			
            Preview_SetRefresh(pPreview, CAMERA_PREVIEW_REFRESH_YUV);
            Preview_StartTimer(&pPreview->prev_timer_id, PREVIEW_INTERVAL, AK_TRUE);
            retState = eStay;
            if (gb.isDetectMode && (gs.CamMode == CAM_DV))
            {
                if(VideoZoom_DetectIsMoving(pPreview->hZoom, pPreview->bufPtr))
                {
                    //VideoZoom_DetectEnable(pPreview->hZoom,AK_FALSE);
                    retState = eNext;
                    Fwl_Print(C3, M_CAMERA,"\tStart Record\n");
                }
            }
        }
        
        else if ((pEventParm->w.Param1 == (T_U32)pPreview->delay_shot_tm_id)\
                 && (pEventParm->w.Param2 == SHOT_INTERVAL_TIME))//capture delay timer
        {
        	gbCameraPreviewOsdFreshFlag = AK_TRUE;
			
             /*decrease delay timer count*/
            Preview_DecCapDelayCount(pPreview);

            if (!pPreview->shot_delay_count)
            {
                /*capture at once*/
                retState = eNext;

                /**stop delay timer*/
                Preview_StopTimer(&pPreview->delay_shot_tm_id);
            }
        }
        else if ((pEventParm->w.Param1 == (T_U32)pPreview->zoomlevel_shown_timer_id)\
            && ( ZOOM_LVL_SET_INTERVAL_TIME == pEventParm->w.Param2))//hxq modify 2011.9.6
        {
            Preview_StopTimer(&pPreview->zoomlevel_shown_timer_id);
        }
        break;
        
    // touch screen event mapping
    case  M_EVT_TOUCH_SCREEN:
        {
            T_POS x = (T_POS)pEventParm->s.Param2;
            T_POS y = (T_POS)pEventParm->s.Param3;
            T_MMI_KEYPAD phyKey;

            phyKey.keyID = kbNULL;
            phyKey.pressType = PRESS_SHORT;

            switch (pEventParm->s.Param1) 
            {
            case eTOUCHSCR_UP:

                /* if the point(x,y) hit in the control buttons rect,  
                          transform it to the corresponding key */
                phyKey = Preview_MapTSCR_To_Key(pPreview, x, y);

                //do corresponding process,according to the user key 
                retState = Preview_UserKey_Handle(pPreview, phyKey);
                break;
            case eTOUCHSCR_DOWN:
                 break;
            case eTOUCHSCR_MOVE:
                 break;
            default:
                 break;
            }
        }            
    default:
        break;
    }

    return retState;
}

//handle user key event
static T_eBACK_STATE Preview_UserKey_Handle(T_PREVIEW* pPreview, T_MMI_KEYPAD phyKey)
{
    T_eFUNC_KEY funcKey;
    T_eBACK_STATE retState = eStay;

    funcKey = MappingCameraKey(phyKey);
    if (funcKey != fkNULL)// if preview delay is over
    {
        if (ERROR_TIMER != pPreview->delay_shot_tm_id) // if delay capture is started
        {
            if (fkCAMERA_EXIT != funcKey)
            {
                return retState;
            }
        }

        
        VideoZoom_DetectEnable(pPreview->hZoom,AK_FALSE);
        
        switch (funcKey)
        {
        case fkCAMERA_CAPTURE:
            switch (gs.CamMode)
            {
            case CAM_DC: // take photo
                /*delay capture*/
                if (gs.ShotDelayCount != 0)
                {
                    pPreview->shot_delay_count = gs.ShotDelayCount;
                    /*start delay timer*/
                    Preview_StartTimer(&pPreview->delay_shot_tm_id, SHOT_INTERVAL_TIME, AK_TRUE);
                }
                else
                {
                    /*capture at once*/
                    retState = eNext;
                }
                break;
                
            case CAM_DV: // video record
                retState = eNext;
                break;
                
            default:
                Fwl_Print(C3, M_CAMERA,  "():gs.CamMode is invalid\n");
                break;
            }    
            break;
            
        case fkCAMERA_MENU:
            retState = eMenu;
            break;
       
        case fkCAMERA_ZOOM_IN:
/*        
            if ((SDRAM_MODE >= 16) || Is_04CHIP())
            {
                gb.nZoomInMultiple++;
                if (gb.nZoomInMultiple > 9)//max=9
                {
                    gb.nZoomInMultiple = 9;
                }
    			
                Preview_SetFocusLevel(pPreview, gb.nZoomInMultiple);
                Preview_SetFocusWindow(pPreview);
            }
*/            
            break;
            
        case fkCAMERA_ZOOM_OUT:
/*        
            if ((SDRAM_MODE >= 16) || Is_04CHIP())
            {
                gb.nZoomInMultiple--;
                if (gb.nZoomInMultiple < 0)
                {
                    gb.nZoomInMultiple = 0;
                }
    			
                Preview_SetFocusLevel(pPreview, gb.nZoomInMultiple);
                Preview_SetFocusWindow(pPreview);
            }
*/            
            break;
	                
        case fkCAMERA_BRIGHT_UP:
            if(pPreview->brightness > CAMERA_BRIGHTNESS_0 )
            {
                pPreview->brightness--;
                gs.CamBrightness = pPreview->brightness;
                Fwl_CameraChangeBrightness(pPreview->brightness );
                Preview_SetRefresh(pPreview, CAMERA_PREVIEW_REFRESH_YUV);
            }
            break;
            
        case fkCAMERA_BRIGHT_DOWN:
            if( pPreview->brightness < CAMERA_BRIGHTNESS_NUM - 1 )
            {
                pPreview->brightness++;
                gs.CamBrightness = pPreview->brightness;
                Fwl_CameraChangeBrightness( pPreview->brightness );
                Preview_SetRefresh(pPreview, CAMERA_PREVIEW_REFRESH_YUV);
            }
            break;
            
        case fkCAMERA_SWICH_2_DC:
        case fkCAMERA_SWICH_2_DV:
            if (gs.CamMode == CAM_DV)
            {
                gs.CamMode = CAM_DC;
            }
            else if (gs.CamMode == CAM_DC)
            {
                gs.CamMode = CAM_DV;
            }
			
            Preview_SwitchCamMode(pPreview, gs.CamMode);
            break;
            
        case fkCAMERA_EXIT:
            retState = eReturn;
            break;
            
        default:
            retState = eStay;
            break;
            
        }
    }
    
    else if(phyKey.pressType == PRESS_LONG)
    {
        if (phyKey.keyID == kbOK)
        {
            Fwl_KeyStop();
        }
        else if (phyKey.keyID == kbCLEAR)
        {
            retState = eReturn;
        }
    }

    if (gb.isDetectMode && (gs.CamMode == CAM_DV))
    {
        VideoZoom_DetectEnable(pPreview->hZoom,AK_TRUE);
    }
    
    return retState;
    
}

static T_MMI_KEYPAD Preview_MapTSCR_To_Key(T_PREVIEW* pPreview, T_POS x, T_POS y)
{
    T_MMI_KEYPAD    phyKey;
    T_RECT            realCamDVModeRect;

    phyKey.keyID = kbNULL;
    phyKey.pressType = PRESS_SHORT;

    //hit DC button
    if (PointInRect(&pPreview->CamDCModeRect, x, y))
    {
       phyKey.keyID = kbUP;
       return phyKey;
    }

    //hit DV button
    realCamDVModeRect.height = pPreview->CamDVModeRect.height;
    realCamDVModeRect.top= pPreview->CamDVModeRect.top+CAMMODE_PIC_HEIGHT/2;
    realCamDVModeRect.width= pPreview->CamDVModeRect.width;
    realCamDVModeRect.left= pPreview->CamDVModeRect.left;
    if (PointInRect(&realCamDVModeRect, x, y))
    {
       phyKey.keyID = kbDOWN;
       return phyKey;
    }

    //hit Menu button
    if (PointInRect(&pPreview->CamMenuRect, x, y))
    {
       phyKey.keyID = kbMENU;
       return phyKey;
    }

    //hit OK button
    if (PointInRect(&pPreview->CamOkRect, x, y))
    {
       phyKey.keyID = kbOK;
       return phyKey;
    }

    //hit Return button
    if (PointInRect(&pPreview->CamRetRect, x, y))
    {
       phyKey.keyID = kbCLEAR;
       return phyKey;
    }

    return phyKey;

}



/***********************************************************************************
FUNC NAME: Preview_UpdateVar
DESCRIPTION: update variable
INPUT:
OUTPUT:
RETURN:
AUTHOR:zhengwenbo
CREATE DATE:2007-12-24
MODIFY LOG:
***********************************************************************************/

T_VOID Preview_UpdateVar(T_PREVIEW*pPreview)
{
    pPreview->camMode = gs.CamMode;
    pPreview->brightness = gs.CamBrightness;
    pPreview->bFlashLight = gs.CamFlashlight;
}


/***********************************************************************************
FUNC NAME: Preview_Free
DESCRIPTION: free memory
INPUT:
OUTPUT:
RETURN:
AUTHOR:zhengwenbo
CREATE DATE:2006-6-29
MODIFY LOG:
***********************************************************************************/
T_VOID Preview_Free(T_PREVIEW*pPreview)
{
    if (AK_NULL == pPreview)
    {
        AK_ASSERT_PTR_VOID(pPreview, "Preview_Exit():pPreview\n");
        return;
    }
    
    Preview_CamStrmClose(pPreview);
    /*stop timer*/
    Preview_StopTimer(&pPreview->delay_shot_tm_id);
    Preview_StopTimer(&pPreview->prev_timer_id);

    /*shut down camera*/
    Fwl_CameraFree();
}

/***********************************************************************************
FUNC NAME: Preview_DecCapDelayCount
DESCRIPTION: sub capture delay count
INPUT:
OUTPUT:
RETURN:
AUTHOR:zhengwenbo
CREATE DATE:2006-7-3
MODIFY LOG:
***********************************************************************************/
static T_VOID Preview_DecCapDelayCount(T_PREVIEW *pPreview)
{
    AK_ASSERT_PTR_VOID(pPreview, "Preview_DecCapDelayCount():pPreview is NULL\n");

    if (pPreview->shot_delay_count > 0)
    {
        pPreview->shot_delay_count--;
    }
}

/**
 * @brief The function return refresh flag
 *
 * @author \b zhengwenbo
 * @date 2006-07-24
 * @param[in] pPreview      the point of video record control
 * @return returns the refresh flag
 */
T_U16 Preview_GetRefresh(T_PREVIEW *pPreview)
{
    AK_ASSERT_PTR(pPreview, "Preview_GetRefresh():input parameter error\n", CAMERA_PREVIEW_REFRESH_NONE);

    return pPreview->refreshFlag;
}

/**
 * @brief The function set refresh flag
 *
 * @author \b zhengwenbo
 * @date 2006-07-24
 * @param[in] pPreview      the point of video record control
 * @param[in] refresh       the flag of refresh
 */
T_VOID Preview_SetRefresh(T_PREVIEW *pPreview, T_U16 refresh)
{
    AK_ASSERT_PTR_VOID(pPreview, "Preview_GetRefresh():input parameter error\n");

    pPreview->refreshFlag |= refresh;
}


/*
 * @brief  the function set camera's feature of saturation, size, etc..
 *
 * @author Zhengwenbo
 * @date 2006-08-01
 * @param[in] pPreview   the point of preview control
 * @return  result 1--success 0--fail
 */
T_BOOL Preview_SetFeature(T_PREVIEW *pPreview)
{
#ifdef  OS_ANYKA
    AK_ASSERT_PTR(pPreview, "Preview_SetFeature():input parameter is error\n", AK_FALSE);

    Fwl_CameraSetEffect((T_U8)gs.camColorEffect);   

    Fwl_CameraChangeBrightness( pPreview->brightness );

    Fwl_CameraChangeSaturation(gs.CamSaturation);

    Fwl_CameraSetNightMode(gs.camFlashMode);   

    Fwl_CameraChangeContrast(gs.CamContrast);//hxq move this code to here.
#endif

    return AK_TRUE;
}

/**
 * @brief The function display infomation message box
 *
 * @author \b zhengwenbo
 * @date 2006-07-11
 * @param[in] pPreview      the point of video record control
 * @param[in] title     the tile of message box
 * @param[in] content       the content of message box
 * @param[in] retLevel  the reture level of message box
 */
T_VOID Preview_DispInfoMsgbox(T_PREVIEW *pPreview, T_pCWSTR title, T_pCWSTR content, T_U16 retLevel)
{
    T_RECT  msgRect;

    MsgBox_InitStr(&pPreview->msgbox, retLevel, title, content, MSGBOX_INFORMATION);
    MsgBox_Show(&pPreview->msgbox);
    MsgBox_GetRect(&pPreview->msgbox, &msgRect);
    Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);
}

T_VOID Preview_SetFocusWindow(T_PREVIEW* pPreview)
{
    VideoZoom_SetFocusLvl(pPreview->hZoom,(T_U8)pPreview->camFocusLevel, AK_FALSE);
    Preview_SetRefresh(pPreview, CAMERA_PREVIEW_REFRESH_YUV);
}


/*
 * @brief   get current selected capture/record size mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pPreview: preview handle
 */
static T_CAMERA_MODE Preview_GetCurFrameMode(T_PREVIEW* pPreview, T_U8 camMode)
{
    T_CAMERA_MODE mode;
    
    if (camMode == CAM_DC)
    {
        if (gs.DCShotMode == DC_NORMAL_SHOT)
        {
            mode = gs.camPhotoMode;
        }
        else
        {
            mode = gs.camMultiShotMode;
        }
    }
    else
    {
        mode = gs.camRecMode[gs.CamRecFileType];
    }

    return mode;
}


/*
 * @brief   calc current window size by selected capture/record size mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pPreview: preview handle
 */
T_VOID Preview_GetCurWinSize(T_PREVIEW* pPreview, T_U8 camMode, T_U32 *pWidth, T_U32 *pHeight)
{
    T_CAMERA_MODE mode;
    
    mode = Preview_GetCurFrameMode(pPreview, camMode);
    
    Preview_GetWinSize(pPreview, camMode, mode, pWidth, pHeight);
}


static T_VOID Preview_GetWinSize(T_PREVIEW* pPreview, T_U8 camMode, T_CAMERA_MODE mode, T_U32 *pWidth, T_U32 *pHeight)
{
    T_U32  frameWidth = 0;
    T_U32  frameHeight = 0;
    T_U32  refWidth = 0;
    T_U32  refHeight = 0;

	//通过帧模式获取帧尺寸
    Fwl_GetRecFrameSize(mode, &frameWidth, &frameHeight);

    refWidth  = frameWidth ;
    refHeight = frameHeight;
	
    //check size for camera driver limit
    Fwl_CameraCheckSize(&refWidth, &refHeight);

    (*pWidth)  = refWidth;
    (*pHeight) = refHeight;

}


T_VOID Preview_ResetWindow(T_PREVIEW* pPreview, T_U32 width, T_U32 height)
{
    T_S32 ret;

    if ((pPreview->prev_window_width == width)\
        && (pPreview->prev_window_height == height))
    {
        return;
    }
    
    
    Fwl_Print(C3, M_CAMERA,  "CamPrev: W: %d, H: %d\n",width, height);

    ret = VideoZoom_RestartVideoSrc(pPreview->hZoom, width, height);
    if (AK_IS_FAILURE(ret))
    {
        Fwl_Print(C3, M_CAMERA,  "Start src Err @%d\n",__LINE__);
    }
    else
    {
        pPreview->prev_window_width = width;
        pPreview->prev_window_height = height;
    }

    Preview_SetFocusLevel(pPreview, CAMERA_FOCUS_0);  // mode change need reset focuslevel
    Preview_SetFocusWindow(pPreview);
}


#endif


