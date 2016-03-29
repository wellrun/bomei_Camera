/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: lib_freetype.c
* Function: memory and file system interface for freetype lib 
*
* Author: songmengxing
* Date: 2012-04-01
* Version: 1.0
*
* Revision: 
* Author: 
* Date: 
***************************************************************************/

#include "fwl_osmalloc.h"
#include "fwl_osfs.h"
#include "gbl_macrodef.h"
#include "eng_string.h"
#include "eng_dataconvert.h"


#if (defined (SUPPORT_VFONT) || defined (SUPPORT_FLASH))

int VF_fclose(int *FileHandle)
{
	//Fwl_Print(C3, M_OFFICE, "VF_fclose fd is %d", FileHandle);
	return Fwl_FileClose((T_hFILE)(FileHandle));
}

// 返回文件句柄:T_hFILE
int *VF_fopen(const char *FileName, const char *Flag)
{
	T_hFILE fd;
    T_WSTR_100  unicodeFileName;
	
	//Fwl_Print(C3, M_OFFICE, "VF_fopen:%s", FileName);

	Utl_MemSet(unicodeFileName, 0, sizeof(unicodeFileName));
	
	Eng_MbsToWcs(
		(T_S8*)FileName, 
		Utl_StrLen(FileName), 
		unicodeFileName,
		sizeof(unicodeFileName)/sizeof(T_TCHR));
	
	fd = Fwl_FileOpen(unicodeFileName, _FMODE_READ, _FMODE_READ);

	if(fd == FS_INVALID_HANDLE)
	{
		return AK_NULL;
	}
	else
	{
		return (int *)fd;
	}
}

// 返回读取的字节数
int VF_fread(void *DataPtr,int Length, int Num, int *FileHandle)
{
	int iReadLen = 0;


	if(Length == 0 || Num == 0)
	{
		return 0;
	}
	
	iReadLen = (int)Fwl_FileRead((T_hFILE)FileHandle, DataPtr, Num*Length);
	//Fwl_Print(C3, M_OFFICE, "VF_fread-fd:%d,Num:%d,Len:%d,ReadLen:%d", FileHandle, Num, Length, iReadLen);
	if(iReadLen >= 0)
	{
		return iReadLen;
	}
	else
	{
		return 0;
	}
}

// 返回错误代码:-1/0
int VF_fseek(int *FileHandle, int Offset, int Origin)
{
	T_S32 ret;
	
	ret = Fwl_FileSeek((T_hFILE)FileHandle, (T_S32)Offset, (T_U16)Origin);
	//Fwl_Print(C3, M_OFFICE, "VF_fseek-fd:%d,Offset:%d,Origin:%d,ret:%d", FileHandle, Offset, Origin, ret);
	
	if(ret < 0)
	{
		return ret;
	}
	else
	{
		return 0;
	}
}

// 返回文件当前的位置
int VF_ftell(int *FileHandle)
{
	T_S32 ret;
	ret = Fwl_FileSeek((T_hFILE)FileHandle, 0, _FSEEK_CUR);
	//Fwl_Print(C3, M_OFFICE, "VF_ftell-fd:%d,ret:%d", FileHandle, ret);
	return ret;
}

/**********************************************************************/
/*                                                                    */
/*                        memory allocation                           */
/*                                                                    */
/**********************************************************************/

void VF_free(void *MemBlock)
{
	//Fwl_Print(C3, M_OFFICE, "VF_free:%x", MemBlock);	
	Fwl_Free(MemBlock);
}

void *VF_malloc(int Size)
{
	T_VOID * pMem= AK_NULL;
	pMem = Fwl_Malloc(Size);
	//Fwl_Print(C3, M_OFFICE, "VF_calloc-size:%d,add:%x", Size, pMem);
	return pMem;
}

void *VF_realloc(void *MemBlock, int Size)
{
	T_VOID * pMem = AK_NULL;
	pMem = Fwl_ReMalloc(MemBlock, Size);
	//Fwl_Print(C3, M_OFFICE, "VF_realloc-size:%d,add:%x,Old:%x", Size, pMem, MemBlock);
	return pMem;
}
#endif



/* EOF */
