/*
Capture Control
*/
#include "eng_string.h"

#ifdef CAMERA_SUPPORT
#include "drv_api.h"
#include "eng_string_uc.h"
#include "ctl_msgbox.h"
#include "fwl_pfcamera.h"
#include "ctl_preview.h"
#include "ctl_capture.h"
#include "Fwl_Image.h"
#include "fwl_keyhandler.h"
#include "eng_keymapping.h"
#include "fwl_osfs.h"
#include "fwl_osmalloc.h"
#include "lib_image_api.h"
#include "Fwl_Initialize.h"
#include "Eng_DataConvert.h"
#include "Eng_font.h"
#include "Eng_Math.h"
#include "fwl_pfcamera.h"
#include "fwl_oscom.h"
#include "lib_sdcodec.h"
#include "Fwl_pfDisplay.h"
#include "Fwl_pfaudio.h"
#include "Lib_state_api.h"
#include "config.h"
#include "fwl_pfcamera.h"
#include "Gbl_Resource.h"
#include "Fwl_display.h"
#include "hal_sysdelay.h" //for mini_delay
#include "fwl_graphic.h" //f
#include "log_media_recorder.h"// for MRec_GetStampFileName
#include "Eng_DynamicFont.h"
#include "fwl_gui.h"
#include "Fwl_ImageLib.h"
#include "Fwl_waveout.h"
#include "hal_camera.h"

//static T_BOOL Capture_FlashLightSwitch(T_CAPTURE *pCapture);
static T_BOOL Capture_Shot(T_CAPTURE *pCapture);
static T_BOOL Capture_GetPhotoName(T_CAPTURE *pCapture, T_pWSTR pFileName);
static T_VOID Capture_FreeBuff(T_CAPTURE *pCapture);
//static T_VOID Capture_DispCaptureMsg(T_CAPTURE *pCapture, T_S16 tms);
static T_BOOL Capture_PlayWave(T_VOID);
static T_BOOL Capture_SavePhoto(T_CAPTURE *pCapture, T_U32 PhotoIndex);

extern T_BOOL Is_04CHIP(T_VOID);

#ifdef CAMERA_H_SHOT  //        shot  mode is H  direct
#define  CAMERA_IMAGE_SHOW_LEFT     0
#define  CAMERA_IMAGE_SHOW_TOP      0
#define  CAMERA_IMAGE_SHOW_WIDTH    MAIN_LCD_WIDTH
#define  CAMERA_IMAGE_SHOW_HEIGHT   MAIN_LCD_HEIGHT
#else
#define  CAMERA_IMAGE_SHOW_LEFT      0
#define  CAMERA_IMAGE_SHOW_TOP       70
#define  CAMERA_IMAGE_SHOW_WIDTH     MAIN_LCD_WIDTH
#define  CAMERA_IMAGE_SHOW_HEIGHT    MAIN_LCD_HEIGHT - 140
#endif

#define CAM_MULTI_MAX       4

const T_U16    camera_photo_quality[CAM_QLTY_NUM] = {200,150,100};

static T_BOOL Capture_EncodeJpg(T_CAPTURE* pCapture, T_U32 PhotoIndex);

