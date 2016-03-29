#ifndef		_MOUNT_H_
#define		_MOUNT_H_

#include "anyka_types.h"
#include "fs.h"

typedef PDRIVER_INFO T_PDRIVER;
typedef DRIVER_INFO  T_DRIVER_INFO;

typedef struct
{
	T_U16 read;
	T_U16 write;
}struct_Medium_OptCnt;


#define NAND_IS_USR(DriverInfo) ((MEDIUM_PARTITION == (DriverInfo).nMainType) && (USER_PARTITION == (DriverInfo).nSubType))
#define NAND_IS_SYS(DriverInfo) ((MEDIUM_PARTITION == (DriverInfo).nMainType) && (SYSTEM_PARTITION == (DriverInfo).nSubType))

//***********************************************************************************/
T_VOID Nand_MountInit(T_VOID);

T_BOOL Nand_UsbRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);
T_BOOL Nand_UsbWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);

T_U8   Nand_GetZoneIdByType(T_U8 zonetype);
T_VOID Nand_DestoryFs();

T_BOOL Nand_UsbNandRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);

T_BOOL Nand_UsbNandWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);

T_BOOL Nand_ResUsbNbRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);

T_BOOL Nand_ResUsbNdWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);



#endif


