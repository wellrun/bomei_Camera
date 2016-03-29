/*****************************************************************************
 * Copyright (C) 2003 Anyka Co. Ltd
 *
 * Transmittal, reproduction and/or dissemination of this document as well
 * as utilization of its contents and communication thereof to others without
 * express authorization are prohibited. Offenders will be held liable for
 * payment of damages. All rights created by patent grant or registration of
 * a utility model or design patent are reserved.
 *****************************************************************************
 *    VME Version: 00.00$
 *****************************************************************************
 * $Workfile: sound.c $
 *     $Date: 2003/10/27 11:57:52 $
 *****************************************************************************
 * Requirements:
 * Target:  Microsoft Visual C++ 6.0
 *****************************************************************************
 * Description:
 *
 *****************************************************************************
*/
#ifdef   OS_WIN32
#include "vme_interface.h"
#include "sound.h"
//#include "..\win\SoundWin.h"
#include "eng_callback.h"

/*****************************************************************************
 * defines
 *****************************************************************************
*/
#define DSP_STD_FREQUENCY     5000
#define DSP_STD_DURATION         1
#define DSP_STD_PAUSE            0
#define DSP_STD_REPEATS          1

#define DSP_BEEP_FREQUENCY     500
#define DSP_BEEP_DURATION       50
#define DSP_BEEP_PAUSE           0
#define DSP_BEEP_REPEATS         1

#define DSP_BATT_FREQUENCY     860
#define DSP_BATT_DURATION       50
#define DSP_BATT_PAUSE          50
#define DSP_BATT_REPEATS        30

/*****************************************************************************
 * globals
 *****************************************************************************
*/
vBOOL             fChipEnabled    = vFALSE;
//volatile T_U8 playing_midi_status = 0; // the current status of sound
static T_BOOL          IsPlayingMp3     = AK_FALSE;
/*****************************************************************************
 * sound
 *****************************************************************************
*/
T_BOOL  VME_SoundOpen ()
{
  //OpenSound();

  fChipEnabled    = vTRUE;

  return fChipEnabled;
}

T_VOID VME_SoundClose(T_U16 delay)
{
  fChipEnabled  = vFALSE;

  //CloseSound();

  return;
}

T_U32 VME_SoundGetRecDuration()
{
	return 0;
}

T_VOID VME_SoundEnableDA()
{

}

T_VOID VME_SoundDiableDA()
{

}

T_VOID  VME_SoundFree(vVOID)
{

}

T_VOID  VME_SoundStop (vVOID)
{
}

T_VOID  VME_SoundStopEx(T_VOID)
{
}

T_S8  VME_SoundStart (vT_SoundData *SoundData)
{
	return -1;
}

T_S8  VME_SerialSoundStart(T_AudioList *list, T_U16 loop, T_U16 volume, T_U8 priority )
{
	return -1;
}

T_VOID  VME_SoundSetVolume(T_U16 Volume)
{
}


T_VOID VME_SoundSetChannel(T_U8 channel_id)
{

}

T_U8 VME_SoundFileType( T_pCWSTR filePath )
{
    return 0;
}

T_U8 VME_SoundType(T_pCDATA buff, T_U32 bufLen)
{
	T_U8 SoundType=0;

	return SoundType;
}

T_S8 VME_SoundStreamPlay(T_pCWSTR file,T_U16 loops, T_U8 priority)
{
	return -1;
}

T_VOID VME_SoundHandleStream(T_BOOL passive)
{

}

T_VOID VME_SoundHandleRecData()
{

}

T_U32 VME_SoundGetTotalSec(T_pCWSTR file)
{
	return 0;
}

T_U32 VME_SoundGetDuration()
{
	return 0;
}

T_U16 VME_SoundResume()
{
	return 0;
}

T_U16 VME_SoundPause()
{
	return 0;
}

T_U16 VME_SoundSeek(T_U32 sec)
{
	return 0;
}

T_SND_STAT VME_SoundGetStatus(T_U8 *priority)
{
	return 0;
}

/*
 * set sound status
 * if there is a higher priority sound playing, set status will failed
 */
T_BOOL VME_SoundSetStatus(T_SND_STAT status, T_U8 priority)
{
	return AK_TRUE;
}

T_VOID SetMP3Playing(T_BOOL flag)
{
    IsPlayingMp3 = flag;
}

T_BOOL GetMP3Playing(T_VOID)
{
    return IsPlayingMp3;
}    


T_U16  VME_SoundGetVolume()
{
	 return 0;
}

//T_BOOL VME_SoundGetMetaDataInfoFromFile(T_pCSTR fileName, T_U8 MediaType, _SD_T_META_INFO* pMetadataInf)
T_U8 VME_SoundGetMetaDataInfoFromFile(T_pCWSTR fileName,  T_U8 *pMediaType, T_U32 *pTotalTime, _SD_T_META_INFO* pMetadataInf) 
{
	return AK_FALSE;
}

T_U8 VME_SoundGetMetaDataInfo(T_pCWSTR fileName,  T_U8 *pMediaType, T_U32 *pTotalTime, _SD_T_META_INFO* pMetadataInf) 
{
	return AK_FALSE;
}

extern T_VOID sdres_load_cb(_SD_T_LOADRESOURCE_CB_PARA *cbparm)
{

}

extern T_VOID sdres_free_cb(T_U8 *buff)
{

}

T_VOID VME_SoundForceEnableDA()
{
	return;
}

T_VOID VME_Sound_SetMode( T_ENUM_AUDIO_MODE mode )
{
}

T_VOID VME_Sound_SetFreq( T_U32 cur_freq  )
{ 
	
}

T_VOID  VME_SoundCancelClose()
{
}
#endif