#if 0
/**Camera flash sound--"KaCha"*/
const T_U8 flash_camera_wav[2871] = {
	82,73,70,70,46,11,0,0,87,65,86,69,102,109,116,32,50,0,0,0,2,0,1,0,17,43,0,0,13,22,0,0,0,1,4,0,32,0,244,1,7,0,0,1,0,0,0,2,0,255,
	0,0,0,0,192,0,64,0,240,0,0,0,204,1,48,255,136,1,24,255,102,97,99,116,4,0,0,0,52,21,0,0,100,97,116,97,220,10,0,0,4,16,0,8,0,241,255,255,0,244,
	241,34,15,11,44,3,255,15,15,66,244,46,224,11,5,222,17,223,52,211,112,4,13,45,157,240,6,16,30,210,71,0,43,45,0,163,195,240,208,96,50,29,13,219,2,255,0,176,96,49,
	0,76,240,223,214,240,254,16,80,18,17,255,250,17,245,220,254,12,97,16,51,243,75,193,161,10,77,35,225,14,17,240,61,17,145,240,241,11,33,19,32,58,1,161,63,237,12,2,81,19,
	254,15,237,81,207,28,1,95,32,210,225,6,31,109,237,0,245,240,16,16,126,16,254,244,221,64,192,49,213,0,1,57,34,208,254,238,28,98,0,95,240,254,1,12,32,14,62,96,2,0,
	160,0,228,43,0,14,98,225,27,241,29,18,237,68,1,32,15,144,237,244,31,4,255,98,241,0,191,237,17,48,65,161,27,68,208,46,241,44,238,27,230,243,18,224,56,16,241,13,16,14,
	38,228,224,240,45,80,225,12,212,13,4,11,80,241,63,212,250,16,253,64,242,16,240,2,1,34,160,61,128,14,3,16,49,19,252,15,31,178,218,1,225,96,243,240,46,76,4,101,0,147,
	1,162,2,44,206,208,230,0,79,4,4,15,208,144,13,1,244,2,21,20,12,255,219,0,242,48,21,31,32,254,14,160,11,33,0,48,228,77,2,14,8,224,2,48,4,176,2,31,76,14,
	224,18,34,43,13,224,96,112,14,254,43,18,1,28,213,16,17,207,224,23,3,13,144,240,16,3,3,93,4,252,31,207,29,4,16,112,0,0,240,250,14,48,7,250,44,64,3,240,210,206,
	62,4,240,209,1,112,32,219,16,242,28,48,225,225,2,246,15,31,0,211,144,1,30,60,1,21,15,230,14,92,242,208,16,2,31,48,224,147,2,0,16,128,251,3,1,64,1,62,208,173,
	240,2,52,17,7,255,12,240,192,49,23,11,47,241,17,243,204,17,0,63,195,237,51,115,224,176,251,17,3,177,3,244,0,12,243,192,255,0,224,7,243,67,193,13,239,176,14,1,245,127,
	0,17,15,197,238,92,0,0,2,4,15,95,222,4,13,15,208,50,48,229,11,80,193,224,64,95,253,208,35,4,28,236,3,38,0,254,240,0,63,219,252,37,18,47,240,240,14,58,57,47,
	223,0,4,98,0,75,0,13,254,38,44,1,240,14,200,1,4,226,0,16,60,33,221,205,2,66,16,211,230,3,0,189,194,1,63,27,18,2,0,11,0,3,1,42,47,178,15,48,239,222,
	127,96,229,233,79,0,240,31,1,255,32,229,15,43,0,18,1,29,251,0,0,0,1,7,37,16,222,192,207,47,32,62,96,80,15,240,145,31,16,237,43,35,243,126,16,193,14,48,254,10,
	242,36,32,222,95,146,27,112,13,162,5,254,0,7,224,14,47,253,50,51,15,128,0,19,223,237,65,198,0,16,0,251,31,2,209,241,18,49,244,240,193,237,0,27,212,176,65,33,34,254,
	234,0,14,224,17,16,49,20,37,225,33,250,251,224,0,243,0,112,19,2,15,29,237,142,255,214,255,49,36,2,13,112,239,222,0,177,0,1,12,69,33,48,0,8,240,10,16,178,194,77,
	67,0,211,215,226,58,128,17,16,47,240,43,34,238,49,129,29,4,211,27,32,31,18,209,56,30,16,0,243,210,3,1,112,208,255,15,251,15,36,0,80,17,0,161,208,224,15,230,2,32,
	110,241,224,12,225,0,35,48,0,43,0,184,0,17,3,113,9,93,224,29,63,112,1,253,240,229,194,15,50,0,61,126,14,207,12,1,7,16,20,224,13,251,1,17,241,192,35,4,59,224,
	2,2,254,241,235,254,2,70,225,31,32,68,206,14,209,190,3,38,3,238,242,64,50,232,239,223,16,3,84,17,18,175,253,243,18,31,250,14,48,35,16,224,32,8,0,67,250,255,223,33,
	112,0,16,242,13,2,7,0,175,222,46,81,247,240,32,0,29,57,0,161,31,0,0,112,16,16,15,222,251,240,0,20,16,48,39,32,16,203,222,209,34,241,83,22,0,242,208,253,216,240,
	2,17,240,53,34,50,144,223,14,178,223,49,80,33,20,61,219,253,1,235,240,208,87,112,224,240,242,24,31,226,34,63,62,128,15,225,16,4,243,15,32,178,239,15,224,239,2,81,19,68,
	228,233,15,14,0,14,2,3,114,48,47,207,175,237,16,2,34,55,34,0,28,220,252,224,0,21,35,81,0,239,176,239,0,16,36,224,10,4,0,61,32,48,176,12,224,219,64,17,96,17,
	15,208,191,2,254,34,52,243,248,0,190,53,17,11,0,122,0,177,255,96,1,245,208,1,0,62,205,3,85,1,192,239,5,0,254,192,208,80,53,0,2,10,30,208,223,18,82,36,241,15,
	236,12,14,16,244,28,19,64,242,94,209,93,12,224,224,35,111,95,211,224,0,24,240,97,224,227,13,21,173,32,33,113,225,235,33,240,0,12,13,83,3,210,13,214,251,16,240,240,64,2,
	0,241,16,50,128,254,209,251,4,36,3,61,192,234,1,0,0,31,95,0,0,2,48,2,203,208,159,2,20,19,0,43,225,15,239,4,130,240,240,65,39,239,10,0,46,63,230,240,47,229,
	192,31,0,1,228,0,208,18,18,177,192,0,33,9,35,209,91,30,16,64,36,176,252,241,2,35,14,2,241,245,175,34,144,0,0,2,242,96,224,45,241,237,127,0,192,15,33,23,48,16,
	221,190,224,240,96,19,253,21,14,50,189,62,15,78,128,33,30,62,178,47,229,79,240,12,160,0,48,16,253,246,49,33,236,208,191,224,48,5,16,39,240,27,32,13,16,144,2,224,51,178,
	64,195,31,178,240,162,240,3,255,18,239,32,96,78,177,27,15,254,55,47,4,47,0,31,0,169,0,175,18,96,4,222,222,220,70,0,60,255,18,244,61,224,218,243,0,20,218,33,19,60,
	16,160,224,0,17,31,35,2,63,249,192,240,65,255,14,178,50,7,14,161,31,4,253,1,208,68,236,49,225,80,176,30,0,16,240,30,7,31,95,208,42,226,31,3,252,68,13,51,189,77,
	162,1,227,29,4,28,21,13,2,237,30,160,2,34,65,15,0,193,42,0,144,47,242,32,35,60,228,9,1,10,1,209,80,1,61,242,11,32,254,1,224,0,43,79,18,32,20,209,216,15,
	30,239,63,7,2,33,234,94,15,16,145,46,228,13,23,224,18,13,48,176,31,192,32,0,96,1,240,0,208,14,4,189,48,224,112,2,31,240,40,3,221,33,130,32,0,28,228,29,4,238,
	64,30,15,250,65,177,15,17,29,96,242,0,176,44,16,255,241,49,98,239,62,130,15,0,236,33,241,34,230,1,1,12,224,191,0,208,81,18,67,215,11,0,223,32,229,15,212,28,52,14,
	32,144,16,195,27,2,240,37,224,62,192,62,213,255,241,253,51,0,78,231,28,2,207,47,224,96,4,21,0,213,255,238,254,225,135,15,48,160,47,240,42,1,0,34,209,64,255,62,130,238,
	226,44,23,1,43,31,31,31,239,1,179,42,64,16,16,192,241,219,47,3,127,3,13,3,232,0,209,64,240,49,211,77,0,232,30,243,14,34,29,114,0,14,128,15,0,32,34,238,96,0,
	92,225,12,3,238,52,239,96,242,0,239,8,0,15,1,229,48,2,15,17,175,253,160,15,36,17,51,4,16,186,11,0,0,51,0,63,15,64,206,236,226,14,7,0,64,0,15,244,191,30,
	242,254,2,7,241,15,81,255,76,146,14,2,239,52,241,48,222,61,128,15,4,255,78,19,31,223,128,1,17,14,0,3,98,224,9,224,13,39,15,3,239,35,223,93,221,41,241,50,6,254,
	3,0,29,237,232,2,242,17,18,63,3,240,14,137,15,242,0,65,17,3,238,126,14,235,224,28,83,1,3,252,32,254,239,161,208,126,18,0,3,10,64,225,254,223,209,65,33,99,164,255,
	209,13,1,15,96,0,49,161,14,243,238,79,226,43,230,16,78,223,237,227,23,17,0,240,15,109,237,252,22,62,74,1,3,140,0,125,4,213,255,128,16,12,31,3,0,240,0,16,19,239,
	11,0,2,46,128,242,52,1,255,26,224,12,33,3,95,196,254,34,32,218,29,207,242,85,34,45,24,193,12,80,17,243,220,0,226,80,18,192,0,238,57,1,17,3,60,48,207,0,252,30,
	210,50,39,33,30,30,205,240,175,29,20,96,17,15,16,12,13,0,223,193,29,116,2,63,0,30,141,15,4,14,2,208,112,66,13,44,176,15,0,211,48,67,94,48,190,208,240,65,251,48,
	231,16,1,176,0,224,2,29,50,197,78,31,176,12,1,0,53,228,30,242,15,31,142,12,243,64,38,14,4,222,17,144,45,227,46,6,31,19,222,61,192,237,63,240,34,23,51,18,251,12,
	208,176,0,14,98,4,18,240,232,0,0,14,212,15,66,47,0,128,32,0,240,222,34,230,47,45,210,224,243,213,32,247,211,13,240,8,240,239,86,1,0,13,192,1,1,8,242,16,49,214,
	240,16,255,9,1,240,213,224,95,5,15,1,206,45,0,35,177,14,7,16,46,193,14,51,206,11,245,0,46,17,0,3,195,41,240,237,5,242,4,41,0,167,255,191,0,55,15,18,144,15,
	193,241,22,242,241,10,47,229,225,15,240,0,76,59,17,1,14,245,14,29,30,17,109,252,80,111,208,0,1,124,0,255,22,241,14,31,243,253,33,190,3,32,92,17,17,30,174,12,21,51,
	10,222,247,3,224,13,16,222,111,16,57,32,0,1,254,240,32,95,239,0,2,241,31,10,19,16,47,193,207,125,15,250,17,49,80,0,13,145,0,0,223,80,16,2,16,1,207,1,10,238,
	208,7,64,17,208,49,1,186,240,240,61,52,255,65,247,0,222,8,16,2,0,240,19,0,61,252,207,215,16,0,255,0,4,241,14,240,242,3,255,41,1,42,242,237,33,133,16,49,176,253,
	11,1,52,240,243,60,21,207,12,242,0,66,224,15,3,160,31,7,13,0,194,64,0,192,0,65,192,238,53,16,15,161,31,32,238,9,5,242,32,239,255,211,31,15,254,76,80,3,47,128,
	45,17,209,1,5,225,222,4,3,227,144,64,1,28,186,18,20,252,15,33,49,163,226,176,240,80,0,222,240,68,17,252,255,32,19,222,142,17,66,30,178,1,94,0,2,71,0,116,254,240,
	253,11,207,253,240,15,100,16,52,16,9,64,132,12,63,208,240,95,0,237,48,95,226,192,30,62,177,208,65,2,14,16,68,255,173,222,18,69,0,240,238,34,16,206,221,114,0,10,223,2,
	36,49,1,238,13,237,173,207,18,82,64,0,239,1,255,8,223,192,0,37,35,64,32,238,203,236,208,2,53,49,16,15,13,190,239,238,0,20,19,48,2,42,176,205,239,240,55,0,13,239,
	193,225,235,0,240,33,69,48,216,223,239,0,4,16,18,210,0,18,219,141,237,208,210,0,116,66,48,254,216,239,219,239,240,52,69,34,16,220,205,207,240,18,34,36,49,15,237,188,220,223,
	0,46,243,53,65,236,237,220,0,48,252,140,239,3,68,16,12,239,222,34,16,184,240,18,0,13,191,240,6,50,31,203,220,255,1,17,34,19,65,46,141,206,239,20,50,14,12,224,224,37,
	48,15,174,239,218,254,0,6,34,65,16,189,190,0,0,253,239,3,55,65,0,235,238,241,15,235,208,240,66,99,52,14,220,206,235,238,241,36,52,53,49,12,205,206,240,255,0,36,97,35,
	4,19,0,200,255,139,0,249,224,251,2,38,18,78,30,176,31,240,239,213,16,80,177,12,48,50,21,252,253,176,0,18,18,66,15,76,14,77,224,142,0,241,83,52,240,237,189,252,4,19,
	64,241,145,226,13,63,20,241,237,19,179,202,0,3,34,38,14,16,159,208,242,52,1,208,11,80,33,240,236,219,223,245,68,32,16,252,176,251,16,1,80,32,241,255,14,238,210,78,94,223,
	66,64,96,207,31,194,1,192,0,197,242,53,0,14,207,204,240,6,34,3,29,254,10,242,191,45,35,53,32,12,255,248,0,241,34,48,195,208,2,66,176,143,0,17,19,65,0,173,13,19,
	50,2,221,192,234,79,35,66,239,222,143,14,15,80,35,94,47,254,252,219,240,3,36,69,0,47,190,252,240,13,83,19,63,31,254,253,204,240,243,51,81,2,14,190,235,1,21,16,32,14,
	255,237,15,16,17,48,1,240,254,255,0,1,1,0,0,0,15,0,0,0,0x0FF,
};
#endif
static T_BOOL Capture_PlayWave(T_VOID)
{
	T_U32 audio_len;
	T_pCDATA SoundData = AK_NULL;	
	
	Fwl_Print(C3, M_CAMERA,  "Camera flash sound! --KaCha...\n");
        
	Fwl_AudioSetVolume(Fwl_GetAudioVolume());
	WaveOut_CloseFade();

	SoundData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_WAV_CAMERA_SHUTTER, &audio_len);
		
     if (Fwl_AudioOpenBuffer(SoundData, audio_len))
		return MPlayer_Play(0);

    return AK_FALSE;
}


