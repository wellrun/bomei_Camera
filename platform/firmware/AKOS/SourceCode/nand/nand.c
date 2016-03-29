/*
 * @(#)nand.c
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include    "mount_pub_api.h"
#include	"nandflash.h"
#include	"mtdlib.h"

#ifdef OS_ANYKA
#include	"fwl_nandflash.h"
#endif

#ifdef OS_WIN32
#include 	<windows.h>
#include	<Windowsx.h>
#include    "mount.h"
#else

#endif


#define NAND_SECTOR_BIT 9
#define NAND_BYTES_SECCTOR	(1 << NAND_SECTOR_BIT)



#ifdef OS_WIN32
extern T_U8 SD_DriverNo;

static T_PMEDIUM Nand_Win32Initial(T_VOID);

static T_U32 Nand_Win32Read(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size)
{
	T_U32 ret;
	HANDLE hDev;
	T_U32 dwCB;
	T_S32 high;
	T_BOOL bRet;
	char devName[10];	//"\\\\.\\g:";
	T_U8 *buf1;

	ret = 0;
	devName[0] = '\\';
	devName[1] = '\\';
	devName[2] = '.';
	devName[3] = '\\';
	devName[4] = SD_DriverNo + 1 + Nand_GetMediumSymbol(medium);  
	devName[5] = ':';
	devName[6] = 0;
	
	hDev = CreateFile(devName, GENERIC_READ , FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDev == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	high = sector >> (32 - NAND_SECTOR_BIT);
	SetFilePointer(hDev, sector << NAND_SECTOR_BIT, &high, FILE_BEGIN);

	buf1 = malloc(NAND_BYTES_SECCTOR * size);
	bRet = ReadFile(hDev, buf1, NAND_BYTES_SECCTOR * size, &dwCB, NULL);
	CloseHandle(hDev);
	if (bRet)
	{
		ret = size;;
	}
	memcpy(buf, buf1, NAND_BYTES_SECCTOR * size);
	free(buf1);
	return ret;
}

static T_U32 Nand_Win32Write(T_PMEDIUM medium, const T_U8* buf, T_U32 sector, T_U32 size)
{
	T_U32 ret;
	DWORD dwCB;
	T_S32 high;
	HANDLE hDev;
	char devName[10];	//"\\\\.\\g:";
	T_U8 *buf1;
	

	ret = 0;
	devName[0] = '\\';
	devName[1] = '\\';
	devName[2] = '.';
	devName[3] = '\\';
	devName[4] = SD_DriverNo + 1 + Nand_GetMediumSymbol(medium); 
	devName[5] = ':';
	devName[6] = 0;
	
	hDev = CreateFile(devName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDev == INVALID_HANDLE_VALUE) return 0;
	high = sector >> (32 - NAND_SECTOR_BIT);
	SetFilePointer(hDev, sector << NAND_SECTOR_BIT, &high, FILE_BEGIN);	

	buf1 = malloc(NAND_BYTES_SECCTOR * size);
	memcpy(buf1, buf, NAND_BYTES_SECCTOR * size);
	if (WriteFile(hDev, buf1, NAND_BYTES_SECCTOR * size, &dwCB, NULL))
	{
		ret = size;
	}
	CloseHandle(hDev);	
	free(buf1);
	return ret;
}

//flush file data to medium.
static T_BOOL Nand_Win32Flush(T_PMEDIUM medium)
{
	return AK_TRUE;
}

static T_PMEDIUM Nand_Win32Initial()
{
	T_U32 capacity, BytsPerSec, i;

	T_PMEDIUM medium;
	medium = (T_PMEDIUM)Ram_Malloc(sizeof(T_MEDIUM));
	if(medium == AK_NULL)
	{
		return AK_NULL;
	}
	// 256M = 512 * 1000 * BytsPerSec
	capacity = (512 * 1000);
	BytsPerSec = NAND_BYTES_SECCTOR;
	//nand_bytes_sector = BytsPerSec;
	i = 0;
	while (BytsPerSec > 1)
	{
		BytsPerSec >>= 1;
		i++;
	}
	medium->SecBit = (T_U8) i;
	((T_POBJECT)medium)->destroy = (F_DESTROY)Medium_Destroy;
	((T_POBJECT)medium)->type = TYPE_MEDIUM;
	medium->read = Nand_Win32Read;
	medium->write = Nand_Win32Write;
	medium->flush = Nand_Win32Flush;
	medium->capacity = capacity;
	medium->type = MEDIUM_NANDFLASH;
	medium->msg = AK_NULL;
	return medium;
}

#endif

//**************************************************************************//
