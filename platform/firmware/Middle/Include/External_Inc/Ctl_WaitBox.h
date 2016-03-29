
#ifndef __CTL_WAITBOX_H__
#define __CTL_WAITBOX_H__

typedef enum{
    WAITBOX_CLOCK = 0,
    WAITBOX_PROGRESS,
    WAITBOX_RAINBOW,
    WAITBOX_ALL
}T_eWAITBOX_MODE;

typedef enum {
    PUBLIC_EVT_WAITBOX_SHOW = 0,
    PUBLIC_EVT_FREQMGR
}T_PUBLIC_EVT;

T_VOID WaitBox_Init(T_VOID);
T_VOID WaitBox_Free(T_VOID);
T_VOID WaitBox_Start(T_eWAITBOX_MODE mode, T_pWSTR title);
T_BOOL WaitBox_Stop(T_VOID);
T_VOID WaitBox_Progress_SetPercent(T_U32 percent);
T_VOID WaitBox_Show(T_eWAITBOX_MODE mode);

#endif

