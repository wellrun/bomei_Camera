
#ifndef __UTL_IMGBROWSE_H__
#define __UTL_IMGBROWSE_H__

#include "anyka_types.h"
#include "ctl_global.h"
#include "Fwl_osFS.h"
#include "lib_image_api.h"
#include "Ctl_DisplayList.h"
#include "Eng_ImgDec.h"
#include "fwl_keyhandler.h"


#define NO_SUPPORT_PIC          L"B:/nonsupport9.bmp"

#define IMG_BROWSE_REFRESH_ALL              0xffff
#define IMG_BROWSE_REFRESH_IMG              0x1
#define IMG_BROWSE_REFRESH_INFO             0x2
#define IMG_BROWSE_REFRESH_NONE             0x0

#define PART2ALL_SHOW_WIDTH                 3
//if slide_interval > MULTIMETHOD_SHOW_INTERVAL, multimethod show.
#define MULTIMETHOD_SHOW_INTERVAL           2

#define IMG_CTL_SHOW_INTERVAL                10   // second , the  timer control of image to show,then hide
#define IMG_SHOW_FAST_TIMER                  10    // ms

#define  CON_DEFAULT_LEFT  10
#define  CON_DEFAULT_TOP   10 

typedef enum {
    IMG_ACTION_NULL = 0,
    IMG_UP_PIC,
    IMG_DOWN_PIC,
    IMG_LEFT_PIC,
    IMG_RIGHT_PIC,
    IMG_ZOOM_OUT,
    IMG_ZOOM_IN,
    IMG_ROLL_LEFT,
    IMG_ROLL_RIGHT,
    IMG_ROLL_UP,
    IMG_ROLL_DOWN,
    IMG_ROTATE_90,
    IMG_SLIDE_START,
    IMG_SLIDE_ONE,
    IMG_SLIDE_STOP,
    IMG_GO_MENU,
    IMG_GO_RETURN,
    IMG_GO_RETURN_HOME,
    IMG_ACTION_NUM
} T_IMG_ACTION;

typedef enum {
    IMG_STAY = 0,
    IMG_RETURN,
    IMG_RETURN_HOME,
    IMG_MENU,
    IMG_OPEN_ERROR,
    IMG_MULTIMETHODSHOW_ERROR,
    IMG_RET_NUM
} T_IMG_RET;

typedef enum {
    IMG_NOT_DISPOSTFIX = 0,
    IMG_DISPOSTFIX,
    IMG_DISPOSTFIX_NUM
} T_IMG_DIS_POSTFIX;

typedef enum {
    IMG_NOT_DISPIXEL = 0,
    IMG_DISPIXEL,
    IMG_DISPIXEL_NUM
} T_IMG_DIS_PIXEL;

typedef enum {
    IMG_BROWSE,
    IMG_MAP,
    IMG_PREVIEW,
    IMG_SLIDE,
    IMG_DISMODE_NUM
} T_IMG_DIS_MODE;

typedef enum {
    IMG_NOT_FAIL = 0,
    IMG_NO_ENOUGH_MOMORY,
    IMG_NOT_SUPPORT_SIZE,
    IMG_NOT_SUPPORT_PIC,
    IMG_FAIL_REASON_NUM
} T_IMG_FAIL_REASON;


typedef enum {
    IMG_SATY = 0,
	IMG_ZOOM,
    IMG_ROTATE,
    IMG_EXIT,
    IMG_MOVE,
    IMG_EX_NUM
} T_IMG_TOUCHSCR_DOWN;



typedef struct
{
    T_RECT      ButtnRect;
    T_pCDATA    pRes;
}T_IMG_BUTTON_RES;

typedef struct
{
    T_pDATA	pData;
	T_U32	DataLen;
	T_RECT	rect;
}T_IMG_WIN_INFO;

typedef struct
{
    T_IMG_WIN_INFO Rotate;
	T_IMG_WIN_INFO Return;
	T_IMG_WIN_INFO ZoomBlock;
	T_IMG_WIN_INFO ZoomSchedule;
	T_IMG_WIN_INFO ZoomPoint;
}T_IMG_WIN_PHOTO;

