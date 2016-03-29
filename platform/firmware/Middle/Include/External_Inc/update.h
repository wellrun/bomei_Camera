#ifndef _UPDATE_H_
#define _UPDATE_H_ 1
#include "akdefine.h"
#include "Fwl_osfs.h"
#include "Eng_String_UC.h"
#include "Eng_String.h"


#define RESULT_BUF_SIZE   FS_MAX_PATH_LEN

#undef  MAX_PATH
#define MAX_PATH          FS_MAX_PATH_LEN


typedef enum
{	
	UPDATE_SUCCESSFUL = 1,
	FOLDER_NO_EXIST,
	FILE_NO_EXIST,	
	OTHER_ERROR,
}E_UPDATE_RESULT_CODE;


typedef struct
{
	E_UPDATE_RESULT_CODE  RetCode;
	T_TCHR                Name[RESULT_BUF_SIZE];	
}T_UPDATE_RESULT;

typedef T_VOID (*CBFUNC_UPDATE_FILE)(E_UPDATE_RESULT_CODE code , T_U32 progress);



typedef enum
{
    CHIP_3224, //AK_3224
    CHIP_322L, //AK_322L
    CHIP_36XX, //Sundance
    CHIP_780X, //Aspen
    CHIP_880X, //Aspen2
    CHIP_10X6, //Snowbird,snowbirdsA~D
    CHIP_3631, //Sundance2
    CHIP_3671, //Sundance2A
    CHIP_980X,    //aspen3s
    CHIP_3671A,    //sundance2a V6
    CHIP_1080A,    //snowbirdsE
    CHIP_37XX,  //sundance3
    CHIP_11XX,  //AK11
    CHIP_RESERVER,
}E_CHIP_TYPE;



typedef struct
{
    T_U8  Disk_Name;				//盘符名
    T_U8  bOpenZone;				//
    T_U8  ProtectType;			//	
    T_U8  ZoneType;				//
	T_U32 Size;
    T_U8  volumeLable[12];
}T_PARTITION_CONFIG;

typedef struct
{
    T_BOOL bCompare;
    T_BOOL bUpdateSelf;
    T_U32 ld_addr;
    T_CHR binPath[MAX_PATH+1];
    T_CHR  fileName[16];
}T_BURN_BIN_CONFIG;

typedef struct
{
    T_BOOL bCompare;
    T_U32 file_mode;
    T_CHR DestPath[MAX_PATH+1];
    T_CHR SourcePath[MAX_PATH+1];
}T_BURN_UDISK_CONFIG;

typedef struct
{
    T_BOOL bCompare;
    T_CHR driverName;
    T_CHR img_path[MAX_PATH];
}T_BURN_IMG_CONFIG;

typedef struct
{
    T_U32   type;                  //RAM类型	
    T_U32   size;                   //RAM大小
    T_U32   banks;                  //RAM Banks
    T_U32   row;                    //RAM row
    T_U32   column;                 //RAM Column
}T_RAM_INFO;

typedef struct
{
    E_CHIP_TYPE  ChipType;
    T_U32 ResvSize;
    T_U32 partitionNum;
    T_PARTITION_CONFIG* partition_param;
    T_U32 fs_resv_block_num;
    T_RAM_INFO  RamInfo;  
    T_U32 NandBinCnt;
    T_BURN_BIN_CONFIG* pNandBinParam;
    T_U32 UdiskFileCnt;
    T_BURN_UDISK_CONFIG* pUdiskFile;
    T_U32 ImgFileCnt;
    T_BURN_IMG_CONFIG* pImgFile;
	T_U32 m_freq;
    T_CHR nandbootpath[MAX_PATH+1];
}T_BURNER_PARAM_CONFIG;



T_BOOL  SPI_UpdateTask(T_TCHR *pFolder, T_TCHR * ppBackupFileList[],
						  CBFUNC_UPDATE_FILE  pFunc, T_VOID *TaskStackAddr, 
						  T_U32 StackSize);

T_VOID UpdateTask_Close(T_VOID);


#endif //_UPDATE_H_
