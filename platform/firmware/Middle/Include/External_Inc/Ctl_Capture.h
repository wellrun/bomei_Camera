
#ifndef __CTL_CAPTURE_H__
#define __CTL_CAPTURE_H__

typedef struct{
    T_MSGBOX    msgbox;
    T_pCDATA    pStatusBar;
    T_U64_INT   free_size;      // free space in nand or sd
    T_U8        *mj;        //jpg picture data
    T_U32       len;        //file size per picture
    T_USTR_FILE curPath;
    T_USTR_FILE pFileName;
    T_U8        *pY;
    T_U8        *pU;
    T_U8        *pV;
    T_BOOL  bFlashLight;

    T_U32   window_width;
    T_U32   window_height;
    T_U32   width;
    T_U32   height;
    T_U32   photoTotal;
    T_U8    DCShotMode;        // shot, multi-shot
    T_U8    shotCnt;
    T_TIMER shotTimer;
}T_CAPTURE;

typedef enum{
    CAM_QLTY_HIGH,
    CAM_QLTY_MIDDLE,
    CAM_QLTY_LOW,
    CAM_QLTY_NUM
} T_CAM_PHOTO_QLTY;  // photo quality


T_VOID Capture_Free(T_CAPTURE *pCapture);
T_BOOL Capture_Init(T_CAPTURE *pCapture);
T_eBACK_STATE Capture_Handle(T_CAPTURE* pCapture, T_EVT_CODE Event, T_EVT_PARAM *pEventParm);
T_VOID Capture_SetParm(T_CAPTURE *pCapture, T_PREVIEW *pPreview);
T_BOOL Capture_MallocBuff(T_CAPTURE *pCapture);


#endif //#ifndef __CTL_CAPTURE_H__

