#ifndef __CTL_FM_H__
#define __CTL_FM_H__

#include "Fwl_pfFm.h"
#include "Ctl_MsgBox.h"

#define FM_FREQ_INVALID             0
#define FM_LIST_STATION_NUM         50
#define FM_EXIT_DELAY_TIME          500  //1000  //5000ms
#define FM_DEFAULT_STEP             100000
#define FM_DEFAULT_FREQ_MIN         87000000
#define FM_DEFAULT_FREQ_MAX         108000000

#define OPERATE_REFRESH_ALL         0x00ff
#define OPERATE_REFRESH_NONE        0x0000
#define FREQ_REFRESH                0x0001
#define VOLUME_REFRESH              0x0002
#define PROGRESS_REFRESH            0x0004
#define STATE_REFRESH               0x0008
#define AREA_REFRESH                0x0010

#define BUTTON_VTL_SPACE            30

typedef enum {
	FM_SEARCH_MODE_NONE = 0,
    FM_SEARCH_MODE_ONE,
    FM_SEARCH_MODE_ALL
}T_FM_SEARCH_MODE;

typedef enum {
    FM_SEARCH_DIR_INC = 0,
    FM_SEARCH_DIR_DEC
}T_FM_SEARCH_DIR;


typedef enum {
    FM_STATE_STOP = 0,
    FM_STATE_PAUSE,
    FM_STATE_PLAY,
    FM_STATE_SEARCH,
    FM_STATE_BG_PLAY       //back ground play
}T_FM_STATE;

typedef enum {
    FM_LIST_NULL = 0,
    FM_LIST_FREQ_EXIST,
    FM_LIST_NORMAL,
    FM_LIST_FULL
}T_FM_LIST_RET;

typedef enum {
	FM_HANDLER_NORMAL = 0,
	FM_HANDLER_MENU,
	FM_HANDLER_LIST_FULL,
	FM_HANDLER_BTN_LIST,// Bennis Zhang added
	FM_HANDLER_EXIT,
	FM_HANDLER_ERROR
}T_FM_HANDLER_RET;

typedef struct {
    T_U32       FreqMin;
    T_U32       FreqMax;
    T_U32       FreqIf;      //中频
    T_U32       FreqRef;     //参考频率
    T_U32       FreqStep;    //步长或波宽
}T_FM_AREA_PARM;


typedef struct {
    T_pCDATA                    pBckgrnd;
    T_pCDATA                    pSubBckgrnd[3];
    T_pCDATA                    pPlayIcon;
    T_pCDATA                    pPauseIcon;
    T_pCDATA                    pStopIcon;
    T_pCDATA                    pSearchIcon[2];
    T_pCDATA                    pFreqPnt;
    T_pCDATA                    pVolume[10];
    T_pCDATA                    pMenu;
    T_pCDATA                    pVolumePlus;
    T_pCDATA                    pVolumeMinus;
	T_pCDATA					pResDataBtnList;// Bitmap resource data for list button; Bennis Zhang added


    T_RECT                      BckgrndRect;
    T_RECT                      SubBckgrndRect[3];
    T_RECT                      StateIconRect;
    T_RECT                      AreaStrRect;
    T_RECT                      FreqStrRect;
    T_RECT                      FreqPntDefRect;
     T_RECT                     FreqBarRect;        // the rect of frequance bar
    T_RECT                      VolumeRect[10];
    T_RECT                      AllVolumeRect;
    T_RECT                      NumRect;
//    T_RECT                      MenuRect;
    T_RECT                      VolumePlusRect;
    T_RECT                      VolumeMinusRect;
	T_RECT						rectBtnList;// Bitmap data rectangle data; Bennis Zhang added
	T_RECT						rectBtnUp;
	T_RECT						rectBtnDown;
	T_RECT						rectBtnLeft;
	T_RECT						rectBtnRight;
   
    T_LEN                       subRadius;
}T_FM_PLAYER_RES;

typedef struct {
    T_U32                       CurFreq;
    T_FM_SEARCH_MODE            SearchMode;
    T_FM_SEARCH_DIR             SearchDir;              // 1:search up   0:search down
    T_FM_STATE                  FmState;
    T_FM_STATE                  SavedState;
    T_U8                        StationIndex;           //The cur Freq index in fm list, used for show and save
    T_U16                       RefreshStatus;
    T_U16                       Volume;
    T_FM_AREA                   FmArea;
    T_U32                       BandWidth;
    T_U32                       FreqMin;
    T_U32                       FreqMax;
    T_FM_PLAYER_RES             PlayerRes;
    T_TIMER                     SearchTimer;
    T_TIMER                     SearchTimerCounter;
    T_BOOL                      StepDir;
    T_BOOL                      bInFmPlayer;            //indicate whether the fm works in fm player statemachine
    T_BOOL                      bSearchStart;   
}T_FM_PLAYER_PARM;

