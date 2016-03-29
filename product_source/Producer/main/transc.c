/**
 * @FILENAME transc.c
 * @BRIEF handle all burn transc
 * Copyright (C) 2009 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR Jiang Dihui
 * @DATE 2009-09-10
 * @version 1.0
 */
#include "transc.h"
#include "nand_list.h"
#include "fha.h"
#include "fha_asa.h"
#include "fsa.h"
#include "arch_nand.h"
#include <string.h>
#include "Fwl_osMalloc.h"
#include "Fwl_osFS.h"
#include "fs.h"
#include "fwl_nandflash.h"
#include "fwl_spiflash.h"
#include "fwl_emmc.h"
#include "fwl_usb_transc.h"
#include "hal_print.h"
#include "file.h"

#define  CHECK_FAIL(result,ret)  {ret = (FHA_SUCCESS != (result)) ? AK_FALSE : AK_TRUE;}
#define  SPIBOOTAREA_PAGE_NUM   257
#define  SD_SECTOR_SIZE         512

#define  FS_S_IFDIR             0x04000000

#define  GET_ALL_SD_BEFORE_FS_DATA   "ALLDATA"

#define  FORMAT_SPIFLASH_FS   "_FORMAT_FS"



typedef struct
{
	T_U32 start_page;
	T_U32 spiflash_totalsize;
}T_SPI_FS_INFO;


typedef struct DownLoadControl
{
    T_U32 file_total_len;
    T_U32 data_total;
}T_DL_CONTROL;

typedef struct
{
	T_U8 status;
	T_U8 mode;
	T_CHR fileName[8];
}T_WRITE_ASA_FILE;

typedef enum
{
    DL_START,
    DL_WRITE,
}T_DL_STATUS;

typedef struct
{
	T_U32 chip_ID;
    T_U32 chip_cnt;
}T_CHIP_INFO;

typedef struct
{
    T_U8 DiskName;        
    T_U8 Rsv1[3];            
    T_U32 PageCnt;          
    T_U32 Rsv2;
    T_U32 Rsv3;
}T_DRIVER_INFO;
typedef struct
{
    T_U16 eMedium;
	T_U16 burn_mode;
}T_MODE_CONTROL;


typedef struct
{
	T_U8 *pLibName;
	T_U8 *(*pVerFun)(T_VOID);
}T_VERSION_INFO;

typedef struct
{
    T_U32 bin_len;
    T_U32 bin_pos;
    T_U32 bin_type;
}T_UP_BIN_INFO;

typedef struct
{
    T_U32 img_len;
    T_U32 img_pos;
    T_U8  img_ID;
}T_DOWN_IMG_INFO;


typedef struct 
{
    T_U32 FormatStartPage;
	T_U32 Spi_TotalSize;
}T_WRITE_SPI_CONFIG_INFO;
T_WRITE_SPI_CONFIG_INFO g_write_config_info = {0};


static T_U32            m_transc_state = TRANS_NULL;
static T_U32            m_transc_channelID = 0;
static T_CHIP_INFO      m_chipInfo;
static T_DL_CONTROL     m_dl_control;

static T_UP_BIN_INFO    m_bin_info = {0};

T_U32                   g_burn_erase = 0;   // the 7th bit of burn_mode. 
T_U32                   g_burn_medium = 0; 
T_U32                   g_burn_mode = 0;
T_PNANDFLASH            m_nandbase = AK_NULL; 
T_PMEDIUM               m_SDMedium = AK_NULL; 
static T_DOWN_IMG_INFO  m_Down_Img_Info = {0};
static T_U32 m_chip_high_ID;/* used for nand flash high id*/


static T_pVOID fhalib_init(T_U32 eMedium, T_U32 eMode, T_pVOID pPhyInfo);

static T_BOOL DL_Check_Len(T_DL_STATUS status, T_U32 len);

static T_BOOL Handle_Transc_Data(T_U8 data[], T_U32 len);

static T_BOOL SetLibVersion(T_VOID);

static T_BOOL WriteAsaFile(T_U8 data[], T_U32 len);
static T_BOOL ReadAsaFile(T_U8 data[], T_U32 len);
static T_BOOL ReadAsa_senddata(T_U8 data[], T_U32 len);
static T_BOOL ReadAsa_getfilename(T_U8 data[], T_U32 len);
static T_BOOL GetChannel_ID(T_U8 data[], T_U32 len);

T_BOOL g_format_spiflash_fs_flag = AK_FALSE;
T_U32 g_format_spiflash_fs_startpage = 0;
T_U32 g_format_spiflash_fs_len = 0;
T_U32 g_format_spiflash_fs_len_temp = 0;





static T_BOOL CheckLibVersion(T_VOID);
static 	T_VERSION_INFO version_info[] = 
{
	{VER_NAME_DRV, 	drvlib_version},
	{VER_NAME_FS,  	FSLib_GetVersion},
	{VER_NAME_MOUNT,FS_GetVersion},
	{VER_NAME_MTD, 	MtdLib_GetVersion},
	{VER_NAME_FHA, 	FHA_get_version}			
};	


