#ifndef		_MOUNT_H_
#define		_MOUNT_H_

#include "anyka_types.h"
#include "fs.h"
#include "fwl_sd.h"
//***********************************************************************************/

#define DRIVER_BUFFER_LEN (100*1024)

typedef enum
{
	ZT_MMI	         =  0,
	ZT_MMI_BK        =  1,
	ZT_UNSTANDARD    =  2,
	ZT_UNSTANDARD_BK =	3,
	ZT_STANDARD	     =	4,
	ZT_FAKE		     =	5,
} eZONE_DATA_TYPE;


typedef PDRIVER_INFO T_PDRIVER;
typedef DRIVER_INFO  T_DRIVER_INFO;

#define NAND_IS_USR(DriverInfo) ((MEDIUM_PARTITION == (DriverInfo).nMainType) && (USER_PARTITION == (DriverInfo).nSubType))
#define NAND_IS_SYS(DriverInfo) ((MEDIUM_PARTITION == (DriverInfo).nMainType) && (SYSTEM_PARTITION == (DriverInfo).nSubType))


//***********************************************************************************/
T_BOOL Fwl_FhaInit(T_VOID);
T_BOOL Nand_MountInit(T_VOID);
T_BOOL Fwl_MountNand(T_VOID);
T_VOID Nand_DestoryFs(T_VOID);
T_U8   Nand_GetMediumSymbol(T_PMEDIUM medium);
T_BOOL Fwl_Fha_SD_Init(T_eINTERFACE_TYPE type);
#ifdef NANDBOOT
T_BOOL CheckLibVersions(T_VOID);
#endif
#endif


