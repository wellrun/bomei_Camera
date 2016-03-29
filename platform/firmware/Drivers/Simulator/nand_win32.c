/**
 * @file nand_win32.c
 * @brief Simulate NAND in Win32
 *
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Micro-electronics Technology Co., Ltd.
 * @author 
 * @MODIFY  
 * @DATE    2010-6-7
 * @version 0.1.0
 * @
 */

#ifdef OS_WIN32
#include <windows.h>
#include <Windowsx.h>
#include "string.h"
#endif

#ifdef OS_WIN32
#include "nandflash.h"
#include "anyka_types.h"
#include "mtdlib.h"
#include "mount.h"
#include "gbl_global.h"
#include "mount_pub_api.h"
#include "driver.h"

extern T_GLOBAL_S gs;


static T_U8 ResDisk_No = 'W';
static T_U8 Res2Disk_No = 'Z';
static T_U8 NandDisk_No = 'X';


#ifdef OS_WIN32
static T_U32 InitReadDisk(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size, T_U8 driver)
{
	T_U32 ret = 0;
	HANDLE hDev;
    T_U32 dwCB;
    T_S32 high;
    T_BOOL bRet;
    char devName[10];   //"\\\\.\\g:";
    T_U8* buf1;

    devName[0] = '\\';
    devName[1] = '\\';
    devName[2] = '.';
    devName[3] = '\\';
    devName[4] = driver;
    devName[5] = ':';
    devName[6] = 0;

    buf1 = malloc(512 * size);

    hDev = CreateFile(devName, GENERIC_READ , FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDev == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    high = sector >> (32 - 9);
    SetFilePointer(hDev, sector << 9, &high, FILE_BEGIN);
    bRet = ReadFile(hDev, buf1, 512 * size, &dwCB, NULL);
    memcpy(buf, buf1, 512 * size);
    CloseHandle(hDev);
    if (bRet)
    {
        ret = size;;
    }
    free(buf1);
	return ret;
}

static T_U32 InitWriteDisk(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size, T_U8 driver)
{
	T_U32 ret = 0;
    DWORD dwCB;
    HANDLE hDev;
    T_S32 high;

    char devName[10];
    T_U8* buf1;

    devName[0] = '\\';
    devName[1] = '\\';
    devName[2] = '.';
    devName[3] = '\\';
    devName[4] = driver;
    devName[5] = ':';
    devName[6] = 0;

    buf1 = malloc(512 * size);
    memcpy(buf1, buf, 512 * size);
    hDev = CreateFile(devName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDev == INVALID_HANDLE_VALUE)
		return 0;
    high = sector >> (32 - 9);

    SetFilePointer(hDev, sector << 9, &high, FILE_BEGIN);
    if (WriteFile(hDev, buf1, 512 * size, &dwCB, NULL))
    {
        ret = size;
    }
    CloseHandle(hDev);
    free(buf1);
	
    return ret;
}
#endif

static T_U32 ResDisk_Read(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size)
{
	return InitReadDisk(medium, buf, sector, size, ResDisk_No);
}

static T_U32 ResDisk_Write(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size)
{
	return  InitWriteDisk(medium, buf, sector, size, ResDisk_No);
}

static T_BOOL ResDisk_Flush(T_PMEDIUM medium)
{
    return AK_TRUE;
}

static T_PMEDIUM ResDisk_Initial()
{
#ifdef WIN32
    T_PMEDIUM medium;

    T_U32 capacity, BytsPerSec; 
    T_U8 i;

    capacity = 200 * 1024;
    BytsPerSec = 512;
    medium = (T_PMEDIUM)Ram_Malloc(sizeof(T_MEDIUM));
    if (medium == AK_NULL)
    {
        return AK_NULL;
    }

    i = 0;
    while (BytsPerSec > 1)
    {
        BytsPerSec >>= 1;
        i++;
    }
    medium->SecBit = i;
	medium->PageBit = i;
	medium->SecPerPg = 0;
    ((T_POBJECT)medium)->destroy = (F_DESTROY)Medium_Destroy;
    ((T_POBJECT)medium)->type = TYPE_MEDIUM;
    medium->read = (F_ReadSector)ResDisk_Read;
    medium->write = (F_WriteSector)ResDisk_Write;
    medium->flush = ResDisk_Flush;
    medium->capacity = capacity;
    medium->type = MEDIUM_DISKETTE;
    medium->msg = AK_NULL;

#endif  //WIN32

    return medium;
}



static T_U32 Res2Disk_Read(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size)
{
	return InitReadDisk(medium, buf, sector, size, Res2Disk_No);
}

static T_U32 Res2Disk_Write(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size)
{
	return  InitWriteDisk(medium, buf, sector, size, Res2Disk_No);
}

static T_BOOL Res2Disk_Flush(T_PMEDIUM medium)
{
    return AK_TRUE;
}

static T_PMEDIUM Res2Disk_Initial()
{
#ifdef WIN32
    T_PMEDIUM medium;
    T_U32 capacity, BytsPerSec;
    T_U8 i;

    capacity = 200 * 1024;
    BytsPerSec = 512;

    medium = (T_PMEDIUM)Ram_Malloc(sizeof(T_MEDIUM));

    if (medium == AK_NULL)
    {
        return AK_NULL;
    }

    i = 0;

    while (BytsPerSec > 1)
    {
        BytsPerSec >>= 1;
        i++;
    }

    medium->SecBit = i;
	medium->PageBit = i;
	medium->SecPerPg = 0;
    ((T_POBJECT)medium)->destroy = (F_DESTROY)Medium_Destroy;
    ((T_POBJECT)medium)->type = TYPE_MEDIUM;
    medium->read = (F_ReadSector)Res2Disk_Read;
    medium->write = (F_WriteSector)Res2Disk_Write;
    medium->flush = Res2Disk_Flush;
    medium->capacity = capacity;
    medium->type = MEDIUM_DISKETTE;
    medium->msg = AK_NULL;

#endif  //WIN32

    return medium;
}



static T_U32 NandDisk_Read(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size)
{
	return InitReadDisk(medium, buf, sector, size, NandDisk_No);
}

static T_U32 NandDisk_Write(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size)
{
	return  InitWriteDisk(medium, buf, sector, size, NandDisk_No);
}

static T_BOOL NandDisk_Flush(T_PMEDIUM medium)
{
    return AK_TRUE;
}

T_PMEDIUM NandDisk_Initial()
{
#ifdef WIN32
    T_PMEDIUM medium;
    T_U32 capacity, BytsPerSec;
    T_U8  i;

    capacity = 200 * 1024;
    BytsPerSec = 512;

    medium = (T_PMEDIUM)Ram_Malloc(sizeof(T_MEDIUM));

    if (medium == AK_NULL)
    {
        return AK_NULL;
    }

    i = 0;

    while (BytsPerSec > 1)
    {
        BytsPerSec >>= 1;
        i++;
    }

    medium->SecBit = i;
	medium->PageBit = i;
	medium->SecPerPg = 0;
    ((T_POBJECT)medium)->destroy = (F_DESTROY)Medium_Destroy;
    ((T_POBJECT)medium)->type = TYPE_MEDIUM;
    medium->read = (F_ReadSector)NandDisk_Read;
    medium->write = (F_WriteSector)NandDisk_Write;
    medium->flush = NandDisk_Flush;
    medium->capacity = capacity;
    medium->type = MEDIUM_NANDFLASH;
    medium->msg = AK_NULL;
#endif  //WIN32

    return medium;
}

static T_VOID  NdRes_SetEmulate(T_U8 DriverNo)
{
    ResDisk_No = DriverNo;
}

static T_VOID  NdRes2_SetEmulate(T_U8 DriverNo)
{
    Res2Disk_No = DriverNo;
}

static T_VOID  Nand_SetEmulate(T_U8 DriverNo)
{
    NandDisk_No = DriverNo;
}

static T_BOOL MountNandResv(T_U8 DriverNo)
{
#ifdef OS_WIN32
    T_PMEDIUM medium;
    T_U32 driver;
	
    if (!Global_DriverAvail(DriverNo))
        return AK_FALSE;

    medium = ResDisk_Initial();
    if (medium == AK_NULL)
    {
        return AK_FALSE;
    }
    driver = Driver_Initial(medium, DRIVER_BUFFER_LEN);
    //driver->separator[0] = '/'; //pls change into system set.
    Global_MountDriver(driver, DriverNo);
#endif
    return AK_TRUE;
}

static T_BOOL MountNandResv2(T_U8 DriverNo)
{
#ifdef OS_WIN32
    T_PMEDIUM medium;
    T_U32 driver;

    if (!Global_DriverAvail(DriverNo))
        return AK_FALSE;

    medium = Res2Disk_Initial();
    if (medium == AK_NULL)
    {
        return AK_FALSE;
    }
    driver = Driver_Initial(medium, DRIVER_BUFFER_LEN);
    //driver->separator[0] = '/'; //pls change into system set.
    Global_MountDriver(driver, DriverNo);
#endif
    return AK_TRUE;
}


T_BOOL MountNandOpen(T_U8 DriverNo)
{
#ifdef OS_WIN32
    T_PMEDIUM medium;
    T_U32 driver;

    if (!Global_DriverAvail(DriverNo))
        return AK_FALSE;

    medium = NandDisk_Initial();
    if (medium == AK_NULL)
    {
        return AK_FALSE;
    }

    driver = Driver_Initial(medium, DRIVER_BUFFER_LEN);
    //driver->separator[0] = '/'; //pls change into system set.
    Global_MountDriver(driver, DriverNo);
#endif
  return AK_TRUE;
}

T_BOOL InitWin32Nand(T_VOID)
{
#ifdef OS_WIN32
	NdRes_SetEmulate('W');
	Nand_SetEmulate('X');
	NdRes2_SetEmulate('Z');
	
	if (!MountNandResv(0))
	{
		printf("NAND:		Mount Nand Reserve Zone Error\r\n");
		return AK_FALSE;
	}
	if (!MountNandResv2(1))
	{
		printf("NAND:		Mount Nand Reserve2 Zone Error\r\n");
		return AK_FALSE;
	}

	if (!MountNandOpen(3))
	{
		printf("NAND:		Mount Nand Open Zone Error\r\n");
		return AK_FALSE;
	}
	
	return AK_TRUE;
#endif
}
#endif

//End of File
