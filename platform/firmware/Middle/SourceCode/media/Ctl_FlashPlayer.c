#include "Anyka_types.h"
#include "arch_lcd.h"
#include "Eng_debug.h"
#include "Ctl_FlashPlayer.h"
#include "Lib_sdcodec.h"
#include "fwl_osmalloc.h"
#include "fwl_oscom.h"
#include "fwl_wave.h"
#include "fwl_waveout.h"
#include "fwl_pfaudio.h"
#include "sha204.h"
#include "gpio_config.h"
#include "drv_api.h"
#include "AF_Player.h"


#ifdef SUPPORT_FLASH
#define FLASH_AUDIO_INBUF_MINLEN			128

T_pVOID Fwl_Malloc_flash(T_S32 size)
{
	T_pVOID *ptr = Fwl_Malloc(size);
	
	AK_ASSERT_PTR(ptr, "Flash Malloc error", AK_NULL);
	return ptr;
}

T_pVOID Fwl_Free_flash(T_pVOID ptr)
{
	AK_ASSERT_PTR(ptr, "Flash free error ptr", AK_NULL);
	return Fwl_Free(ptr);
}

T_pVOID Fwl_ReMalloc_flash(T_pVOID ptr, T_S32 size)
{
	AK_ASSERT_PTR(ptr, "Flash ReMalloc error ptr", AK_NULL);
	return Fwl_ReMalloc(ptr, size);
}

T_pFILE	Fwl_FileOpen_flash(T_pCWSTR path, AF_FILE_OPEN_TYPE flags)
{
	T_hFILE	hFile;
	T_FILE_FLAG fwl_flag = FS_MODE_READ;
	T_FILE_MODE fwl_mode = FS_MODE_READ;
	
	if (flags & AF_WRONLY) 
	{
		Fwl_Print(C3, M_FLASH, "FS_MODE_WRITE\n");
		fwl_flag = FS_MODE_WRITE;
		fwl_mode = FS_MODE_WRITE;
	} 
	else if(flags & AF_APPEND) 
	{
		Fwl_Print(C3, M_FLASH, "DF_APPEND\n");
		fwl_flag = FS_MODE_APPEND;
		fwl_mode = FS_MODE_APPEND;
	}
	else
	{
		//PLAYERASSERT(mode == kFlashFileRead);
		Fwl_Print(C3, M_FLASH, "FS_MODE_READ\n");
		fwl_flag = FS_MODE_READ;
		fwl_mode = FS_MODE_READ;
	}

	hFile = Fwl_FileOpenAsc(path, fwl_flag, fwl_mode);//tianj03

	if (_FOPEN_FAIL == hFile)
	{
	    Fwl_Print(C3, M_FLASH, "open file failed!\n");
		return	_FOPEN_FAIL;
	}
	return	hFile;
}

T_BOOL	Fwl_FileClose_flash(T_pFILE hFile)
{
	Fwl_Print(C3, M_FLASH, "close file:%x\n", hFile);
	return	Fwl_FileClose(hFile);
}

T_S32	Fwl_FileRead_flash(T_pFILE hFile, T_pVOID buffer, T_U32 count)
{
	T_S32 ret;
    ret = Fwl_FileRead(hFile, buffer, count);
	return ret;
}

T_S32	Fwl_FileSeek_flash(T_hFILE hFile, T_S32 offset, T_U16 origin)
{
	T_S32 ret;
    ret = Fwl_FileSeek(hFile, offset, origin);
	return ret;
}

T_S32 Fwl_FileWrite_flash(T_hFILE hFile, T_pVOID  *buf, T_U32 count)
{
	T_S32 ret;
	ret = Fwl_FileWrite(hFile, buf, count);
	return ret;
}

#if 1
T_S32 Fwl_FileTell_flash(T_hFILE hFile)
{
    T_U32 file_size;

    file_size = Fwl_FileTell(hFile);
    if (0 == file_size)
   	{
   		Fwl_Print(C3, M_FLASH, "Fwl_FileTell error\n");
       return -1;
   	}
	
	return file_size;
}
#endif

