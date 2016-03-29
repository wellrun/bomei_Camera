#include "fwl_vme.h"
#include "Fwl_osMalloc.h"
#include "Fwl_Initialize.h"
#include "Lib_event.h"
#include "eng_string.h"
#include "Eng_ImgConvert.h"
#include "gbl_global.h"
#include "Eng_pwr_onoff_video.h"
#include "fwl_osfs.h"
#include "ctl_aviplayer.h"
#include "fwl_keyhandler.h"
#include "fwl_waveout.h"
#include "fwl_pfaudio.h"
#include "fwl_pfdisplay.h"
#include "eng_debug.h"
#include "fwl_display.h"
#include "Fwl_Image.h"
#include "Fwl_tscrcom.h"
#include "Log_MediaPlayer.h"
#include "Eng_ImgDec.h"
#include "Fwl_oscom.h"

#ifdef SUPPORT_VIDEOPLAYER

typedef struct {
    T_U32       play_count;
    T_BOOL      openflag;
} PwrOnOffVideo;

static PwrOnOffVideo *videoplayer = AK_NULL;
#endif
T_BOOL Eng_PwrOnOff_Video_Init(T_VOID)
{
#ifdef SUPPORT_VIDEOPLAYER

    Fwl_SetAudioVolumeStatus(AK_TRUE);
	
	if (videoplayer)
		videoplayer = Fwl_Free(videoplayer);
	
    videoplayer = (PwrOnOffVideo *)Fwl_Malloc(sizeof(PwrOnOffVideo));
    AK_ASSERT_PTR(videoplayer, "Eng_PwrOnOff_Video_Init(): videoplayer malloc error", AK_FALSE);
    
    videoplayer->play_count = 0;
    videoplayer->openflag = AK_FALSE;
#endif
    return AK_TRUE;
}

T_BOOL Eng_PwrOnOff_Video_Open(T_pCWSTR filepath)
{
#ifdef SUPPORT_VIDEOPLAYER
    if (!MPlayer_Open((T_pVOID)filepath, AK_TRUE))
    {
        Fwl_Print(C3, M_CTRL, "Eng_PwrOnOff_Video_Open(): movie open error !\n");
        return AK_FALSE;
    }

    videoplayer->openflag = AK_TRUE;
    
    /*Since video_lib 1.4.4, the 176*144 3gp files are open to 352*288, but it's not needed!
      So,set files to play with the origial size
    */    
#endif
    return AK_TRUE;
}

T_eBACK_STATE Eng_PwrOnOff_Video_Handle(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_MMI_KEYPAD phyKey;

    switch (event)
    {
	case M_EVT_PUB_TIMER:
        if(videoplayer->play_count  >= PONOFF_VIDEO_PLAY_TIME)
        {
            return eReturn;
        }
        else	
        {
            videoplayer->play_count++;
            return eStay;
        }

    case M_EVT_USER_KEY:
        phyKey.keyID = pEventParm->c.Param1;
        phyKey.pressType = pEventParm->c.Param2;
		
        if ((kbCLEAR == phyKey.keyID) && ((PRESS_UP == phyKey.pressType) ||(PRESS_LONG == phyKey.pressType)))
            return eStay;
        else
            return eReturn;

#ifdef TOUCH_SCR
    case M_EVT_TOUCH_SCREEN:
        if(M_EVT_TOUCH_SCREEN == event && eTOUCHSCR_UP == pEventParm->s.Param1)
        {
            return eReturn;
        }
		else
		{
			return eStay;
		}
        break;
#endif

    default:
        return eStay;
        break;
    }
#endif
	return eStay;
}


T_VOID Eng_PwrOnOff_Video_Free(T_VOID)
{
#ifdef SUPPORT_VIDEOPLAYER

    if (AK_NULL != videoplayer)
    {
        if  (videoplayer->openflag)
        {
            if (MPLAYER_STOP != AVIPlayer_GetStatus())
            {
				WaveOut_SetFade(200, FADE_STATE_OUT);
				AK_Sleep(40);
				
                MPlayer_Close();
            }
            
            videoplayer->openflag = AK_FALSE;
        }
        videoplayer = Fwl_Free(videoplayer);
    }
    
    Fwl_AudioDisableDA();

	AK_Sleep(20);//加延迟保证刷新完毕
	Fwl_TurnOff_YUV();
#endif
}

