#include "fwl_emmc.h"
#include "arch_mmc_sd.h"
#include "fha.h"

static T_pCARD_HANDLE m_hSD; 

static T_U32 SDDisk_Read(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size)
{
    if(!sd_read_block(m_hSD, sector, buf, size))
    {
        return 0;
    }
    else
    {
        return size;
    } 
}

static T_U32 SDDisk_Write(T_PMEDIUM medium, const T_U8* buf, T_U32 sector, T_U32 size)
{
    if(!sd_write_block(m_hSD, sector, buf, size))
    {
        return 0;
    }
    else
    {
        return size;
    }    
}

static T_BOOL SDDisk_Flush(T_PMEDIUM medium)
{
    return AK_TRUE;
}

T_PMEDIUM SDDisk_Initial(T_VOID)
{      
    T_U32 capacity = 0, BytsPerSec = 0, i;
    T_PMEDIUM medium;

    m_hSD = sd_initial(INTERFACE_SDMMC8, USE_FOUR_BUS);

    if (AK_NULL == m_hSD)
    {
        printf("SD Init mmc fail,try to init sdio\r\n");
        m_hSD = sd_initial(INTERFACE_SDIO, USE_FOUR_BUS);
    }    

    if (AK_NULL == m_hSD)
    {
        printf("SD Init Error\r\n");
        return AK_NULL;
    }
    else
    {
        sd_get_info(m_hSD, &capacity, &BytsPerSec);
    }    
    
    medium = (T_PMEDIUM)Fwl_Malloc(sizeof(T_MEDIUM));
    if(medium == AK_NULL)
    {
        printf("SD Malloc Error\r\n");
        return AK_NULL;
    }

    i = 0;
    while (BytsPerSec > 1)
    {
        BytsPerSec >>= 1;
        i++;
    }

    medium->SecBit = i;        
	medium->PageBit  = i + 5;
    medium->SecPerPg = 5;
    ((T_POBJECT)medium)->destroy = (F_DESTROY)Medium_Destroy;
    ((T_POBJECT)medium)->type = TYPE_MEDIUM;
    medium->read = SDDisk_Read;
    medium->write = SDDisk_Write;
    medium->flush = SDDisk_Flush;
    medium->capacity = capacity;
    medium->type = MEDIUM_SD;
    medium->msg = AK_NULL;

    return medium;
}

//**************************************************************************************************************************

T_U32 FHA_SD_Erase(T_U32 nChip,  T_U32 nPage)
{
    return 0;  
}

T_U32 FHA_SD_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType)
{  
    if(sd_read_block(m_hSD, nPage, pData, nDataLen))
    {
        return FHA_SUCCESS;
    }
    else
    {
        return FHA_FAIL;
    }  
}

T_U32 FHA_SD_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType)
{
    if(sd_write_block(m_hSD, nPage, pData, nDataLen))
    {
        return FHA_SUCCESS;
    }
    else
    {
        return FHA_FAIL;
    }    
}