//Table of transc handle function
static T_ANYKA_TRANSC m_producer_transc[]=
{
    {TRANS_SWITCH_USB,      Transc_SwitchUsbHighSpeed,      AK_NULL,                AK_NULL},
    {TRANS_TEST_CONNECT,    Transc_TestConnection,          AK_NULL,                AK_NULL},
    {TRANS_SET_MODE,        Transc_SetMode,                 AK_NULL,                AK_NULL},
    {TRANS_GET_FLASH_HIGH_ID,     Transc_GetFlashHighID,              AK_NULL,                Handle_Transc_Data}, 
    {TRANS_SET_ERASE_NAND_MODE,   Transc_SetErase_Mode,     AK_NULL,                AK_NULL},
    {TRANS_GET_FLASHID,     Transc_GetFlashID,              AK_NULL,                Handle_Transc_Data}, 
    {TRANS_SET_NANDPARAM,   Transc_SetNandParam,            Handle_Transc_Data  ,   AK_NULL},
    {TRANS_INIT_SECAREA,    Transc_InitSecArea,             AK_NULL,                AK_NULL},
    {TRANS_SET_RESV,        Transc_SetResvAreaSize,         AK_NULL,                AK_NULL},
    {TRANS_DOWNLOAD_BIN_START,Transc_StartDLBin,            Handle_Transc_Data, AK_NULL},
    {TRANS_DOWNLOAD_BIN_DATA, Transc_DLBin,                 Handle_Transc_Data, AK_NULL},
    {TRANS_DOWNLOAD_BOOT_START,Transc_StartDLBoot,          Handle_Transc_Data, AK_NULL},
    {TRANS_DOWNLOAD_BOOT_DATA,Transc_DLBoot,                Handle_Transc_Data, AK_NULL},
    {TRANS_CLOSE,             Transc_Close,                 AK_NULL,                AK_NULL},
    {TRANS_CREATE_PARTITION, Transc_CreatePartion,          Handle_Transc_Data,     AK_NULL},
    {TRANS_GET_DISK_INFO,    Transc_GetDiskInfo,            AK_NULL,                Handle_Transc_Data},
    {TRANS_MOUNT_DRIVER,    Transc_MountDriver,             Handle_Transc_Data,     AK_NULL},
    {TRANS_FORMAT_DRIVER,   Transc_FormatDriver,            Handle_Transc_Data,     AK_NULL},
    {TRANS_DOWNLOAD_FILE_START,Transc_StartDLFile,          Handle_Transc_Data,     AK_NULL},
    {TRANS_DOWNLOAD_FILE_DATA, Transc_DLFile,               Handle_Transc_Data,     AK_NULL},
    {TRANS_DOWNLOAD_IMG_START, Transc_StartDLImg,           Handle_Transc_Data,     AK_NULL},
    {TRANS_DOWNLOAD_IMG_DATA,  Transc_DLImg,                Handle_Transc_Data,     AK_NULL},
    {TRANS_SET_SPIPARAM,       Transc_SetSPIParam,          Handle_Transc_Data  ,   AK_NULL},
    {TRANS_UPLOAD_BIN_START,   Transc_GetBinStart,          Handle_Transc_Data,     AK_NULL},
    {TRANS_UPLOAD_BIN_LEN,     Transc_GetBinLength,         AK_NULL,                Handle_Transc_Data},
    {TRANS_UPLOAD_BIN_DATA,    Transc_GetBinData,           AK_NULL,                Handle_Transc_Data},	
	{TRANS_WRITE_ASA_FILE,	   Transc_WriteAsaFile, 		WriteAsaFile,			AK_NULL},
	{TRANS_RESET,              Transc_Reset,                AK_NULL,                AK_NULL},
	{TRANS_SET_CHANNEL_ID,     Transc_SetChannel_ID,        AK_NULL,                AK_NULL},
	{TRANS_GET_CHANNEL_ID,     Transc_GetChannel_ID,        AK_NULL,                GetChannel_ID},
	{TRANS_READ_ASA_FILE,	   Transc_ReadAsaFile, 		    ReadAsa_getfilename,    ReadAsa_senddata},
    {TRANS_UPLOAD_BOOT_START,   Transc_GetBootStart,        Handle_Transc_Data,   AK_NULL},
    {TRANS_UPLOAD_BOOT_DATA,    Transc_GetBootData,         AK_NULL,              Handle_Transc_Data},	
    {TRANS_UPLOAD_BOOT_LEN,     Transc_GetBootLen,           AK_NULL,              Handle_Transc_Data},
    {TRANS_SET_BIN_RESV_SIZE,   Transc_Set_Bin_Resv_Size,     AK_NULL,                AK_NULL},
    {TRANS_UPLOAD_SPIDATA_START, Transc_GetSpiDataStart,    Handle_Transc_Data,     AK_NULL},
    {TRANS_UPLOAD_SPIDATA_DATA,  Transc_GetSpiData,         AK_NULL,             Handle_Transc_Data},
    {TRANS_WRITE_OTP_SERIAL, Transc_write_otp_serial,    Handle_Transc_Data,     AK_NULL},
    {TRANS_READ_OTP_SERIAL,  Transc_read_otp_serial,         AK_NULL,             Handle_Transc_Data},
};