T_VOID Eng_PwrOnOff_Video_Suspend(T_VOID)
{
#ifdef SUPPORT_VIDEOPLAYER

    Fwl_Print(C3, M_CTRL, "Movie Suspend.\n");
    
    MPlayer_Pause();
#endif
}

T_VOID Eng_PwrOnOff_Video_Resume(T_VOID)
{
#ifdef SUPPORT_VIDEOPLAYER

    Fwl_Print(C3, M_CTRL, "Movie Resume.\n");
	
    Fwl_FillSolid(HRGB_LAYER,COLOR_BLACK);
    Fwl_RefreshDisplay();
	
	MPlayer_Resume();
#endif
}


T_VOID AK_PowerOnFrameShow(T_U8 *y, T_U8 *u, T_U8 *v, T_U16 srcW, T_U16 srcH, T_U16 oriW, T_U16 oriH)
{
#ifdef OS_ANYKA
#ifdef SUPPORT_VIDEOPLAYER
  	AVIPlayer_YUV2DispBuff(y, u, v, srcW, srcH);
	Fwl_RefreshDisplay();
#endif
#endif
}

/* @show the power on/off picture
** @return: 1: gif
**          0: read sucess
**         -1: fail
*/
T_S8 PowerOnOff_ShowPicture(T_USTR_FILE file_path)
{
    T_U8	*bmp_buf;
	T_hFILE	fp = _FOPEN_FAIL;
	T_U8	typeInfo[IMG_TYPE_INFO_SIZE];

	// Open File
	if (_FOPEN_FAIL == (fp = Fwl_FileOpen(file_path, _FMODE_READ, _FMODE_READ)))
	   	return -1;

	Fwl_FileRead(fp, typeInfo, IMG_TYPE_INFO_SIZE);
	Fwl_FileClose(fp);
		
    if (FILE_TYPE_GIF == ImgDec_GetImgType(typeInfo))
        return 1;

	// Get BMP File Path
    if (file_path == gs.PathPonPic)
    {
        Utl_UStrCpy(file_path, _T(PON_CACHE_PIC));
    }
    else if (file_path == gs.PathPoffPic)
    {
        Utl_UStrCpy(file_path, _T(POFF_CACHE_PIC));
    }
	else
		return -1;

	// Decode Image
	bmp_buf = ImgDec_GetImageData(file_path);
    if (AK_NULL != bmp_buf)
    {
		T_U32 	scale;    
    
        GetBMP2RGBBuf(bmp_buf, Fwl_GetDispMemory(), Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), 0, 0, 100, 0, &scale, COLOR_BLACK);
        ImgDec_FreeImageData(bmp_buf);
		
		Fwl_RefreshDisplay();
		
        return 0;
    }
    else
    {
        return -1;
    }    
}


T_BOOL PowerOnOff_PlayAudio(T_USTR_FILE path, T_fEND_CB end_cb)
{
#ifdef SUPPORT_AUDIOPLAYER

   	if (MPlayer_Open(path, AK_TRUE))
   	{
		MPlayer_SetEndCB(end_cb);
		WaveOut_SetFade(1000, FADE_STATE_IN);
       	if (MPlayer_Play(0))
       	{
			return AK_TRUE;
       	}
   	}
#endif
	return AK_FALSE;
}

T_BOOL PowerOnOff_PlayVideo(T_USTR_FILE path, T_fEND_CB endCB)
{
#ifdef SUPPORT_VIDEOPLAYER

	if (AVIPlayer_IsSupportFileType(path)
    	&& Eng_PwrOnOff_Video_Init() 
    	&& Eng_PwrOnOff_Video_Open(path))
    {     	
		MPlayer_SetEndCB(endCB);
		MPlayer_SetShowFrameCB(AK_PowerOnFrameShow);  
		Fwl_AudioSetVolume(Fwl_GetAudioVolume());     

		Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
		WaveOut_SetFade(1000, FADE_STATE_IN);
		
		if (MPlayer_Play(0))
		{
        	Fwl_Print(C3, M_CTRL, "POWER_ON_OFF:	Play Video Success!\n");
			return AK_TRUE;
		}
    }
#endif
	return AK_FALSE;
}

