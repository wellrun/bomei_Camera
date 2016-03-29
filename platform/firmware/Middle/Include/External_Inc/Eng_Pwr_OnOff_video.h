
#ifndef ENG_PWR_ONOFF_VIDEO
#define ENG_PWR_ONOFF_VIDEO


#include "akdefine.h"
#include "Log_MediaPlayer.h"
#include "Lib_Image_api.h"

#define PONOFF_VIDEO_PLAY_TIME 	10
#define PONOFF_VIDEO_PLAY_SPEED    20  //ms


typedef struct tag_GifPlayer{
	T_TIMER     timer;
    T_hGIFENUM  gifenum;
    T_pDATA     pGif;
	T_BOOL      bMalloc;
    T_BOOL      bOpen;
}T_GIF_PLAYER;

typedef struct {
	T_GIF_PLAYER gif;
    T_TIMER timer_off;      //for the whole pwr_off process to use    
    T_BOOL	audioPlay;
	T_BOOL	videoPlay;
    T_BOOL	playEnd;
} T_POWER_PARM;

T_BOOL Eng_PwrOnOff_Video_Init(T_VOID);

T_BOOL Eng_PwrOnOff_Video_Open(T_pCWSTR filepath);

T_eBACK_STATE Eng_PwrOnOff_Video_Handle(T_EVT_CODE event, T_EVT_PARAM *pEventParm);

T_VOID Eng_PwrOnOff_Video_Free(T_VOID);

T_VOID Eng_PwrOnOff_Video_Suspend(T_VOID);

T_VOID Eng_PwrOnOff_Video_Resume(T_VOID);

/*****************************************************************************/

T_VOID AK_PowerOnFrameShow(T_U8 *y, T_U8 *u, T_U8 *v, T_U16 srcW, T_U16 srcH, T_U16 oriW, T_U16 oriH);

T_VOID AK_PowerOnOffAudioStop(T_POWER_PARM *pPower);

T_VOID PowerOnOff_CloseGif(T_GIF_PLAYER *pGPlayer);
T_VOID PowerOnOff_HandleGif(T_POWER_PARM *pPower);

T_VOID PowerOnOff_PlayMedia(T_U8 medaiType, T_USTR_FILE vidoe, T_USTR_FILE audio, T_USTR_FILE picture, 
				T_POWER_PARM *param, T_fEND_CB endCB);
T_BOOL PowerOnOff_PlayVideo(T_USTR_FILE path, T_fEND_CB endCB);

#endif