T_S32 Fwl_FileStat_flash(T_S8 * file_name, T_S16 *st_dev,T_U16 *st_ino, T_U16 *st_mode,T_S16 *st_nlink, T_U16 *st_uid, T_U16 *st_gid,T_S16 *st_rdev, T_S32 *st_size, long int *st_atime_1, long int *st_mtime_1, long int *st_ctime_1)
{
	T_U32 file_size;
	Fwl_Print(C3, M_FLASH, "state file:%s\n", file_name);

	file_size = Fwl_FileGetSizeAsc(file_name);
	if (0 == file_size)
	{
		Fwl_Print(C3, M_FLASH, "state file error\n");
		return -1;
	}

	if (st_size)
           *st_size = file_size;
	
	return 0;
}

T_S32 Fwl_SoundOpen_flash(T_S32 freq,T_S32 channel,T_S32 bits)
{
    T_PCM_FORMAT fmt;
	//GIOChannel *channel;
	Fwl_AudioEnableDA();

    fmt.channel = 2;
    fmt.sampleBits = 16;
    fmt.sampleRate = 44100;
    Fwl_Print(C3, M_FLASH, "open sound = %d\n", WaveOut_Open(20, &fmt, AK_NULL));
    Fwl_Print(C3, M_FLASH, "freq=%d, ch=%d, bits=%d\n", freq, channel, bits);
    
    Fwl_AudioSetVolume(Fwl_GetAudioVolume());
		
    return 1;
}

T_S32 Fwl_SoundWrite_flash(T_pVOID buf, T_S32 Length)
{
	T_S32 len = Length;
	T_S32 free_space;
	T_pDATA pcm_buf;
	
	pcm_buf = (T_pDATA)buf;
	//Fwl_Print(C3, M_FLASH, "F-0x%x, %d, T: %d\n",buf, Length, get_tick_count());

	/* Get some sample to judge buf whether is null , if is null not to write da*/
	if (!pcm_buf[0]
		&& !pcm_buf[200]
		&& !pcm_buf[300]
		&& !pcm_buf[Length - 100] 
		&& !pcm_buf[Length - 1])
	{
		Fwl_Print(C4, M_FLASH, "pcm null ...\n");
		return Length;
	}

	/* judge da free space whether enough */
	WaveOut_GetStatus(&free_space, WAVEOUT_SPACE_SIZE);
	if(free_space < Length)
		return 0;
			
	len -= WaveOut_Write(pcm_buf + Length - len, len, INVALID_TIME_STAMP);
	if (len > 0)
		Fwl_Print(C3, M_FLASH, "Waiting ...\n");

	return Length;

}

T_VOID Fwl_SoundClose_flash(T_VOID)
{
	WaveOut_Close();
	Fwl_AudioDisableDA();
}

T_pVOID mp3codec_handle = 0;
T_S32  mp3codec_open(T_S32 freq, T_S32 channel, T_S32 bits)
{
    T_AUDIO_DECODE_INPUT audio_input; // open的参数
    T_AUDIO_DECODE_OUT audio_output; 

	if (mp3codec_handle)
		return -1;

	memset(&audio_input, 0, sizeof(T_AUDIO_DECODE_INPUT));
	memset(&audio_output, 0, sizeof(T_AUDIO_DECODE_OUT));
	audio_input.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)Fwl_Malloc_flash;
    audio_input.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)Fwl_Free_flash;
    audio_input.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)AkDebugOutput;
    audio_input.cb_fun.delay = AK_NULL;
	// 设置码流属性，如果确定是MP3这些属性不传也可以，
	// 但是 m_Type = _SD_MEDIA_TYPE_MP3; 这个是一定要的。
    audio_input.m_info.m_BitsPerSample = bits;
    audio_input.m_info.m_Channels = channel;
    audio_input.m_info.m_SampleRate = freq;
    audio_input.m_info.m_Type = _SD_MEDIA_TYPE_MP3;

	mp3codec_handle = _SD_Decode_Open(&audio_input,& audio_output);
	if (!mp3codec_handle)
    {
    	Fwl_Print(C3, M_FLASH, "error:sound decode open error\n");
        return -1;
    }
	
	_SD_SetBufferMode(mp3codec_handle, _SD_BM_NORMAL);
	_SD_SetInbufMinLen(mp3codec_handle, FLASH_AUDIO_INBUF_MINLEN);
	Fwl_Print(C3, M_FLASH, "Open sound decoder\n");
	return 0;
}