/***********************************************************************************
FUNC NAME: Capture_Init
DESCRIPTION: init capture control
INPUT:
OUTPUT:
RETURN: return code
AUTHOR:zhengwenbo
CREATE DATE:2006-6-29
MODIFY LOG:
***********************************************************************************/
T_BOOL Capture_Init(T_CAPTURE *pCapture)
{
    AK_ASSERT_PTR(pCapture, "Capture_Init(): pCapture\n",AK_FALSE);

    Utl_UStrCpy(pCapture->curPath, Fwl_GetDefPath(eIMAGEREC_PATH));
    Fwl_FsMkDirTree(Fwl_GetDefPath(eIMAGE_PATH));
    Fwl_FsMkDirTree(pCapture->curPath);
    Fwl_FsGetFreeSize(pCapture->curPath[0], &pCapture->free_size);

    /**init var*/
    pCapture->pY = AK_NULL;
    pCapture->pU = AK_NULL;
    pCapture->pV = AK_NULL;

    pCapture->photoTotal = 1;
    pCapture->mj = AK_NULL;    
    pCapture->len = JPG_BUF_SIZE;
    pCapture->shotCnt = 0;
    pCapture->shotTimer = ERROR_TIMER;

    pCapture->DCShotMode = gs.DCShotMode;
    if ((pCapture->DCShotMode != DC_NORMAL_SHOT) && (pCapture->DCShotMode != DC_MULTI_SHOT))
    {
        Fwl_Print(C3, M_CAMERA,  "Capture_Init(): Camera mode error.\n");
        pCapture->DCShotMode = DC_NORMAL_SHOT;
    }

    return AK_TRUE;
}