/**
 * @BREIF    Initial burn transc process to enter main loop by calling usb function
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Prod_Transc_Main()
{
    Fwl_Usb_Set_Trans(m_producer_transc, sizeof(m_producer_transc) / sizeof(T_ANYKA_TRANSC));
      
    Fwl_Usb_Main();

    return AK_TRUE;
}

  
/**
 * @BREIF    handle function of transc data stage(send or receive)
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Handle_Transc_Data(T_U8 data[], T_U32 len)
{
    T_BOOL ret = AK_TRUE;
    T_NAND_PHY_INFO* pNandInfo = AK_NULL;
    T_PFHA_BIN_PARAM pBinInfo = AK_NULL;
    T_IMG_INFO* pImgInfo = AK_NULL;
    T_UDISK_FILE_INFO* pFileInfo = AK_NULL;
    T_U32 tmp;
    T_U32* pTmp;
    T_U32 i, DriverID;


    //printf("PR_T state:%d, len:%d\r\n", m_transc_state, len);
   
    switch(m_transc_state)
    {
        case TRANS_GET_FLASHID:     //Get flash chip ID
            memcpy(data, &m_chipInfo, sizeof(T_CHIP_INFO));
            break;
            
        case TRANS_GET_FLASH_HIGH_ID:     //Get flash chip high ID
            memcpy(data, &m_chip_high_ID, sizeof(T_U32));
             break;
             
        case TRANS_SET_NANDPARAM:   //Set nandflash param
            pNandInfo = (T_NAND_PHY_INFO*)data;

            // pNandInfo->flag &= ~0x10000000;      // 强行关闭multiplane
            m_nandbase = (T_PNANDFLASH)fhalib_init(g_burn_medium, g_burn_mode, pNandInfo);
                        
            if (AK_NULL == m_nandbase)
            {
                ret = AK_FALSE;
            }

            break;
        case TRANS_SET_SPIPARAM:  //set spiflash
            if (AK_NULL == fhalib_init(g_burn_medium, g_burn_mode, (T_SFLASH_PARAM *)data))
            {
                ret = AK_FALSE;
            }
            
            break;
        case TRANS_DOWNLOAD_BIN_START:  //receive bin file information
        	pBinInfo = (T_PFHA_BIN_PARAM)data;
        	DL_Check_Len(DL_START, pBinInfo->data_length);

            //由于fha库里已进行如果传bin保留区为0，那么就会按多 一个文件进行保存,所以这里不需要
            /*
            if(MEDIUM_EMMC == g_burn_medium)
			{        
	            if (!memcmp(pBinInfo->file_name, "BIOS", strlen(pBinInfo->file_name)))
	            {
	                if (FHA_FAIL == FHA_set_bin_resv_size(0))
	                {
	                    return AK_FALSE;
	                }
	            }
			}
            */
            if((MEDIUM_SPIFLASH == g_burn_medium) && (1 == pBinInfo->bBackup))
            {
                printf("error spiflash can not bBackup,  pBinInfo->bBackup: %d \r\n", pBinInfo->bBackup);
                return AK_FALSE;
            }
            
            printf("g_burn_medium:%d\n",g_burn_medium);
            if((MEDIUM_SPIFLASH == g_burn_medium) 
                && memcmp(pBinInfo->file_name, FORMAT_SPIFLASH_FS, sizeof(FORMAT_SPIFLASH_FS)) == 0)
            {
                printf("pBinInfo->file_name:%s\n",pBinInfo->file_name);
                g_format_spiflash_fs_flag = AK_TRUE;
                g_format_spiflash_fs_len = pBinInfo->data_length;
                g_format_spiflash_fs_len_temp = 0;
                printf("pBinInfo->ld_addr: %x\n", pBinInfo->ld_addr);
                g_format_spiflash_fs_startpage = pBinInfo->ld_addr;
                printf("g_format_spiflash_fs_startpage: %d\n", g_format_spiflash_fs_startpage);
                
            }
            else
            {
                g_format_spiflash_fs_flag = AK_FALSE;
            }
            
        	CHECK_FAIL(FHA_write_bin_begin(pBinInfo), ret)
            
        	break;
    	case TRANS_DOWNLOAD_BIN_DATA:   //receive bin file data
        	CHECK_FAIL(FHA_write_bin(data, len), ret)
            printf("g_format_spiflash_fs_flag:%d\n",g_format_spiflash_fs_flag);
            printf("g_burn_mode:%d\n",g_burn_mode);
        	break;
		case TRANS_DOWNLOAD_BOOT_START: //Get download boot information
        	DL_Check_Len(DL_START, *(T_U32*)data);
            CHECK_FAIL(FHA_write_boot_begin(*(T_U32*)data), ret);
        	break;
    	case TRANS_DOWNLOAD_BOOT_DATA:  //download boot data
        	CHECK_FAIL(FHA_write_boot(data, len), ret)
        	break; 

        case TRANS_CREATE_PARTITION:    //create zone partion
        #ifdef CLIENT_HAOJIXING
            break;
        #endif
            tmp = FHA_get_last_pos();
            if (MODE_NEWBURN == g_burn_mode)
            {
                FORMAT_INFO FormatInfo;

                pTmp = (T_U32*)data;
                
                if (MEDIUM_NAND == g_burn_medium || MEDIUM_SPI_NAND == g_burn_medium )
                {
                    FormatInfo.MediumType = FS_NAND;
                    FormatInfo.obj = (T_U32)m_nandbase;
                }
                else
                {
                    FormatInfo.MediumType = FS_SD;
                    FormatInfo.obj = (T_U32)m_SDMedium;
                }
                
                printf("partition para:%x,%d,%d,%d,%x \r\n", (pTmp+2), pTmp[1], pTmp[0], tmp, &FormatInfo);
                ret = FS_LowFormat((T_FS_PARTITION_INFO*)(pTmp+2), pTmp[1], pTmp[0], tmp, &FormatInfo);
            }
            else
            {
                DRIVER_INFO DriverInfo;
        
                if (MEDIUM_NAND == g_burn_medium || MEDIUM_SPI_NAND == g_burn_medium )
                {
                    T_U8 DriverList[26];
                    T_U8 DriverCnt;
                    
                    if (T_U8_MAX == FS_MountNandFlash(m_nandbase, tmp, DriverList, &DriverCnt))
                    {
                        ret = AK_FALSE;
                    }
                }
                else
                {
                    T_U8 DriverCnt;
                    DriverInfo.fRead  = m_SDMedium->read;
                    DriverInfo.fWrite = m_SDMedium->write;
                    DriverInfo.nBlkSize = 1 << m_SDMedium->SecBit;
                    DriverInfo.nBlkCnt  = m_SDMedium->capacity;
                    
                    if (T_U8_MAX == FS_MountMemDev(&DriverInfo, &DriverCnt, T_U8_MAX))
                    {
                        ret = AK_FALSE;
                    }
                }
            }    
            break;    
           
        case TRANS_GET_DISK_INFO:
            {
                T_U32 *pDriverNum;
                T_DRIVER_INFO *pDriverInfo;
                DRIVER_INFO DriverInfo;

                pDriverNum = (T_U32 *)data;
                pDriverInfo = (T_DRIVER_INFO *)(data + 4);

                *pDriverNum = 0;
                if (FS_GetFirstDriver(&DriverInfo))
                {
                    do
                    {
                        (*pDriverNum)++;
                        pDriverInfo->DiskName = DriverInfo.DriverID + 'A';
                        if (MEDIUM_NAND == g_burn_medium || MEDIUM_SPI_NAND == g_burn_medium )
                        {
                            pDriverInfo->PageCnt  = DriverInfo.medium->capacity >> DriverInfo.medium->SecPerPg;
                        }
                        else
                        {
                            pDriverInfo->PageCnt  = DriverInfo.medium->capacity;
                        }
                        
                        printf("pDriverInfo->DiskName:%d \r\n", pDriverInfo->DiskName);
                        printf("pDriverInfo->PageCnt:%d \r\n", pDriverInfo->PageCnt);
                        pDriverInfo++;
                    }while (FS_GetNextDriver(&DriverInfo));
                }

                if (0 == *pDriverNum)
                {
                    ret = AK_FALSE;
                }
            }
            
            break;
        case TRANS_FORMAT_DRIVER:       //format driver
            memcmp(&tmp, data, sizeof(T_U32));
      
            break;
        case TRANS_MOUNT_DRIVER:        //mount driver
            ret = AK_TRUE;
            break;    
        case TRANS_DOWNLOAD_IMG_START:  //Get download image information
            pImgInfo =(T_IMG_INFO*)data;
            pImgInfo->wFlag = AK_FALSE;
            DL_Check_Len(DL_START,pImgInfo->data_length);
            ret = (FSA_SUCCESS == FSA_write_image_begin(pImgInfo));
            m_Down_Img_Info.img_len  = pImgInfo->data_length;
            m_Down_Img_Info.img_pos  = 0;
            m_Down_Img_Info.img_ID   = (T_U8)(pImgInfo->DriverName - 'A');
            
            break;
        case TRANS_DOWNLOAD_IMG_DATA:   //download image data
            ret = (FSA_SUCCESS == FSA_write_image(data, len));
            m_Down_Img_Info.img_pos += len;
            if (ret && m_Down_Img_Info.img_pos >= m_Down_Img_Info.img_len)
            {
                FS_UnInstallDriver((T_U8)m_Down_Img_Info.img_ID, 1);
                if (0 == FS_InstallDriver((T_U8)m_Down_Img_Info.img_ID, 1))
                {
                    printf("Write image end, Install driver fail! ID=%d\r\n", (T_U8)m_Down_Img_Info.img_ID);
                    ret = AK_FALSE;
                }
            }
            break;

        case TRANS_DOWNLOAD_FILE_START: //start download udisk file
            pFileInfo = (T_UDISK_FILE_INFO *)data;
            printf("dowload file:%s, %d, [%d]\r\n", pFileInfo->apath, pFileInfo->bCheck, pFileInfo->file_mode);
            
            DL_Check_Len(DL_START,pFileInfo->file_length);

            if (pFileInfo->file_mode != FS_S_IFDIR)     // not directory
            {
                pFileInfo->file_mode = FSA_Create_file;
            }
            
            ret = (FSA_SUCCESS == FSA_write_file_begin(pFileInfo)); 
            break;
        case TRANS_DOWNLOAD_FILE_DATA:  //download udisk file data
            ret = (FSA_SUCCESS == FSA_write_file(data, len));
            break;
            
        case TRANS_UPLOAD_SPIDATA_START:
            {
                T_FHA_BIN_PARAM spi_param = {0};

                spi_param.data_length = ((T_U32*)data)[0];
                 printf("bin_param.data_length = %d \r\n", spi_param.data_length);
                FHA_read_AllDatat_begin(&spi_param);
                break;
            }
        case  TRANS_UPLOAD_SPIDATA_DATA:
            if (FHA_SUCCESS == FHA_read_AllDatat(data, len))
            {
                ret = AK_TRUE;
            }
            else
            {
                ret = AK_FALSE;
            }
            break;
            
        case TRANS_UPLOAD_BIN_START:
            if (!memcmp(data, GET_ALL_SD_BEFORE_FS_DATA, sizeof(GET_ALL_SD_BEFORE_FS_DATA)))
            {
                if (MEDIUM_NAND == g_burn_medium || MEDIUM_SPI_NAND == g_burn_medium  || AK_NULL == m_SDMedium)
                {
                    ret = AK_FALSE;
                }
                else
                {
                    m_bin_info.bin_len = FHA_get_last_pos();
                    m_bin_info.bin_len *= SD_SECTOR_SIZE;
                    ret = (m_bin_info.bin_len != 0);
                    m_bin_info.bin_type = 1;
                    m_bin_info.bin_pos  = 0;
                }
            }
            else
            {
                T_FHA_BIN_PARAM bin_param = {0};
                
                memcpy(&bin_param.file_name, data, sizeof(bin_param.file_name));
                //memcpy(&bin_param.ld_addr, data+sizeof(bin_param.file_name), sizeof(T_U32));
                
                printf("++ bin_param.file_name=%s ++\r\n", bin_param.file_name);
                printf("++ bin_param.ld_addr=%x ++\r\n", bin_param.ld_addr);
                printf("++ bin_param.data_length=%d ++\r\n", bin_param.data_length);
                printf("++ bin_param.bBackup=%d ++\r\n", bin_param.bBackup);
                printf("++ bin_param.bCheck=%d ++\r\n", bin_param.bCheck);
                printf("++ bin_param.bUpdateSelf=%d ++\r\n", bin_param.bUpdateSelf);
                
                ret = (FHA_SUCCESS == FHA_read_bin_begin(&bin_param));
                m_bin_info.bin_len = bin_param.data_length;
                m_bin_info.bin_type = 0;
                m_bin_info.bin_pos  = 0;
            }
            break;
        case TRANS_UPLOAD_BIN_LEN:
            printf("++ upload bin len=%d ++\r\n", m_bin_info.bin_len);
            ((T_U32*)data)[0] = m_bin_info.bin_len;
            break;
        case TRANS_UPLOAD_BIN_DATA:
            printf(".");
            if (0 == m_bin_info.bin_type)
            {
                ret = (FHA_SUCCESS == FHA_read_bin(data, len));
            }
            else
            {
                T_U32 SecCnt = (len + SD_SECTOR_SIZE - 1) / SD_SECTOR_SIZE;

                if (SecCnt == m_SDMedium->read(m_SDMedium, data, m_bin_info.bin_pos, SecCnt))
                {
                    m_bin_info.bin_pos += SecCnt;
                    ret = AK_TRUE;
                }
                else
                {
                    ret = AK_FALSE;
                }
            }

            break;    

        case TRANS_UPLOAD_BOOT_START:
            {
                T_FHA_BIN_PARAM boot_param;
                    
                memcpy(&boot_param.file_name, data, sizeof(boot_param.file_name));
                //printf("++bin_param.file_name=%s ++\r\n", boot_param.file_name);
                
                if(MEDIUM_SPIFLASH == g_burn_medium)
                {
                    // 当介质是spi时，那先从boot_param.data_length传入boot的页个数
                    boot_param.data_length = SPIBOOTAREA_PAGE_NUM - 1;
                }
                
                ret = (FHA_SUCCESS == FHA_read_boot_begin(&boot_param));
                m_bin_info.bin_len = boot_param.data_length;
                //printf("++m_bin_info.bin_len=%d ++\r\n", m_bin_info.bin_len);
            }
            break;
            
        case TRANS_UPLOAD_BOOT_LEN:
            //printf("++ upload bin len=%d ++\r\n", m_bin_info.bin_len);
            ((T_U32*)data)[0] = m_bin_info.bin_len;
            break;
            
        case TRANS_UPLOAD_BOOT_DATA:
            //printf("++len=%d ++\r\n", len);
            ret = (FHA_SUCCESS == FHA_read_boot(data, len));
            break; 
        case TRANS_WRITE_OTP_SERIAL:
            //printf("++len=%d ++\r\n", len);
            //ret = spi_nand_write_otpregion(0, 0, data,  len);
            break; 
        case TRANS_READ_OTP_SERIAL:
            //printf("++len=%d ++\r\n", len);
            //ret = spi_nand_read_otpregion(0, 0, data, len);
            break; 
            
            
        default:
            break;           
    }

    return ret;    
}