typedef struct {
    T_BOOL                  DEFLAT_FLAG;
	T_BOOL                  IMG_GETDATA_FLAG;
    T_U32                   offsetX;
    T_U32                   offsetY;
    T_U32                   zoom;
    T_U16                   rotate;
    T_U32                   scale;
    T_BOOL                  largeFlag;
    T_DISPLAYLIST           *pDisplayList;
    T_TIMER                 slideTimerId;
    T_U16                   refreshFlag;
    T_IMGDEC_ATTRIB         ImgAttrib;
    T_IMGDEC_LARGE_ATTRIB   LargeImgAttrib;
    T_TIMER                 DecTimerId;
    T_IMG_DIS_POSTFIX       DisPostfix;
    T_IMG_DIS_PIXEL         DisPixel;
    T_IMG_DIS_MODE          DisMode;
    T_U32                   OffsetChangeV;
    T_U32                   ZoomChangeV;
	T_U16                   T_ZoomChangeV;

    T_BOOL                  bShowBttn;
    
    T_IMG_BUTTON_RES        BkgdRect;
    T_IMG_BUTTON_RES        MenuBttn;
    T_IMG_BUTTON_RES        NextBttn;
    T_IMG_BUTTON_RES        PrvBttn;
    T_IMG_BUTTON_RES        UpBttn;
    T_IMG_BUTTON_RES        DownBttn;
    T_IMG_BUTTON_RES        ZoomInBttn;
    T_IMG_BUTTON_RES        ZoomOutBttn;
    T_IMG_BUTTON_RES        RotateBttn;
    T_IMG_BUTTON_RES        ReturnBttn;

    T_FILE_INFO             fsFileInfo;
    T_TIMER                 imgCtlTimer;
	T_POS					virleft;
	T_POS					virtop;
	T_POS					down_x;
	T_POS					down_y;
	T_POS					move_x;
	T_POS					move_y;
	T_U16                   ExOffsetX;
	T_U16                   ExOffsetY;
	T_U16                   DispPartX;
	T_U16                   DispPartY;
	T_U32					srcW;
	T_U32					srcH;
	T_pDATA                 pDatabuf;  
	T_TIMER                 imgTimerShowFast;
	T_BOOL                  IMG_ROTATE_90_FLAG;
	T_IMG_WIN_PHOTO         ctrl_photo;
	T_U8                    Deep;
	T_BOOL                  IMG_ZOOM_FLAG;
	T_BOOL                  IMG_MOVE_FLAG;
	T_BOOL                  IMG_RETURN_FLAG;
	T_BOOL					bSupportPic;
	T_U32					srcImgW;
	T_U32					srcImgH;
	T_BOOL					bGifStepFlag;
	T_U8					FailReason;
} T_IMGBROWSE;

T_BOOL ImgBrowse_Init(T_IMGBROWSE * pImgBrowse);
T_VOID ImgBrowse_Free(T_IMGBROWSE * pImgBrowse);
T_BOOL ImgBrowse_Show(T_IMGBROWSE * pImgBrowse);
T_IMG_RET ImgBrowse_Handle(T_IMGBROWSE * pImgBrowse, T_EVT_CODE event, T_EVT_PARAM *pEventParm);
T_BOOL ImgBrowse_Open(T_IMGBROWSE * pImgBrowse, T_DISPLAYLIST *pDisplayList);
T_BOOL ImgBrowse_Change(T_IMGBROWSE * pImgBrowse);
T_VOID ImgBrowse_SetRefresh(T_IMGBROWSE * pImgBrowse, T_U16 refreshFlag);
T_U16 ImgBrowse_GetRefresh(T_IMGBROWSE * pImgBrowse);
//T_U8 *ImgBrowse_GetImgBufPtr(T_IMGBROWSE * pImgBrowse);
T_VOID ImgBrowse_StartSlideShow(T_IMGBROWSE * pImgBrowse);
T_VOID ImgBrowse_StopSlideShow(T_IMGBROWSE * pImgBrowse);
T_U32 ImgBrowse_GetImgZoom(T_IMGBROWSE * pImgBrowse);
//T_BOOL ImgBrowse_ImgOpen(T_IMGBROWSE * pImgBrowse);
//T_VOID ImgBrowse_ImgClose(T_IMGBROWSE * pImgBrowse);
//T_BOOL ImgBrowse_ImgStep(T_IMGBROWSE * pImgBrowse);
T_pDATA ImgBrowse_GetOutBuf(T_IMGBROWSE * pImgBrowse);
T_BOOL ImgBrowse_ShowImg(T_IMGBROWSE * pImgBrowse);
T_BOOL ImgBrowse_IsBrowseSupportFileType(T_USTR_FILE pFileName);
T_BOOL ImgBrowse_IsEmapSupportFileType(T_USTR_FILE pFileName);
T_BOOL ImgBrowse_SetDisMode(T_IMGBROWSE * pImgBrowse, T_IMG_DIS_MODE DisMode);
T_IMG_DIS_MODE ImgBrowse_GetDisMode(T_IMGBROWSE * pImgBrowse);
T_U32  ImgBrowse_GetZoom(T_IMGBROWSE * pImgBrowse);
T_U32  ImgBrowse_GetScale(T_IMGBROWSE * pImgBrowse);
T_U32  ImgBrowse_GetOffsetX(T_IMGBROWSE * pImgBrowse);
T_U32  ImgBrowse_GetOffsetY(T_IMGBROWSE * pImgBrowse);
T_BOOL ImgBrowse_ChangeZoom(T_IMGBROWSE * pImgBrowse, T_U16 zoomAct, T_U32 changevalue);
T_BOOL ImgBrowse_SetZoomChangValue(T_IMGBROWSE * pImgBrowse, T_U32 changeValue);
T_U32 ImgBrowse_GetZoomChangValue(T_IMGBROWSE * pImgBrowse);
T_BOOL ImgBrowse_SetOffsetChangValue(T_IMGBROWSE * pImgBrowse, T_U32 changeValue);
T_U32 ImgBrowse_GetOffsetChangValue(T_IMGBROWSE * pImgBrowse, T_U32 changeValue);
T_U32 ImgBrowse_GetOutImgW(T_IMGBROWSE * pImgBrowse);
T_U32 ImgBrowse_GetOutImgH(T_IMGBROWSE * pImgBrowse);
T_U32 ImgBrowse_GetInImgW(T_IMGBROWSE * pImgBrowse);
T_U32 ImgBrowse_GetInImgH(T_IMGBROWSE * pImgBrowse);
T_VOID ImgBrowse_StartImgShowFastTimer(T_IMGBROWSE * pImgBrowse);
T_VOID ImgBrowse_StopImgShowFastTimer(T_IMGBROWSE * pImgBrowse);



#endif