T_VOID PowerOnOff_PlayImage(T_USTR_FILE file_path, T_GIF_PLAYER *pGPlayer)
{
	T_S8	imageType;
	T_U32 	fileLen = 0;
	T_pFILE fd;

	if (0 == (imageType = PowerOnOff_ShowPicture(file_path)))
	{
		AK_Sleep(100);	// Show Picture
		return;
	}

	// Open GIF File
	if (1 == imageType
		&& _FOPEN_FAIL != (fd = Fwl_FileOpen(file_path, _FMODE_READ, _FMODE_READ)))
    {
		pGPlayer->pGif = (T_U8*)Fwl_Malloc(fileLen = Fwl_GetFileLen(fd));
		if (pGPlayer->pGif
			&& fileLen == Fwl_FileRead(fd, pGPlayer->pGif, fileLen))
        {
			 pGPlayer->bMalloc = AK_TRUE;            
        }
		else
			pGPlayer->pGif = Fwl_Free(pGPlayer->pGif);

		Fwl_FileClose(fd);            
    }    

	// if open fail, use default gif
	if (!pGPlayer->pGif)
		pGPlayer->pGif = (T_pDATA)Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_GIF_BOOT_ANIMATION, &fileLen);

	// Init GIF Decoder
    if (IMG_INVALID_HANDLE != (pGPlayer->gifenum = GIFEnum_New(pGPlayer->pGif, fileLen)))
    {
		T_U32 	delay;
		
        if (!GIFEnum_FirstFrame(pGPlayer->gifenum))
        {
            Fwl_Print(C3, M_CTRL, "GIFEnum_FirstFrame() error!!!\n");
        }
        else
        {
            delay = GIFEnum_GetCurDelay(pGPlayer->gifenum);
            if (delay < 10)
                delay = 10;
            pGPlayer->timer = Fwl_SetTimerMilliSecond(delay, AK_TRUE);

            pGPlayer->bOpen = AK_TRUE;
        }
    }
}

T_VOID PowerOnOff_CloseGif(T_GIF_PLAYER *pGPlayer)
{
	pGPlayer->bOpen = AK_FALSE;
	GIFEnum_Close(pGPlayer->gifenum);
	
	if (pGPlayer->bMalloc)
	{
		Fwl_Free(pGPlayer->pGif);
	}
	
	Fwl_StopTimer(pGPlayer->timer);
	pGPlayer->timer = ERROR_TIMER;	
}

T_VOID PowerOnOff_HandleGif(T_POWER_PARM *pPowerParam)
{
	T_U32 dataLen;
    T_U8 bitsPerPix;
    T_U8 *BmpData;
    T_U32 scale;

    BmpData = (T_U8 *)GIFEnum_GetCurBMP(pPowerParam->gif.gifenum, &dataLen, &bitsPerPix);
    GetBMP2RGBBuf(BmpData, Fwl_GetDispMemory(), Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), 0, 0, 100, 0,&scale,COLOR_BLACK);
    Fwl_RefreshDisplay();
	
    if(!GIFEnum_NextFrame(pPowerParam->gif.gifenum))
    {
		if (pPowerParam->audioPlay)
			GIFEnum_FirstFrame(pPowerParam->gif.gifenum);
		else
        	PowerOnOff_CloseGif(&pPowerParam->gif);
    }
}

T_VOID PowerOnOff_PlayMedia(T_U8 medaiType, T_USTR_FILE vidoe, T_USTR_FILE audio, T_USTR_FILE picture, 
				T_POWER_PARM *param, T_fEND_CB endCB)
{
	Fwl_Print(C3, M_CTRL, "Media Type: %d.\n", medaiType);

	switch (medaiType)
	{
	case VIDEO_MEDIA:		
		if(PowerOnOff_PlayVideo(vidoe, endCB))
			param->videoPlay = AK_TRUE;        
		break;

	case PIC_AUDIO_MEDIA:
	case PICTURE_MEDIA:
		PowerOnOff_PlayImage(picture, &param->gif);
		
		if (PICTURE_MEDIA == medaiType)
			break;
		
	case AUDIO_MEDIA:
		if (PowerOnOff_PlayAudio(audio, endCB))
			param->audioPlay = AK_TRUE;
		break;

	default:
		break;
	}
}

//end of file

