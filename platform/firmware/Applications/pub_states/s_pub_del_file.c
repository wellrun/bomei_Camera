
#include "Fwl_public.h"
#include "Fwl_Initialize.h"
#include "Fwl_osFS.h"
#include "Ctl_Msgbox.h"
#include "Eng_KeyMapping.h"
#include "Eng_ScreenSave.h"
#include "Eng_DataConvert.h"
#include "fwl_oscom.h"
#include "fwl_osfs.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_USTR_FILE     CurPath;
    T_FILE_INFO     *FileInfo;
    T_MSGBOX        msgbox;
    T_BOOL          flag;
    T_U8            result;
    T_TIMER         timer_id;
    T_U8            dealy_timer;
    T_BOOL          cancle;
    T_BOOL          screen_status;
} T_DEL_FILE_PARM;

static T_DEL_FILE_PARM *pDel_File_Parm;

/*---------------------- BEGIN OF STATE s_pub_del_file ------------------------*/
void initpub_del_file(void)
{
    pDel_File_Parm = (T_DEL_FILE_PARM *)Fwl_Malloc(sizeof(T_DEL_FILE_PARM));
    AK_ASSERT_PTR_VOID(pDel_File_Parm, "initpub_del_file(): malloc error");

    pDel_File_Parm->flag = AK_FALSE;
    pDel_File_Parm->result = 0;
    pDel_File_Parm->timer_id = ERROR_TIMER;
    pDel_File_Parm->dealy_timer = 0;
    pDel_File_Parm->cancle = AK_FALSE;

    pDel_File_Parm->screen_status = ScreenSaverIsOn();
    if (pDel_File_Parm->screen_status)
        ScreenSaverDisable();
}

void exitpub_del_file(void)
{
    if (pDel_File_Parm->screen_status)
        ScreenSaverEnable();

    if (pDel_File_Parm->timer_id != ERROR_TIMER)
    {
        Fwl_StopTimer(pDel_File_Parm->timer_id);
        pDel_File_Parm->timer_id = ERROR_TIMER;
    }
    pDel_File_Parm = Fwl_Free(pDel_File_Parm);
}

void paintpub_del_file(void)
{
    MsgBox_Show(&pDel_File_Parm->msgbox);
    Fwl_RefreshDisplay();

    if (pDel_File_Parm->flag)
    {
        T_USTR_FILE file_path;

        Utl_UStrCpy(file_path, pDel_File_Parm->CurPath);

        pDel_File_Parm->result = 0;
        if (Utl_UStrLen(file_path) > 0)
        {
            if (Fwl_FsIsDir(file_path))
            {
                if (Fwl_DirIsDefPath(file_path) == AK_FALSE)
                {
                    if (Fwl_FsRmDir(file_path) == AK_TRUE)
                        pDel_File_Parm->result = 1;
                    else
                        pDel_File_Parm->result = 10;        //re_dir error
                }
            }
            else
            {
                if(Fwl_FileDelete(file_path)  == AK_TRUE)
                    pDel_File_Parm->result = 1;
            }
        }

        pDel_File_Parm->timer_id = Fwl_SetTimerMilliSecond(10, AK_FALSE);
        pDel_File_Parm->flag = AK_FALSE;
    }
}

unsigned char handlepub_del_file(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_MMI_KEYPAD phyKey;
    T_USTR_INFO utmpstr;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pDel_File_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    switch (event)
    {
    case M_EVT_NEXT:
        Utl_UStrCpy(pDel_File_Parm->CurPath, (T_U16 *)pEventParm->p.pParam1);
        pDel_File_Parm->FileInfo = (T_FILE_INFO *)pEventParm->p.pParam2;
        Utl_UStrCat(pDel_File_Parm->CurPath, pDel_File_Parm->FileInfo->name);

        Utl_UStrCpy(utmpstr, GetCustomString(csFILE_DELETING));
        Utl_UStrCat(utmpstr, _T(" "));
        Utl_UStrCat(utmpstr, pDel_File_Parm->CurPath);
        MsgBox_InitStr(&pDel_File_Parm->msgbox, 0, GetCustomTitle(ctHINT), utmpstr, MSGBOX_INFORMATION);
        pDel_File_Parm->flag = AK_TRUE;
        break;

    case M_EVT_1:
        Utl_UStrCpy(pDel_File_Parm->CurPath, (T_U16 *)pEventParm->p.pParam1);

        Utl_UStrCpy(utmpstr, GetCustomString(csFILE_DELETING));
        Utl_UStrCat(utmpstr, _T(" "));
        Utl_UStrCat(utmpstr, pDel_File_Parm->CurPath);
        MsgBox_InitStr(&pDel_File_Parm->msgbox, 0, GetCustomTitle(ctHINT), utmpstr, MSGBOX_INFORMATION);
        pDel_File_Parm->flag = AK_TRUE;
        break;

    case M_EVT_USER_KEY:
        phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_BOOL)pEventParm->c.Param2;
        switch (phyKey.keyID)
        {
        case kbCLEAR:
            pDel_File_Parm->cancle = AK_TRUE;
            break;
        default:
            break;
        }
        break;
    case M_EVT_PUB_TIMER:
        pDel_File_Parm->dealy_timer++;
        if (pDel_File_Parm->dealy_timer<2)
            break;
        pEventParm->w.Param1 = pDel_File_Parm->timer_id;
    case VME_EVT_TIMER:
        if (pEventParm->w.Param1 == (T_U32)pDel_File_Parm->timer_id)
        {
            Fwl_StopTimer(pDel_File_Parm->timer_id);
            pDel_File_Parm->timer_id = ERROR_TIMER;

            if (pDel_File_Parm->cancle && (pDel_File_Parm->result == 1))
                pDel_File_Parm->result = 2;

            pEventParm->c.Param1 = pDel_File_Parm->result;
            m_triggerEvent(M_EVT_DEL_EXIT, pEventParm);
        }
        break;
    default:
        break;
    }

    return 0;
}

/* end of files */