/**
 * @brief FM init(hardware and resource)
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return AK_TRUE: init ok
           AK_FALSE:init fail
 */
T_BOOL Ctl_FmInit(T_VOID);

/**
 * @brief  check whether the fm is free
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return  AK_TRUE ,if  has been freed, else AK_FALSE
 */
T_BOOL Ctl_FmBeFree(T_VOID);


/**
 * @brief Fm show refresh
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return T_VOID
 */
T_VOID Ctl_FmShow(T_VOID);

/**
 * @brief FM free(hardware and resource)
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return T_VOID
 */
T_VOID Ctl_FmFree(T_VOID);

/**
 * @brief FM event process function
 * @author GuoHui
 * @date 2008-03-28
 * @param Event:the system event
          pEventParm:the system event param
 * @return T_FM_HANDLER_RET: accord to the return value, do state switch in 
                             state machine
 */
T_FM_HANDLER_RET Ctl_FmHandler(T_EVT_CODE Event, T_EVT_PARAM *pEventParm);

/**
 * @brief get fm work state
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return T_FM_STATE: fm state
 */
T_FM_STATE Ctl_FmGetState(T_VOID);

/**
 * @brief set fm work state
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return  AK_TRUE if success , else AK_FALSE
 */
T_BOOL Ctl_FmSetState(T_FM_STATE status);


/***********************************************************************************
FUNC NAME: Search_Stop
DESCRIPTION: stop search
INPUT:
OUTPUT:
RETURN:
AUTHOR:
MODIFY LOG:
***********************************************************************************/
T_VOID FmSearchStop(T_VOID);

/**
 * @brief play a freq
 * @author GuoHui
 * @date 2008-03-28
 * @param Freq: the freq to be played
          bList:whether is list play(0:no, 1:yes)
 * @return AK_TRUE: play ok
           AK_FALSE:play failed
 */
T_BOOL Ctl_FmPlayFreq(T_U32 Freq, T_BOOL bList);

/**
 * @brief get fm station list storage state
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return T_FM_LIST_RET: the list stroage status
 */
T_FM_LIST_RET Ctl_FmGetListState(T_VOID);

/**
 * @brief save a freq to the fm station list
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return T_FM_LIST_RET: the list stroage status
 */
T_FM_LIST_RET Ctl_FmSaveFreq(T_U32 Freq);

/**
 * @brief get fm current freq
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return T_U32: the freq to be return
 */
T_U32 Ctl_FmGetCurFreq(T_VOID);

/**
 * @brief start fm auto search
 * @author GuoHui
 * @date 2008-03-28
 * @param Dir:the search diredtion
          Mode:search mode
 * @return T_VOID
 */
T_VOID Ctl_FmAutoSearch(T_FM_SEARCH_DIR Dir, T_FM_SEARCH_MODE Mode);

/**
 * @brief set fm refresh status
 * @author GuoHui
 * @date 2008-03-28
 * @param refresh:the refresh flag
 * @return T_VOID
 */
T_VOID Ctl_FmSetRefresh(T_U16 Refresh);

/**
 * @brief change fm work area(here need change some param)
 * @author GuoHui
 * @date 2008-03-28
 * @param Area:the area to be turn to
 * @return AK_TRUE: change area ok
           AK_FALSE:change area failed
 */
T_BOOL Ctl_FmChangeArea(T_FM_AREA Area);


/**
 * @brief delay to close fm(when headphone plug out)
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return T_VOID
 */
T_VOID Ctl_FmDelayFree(T_VOID);


/**
 * @brief cance delay close fm(when headphone plug in)
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return T_VOID
 */
T_VOID Ctl_FmDelayFreeCancle(T_VOID);

/**
 * @brief load image data
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID 
 * @return T_VOID
 */
T_VOID FmPlayerGetRes(T_VOID);

T_VOID FmDelayCallback(T_TIMER TimerId, T_U32 Delay);

T_BOOL Ctl_FmSetVol(T_U16 vol);
T_BOOL Ctl_FmSetFreq(T_U32 freq);
T_BOOL Ctl_FmCheck(T_VOID);
T_BOOL Ctl_CheckFmIsBgPlay(T_VOID);

#endif