/***********************************************************************************
FUNC NAME: Capture_Free
DESCRIPTION: free resource when exit
INPUT:
OUTPUT:
RETURN:
AUTHOR:zhengwenbo
CREATE DATE:2006-6-29
MODIFY LOG:
***********************************************************************************/
T_VOID Capture_Free(T_CAPTURE *pCapture)
{
    AK_ASSERT_PTR_VOID(pCapture, "Capture_Free(): pCapture Is NULL.\n");

    /*free jpg*/
    if (AK_NULL != pCapture->mj)
    {
        pCapture->mj = Fwl_Free(pCapture->mj);		
    }

	if (pCapture->shotTimer != ERROR_TIMER)
	{
		pCapture->shotTimer = Fwl_StopTimer(pCapture->shotTimer);
	}

    /**free capture's YUV buffer*/
    Capture_FreeBuff(pCapture);    
}

/***********************************************************************************
FUNC NAME: Capture_Handle
DESCRIPTION: response to command
INPUT:
OUTPUT:
RETURN: back state
AUTHOR:zhengwenbo
CREATE DATE:2006-6-27
MODIFY LOG:
***********************************************************************************/
T_eBACK_STATE Capture_Handle(T_CAPTURE* pCapture, T_EVT_CODE Event, T_EVT_PARAM *pEventParm)
{
    T_eBACK_STATE retState = eStay;
	int i = 0x00;

    AK_ASSERT_PTR(pCapture, "Capture_Handle(): pCapture is NULL.\n", eReturn);
    AK_ASSERT_PTR(pEventParm, "Capture_Handle(): pEventParm is NULL.\n", eReturn);
   
    switch (Event)
    {
    case M_EVT_1:
        if (DC_MULTI_SHOT == gs.DCShotMode)
        {
            if (ERROR_TIMER == pCapture->shotTimer)
            {
                pCapture->shotTimer = Fwl_SetTimerMilliSecond(1000, AK_TRUE);
                Fwl_Print(C3, M_CAMERA,  "Capture_Handle(): MULTI_SHOT.\n");
            }

            pCapture->shotCnt = 0;

            Capture_Shot(pCapture);
            
            pCapture->shotCnt++;
        }
        else
        {
            T_pWSTR pStr = AK_NULL;
            /**Normal capture*/
            Capture_Shot(pCapture);

            /*display "save photo..."*/
            pStr = (T_pWSTR)GetCustomString(csCAMERA_SAVE_PHOTO);
            Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
			//modify by wgtbupt
            Fwl_UDispSpeciString(HRGB_LAYER, (T_POS)((Fwl_GetLcdWidth() - (T_S16)UGetSpeciStringWidth(pStr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pStr))) >> 1),
                                (T_POS)((T_S16)(Fwl_GetLcdHeight() - g_Font.SCHEIGHT) >> 1), 
                                pStr, COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pStr));

            Fwl_RefreshDisplay();
		
			memset(pCapture->pY + 640*100,0xff, 640*100);
			
			
            if (!Capture_SavePhoto(pCapture, 0))
            {
            	retState = eStay;
            	Fwl_Print(C3, M_CAMERA,  "Capture_SavePhoto = 0\n");
            }
            else
            {
            	retState = eNext; //to show photo state
            	Fwl_Print(C3, M_CAMERA,  "Capture_SavePhoto = 1\n");
            }
        }

        break;
        
    case VME_EVT_TIMER:
        if (pEventParm->w.Param1 == (T_U32)pCapture->shotTimer)
        {
            T_U32 i;
            /**successive capture*/
            Capture_Shot(pCapture);
            
            pCapture->shotCnt++;
            if (pCapture->shotCnt > pCapture->photoTotal - 1)
            {
                if (pCapture->shotTimer != ERROR_TIMER)
                {
                    pCapture->shotTimer = Fwl_StopTimer(pCapture->shotTimer);
                    pCapture->shotTimer = ERROR_TIMER;
                }

                for (i=0; i<pCapture->photoTotal; i++)
                    Capture_SavePhoto(pCapture, i);

                retState = eNext; //to show multi photo state
            }
        }
        break;
        
    default:
        break;
    }

    return retState;
}