/**
 * @BREIF    transc of switch usb to high speed
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_SwitchUsbHighSpeed(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_U32 i,j;
    printf("++switch usb++\r\n");
    m_transc_state = TRANS_SWITCH_USB;

    //disable first
    printf("T disable usb\r\n");
    ausb_disable();

#ifdef CHIP_AK37XX
    printf("asic_before: %d\n", get_asic_freq());
    printf("pll_before: %dMhz\n", get_pll_value());
#endif
     //wait again
    for(i = 0; i < 60*1000000/1; i++)
    {
        j++;
    }

#ifdef CHIP_AK37XX
    set_pll_value(248); //set pll to 248M
    set_cpu_2x_asic(AK_TRUE);
#endif   
  
    printf("asic: %d\n", get_asic_freq());
    printf("pll: %dMhz\n", get_pll_value());    
    
    //enable
    printf("T enable usb\r\n");

    ausb_enable(1);

    return AK_TRUE;
}


/**
 * @BREIF    transc of test connection
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_TestConnection(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;
    T_U32* pData = data;
  
    printf("++test connect++\r\n");
    m_transc_state = TRANS_TEST_CONNECT;

    if('B' != pData[0] || 'T' != pData[1])
    {
        ack = AK_FALSE;
    }
    
    return ack;
}

/**
 * @BREIF    transc of Get nand flash high ID(5th,6th bytes)
 * @AUTHOR   Jiang Dihui
 * @DATE     2013-09-1
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */

