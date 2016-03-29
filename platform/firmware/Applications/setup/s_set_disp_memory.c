
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Eng_DynamicFont.h"
#include "Eng_DataConvert.h"
#include "Ctl_Msgbox.h"
#include "Fwl_osFS.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

#define     DISPMEMORY_DISK_NUM 27
#define     M_SIZE              (1<<20)
#define     K_SIZE              (1<<10)

typedef struct {
    T_MSGBOX        msgbox;
} T_MEMORY_PARM;

static T_MEMORY_PARM *pMemoryParm;

#ifndef SPIBOOT
#define FIRST_USERDISK_ID 3
#else
#define FIRST_USERDISK_ID 0
#endif
#endif
/*---------------------- BEGIN OF STATE s_set_disp_memory ------------------------*/
void initset_disp_memory(void)
{
#ifdef SUPPORT_SYS_SET

    T_STR_INFO      tmpstrtotal;
    T_STR_INFO      tmpstrcurrent;
    T_STR_INFO      tmpstr;
    T_USTR_INFO     utmpstr;
    T_U16           i;
    T_S8            diskId[DISPMEMORY_DISK_NUM];
    T_U8			diskType[DISPMEMORY_DISK_NUM];
    T_U64_INT       diskSize[DISPMEMORY_DISK_NUM];
    T_U64_INT       diskUsedSize[DISPMEMORY_DISK_NUM];
    T_U16           mountDiskNum = 0;

    pMemoryParm = (T_MEMORY_PARM *)Fwl_Malloc(sizeof(T_MEMORY_PARM));
    AK_ASSERT_PTR_VOID(pMemoryParm, "initset_disp_memory(): malloc error");

    mountDiskNum = Fwl_GetDiskList(diskId, diskType, DISPMEMORY_DISK_NUM);

	if (0 < mountDiskNum)
	{
		for(i=FIRST_USERDISK_ID; i<mountDiskNum; i++)
	    {
	        Fwl_FsGetSize(diskId[i], &diskSize[i]);
	        Fwl_FsGetUsedSize(diskId[i], &diskUsedSize[i]);

	 		Fwl_Print(C3, M_EXPLORER, "SD3700001643 debug -- high:%lu, low:%u\r\n", diskUsedSize[i].high, diskUsedSize[i].low);
	    }

	    utmpstr[0] = 0;
	    MsgBox_InitStr(&pMemoryParm->msgbox, 0, GetCustomTitle(ctHINT), utmpstr, MSGBOX_INFORMATION | MSGBOX_OK);
	    Eng_StrUcs2Mbcs(GetCustomString(csMEMORY_TOTAL), tmpstrtotal);
	    Eng_StrUcs2Mbcs(GetCustomString(csMEMORY_CURRENT), tmpstrcurrent);

	    for (i=FIRST_USERDISK_ID; i<mountDiskNum; i++)
	    {
	        T_U32   m_totel;

	        m_totel = diskSize[i].high * 4096 + diskSize[i].low/M_SIZE;
	   		Fwl_Print(C3, M_EXPLORER, "SD3700001643 debug -- m_totel:%lu\n\n", m_totel);
	   		
	        if ((diskSize[i].low%M_SIZE/K_SIZE) < 10)
	            sprintf(tmpstr, "(%c:)%s%lu.00%luMB ", diskId[i], tmpstrtotal, m_totel, ((diskSize[i].low%M_SIZE/K_SIZE)*1000/K_SIZE));
	        if ((diskSize[i].low%M_SIZE/K_SIZE) < 100)
	            sprintf(tmpstr, "(%c:)%s%lu.0%luMB ",  diskId[i], tmpstrtotal, m_totel, ((diskSize[i].low%M_SIZE/K_SIZE)*1000/K_SIZE));
	        else
	            sprintf(tmpstr, "(%c:)%s%lu.%luMB ",   diskId[i], tmpstrtotal, m_totel, ((diskSize[i].low%M_SIZE/K_SIZE)*1000/K_SIZE));

	        Eng_StrMbcs2Ucs(tmpstr, utmpstr);
	        MsgBox_AddLine(&pMemoryParm->msgbox, utmpstr);

	        m_totel = diskUsedSize[i].high * 4096 + diskUsedSize[i].low/M_SIZE;
	        if ((diskUsedSize[i].low%M_SIZE/K_SIZE) < 10)
	            sprintf(tmpstr, "(%c:)%s%lu.00%luMB ",diskId[i], tmpstrcurrent, m_totel, ((diskUsedSize[i].low%M_SIZE/K_SIZE)*1000/K_SIZE));
	        if ((diskUsedSize[i].low%M_SIZE/K_SIZE) < 100)
	            sprintf(tmpstr, "(%c:)%s%lu.0%luMB ", diskId[i], tmpstrcurrent, m_totel, ((diskUsedSize[i].low%M_SIZE/K_SIZE)*1000/K_SIZE));
	        else
	            sprintf(tmpstr, "(%c:)%s%lu.%luMB ",  diskId[i], tmpstrcurrent, m_totel, ((diskUsedSize[i].low%M_SIZE/K_SIZE)*1000/K_SIZE));

	        Eng_StrMbcs2Ucs(tmpstr, utmpstr);
	        MsgBox_AddLine(&pMemoryParm->msgbox, utmpstr);
	    }
	}
	else
	{
		Fwl_Print(C2,M_EXPLORER,"WARNING:LHS:Not Fine User Driver\n");		
		MsgBox_InitAfx(&pMemoryParm->msgbox, 0, ctSUCCESS, csSETUP_NO_DRIVER, MSGBOX_INFORMATION | MSGBOX_OK);
	}
#endif
}

void exitset_disp_memory(void)
{
#ifdef SUPPORT_SYS_SET
    pMemoryParm = Fwl_Free(pMemoryParm);
#endif
}

void paintset_disp_memory(void)
{
#ifdef SUPPORT_SYS_SET
    MsgBox_Show(&pMemoryParm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_disp_memory(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE msgRet;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pMemoryParm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    msgRet = MsgBox_Handler(&pMemoryParm->msgbox, event, pEventParm);
    switch(msgRet)
    {
    case eNext:
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
    default:
        ReturnDefauleProc(msgRet, pEventParm);
        break;
    }
#endif
    return 0;
}