static T_BOOL Capture_EncodeJpg(T_CAPTURE* pCapture, T_U32 PhotoIndex)
{
	T_BOOL bSuccess = AK_FALSE;
	T_BOOL ret = AK_TRUE;
	T_U8 *srcY = AK_NULL, *srcU = AK_NULL, *srcV = AK_NULL;
	T_U8 *desY = AK_NULL;
	T_U32 srcWidth	 = 0;
	T_U32 srcHeight  = 0;
	T_RECT focusRect;
    T_CAMERA_MODE photoMode = 0;
    
	srcWidth   = pCapture->window_width;
	srcHeight  = pCapture->window_height;
	
	/**YUV format: 4:2:0*/
	srcY = pCapture->pY + (srcWidth * srcHeight * PhotoIndex);
	srcU = pCapture->pU + (((srcWidth * srcHeight) >> 2) * PhotoIndex);
	srcV = pCapture->pV + (((srcWidth * srcHeight) >> 2) * PhotoIndex);

    photoMode = gs.camPhotoMode;
	switch(photoMode)
	{		
		case CAMERA_MODE_UXGA:
            if (0 != gb.nZoomInMultiple)
            {
                //Fwl_YuvZoom() only support 1280x1280~18x18, so 1600x1200 zoom 10~20
			    Fwl_CameraGetFocusWin(&focusRect, (gb.nZoomInMultiple + 10), 20, srcWidth, srcHeight);	
			    break;
            }
            //no break
		default:
			Fwl_CameraGetFocusWin(&focusRect, gb.nZoomInMultiple, 10, srcWidth, srcHeight);
			break;
	}
        
	if ((T_U32)(focusRect.width) != srcWidth || (T_U32)(focusRect.height) != srcHeight)
	{		
		T_RECT dstWin;
        
		desY =	Fwl_Malloc((focusRect.width * focusRect.height) * 3/2 + 64);
		AK_ASSERT_PTR(desY, "CAMERA:	desY Malloc Failure", AK_FALSE);

        dstWin.left   = 0;
        dstWin.top    = 0;
        dstWin.width  = focusRect.width;
        dstWin.height = focusRect.height;

		Fwl_Print(C3, M_CAMERA,  "Src[%dx%d]Clip:(%d,%d)[%dx%d]\n",
					srcWidth, srcHeight,
					focusRect.left, focusRect.top,
					focusRect.width, focusRect.height);
		
		ret = Fwl_YuvZoom(srcY, srcU, srcV, (T_LEN)srcWidth, &focusRect, desY, focusRect.width, YUV420, &dstWin);
        
		srcWidth  = focusRect.width;
		srcHeight = focusRect.height;
		
		srcY = desY;
		srcU = srcY + srcWidth*srcHeight;
		srcV = srcU + (srcWidth*srcHeight >> 2);
		
	}

	if (ret && (AK_NULL != pCapture->mj) && (AK_NULL != srcY) && (AK_NULL != srcU) && (AK_NULL != srcV))
	{
		Fwl_Print(C3, M_CAMERA,  "Encode Jpg:photototal=%d,WxH:%dx%d=>%dx%d,pJPG=0x%x.\n",
					pCapture->photoTotal, \
					srcWidth, srcHeight,\
					pCapture->width, pCapture->height,\
					pCapture->mj);

		pCapture->len = JPG_BUF_SIZE;
		bSuccess = Img_YUV2JPEG_Stamp_Mutex(srcY,srcU,srcV,pCapture->mj,&pCapture->len
			,srcWidth, srcHeight,(T_U8)camera_photo_quality[gs.camPhotoQty],AK_NULL);
		
		
	}

	Fwl_Print(C3, M_CAMERA,  "Jpeg Encode %s len:%d\n",bSuccess?"Ok.":"Fail!",pCapture->len);

	if (AK_NULL != desY)
	{
		desY = Fwl_Free(desY);
	}

	if (0 == pCapture->len)
	{
		Fwl_Print(C3, M_CAMERA,  "Jpeg Len Er!\n");
		return AK_FALSE;
	}

	return bSuccess;	
}