T_BOOL Transc_GetFlashHighID(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    m_transc_state = TRANS_GET_FLASH_HIGH_ID;
    printf("++get chip ID++\r\nnand high id:%x\n",m_chip_high_ID);    
    return AK_TRUE;
}


/**
 * @BREIF    transc of set erase nand mode
 * @AUTHOR   lixingjian
 * @DATE     2012-10-26
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_SetErase_Mode(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;
    T_U32* ptmp = (T_U32*)data;

    printf("++set erase nand mode: %d++\r\n", ptmp[0]);
    m_transc_state = TRANS_SET_ERASE_NAND_MODE;

    g_burn_erase = ptmp[0]; //setmode后重新给值
    //实现erase功能
    if (g_burn_erase && MODE_NEWBURN == g_burn_mode && (MEDIUM_NAND == g_burn_medium || MEDIUM_SPI_NAND == g_burn_medium ))   // ERASE
    {
        if(AK_FALSE == Fwl_Erase_block())
        {
            printf("Fwl_Erase_block fail\r\n");
            return AK_FALSE;
        }
    }
    
    printf("++g_burn_erase: %d++\r\n", g_burn_erase);
    return ack;
}



/**
 * @BREIF    transc of set burn mode
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_SetMode(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;
    T_MODE_CONTROL ModeCtrl;

    printf("++set mode++\r\n");
    m_transc_state = TRANS_SET_MODE;

    memcpy(&ModeCtrl, data, sizeof(T_MODE_CONTROL));
    g_burn_medium = ModeCtrl.eMedium;
    g_burn_mode = ModeCtrl.burn_mode;

    printf("T mode:%d, medium:%d\r\n", g_burn_mode, g_burn_medium);

    if (MEDIUM_EMMC == g_burn_medium)
    {
        if (AK_NULL == (m_SDMedium = fhalib_init(g_burn_medium, g_burn_mode, AK_NULL)))
        {
            ack = AK_FALSE;
        }    
    }
    
    return ack;
}

/**
 * @BREIF    transc of Get flash ID
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_GetFlashID(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;
    T_U32 *pTmp;
    T_U32 nChipCnt;
    T_U32 nChipID[2];
    
    printf("++get chip ID++\r\n");
    m_transc_state = TRANS_GET_FLASHID;

    //Get flash ID and chip count
    pTmp = (T_U32*)data;
    if (MEDIUM_NAND == g_burn_medium || MEDIUM_SPI_NAND == g_burn_medium)
    {
        m_chip_high_ID = 0;
        ack = Fwl_NandHWInit(pTmp[0], pTmp[1], nChipID, &nChipCnt);
        m_chipInfo.chip_ID = nChipID[0];
        m_chip_high_ID = nChipID[1];
        m_chipInfo.chip_cnt = nChipCnt;
        //printf("nand id:%x,%x\n",m_chip_high_ID,m_chipInfo.chip_ID);
    }
    else if (MEDIUM_SPIFLASH  == g_burn_medium
        ||MEDIUM_SPI_EMMC     == g_burn_medium
        ||MEDIUM_EMMC_SPIBOOT == g_burn_medium)
    {
	    ack = Fwl_SPIHWInit(&m_chipInfo.chip_ID, &m_chipInfo.chip_cnt);
    }
    
    return ack;
}

/**
 * @BREIF    transc of Initial sec area
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_InitSecArea(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;
    T_U32 ret = FHA_SUCCESS;
    printf("++init secArea++\r\n");
    m_transc_state = TRANS_INIT_SECAREA;

    ret = FHA_asa_scan(AK_TRUE);

    if (FHA_SUCCESS != ret)
    {
        if(MODE_NEWBURN == g_burn_mode)
        {
           printf("T asa format\r\n");

           if(FHA_FAIL == FHA_asa_format(((T_U32*)data)[0]))
           {
                printf("T asa format err\r\n");
                ack = AK_FALSE;
           }
        }
        else
        {
            printf("T scan asa err\r\n");
            ack = AK_FALSE;
        }
    }
    
    return ack;
}

/**
 * @BREIF    transc of set nand param
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_SetNandParam(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    printf("++set nand param++\r\n");
    m_transc_state = TRANS_SET_NANDPARAM;
    
    return AK_TRUE;
}

/**
 * @BREIF    transc of set SPI param
 * @AUTHOR   Jiang Dihui
 * @DATE     2011-05-23
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_SetSPIParam(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    printf("++set SPI param++\r\n");
    m_transc_state = TRANS_SET_SPIPARAM;
    
    return AK_TRUE;
}

/**
 * @BREIF    transc of set user reserver area size
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_Set_Bin_Resv_Size(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;
    T_U32* ptmp = (T_U32*)data;
    
    printf("++Transc_Set_Bin_Resv_Size++\r\n");
    m_transc_state = TRANS_SET_BIN_RESV_SIZE;

    printf("Set Bin size:%dM\r\n", ptmp[0]);

    if (FHA_FAIL == FHA_set_bin_resv_size(ptmp[0]))
    {
        ack = AK_FALSE;
    }  
      
    return ack;
}


T_BOOL Transc_GetSpiDataStart(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    printf("PR_T ++start upload spi data++\r\n");
    m_transc_state = TRANS_UPLOAD_SPIDATA_START;

    return AK_TRUE;
}

T_BOOL Transc_GetSpiData(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    //printf("PR_T ++upload bin len:%d++\r\n", len);
    m_transc_state = TRANS_UPLOAD_SPIDATA_DATA;

    return AK_TRUE;
}


/**
 * @BREIF    transc of set user reserver area size
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_SetResvAreaSize(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;
    T_U32* ptmp = (T_U32*)data;
    
    printf("++set ResvArea++\r\n");
    m_transc_state = TRANS_SET_RESV;

    printf("T ResvArea size:%d, bErase:%d\r\n", ptmp[0], ptmp[1]);

    if (FHA_FAIL == FHA_set_resv_zone_info(ptmp[0], (T_BOOL)ptmp[1]))
    {
        ack = AK_FALSE;
    }  
      
    return ack;
}


/**
 * @BREIF    transc of start download bin
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_StartDLBin(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    printf("++start bin++\r\n");
    m_transc_state = TRANS_DOWNLOAD_BIN_START;

    return AK_TRUE;
}

/**
 * @BREIF    transc of download bin data
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_DLBin(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;

   // printf("++DL bin, len:%d++\r\n", len);
    m_transc_state = TRANS_DOWNLOAD_BIN_DATA;

    //check length of data
    ack = DL_Check_Len(DL_WRITE, len);

    return ack;
}

/**
 * @BREIF    transc of start download boot
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_StartDLBoot(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    printf("++start boot++\r\n");
    m_transc_state = TRANS_DOWNLOAD_BOOT_START;

    return AK_TRUE;
}

/**
 * @BREIF    transc of download boot data
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_DLBoot(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;

    printf("++DL boot, len:%d++\r\n", len);
    m_transc_state = TRANS_DOWNLOAD_BOOT_DATA;

   //check length of data
    ack = DL_Check_Len(DL_WRITE, len);

    return ack;
}

/**
 * @BREIF    transc of close to write config information
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_Close(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;
    T_SPI_FS_INFO spi_fs_info;
    T_U32 page,offset;
	
    printf("++close++\r\n");
	m_transc_state = TRANS_CLOSE;

	
	if((MEDIUM_NAND == g_burn_medium) || (MEDIUM_EMMC == g_burn_medium) || (MEDIUM_SPI_NAND == g_burn_medium ))
	{
        #ifndef CLIENT_HAOJIXING 
		ack = SetLibVersion();
        #endif
	}


    if(g_format_spiflash_fs_flag && MODE_NEWBURN == g_burn_mode && MEDIUM_SPIFLASH == g_burn_medium)
    {
        //格式spi文件系统
        if (g_format_spiflash_fs_startpage != 0)
        {
            //获取文件系统可用的起始页
            page = FHA_get_last_pos();
            printf("FHA_get_last_pos: %d\n", page);

            if(g_format_spiflash_fs_startpage < page)
            {
                printf("error set fs start page less than bin last pos\n");
			    return AK_FALSE;
            }
            else
            {
                page = g_format_spiflash_fs_startpage;
                printf("page: %d\n", page);
            }
		}
        else
        {
            //获取文件系统可用的起始页
            page = FHA_get_last_pos();
            printf("FHA_get_last_pos 00 : %d\n", page);
        }
        printf("produce spiflash start page: %d\n", page);
        
        //由于producer将spi以64KB单位删除的，所以需要64KB对齐
        //256是SPI的页大小
        offset = (page * 256)%(64*1024);
        if(offset != 0)
        {
            page = page*256+(64*1024-offset);
            page = page>>8;
        }
        
        g_write_config_info.FormatStartPage = page;
        printf("now is format,%d, %d\n",page,g_write_config_info.Spi_TotalSize);
        //对文件系统进行格式化
		if(!VME_FsbFormat(g_write_config_info.FormatStartPage,g_write_config_info.Spi_TotalSize))
	   	{
        	printf("VME_FsbFormat fail\n");
			return;
        }
        spi_fs_info.start_page = g_write_config_info.FormatStartPage;
        spi_fs_info.spiflash_totalsize = g_write_config_info.Spi_TotalSize;

        printf("spi_fs_info->startpag:%d, totalsize:%d\n", spi_fs_info.start_page,spi_fs_info.spiflash_totalsize);
        printf("buf_len:%d\n", sizeof(T_SPI_FS_INFO));
        
        if(!FHA_set_fs_part(&spi_fs_info , sizeof(T_SPI_FS_INFO)))
        {
            printf("FHA_set_fs_part fail\n");
			return;
        }
    }

    if(FHA_SUCCESS != FHA_close())
    {
        ack = AK_FALSE;
    }    

	printf("Burn %s!\n", (AK_TRUE == ack)?"successful":"unsuccessful");
   
    return ack;
}

/**
 * @BREIF    transc of create file system partion
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */

static T_BOOL  Transc_CreatePartion(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;
 
    printf("++create partion++\r\n");
    m_transc_state = TRANS_CREATE_PARTITION;

    if (Fwl_MountInit())
    {
        if (!Fwl_FSAInit())
        {
            ack = AK_FALSE;
        }    
    }    

    return ack;
}

/**
 * @BREIF    transc of format driver
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL  Transc_FormatDriver(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    printf("++format driver++\r\n");
    m_transc_state = TRANS_FORMAT_DRIVER;
    
    return AK_TRUE;
}

/**
 * @BREIF    transc of get disk info
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_GetDiskInfo(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    printf("++get disk info++\r\n");
    m_transc_state = TRANS_GET_DISK_INFO;
    
    return AK_TRUE;

}

/*
 * @BREIF    transc of mount driver
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL  Transc_MountDriver(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    printf("++mount driver++\r\n");
    m_transc_state = TRANS_MOUNT_DRIVER;
    
    return AK_TRUE;
}

/**
 * @BREIF    transc of start download udisk file
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_StartDLFile(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    printf("++start file++\r\n");
    m_transc_state = TRANS_DOWNLOAD_FILE_START;

    return AK_TRUE;
}

/**
 * @BREIF    transc of download udisk file
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_DLFile(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;

    //gpf.fDriver.Printf("PR_T ++DL file, len:%d++\r\n", len);
    m_transc_state = TRANS_DOWNLOAD_FILE_DATA;

    //check length of data
    ack = DL_Check_Len(DL_WRITE, len);

    return ack;
}

/**
 * @BREIF    transc of start download image
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_StartDLImg(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
     printf("++start img++\r\n");
     m_transc_state = TRANS_DOWNLOAD_IMG_START;
         
     return AK_TRUE;
}

/**
 * @BREIF    transc of download image data
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-09-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_BOOL Transc_DLImg(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_BOOL ack = AK_TRUE;
 
    m_transc_state = TRANS_DOWNLOAD_IMG_DATA;

    //check length of data
    ack = DL_Check_Len(DL_WRITE, len);
   
    return ack;
}

//*************************************************************
static T_BOOL DL_Check_Len(T_DL_STATUS status, T_U32 len)
{
    return AK_TRUE;
/*
    T_BOOL ret = AK_TRUE;
    
    switch(status)
    {
        case DL_START:  
            m_dl_control.file_total_len = len;
            m_dl_control.data_total = 0;
            break;
         case DL_WRITE:
            m_dl_control.data_total += len;

            if(m_dl_control.data_total > m_dl_control.file_total_len 
                || ((0 == len) && (m_dl_control.data_total < m_dl_control.file_total_len)))
            {
                printf("LOST DATA,data_toal:%d, file_len:%d, len:%d\r\n", m_dl_control.data_total, m_dl_control.file_total_len, len);
                ret = AK_FALSE;
            }
            break;
    }

    return ret;
    */
}