T_VOID mp3codec_close()
{
	if (mp3codec_handle)
	{
		Fwl_Print(C3, M_FLASH, "ready to close sound decoder\n");
		_SD_Decode_Close(mp3codec_handle);
		mp3codec_handle = 0;
	}
}

T_S32 mp3codec_fill(T_U8 *src_data, T_S32 size)
{
	T_U32 retval;
    T_AUDIO_BUFFER_CONTROL buffer_control;

    retval = _SD_Buffer_Check(mp3codec_handle, &buffer_control);
	if (_SD_BUFFER_WRITABLE == retval)
    {
    	//Fwl_Print(C3, M_FLASH, "enter _SD_BUFFER_WRITABLE\n");
        if(buffer_control.free_len < size)
            size = buffer_control.free_len;
		
        memcpy(buffer_control.pwrite, src_data, size);
        _SD_Buffer_Update(mp3codec_handle, size);
        return size;
    }
    else if(_SD_BUFFER_WRITABLE_TWICE == retval)
    {
    	//Fwl_Print(C3, M_FLASH, "enter _SD_BUFFER_WRITABLE_TWICE\n");
        if(buffer_control.free_len < size)
			retval = buffer_control.free_len;
		else
			retval = size;
		
		memcpy(buffer_control.pwrite, src_data, retval);
        _SD_Buffer_Update(mp3codec_handle, retval);
		if(retval >= size)
			return size;

        if (buffer_control.start_len > size -  retval)
            buffer_control.start_len = size -  retval;
		
		memcpy(buffer_control.pstart, src_data+retval, buffer_control.start_len);
        _SD_Buffer_Update(mp3codec_handle, buffer_control.start_len);
		retval += buffer_control.start_len;
        return retval;
    }
	else
	{
		Fwl_Print(C3, M_FLASH, "enter nothing, error happened\n");
	}
	return 0;
}

T_S32 mp3codec_decode(T_pVOID pcm, T_S32 size)
{
	T_AUDIO_DECODE_OUT audio_output;
	T_S32 ret;

	audio_output.m_pBuffer = pcm;
    audio_output.m_ulSize = size;

	ret = _SD_Decode(mp3codec_handle, &audio_output);
	if(ret <0)
	{
		Fwl_Print(C3, M_FLASH, "decoder have something wrong\n");
		return 0;
	}
	
	return ret;
}

T_S32 Flash_GetHash(T_U8 *data, T_U8 *haData,const T_U8 key_Id)
{
	T_U8 ret_code;
	T_U8 i;
	// Make the command buffer the size of the MAC command.
	static T_U8 command[MAC_COUNT_LONG];
	// Make the response buffer the size of a READ response.
	static T_U8 response[MAC_RSP_SIZE];

	// Init encrypt Ic device
	sha204_I2C_init(GPIO_I2C_SCL, GPIO_I2C_SDA);
	
	for (i = 0; i < sizeof(response); i++)
		response[i] = 0;

	// Mac command with mode = 0.
	ret_code = sha204_I2C_execute(SHA204_MAC, 0, key_Id
				, MAC_CHALLENGE_SIZE, data, 0, AK_NULL, 0, AK_NULL
				, WRITE_COUNT_LONG, &command[0], MAC_RSP_SIZE, &response[0]);

	if (ret_code != SHA204_SUCCESS)
	{
		AK_DEBUG_OUTPUT("mac fail:%x\r\n",ret_code);
		return 0;
	}

	for (i = 1; i < sizeof(response)-2; i++) 
	{
		haData[i-1] = response[i];
	}

	return 1;
}

T_VOID  Flash_Set_ChipID(T_VOID)
{
	if (CHIP_3771_L == drv_get_chip_version() )
	{
		Fwl_Print(C3, M_FLASH, "AF_Player_SetChipID(ANYKA_CHIP_37xx_L)");
		AF_Player_SetChipID(ANYKA_CHIP_37xx_L);
	}
	else
	{
		Fwl_Print(C3, M_FLASH, "AF_Player_SetChipID(ANYKA_CHIP_37xx)");
		AF_Player_SetChipID(ANYKA_CHIP_37xx);
	}
}

#endif