/***********************************************************************************
FUNC NAME: Capture_Shot
DESCRIPTION: take photo
INPUT:
OUTPUT:
RETURN:
AUTHOR:zhengwenbo
CREATE DATE:2006-6-27
MODIFY LOG:
***********************************************************************************/
static T_BOOL Capture_Shot(T_CAPTURE *pCapture)
{
    T_U8    *y = AK_NULL;
    T_U8    *u = AK_NULL;
    T_U8    *v = AK_NULL;
    T_BOOL ret = AK_TRUE;
    T_U32  width = pCapture->window_width;
    T_U32  height = pCapture->window_height;
	T_RECT focusRect = {0}, dstWin = {0};
	
    AK_ASSERT_PTR(pCapture, "Capture_Shot():Input parameter is error", AK_FALSE);

    Fwl_Print(C3, M_CAMERA,  "Capture_Shot(): shot count = %d.\n",  pCapture->shotCnt);

    /**YUV format: 4:2:0*/
    y = pCapture->pY + (width * height) * pCapture->shotCnt;
    u = pCapture->pU + ((width * height) >> 2) * pCapture->shotCnt;
    v = pCapture->pV + ((width * height) >> 2) * pCapture->shotCnt;


	/**KaCha Sound*/
	if ((0 == pCapture->shotCnt) && (1 == gs.camSoundSw))
    	Capture_PlayWave();
	
#ifdef CAMERA_FLASHLIGHT
    if (pCapture->bFlashLight)
    {
        Fwl_CameraFlashOn();
    }
#endif    
    
    // drop 3 frame data 
    if (0 == pCapture->shotCnt)
    {
        Fwl_CameraCaptureYUV(y, u, v, width, height, CAMERA_TIMEOUT);
        Fwl_CameraCaptureYUV(y, u, v, width, height, CAMERA_TIMEOUT);
        Fwl_CameraCaptureYUV(y, u, v, width, height, CAMERA_TIMEOUT);
    }
    mini_delay(5);
    ret = Fwl_CameraCaptureYUV(y, u, v, width, height, CAMERA_TIMEOUT);
	
#ifdef CAMERA_FLASHLIGHT
    if (pCapture->bFlashLight)
    {
        Fwl_CameraFlashOff();
    }
#endif

    if (!ret)
    {
        Fwl_Print(C3, M_CAMERA,  "capture YUV fail.\n");
        MsgBox_InitAfx(&pCapture->msgbox, RETURN_TO_HOME, ctFAILURE, csCAMERA_CAPTURE_FAILED, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pCapture->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pCapture->msgbox);
        return AK_FALSE;
    }

    Fwl_Print(C3, M_CAMERA,  "Capture_Shot(): width = %d, height = %d.\n", width, height);
#ifdef OS_ANYKA
    // show YUV For Multi-Shot
    if (DC_NORMAL_SHOT != pCapture->DCShotMode)
    {
		Fwl_CameraGetFocusWin(&focusRect, gb.nZoomInMultiple, 10, width, height);
		dstWin.left   = CAMERA_IMAGE_SHOW_LEFT;
		dstWin.top    = CAMERA_IMAGE_SHOW_TOP;
		dstWin.width  = CAMERA_IMAGE_SHOW_WIDTH;
		dstWin.height = CAMERA_IMAGE_SHOW_HEIGHT;
		ret = Fwl_YuvZoom(y, u, v, width, &focusRect, Fwl_GetDispMemory565(), 
			MAIN_LCD_WIDTH, RGB565, &dstWin);

		if (!ret)
		{
		   Fwl_Print(C3, M_CAMERA,  "Capture_MultShot Failure.\n"); 
		}
		
		Fwl_RefreshDisplay();
		
    	Fwl_Print(C3, M_CAMERA,  "Capture_MultShot: multi shot####.\n");
    }
#endif    
    return ret;
}