//**********************************************************************
static T_pVOID fhalib_init(T_U32 eMedium, T_U32 eMode, T_pVOID pPhyInfo)
{
    T_FHA_LIB_CALLBACK  pCB;
    T_FHA_INIT_INFO     pInit;
    T_PNANDFLASH        pNandFlash = AK_NULL;
    T_PMEDIUM           pmedium = AK_NULL;
    T_PSPIFLASH         pspiflash = AK_NULL;
    T_SPI_INIT_INFO     spi_fha; 
    T_pVOID             pFwlInfo = AK_NULL;
    T_pVOID             pInfo = AK_NULL;
   
    pInit.nChipCnt   = m_chipInfo.chip_cnt;
    pInit.nBlockStep = 1;
#ifdef CHIP_AK980X    
    pInit.eAKChip = FHA_CHIP_980X;
#endif

#ifdef CHIP_AK37XX
    pInit.eAKChip = FHA_CHIP_37XX;
#endif    
    pInit.ePlatform  = PLAT_SWORD;
    pInit.eMedium    = eMedium;
    pInit.eMode      = eMode;

    if (MEDIUM_NAND == eMedium || MEDIUM_SPI_NAND == eMedium )
    {
        //nand init
        pCB.Erase = FHA_Nand_EraseBlock;
        pCB.Write = FHA_Nand_WritePage;
        pCB.Read  = FHA_Nand_ReadPage;
        pCB.ReadNandBytes = ASA_ReadBytes;

        pNandFlash = Nand_Init((T_NAND_PHY_INFO *)pPhyInfo);
        pFwlInfo   = (T_pVOID)pNandFlash;
        pInfo = pPhyInfo;
    }
    else if (MEDIUM_EMMC == eMedium)
    {
        //SD init
        pCB.Erase  = FHA_SD_Erase;
        pCB.Write  = FHA_SD_Write;
        pCB.Read   = FHA_SD_Read;
        pCB.ReadNandBytes = AK_NULL;

        pmedium = SDDisk_Initial(); 
        pFwlInfo = (T_pVOID)pmedium;
    }
    else if (MEDIUM_SPIFLASH == eMedium)
    {
        //for spi init
        pCB.Erase  = FHA_Spi_Erase;
        pCB.Write  = FHA_Spi_Write;
        pCB.Read   = FHA_Spi_Read;
        pCB.ReadNandBytes = AK_NULL;

        pspiflash = Fwl_SPIFlash_Init((T_SFLASH_PARAM *)pPhyInfo);

        spi_fha.PageSize = pspiflash->page_size;
        spi_fha.PagesPerBlock = pspiflash->PagesPerBlock;
        spi_fha.BinPageStart = SPIBOOTAREA_PAGE_NUM - 1;
        g_write_config_info.Spi_TotalSize = pspiflash->page_size * pspiflash->total_page;

        pFwlInfo = (T_pVOID)pspiflash;
        pInfo    = (T_pVOID)(&spi_fha);
    }
    else
    {
        printf("medium type is err\n");
        return AK_NULL;
    } 
    
    pCB.RamAlloc = Fwl_Malloc; 
    pCB.RamFree  = Fwl_Free;
    pCB.MemSet   = memset;
    pCB.MemCpy   = memcpy;
    pCB.MemCmp   = memcmp;
    pCB.MemMov   = memmove;
    pCB.Printf   = printf;
    
    if (FHA_SUCCESS == FHA_burn_init(&pInit, &pCB, pInfo))
    {
        return pFwlInfo;
    }
    else
    {
        printf("fha lib inits fail\n");
        return AK_NULL;
    }    
}



static T_BOOL SetLibVersion(T_VOID)
{

	T_LIB_VER_INFO Lib_version[sizeof(version_info)/sizeof(version_info[0])];
	T_U32 i;

	for(i = 0; i < sizeof(version_info)/sizeof(version_info[0]); ++i)
	{
		strncpy(Lib_version[i].lib_name,version_info[i].pLibName,
			sizeof(Lib_version[i].lib_name));
		strncpy(Lib_version[i].lib_version,version_info[i].pVerFun(),
			sizeof(Lib_version[i].lib_version));

		printf("%s->%s\n", Lib_version[i].lib_name, 
			Lib_version[i].lib_version);
	}

	i = FHA_set_lib_version(&Lib_version, 
			sizeof(version_info)/sizeof(version_info[0]));	

	return (FHA_SUCCESS == i)? AK_TRUE:AK_FALSE;	
}


static T_BOOL CheckLibVersion(T_VOID)
{
	T_LIB_VER_INFO Lib_version[sizeof(version_info)/sizeof(version_info[0])];
	T_U32 i;
	T_U32 uRet;

	for(i = 0; i < sizeof(version_info)/sizeof(version_info[0]); ++i)
	{
		strncpy(Lib_version[i].lib_name,version_info[i].pLibName,
			sizeof(Lib_version[i].lib_name));
		strncpy(Lib_version[i].lib_version,version_info[i].pVerFun(),
			sizeof(Lib_version[i].lib_version));

		printf("call FHA_check_lib_version\n");
		uRet = FHA_check_lib_version(&(Lib_version[i]));
		if(FHA_FAIL == uRet)
		{
			printf("%s no mathc\n", Lib_version[i].lib_name);
			printf("%s->%s\n", Lib_version[i].lib_name, 
				Lib_version[i].lib_version);

			return AK_FALSE;
		}
	}

	return AK_TRUE;
	
}

T_BOOL Transc_GetBootStart(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    printf("PR_T ++start upload boot++\r\n");
    m_transc_state = TRANS_UPLOAD_BOOT_START;

    return AK_TRUE;
}
T_BOOL Transc_GetBootData(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    //printf("PR_T ++upload boot data++\r\n");
    m_transc_state = TRANS_UPLOAD_BOOT_DATA;

    //Ack_Transc(USB_STATUS_SUCCESS, BT_SUCCESS, DATA_STAGE_SEND, len, result);

    return AK_TRUE;
}