/***********************************************************************************
FUNC NAME: Capture_SavePhoto
DESCRIPTION: successive take photo
INPUT:
OUTPUT:
RETURN:
AUTHOR:zhengwenbo
CREATE DATE:2007-12-21
MODIFY LOG:
***********************************************************************************/
static T_BOOL Capture_SavePhoto(T_CAPTURE *pCapture, T_U32 PhotoIndex)
{
	T_USTR_FILE             pFilePath;    
	T_pFILE                 fp;
    T_BOOL                  ret = AK_TRUE;
#ifndef TIME_STAMP_FILE_NAME
    T_STR_INFO              strFnExt;
    T_USTR_FILE             pFnTmp;
    T_USTR_FILE             pFileName;    
	T_USTR_INFO             strInfo, tmpStr;  
#endif

    Fwl_Print(C3, M_CAMERA,  "Capture_SavePhoto(): PhotoIndex = %d.\n", PhotoIndex);

    if (!Capture_EncodeJpg(pCapture, PhotoIndex))
    {
        MsgBox_InitAfx(&pCapture->msgbox, 2, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pCapture->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pCapture->msgbox);

        return AK_FALSE;
    }
	
#ifdef TIME_STAMP_FILE_NAME

	if (!Capture_GetPhotoName(pCapture, pFilePath))
    {
        /**return to preview state machine if get name fail*/
        MsgBox_InitAfx(&pCapture->msgbox, 2, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pCapture->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pCapture->msgbox);
        return AK_FALSE;
    }

    Utl_UStrCpy(pCapture->pFileName, pFilePath);

#else

    if (PhotoIndex == 0)
    {
        if (Capture_GetPhotoName(pCapture, pFileName) == AK_FALSE)
        {
            /**return to preview state machine if get name fail*/
            MsgBox_InitAfx(&pCapture->msgbox, 2, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pCapture->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pCapture->msgbox);
            return AK_FALSE;
        }

        Utl_UStrCpy(pCapture->pFileName, pFileName);
    }

    // create file name
    if (DC_MULTI_SHOT == pCapture->DCShotMode)
    {
        Utl_UStrCpy(pFilePath, pCapture->curPath);
        sprintf(strFnExt, "_%ld.JPG", (PhotoIndex + 1));
        Eng_StrMbcs2Ucs(strFnExt, tmpStr);
        Utl_UStrCpy(pFnTmp, pCapture->pFileName);
        Utl_UStrCat(pFnTmp, tmpStr);
        Utl_UStrCat(pFilePath, pFnTmp);
    }
    else
    {
        Utl_UStrCpy(pFilePath, pCapture->curPath);
        Utl_UStrCat(pFilePath, pFileName);

        Utl_UStrCpy(strInfo, GetCustomString(csCAMERA_SAVE_NAME));
        Utl_UStrCat(strInfo, pFilePath);
        Utl_UStrCat(strInfo, _T(","));
        Utl_UStrCat(strInfo, GetCustomString(csWAITING));
    }
	
#endif

    fp = Fwl_FileOpen(pFilePath, _FMODE_CREATE, _FMODE_CREATE);
    if (fp == _FOPEN_FAIL)
    {
        /**return to preview state machine if open file fail*/
        MsgBox_InitAfx(&pCapture->msgbox, 2, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pCapture->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pCapture->msgbox);

        ret = AK_FALSE;
    }
    else
    {
        Fwl_FileWrite(fp, pCapture->mj, pCapture->len);
        Fwl_FileClose(fp);

        U64subU32(&pCapture->free_size, pCapture->len);

        ret = AK_TRUE;
    }
    
	return ret;
}


/***********************************************************************************
FUNC NAME: Capture_GetPhotoName
DESCRIPTION: get photo file name
INPUT:
OUTPUT:
RETURN:
AUTHOR:lizhuobin
CREATE DATE:
MODIFY LOG:
***********************************************************************************/
static T_BOOL Capture_GetPhotoName(T_CAPTURE *pCapture, T_pWSTR pFileName)
{
#ifdef TIME_STAMP_FILE_NAME

	return MRec_GetStampFileName(pCapture->curPath, "IMG", "JPG", pFileName);

#else

    T_pFILE     fp;
    T_U32       i = 0;
    T_USTR_FILE filePath;
    T_USTR_FILE TmpPath;
    T_STR_FILE  TmpName;    //not unicode string!
    T_USTR_FILE multiFileName;

    Fwl_FsMkDirTree(pCapture->curPath);
    gs.CapFileNum++;
    while (1)
    {
        if (gs.CapFileNum > 999999)
        gs.CapFileNum = 1;

        if (DC_MULTI_SHOT == pCapture->DCShotMode)
        {
            sprintf(TmpName, "AK%06ld", gs.CapFileNum);
            Eng_StrMbcs2Ucs(TmpName, pFileName);
            
            sprintf(TmpName, "AK%06ld_1.JPG", gs.CapFileNum);
            Eng_StrMbcs2Ucs(TmpName, multiFileName);
        }
        else
        {
            sprintf(TmpName, "AK%06ld.JPG", gs.CapFileNum);
            Eng_StrMbcs2Ucs(TmpName, pFileName);
        }

        if (i > 1024)
        {
            return AK_FALSE;
        }
        
        Utl_UStrCpy(filePath, pCapture->curPath);
        if (DC_MULTI_SHOT == pCapture->DCShotMode)
        {
            Utl_UStrCat(filePath, multiFileName);
        }
        else
        {
            Utl_UStrCat(filePath, pFileName);
        }
        Utl_UStrCpy(TmpPath, filePath);
        
        if ((fp = Fwl_FileOpen(TmpPath, _FMODE_READ, _FMODE_READ)) != _FOPEN_FAIL)
        {
            Fwl_FileClose(fp);
        }
        else
        {
            break;
        }
        gs.CapFileNum++;
        i++;
    }

    return AK_TRUE;
#endif	
}


/***********************************************************************************
FUNC NAME: Capture_SetParm
DESCRIPTION: set camera parmeters accord to preview state machine
INPUT:
OUTPUT:
RETURN:
AUTHOR:zhengwenbo
CREATE DATE:2006-7-5
MODIFY LOG:
***********************************************************************************/
T_VOID Capture_SetParm(T_CAPTURE *pCapture, T_PREVIEW *pPreview)
{
    
    T_U32 srcW = 0, srcH = 0;
    T_CAMERA_MODE photoMode = 0;
	T_CAMERA_TYPE camType = 0;
    
    if ((AK_NULL == pCapture) || (AK_NULL == pPreview))
    {
        AK_ASSERT_PTR_VOID(pPreview, "CameraCapture_GetParm(): pCapture or pPreview is NULL.\n");
        return;
    }

	if (DC_NORMAL_SHOT == pCapture->DCShotMode)
    {
        pCapture->photoTotal = 1;
		photoMode = gs.camPhotoMode;		
    }
    else
    {
        pCapture->photoTotal = CAM_MULTI_MAX; 
		photoMode = gs.camMultiShotMode;
    }
	
    pCapture->bFlashLight = pPreview->bFlashLight;	

	pCapture->width  = pPreview->shotParm.c1;
    pCapture->height = pPreview->shotParm.c2;

	srcW = pCapture->width;
	srcH = pCapture->height;

	switch(photoMode)
	{		
		case CAMERA_MODE_UXGA:		
		case CAMERA_MODE_SXGA:
        case CAMERA_MODE_720P:    
            camType = Fwl_CameraGetType();
            
            if ((CAMERA_2M <= camType) && (SDRAM_MODE >= 16)) //support real 1600x1200,1280x1024,1280x720
            {
			    pCapture->window_width = pPreview->shotParm.c1;
    		    pCapture->window_height = pPreview->shotParm.c2;
                Fwl_CameraSetToCap(srcW, srcH);                
#ifdef OS_ANYKA
                //0x2643 Sensor is ready after 1.5s .
                if (0x2643 == cam_get_id())
                {                    
                    mini_delay(1500);
                }
#endif
                
			    break;
            }
            //no break
		default:
            pCapture->window_width = pPreview->shotParm.width;
            pCapture->window_height = pPreview->shotParm.height;
			Fwl_CameraGetRecSize(&srcW, &srcH);// avoid return from record, when diffrent size will error
            Fwl_CameraSetToCap(srcW, srcH);             
            break;
	}

    Fwl_CameraSetWindows(pCapture->window_width, pCapture->window_height);
    
    Fwl_Print(C3, M_CAMERA,  "pCapture->window_width=%d\n",pCapture->window_width);
    Fwl_Print(C3, M_CAMERA,  "pCapture->window_height=%d\n",pCapture->window_height);
    Fwl_Print(C3, M_CAMERA,  "pCapture->width=%d\n",pCapture->width);
    Fwl_Print(C3, M_CAMERA,  "pCapture->height=%d\n",pCapture->height);
}

/*
 * @brief  the function malloc YUV buffer for capture
 *
 * @author Zhengwenbo
 * @date 2006-08-08
 * @param[in] pPreview   the point of capture control
 * @return  result 1--success 0--fail
 */
T_BOOL Capture_MallocBuff(T_CAPTURE *pCapture)
{
    T_U32  width = pCapture->window_width;
    T_U32  height = pCapture->window_height;
    T_U32 szbuf;
    
    AK_ASSERT_PTR(pCapture, "input parameter is error\n", AK_FALSE);
    
    width  = width%2  ? width+1  :  width;	// Must Even, Else Memory Beyond
    height = height%2 ? height+1 : height;

    szbuf = width * height * pCapture->photoTotal;
    
    /*malloc for capturing YUV*/
    pCapture->pY = (T_U8*)Fwl_Malloc(szbuf + 64);
    AK_ASSERT_PTR(pCapture->pY, "Capture YUV malloc fail\n", AK_FALSE);

    /**YUV format: 4:2:0*/
    pCapture->pU = (T_U8*)Fwl_Malloc((szbuf >> 2) + 64);
    AK_ASSERT_PTR(pCapture->pU, "Capture YUV malloc fail\n", AK_FALSE);

    pCapture->pV = (T_U8*)Fwl_Malloc((szbuf >> 2) + 64);
    AK_ASSERT_PTR(pCapture->pV, "Capture YUV malloc fail\n", AK_FALSE);

    if (AK_NULL == pCapture->mj)
    {
        pCapture->mj = (T_U8 *)Fwl_Malloc(JPG_BUF_SIZE);
        AK_ASSERT_PTR(pCapture->mj, "JPEG buffer malloc fail.\n", AK_FALSE);
    }
    
    return AK_TRUE;
}

/*
 * @brief  free YUV buffer
 *
 * @author Zhengwenbo
 * @date 2006-08-08
 * @param[in] pPreview   the point of capture control
 * @return  void
 */
static T_VOID Capture_FreeBuff(T_CAPTURE *pCapture)
{
    AK_ASSERT_PTR_VOID(pCapture, "Capture_MallocBuff():input parameter is error\n");

    if (AK_NULL != pCapture->pY)
    {
        pCapture->pY = Fwl_Free(pCapture->pY);
    }

    if (AK_NULL != pCapture->pU)
    {
        pCapture->pU = Fwl_Free(pCapture->pU);
    }

    if (AK_NULL != pCapture->pV)
    {
        pCapture->pV = Fwl_Free(pCapture->pV);
    }

    pCapture->pY = AK_NULL;
    pCapture->pU = AK_NULL;
    pCapture->pV = AK_NULL;
}

#if 0
/*
 * @brief  display saving message box
 *
 * @author Zhengwenbo
 * @date 2006-08-18
 * @param[in] pCapture   the point of capture control
 * @param[in] tms the number of millionsecond
 * @return  void
 */
static T_VOID Capture_DispCaptureMsg(T_CAPTURE *pCapture, T_S16 tms)
{
    T_USTR_INFO              strInfo;
    T_RECT                  msgRect;

    Utl_UStrCpy(strInfo, GetCustomString(csCAMERA_SAVE_PHOTO));
    Utl_UStrCat(strInfo, GetCustomString(csWAITING));
    MsgBox_InitStr(&pCapture->msgbox, 0, GetCustomString(csCAMERA_SAVE_PHOTO),strInfo, MSGBOX_INFORMATION);
    MsgBox_SetDelay(&pCapture->msgbox, tms);
    MsgBox_Show(&pCapture->msgbox);
    MsgBox_GetRect(&pCapture->msgbox, &msgRect);
    Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);
}
#endif

#endif