T_BOOL Transc_GetBootLen(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    printf("PR_T ++upload boot data++\r\n");
    m_transc_state = TRANS_UPLOAD_BOOT_LEN;

    //Ack_Transc(USB_STATUS_SUCCESS, BT_SUCCESS, DATA_STAGE_SEND, len, result);

    return AK_TRUE;
 }




T_BOOL Transc_GetBinStart(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    printf("PR_T ++start upload bin++\r\n");
    m_transc_state = TRANS_UPLOAD_BIN_START;

    return AK_TRUE;
}

T_BOOL Transc_GetBinLength(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    //printf("PR_T ++upload bin len:%d++\r\n", len);
    m_transc_state = TRANS_UPLOAD_BIN_LEN;

    return AK_TRUE;
}

T_BOOL Transc_GetBinData(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    //printf("PR_T ++upload bin data++\r\n");
    m_transc_state = TRANS_UPLOAD_BIN_DATA;

    //Ack_Transc(USB_STATUS_SUCCESS, BT_SUCCESS, DATA_STAGE_SEND, len, result);

    return AK_TRUE;
}



T_BOOL Transc_write_otp_serial(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    
    printf("PR_T ++Transc_write_otp_serial++\r\n");
    m_transc_state = TRANS_WRITE_OTP_SERIAL;

    return AK_TRUE;
}


T_BOOL Transc_read_otp_serial(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    
    printf("PR_T ++Transc_read_otp_serial++\r\n");
    m_transc_state = TRANS_READ_OTP_SERIAL;

    return AK_TRUE;
}



static T_WRITE_ASA_FILE* pAsaFile = AK_NULL;
T_BOOL Transc_WriteAsaFile(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    T_U32* pBuf = AK_NULL;
    
    printf("PR_T ++write asa file++\r\n");

    pBuf = (T_U32 *)data;

    if(0 == pBuf[0])
    {
        pAsaFile = Fwl_Malloc(sizeof(T_WRITE_ASA_FILE));

        if(AK_NULL == pAsaFile)
        {
                return AK_FALSE;
        }
        pAsaFile->status = 0;
    }
    else
    {
        pAsaFile->status = 1;
        pAsaFile->mode = (T_U8)pBuf[1];
    }


    return AK_TRUE;
}

static T_BOOL WriteAsaFile(T_U8 data[], T_U32 len)
{
    if(0 == pAsaFile->status)
    {
        memcpy(pAsaFile->fileName, data, sizeof(pAsaFile->fileName));
        printf("asafile name:%s\r\n", pAsaFile->fileName);
    }
    else
    {
        if(ASA_FILE_FAIL == FHA_asa_write_file(pAsaFile->fileName, data, len, pAsaFile->mode))
            return AK_FALSE;
    }

    return AK_TRUE;
}

static T_BOOL Transc_Reset(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
    void (*f)() = 0;
    T_U32 i, j;
        
    printf("T ++reset++\r\n");
    
    ausb_disable();

    for(i = 0; i < 1000*1000; i++)
    {
        j = i;
    }
    
    f();
}


 /**
  * @BREIF    transc of set cannel id
  * @AUTHOR   lixingjian
  * @DATE     2012-5-29
  * @PARAM    [in] data buffer
  * @PARAM    [in] length of data
  * @PARAM    [out] handle result
  * @RETURN   T_BOOL
  * @retval   AK_TRUE :  succeed
  * @retval   AK_FALSE : fail
  */
 
T_BOOL Transc_SetChannel_ID(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
	T_U32* pBuf = AK_NULL;

	 pBuf = (T_U32 *)data;
	
    printf("++set cannel id:%d ++\r\n", pBuf[0]);
    m_transc_channelID = pBuf[0];
    printf("++set cannel id:%d ++\r\n", m_transc_channelID);
    
    return AK_TRUE;
}


 /**
 * @BREIF    transc of get cannel id
 * @AUTHOR   lixingjian
 * @DATE     2012-5-29
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
 
T_BOOL GetChannel_ID(T_U8 data[], T_U32 len)
{
	T_U32* pBuf = AK_NULL;
	
	memcpy(data, &m_transc_channelID, sizeof(m_transc_channelID));
    printf("++get cannel id:%d,%d,%d,%d ++\r\n", data[0], data[1], data[2], data[3]);
    
    return AK_TRUE;
}

/**
 * @BREIF    transc of get cannel id
 * @AUTHOR   lixingjian
 * @DATE     2012-5-29
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
 
T_BOOL Transc_GetChannel_ID(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
	T_U32* pBuf = AK_NULL;
	printf("++get cannel id:%d ++\r\n", m_transc_channelID);
    m_transc_state = TRANS_GET_CHANNEL_ID;
    
    return AK_TRUE;
}


/**
 * @BREIF   read asa file
 * @AUTHOR   lixingjian
 * @DATE     2012-5-29
 * @PARAM    [in] data buffer
 * @PARAM    [in] length of data
 * @PARAM    [out] handle result
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
 
T_BOOL Transc_ReadAsaFile(T_U8 data[], T_U32 len, T_CMD_RESULT *result)
{
  	T_U32* pBuf = AK_NULL;
    
    printf("++read asa file++\r\n");

    pBuf = (T_U32 *)data;

    if(0 == pBuf[0])
    {
        pAsaFile = Fwl_Malloc(sizeof(T_WRITE_ASA_FILE));

        if(AK_NULL == pAsaFile)
        {
		    printf("++read asa file fail ++\r\n");		
            return AK_FALSE;
        }	
       pAsaFile->status = 0;
    }
    else
    {
        pAsaFile->status = 1;
    }
   
    return AK_TRUE;
}


static T_BOOL ReadAsa_getfilename(T_U8 data[], T_U32 len)
{
	printf("++read asa file++\r\n");		
	if(0 == pAsaFile->status)
    {	
		printf("++read asa fil++: %s\r\n",data);		
        memcpy(pAsaFile->fileName, data, sizeof(pAsaFile->fileName));
        printf("asafile name:%s\r\n", pAsaFile->fileName);
    }
    return AK_TRUE;
}

static T_BOOL ReadAsa_senddata(T_U8 data[], T_U32 len)
{
	printf("++read asa file++ \r\n");		
	if(1 == pAsaFile->status)	
    {	
		printf("++read asa file++: %s\r\n", pAsaFile->fileName);		
        if(ASA_FILE_FAIL == FHA_asa_read_file(pAsaFile->fileName, data, len))
        {
			printf("++read asa file fail ++\r\n");		
            return AK_FALSE;
        }	
    }

    return AK_TRUE;
}






