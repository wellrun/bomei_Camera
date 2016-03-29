/************************************************************************
 * Copyright (c) 2011, Anyka Co., Ltd. 
 * All rights reserved.    
 *  
 * File Name：
 * Function：this file is for spi flash read.
 *
 * Author：luheshan
**************************************************************************/
#include "anyka_types.h"

#ifdef SPIBOOT
#include "fwl_spiflash.h"
#include "hal_spiflash.h"
#include "mem_api.h"
#include "nand_list.h"
#include "fha.h"
#include <string.h>
#include "drv_gpio.h"


//#define SPI_TEST
//#define SPI_TEST2
//#define     SPI_test

extern  T_U32 spiflash_param;

extern T_VOID AkDebugOutput(const T_U8 *s, ...);
extern T_VOID  Printf_UC(T_pCWSTR ustr);
/***********************************
 * spi flash list
 *
 **********************************/
T_SFLASH_STRUCT pSflash;

T_BOOL Fwl_SPIFlash_Init(T_VOID)
{    
    if (!spi_flash_init(SPI_ID0,SPI_BUS4))
    {
        return AK_FALSE;
    }     
    
    memcpy(&pSflash, &spiflash_param, sizeof(T_SFLASH_STRUCT));
#if 0
{
    T_U32 i,test;
    T_U32 *ptest;
    AkDebugOutput("@#@#@#@#@#%x,%x\n",&spiflash_param,spiflash_param);
    for(i=0;i<7;i++)
    {
        ptest = (T_U32*)&spiflash_param + i;
        memcpy(&test,ptest,sizeof(T_U32));
        AkDebugOutput("0x%x,",test);
    }
}
#endif
    //set param
    spi_flash_set_param(&pSflash.param);

#ifdef SPI_TEST   
    spi_read_test();
    while(1);
#endif
    return AK_TRUE;

}


E_SPIFLASHERRORCODE Fwl_SPIFlash_WritePage(T_U32 page, const T_U8 data[], T_U32 page_cnt)
{  
    if (!spi_flash_write(page, data,page_cnt))
    {
        return SF_FAIL;
    }    
   
    return SF_SUCCESS;
}

E_SPIFLASHERRORCODE Fwl_SPIFlash_ReadPage(T_U32 page, T_U8 *data, T_U32 page_cnt)
{
    if (!spi_flash_read(page, data, page_cnt))
    {
        return SF_FAIL;
    }   
    
    return SF_SUCCESS;
}

E_SPIFLASHERRORCODE  Fwl_SPIFlash_EraseBlock(T_U32 sector)
{
    if (!spi_flash_erase(sector))
    {
        return SF_FAIL;
    }   
        
    return SF_SUCCESS;
}

T_SFLASH_STRUCT Fwl_SPIFlash_GetSPI_Inio(T_VOID)
{
    return pSflash;
}

T_BOOL Fwl_SPIFlash_ReadCfg(T_FILE_CONFIG *pFikeConfig, const T_U8 name[])
{
    T_SFLASH_STRUCT spi_inio;
    T_FILE_CONFIG *pFileCfg = AK_NULL;
    T_U8 *buf =AK_NULL;
    T_U32  i = 0;
    T_U8 file_cnt = 0;
    T_U8 *pFileName = AK_NULL;
    T_BOOL bMatch;

    spi_inio = Fwl_SPIFlash_GetSPI_Inio();
    
    buf = (T_U8 *)Fwl_Malloc(spi_inio.param.page_size);
    if (AK_NULL == buf)
    {
        AkDebugOutput("ERR:LHS:SPI malloc %d fail\n",spi_inio.param.page_size);
        return AK_FALSE;
    }
    
    memset(buf,0,spi_inio.param.page_size);   
    
    if (!Fwl_SPIFlash_ReadPage(SPI_CFG_PAGE,buf,1))
    {
        AkDebugOutput("ERR:LHS:SPI read cfg fail\n");
        buf = Fwl_Free(buf);
        return AK_FALSE;
    }
    
    memcpy(&file_cnt, buf, sizeof(file_cnt));
        
    for(i = 0; i < file_cnt; i++)
    {
        pFileCfg = (T_FILE_CONFIG *)(buf + 4 + i * sizeof(T_FILE_CONFIG) / sizeof(T_U8));
        pFileName = pFileCfg->file_name;

        if (strcmp(pFileName, name) == 0)
        bMatch = AK_TRUE;
        else
                bMatch = AK_FALSE;

        if(bMatch)
        {
              memcpy(pFikeConfig, pFileCfg, sizeof(T_FILE_CONFIG));
            buf = Fwl_Free(buf);
            return AK_TRUE;
        }
    }
    
    AkDebugOutput("ERR:LHS:Cannot find %s\r\n",name);
    buf = Fwl_Free(buf);
    return AK_FALSE;
    
}

T_FILE_CURRENT *Fwl_SPI_FileOpen(T_pCSTR filename)
{
    T_FILE_CONFIG *pFileCfg;
    T_FILE_CURRENT* pTempFile;
    T_SFLASH_STRUCT spi_inio;

    spi_inio = Fwl_SPIFlash_GetSPI_Inio();

    pFileCfg = (T_FILE_CONFIG *)Fwl_Malloc(sizeof(T_FILE_CONFIG));
    if (AK_NULL == pFileCfg)
    {
        AkDebugOutput("ERR:LHS: SPI Open File Malloc Fail\n");
        return AK_NULL;
    }
    if (!Fwl_SPIFlash_ReadCfg(pFileCfg, filename))
    {
        AkDebugOutput("ERR:LHS: SPI Open File Read Cfg Fail\n");
        pFileCfg = Fwl_Free(pFileCfg);
        return AK_NULL;
    }
    
    pTempFile = (T_FILE_CURRENT *)Fwl_Malloc(sizeof(T_FILE_CURRENT));
    if (AK_NULL != pTempFile)
    {
        pTempFile->offset = 0;
        pTempFile->fileCfg.start_page = pFileCfg->start_page;
        pTempFile->fileCfg.file_length = pFileCfg->file_length;
        pTempFile->fileCfg.ld_addr = pFileCfg->ld_addr;
        memcpy(pTempFile->fileCfg.file_name, pFileCfg->file_name, 16); //Utl_StrCpyN
        
        AkDebugOutput("open file :%s file length :%d \r\n",  pFileCfg->file_name,pTempFile->fileCfg.file_length);    
        pFileCfg = Fwl_Free(pFileCfg);
        return pTempFile;    
    }
    else
    {
        AkDebugOutput("\r\nERR:LHS: Open File Malloc error...\r\n");
        pFileCfg = Fwl_Free(pFileCfg);
        return AK_NULL;
    }
}

T_U32  Fwl_SPI_GetFileLen(T_FILE_CURRENT * pFile)
{
    if (pFile != AK_NULL)
        return pFile->fileCfg.file_length;
    return 0;
}

T_S32   Fwl_SPI_FileSeek(T_FILE_CURRENT *pFile, T_S32 offset, T_U16 origin)
{
    T_S32 ret = -1;
    T_U32 fileLen = 0;

    if (pFile == AK_NULL)
    {
        return ret;
    }
    fileLen = pFile->fileCfg.file_length;
    switch (origin)
    {
        case SPI_FSEEK_SET:
            if (offset > fileLen)
            {
               pFile->offset = fileLen;
            }
            else
            {
               pFile->offset = offset;
            }
            ret = pFile->offset;
            break;
        case SPI_FSEEK_CUR:
            if ((offset + pFile->offset) > fileLen)
            {
               pFile->offset = fileLen;
            }
            else
            {
               pFile->offset += offset;
            }
            ret = pFile->offset;
            break;
        case SPI_FSEEK_END:
            pFile->offset = fileLen;
            ret = pFile->offset;
            break;
        default:
            break;
    }
    return ret;
}


T_S32   Fwl_SPI_FileRead(T_FILE_CURRENT *pFile, T_U8* buffer, T_U32 count)
{
    T_SFLASH_STRUCT spi_inio;
    T_U8* pDATA = AK_NULL;
    T_S32 ret = -1;
    T_U32 page = 0;
    T_U32 page_cnt = 0;
    T_U32 addr = 0;
    T_U32 page_read_32k = 128;//one times read 32K
    T_U32 page_read_cnt;
    T_U32 start_add_dst=0;
    T_U32 start_add_src=0;
    
    if ((AK_NULL == buffer)||(AK_NULL == pFile))
    {
        return ret;
    }
    if (pFile->offset > pFile->fileCfg.file_length)
    {
        return ret;
    }

    spi_inio = Fwl_SPIFlash_GetSPI_Inio();
    
    pDATA = (T_U8*)Fwl_Malloc(spi_inio.param.page_size);
    if (pDATA == AK_NULL)
    {
        return ret;
    }
    
    if ((pFile->offset+count) > pFile->fileCfg.file_length)
    {
        count = pFile->fileCfg.file_length - pFile->offset;
    }
    
    page = pFile->fileCfg.start_page + pFile->offset/spi_inio.param.page_size;

    start_add_src = pFile->offset%spi_inio.param.page_size;
    if ((start_add_src+count) < spi_inio.param.page_size)
    {
        addr = 0;
    }
    else
    {
        addr = (count+start_add_src)%spi_inio.param.page_size;//pFile->offset==(pFile->offset%spi_inio.param.page_size)
    }
        
    page_cnt = (count+start_add_src)/spi_inio.param.page_size; //count+pFile->offset 
    if (0 == page_cnt)
    {
        page_cnt = 1;
    }
    else if (1 <= page_cnt)
    {
        if (0 != addr)
        {
            page_cnt++;
        }
    }
#ifdef SPI_TEST2        
    AkDebugOutput("\n##%d,%d,%d,%d,%d,%d,%d\n",page,addr,count,page_cnt,start_add_src,pFile->offset,pFile->fileCfg.file_length);
#endif

    if ((0 != start_add_src) || (1 == page_cnt))
    {
        Fwl_SPIFlash_ReadPage(page,pDATA,1);
        if (spi_inio.param.page_size <= (count+start_add_src))
        {            
            memcpy(buffer,&pDATA[start_add_src],(spi_inio.param.page_size-start_add_src));
            start_add_dst += (spi_inio.param.page_size-start_add_src);
        }
        else
        {
            memcpy(buffer,&pDATA[start_add_src],count);
            start_add_dst += count;
        }
        page_cnt--;
        page++;
    }

    if ((0 != addr) && (1 <= page_cnt))
    {
        page_cnt--;
    }
    page_read_cnt = page_cnt/page_read_32k;
    while(page_read_cnt--)
    {
        Fwl_SPIFlash_ReadPage(page,&buffer[start_add_dst],page_read_32k);
        //memcpy(&buffer[start_add_dst],pDATA,buf_size);
        start_add_dst += page_read_32k*spi_inio.param.page_size;
        page_cnt -= page_read_32k;
        page += page_read_32k;
    }
    page_read_cnt = page_cnt%page_read_32k;
    while(page_read_cnt--)
    {
        Fwl_SPIFlash_ReadPage(page,&buffer[start_add_dst],1);
        //memcpy(&buffer[start_add_dst],pDATA,spi_inio.param.page_size);
        start_add_dst += spi_inio.param.page_size;
        page_cnt--;
        page ++;
    }
    
    if ((0 != addr) && (0 == page_cnt))
    {            
        Fwl_SPIFlash_ReadPage(page,pDATA,1);
        memcpy(&buffer[start_add_dst],pDATA,addr);
        start_add_dst += addr;
        page ++;
    }    
    
    pDATA = Fwl_Free(pDATA);
    
    if (start_add_dst == count)
    {
        pFile->offset += count;
    }
    else
    {
        AkDebugOutput("ERR:LHS:SPI read %s fail..P:%d,C:%d,S:%d,F:%d\n",pFile->fileCfg.file_name,page,count,start_add_src,pFile->offset);
        start_add_dst = 0;
    }
#ifdef SPI_TEST2        
    AkDebugOutput("num:%d\n",start_add_dst);
#endif
    return start_add_dst;
}

T_BOOL  Fwl_SPI_FileClose(T_FILE_CURRENT **pFile)
{
    T_BOOL  ret = AK_FALSE;
    

    if (AK_NULL != *pFile);
    {

        AkDebugOutput("spi close file:");
        Printf_UC((*pFile)->fileCfg.file_name);
        Fwl_Free(*pFile);
        *pFile = AK_NULL;
        ret = AK_TRUE;
    }
    return ret;
}



static T_U32 CB_FHA_Spi_Erase(T_U32 nChip,  T_U32 nPage)
{
    T_U32 PagesPerBlock = pSflash.param.erase_size / pSflash.param.page_size;
    T_U32 total_page = pSflash.param.total_size / pSflash.param.page_size;
    T_U32 nBlock = nPage / PagesPerBlock;

    if ((nBlock + 1) * PagesPerBlock >= total_page)
    {
        AkDebugOutput("erase sector %d over the max space\r\n", nBlock);
        return SF_FAIL;
    } 

    if (!spi_flash_erase(nBlock))
    {
        return FHA_FAIL;
    }   
        
    return FHA_SUCCESS;
}

static T_U32 CB_FHA_Spi_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType)
{ 
    T_U32 i;    
    T_U32 total_page = pSflash.param.total_size / pSflash.param.page_size;
    T_BOOL bRet;

    if ((nPage + nDataLen) >= total_page)
    {
        AkDebugOutput("read page %d over the max space\r\n", (nPage + nDataLen));
        return SF_FAIL;
    } 

    for (i = 0; i < nDataLen; i++)
    {
        
        
        bRet = spi_flash_read(nPage + i, pData + i * pSflash.param.page_size, 1);


        if (!bRet)
        {
            return FHA_FAIL;
        }
        
    }

    return FHA_SUCCESS;
}

static T_U32 CB_FHA_Spi_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType)
{    
    T_U32 total_page = pSflash.param.total_size / pSflash.param.page_size;
    T_BOOL bRet;


    
    if ((nPage + nDataLen) >= total_page)
    {
        AkDebugOutput("write page %d over the max space\r\n", (nPage + nDataLen));
        return SF_FAIL;
    } 

        

    bRet = spi_flash_write(nPage, pData, nDataLen);
    
    
    if (!bRet)
    {
        return FHA_FAIL;
    }    
   
    return FHA_SUCCESS;
}


static T_pVOID CB_FHA_Malloc(T_U32 size)
{
   return Fwl_MallocAndTrace((size), ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}


static T_pVOID CB_FHA_Free(T_pVOID var)
{
    return Fwl_FreeAndTrace(var, ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}
 
T_BOOL SpiInitPreUpdate(T_VOID)
{
    T_FHA_LIB_CALLBACK  stFhaLibCB;
    T_FHA_INIT_INFO     stFhaLibInitInfo;     
    T_SPI_INIT_INFO     spi_fha; 

    stFhaLibInitInfo.nChipCnt      = 1;
    stFhaLibInitInfo.nBlockStep = 1;

    stFhaLibInitInfo.eAKChip = FHA_CHIP_37XX;

    stFhaLibInitInfo.ePlatform  = PLAT_SWORD;
    stFhaLibInitInfo.eMedium      = MEDIUM_SPIFLASH;
    stFhaLibInitInfo.eMode   = MODE_NEWBURN;


    stFhaLibCB.Erase  = CB_FHA_Spi_Erase;
    stFhaLibCB.Write  = CB_FHA_Spi_Write;
    stFhaLibCB.Read   = CB_FHA_Spi_Read;
    stFhaLibCB.ReadNandBytes = AK_NULL;

    stFhaLibCB.RamAlloc = CB_FHA_Malloc; 
    stFhaLibCB.RamFree  = CB_FHA_Free;
    stFhaLibCB.MemSet     = memset;
    stFhaLibCB.MemCpy     = memcpy;
    stFhaLibCB.MemCmp     = memcmp;
    stFhaLibCB.MemMov     = memmove;
    stFhaLibCB.Printf     = AkDebugOutput;


    spi_fha.PageSize = pSflash.param.page_size;
    spi_fha.PagesPerBlock = pSflash.param.erase_size / pSflash.param.page_size;
    spi_fha.BinPageStart = SPI_CFG_PAGE - 1;

    if (FHA_SUCCESS == 
         FHA_burn_init(&stFhaLibInitInfo, &stFhaLibCB, (T_pVOID)&spi_fha))
    {
        return AK_TRUE;
    }
    else
    {
        AkDebugOutput("fha lib inits fail\n");
        return AK_FALSE;
    }         
}

T_BOOL Fwl_SPI_Fha_Init(T_VOID)
{
    T_FHA_LIB_CALLBACK  stFhaLibCB;
    T_FHA_INIT_INFO     stFhaLibInitInfo;     
    T_SPI_INIT_INFO     spi_fha; 

    stFhaLibInitInfo.nChipCnt      = 1;
    stFhaLibInitInfo.nBlockStep = 1;

    stFhaLibInitInfo.eAKChip = FHA_CHIP_37XX;

    stFhaLibInitInfo.ePlatform  = PLAT_SWORD;
    stFhaLibInitInfo.eMedium      = MEDIUM_SPIFLASH;
    stFhaLibInitInfo.eMode   = MODE_UPDATE;


    stFhaLibCB.Erase  = CB_FHA_Spi_Erase;
    stFhaLibCB.Write  = CB_FHA_Spi_Write;
    stFhaLibCB.Read   = CB_FHA_Spi_Read;
    stFhaLibCB.ReadNandBytes = AK_NULL;

    stFhaLibCB.RamAlloc = CB_FHA_Malloc; 
    stFhaLibCB.RamFree  = CB_FHA_Free;
    stFhaLibCB.MemSet     = memset;
    stFhaLibCB.MemCpy     = memcpy;
    stFhaLibCB.MemCmp     = memcmp;
    stFhaLibCB.MemMov     = memmove;
    stFhaLibCB.Printf     = AkDebugOutput;


    spi_fha.PageSize = pSflash.param.page_size;
    spi_fha.PagesPerBlock = pSflash.param.erase_size / pSflash.param.page_size;
    spi_fha.BinPageStart = SPI_CFG_PAGE - 1;

    if (FHA_SUCCESS == 
         FHA_burn_init(&stFhaLibInitInfo, &stFhaLibCB, (T_pVOID)&spi_fha))
    {
        return AK_TRUE;
    }
    else
    {
        AkDebugOutput("fha lib inits fail\n");
        return AK_FALSE;
    }         
}


T_BOOL  Spi_write_bin_begin(T_U32 data_length, T_U32 ld_addr, T_U8 *file_name,
    T_BOOL bBackup, T_BOOL bCheck, T_BOOL bUpdateSelf)
{
    T_FHA_BIN_PARAM   FahBinParam;

    FahBinParam.data_length = data_length;
    FahBinParam.ld_addr = ld_addr;
    FahBinParam.bBackup = bBackup;       
    FahBinParam.bCheck = bCheck;
    FahBinParam.bUpdateSelf = bUpdateSelf;
    memcpy(FahBinParam.file_name ,file_name, sizeof(FahBinParam.file_name));

    return (FHA_SUCCESS == FHA_write_bin_begin(&FahBinParam)? AK_TRUE : AK_FALSE);
}


T_BOOL  Spi_write_bin(const T_U8 * pData,  T_U32 data_len)
{        
    return (FHA_SUCCESS == FHA_write_bin(pData, data_len)? AK_TRUE : AK_FALSE);
}


T_BOOL  Spi_write_boot_begin(T_U32 bin_len)
{    
    return (FHA_SUCCESS == FHA_write_boot_begin(bin_len)? AK_TRUE : AK_FALSE);
}


T_BOOL  Spi_write_boot(const T_U8 *pData,  T_U32 data_len)
{
    return (FHA_SUCCESS == FHA_write_boot(pData,  data_len)? AK_TRUE : AK_FALSE);    
}


T_BOOL  Spi_FHA_close(T_VOID)
{
    return (FHA_SUCCESS == FHA_close() ? AK_TRUE : AK_FALSE);
}



T_BOOL  SetBootCodeSpiParam(T_U8 *pData, T_U32 len)
{
    if(len < sizeof(pSflash))
    {
        return AK_FALSE;
    }
    
    memcpy(pData, &pSflash, sizeof(pSflash));

    return AK_TRUE;
}

//获取剩余的空闲块数量
T_S32 Fwl_SPI_Get_Spare_Block_Num(T_VOID)
{
    T_SFLASH_STRUCT spi_inio;
    T_U8 * pCfgBuf;
	T_U32 iFileNum;
	T_U32 iFilePos;
	T_U32  iLastPage,iLastFileStartPage;
	T_U32 iLastBlock ,iSpareBlock;
	T_FILE_CONFIG  fileCfg;
	

    spi_inio = Fwl_SPIFlash_GetSPI_Inio();
    {
    	T_SFLASH_PARAM param = spi_inio.param;
    	AkDebugOutput("spi info :total size=%d , page size=%d,erase_size=%d\n",param.total_size
    		,param.page_size,param.erase_size);
    }
    pCfgBuf = (T_U8 *)Fwl_Malloc(spi_inio.param.page_size);
    if (AK_NULL ==pCfgBuf)
    	return -1;
	
	if (SF_SUCCESS != Fwl_SPIFlash_ReadPage(SPI_CFG_PAGE, pCfgBuf, 1))
	{
		Fwl_Free(pCfgBuf);
		return -1;
	}
		
	memcpy(&iFileNum, pCfgBuf, sizeof(iFileNum));
	//取最后一个文件的信息

	if (iFileNum == 0)
	{
		iLastPage = SPI_CFG_PAGE;
	}else
	{
		iFilePos = sizeof(iFileNum) + sizeof(fileCfg) * (iFileNum -1);
			
		memcpy(&fileCfg , iFilePos + pCfgBuf , sizeof(fileCfg));
		AkDebugOutput("spi info: last file ,name=%s,file length =%d, start_page=%d,back_page=%d\n"
			, fileCfg.file_name,fileCfg.file_length,fileCfg.start_page,fileCfg.backup_page);
		if (fileCfg.backup_page != T_U32_MAX)
			iLastFileStartPage = fileCfg.backup_page;
		else
			iLastFileStartPage = fileCfg.start_page;
				
		iLastPage = SPI_CFG_PAGE + iLastFileStartPage+ fileCfg.file_length /spi_inio.param.page_size
					+ (( fileCfg.file_length % spi_inio.param.page_size==0) ? 0: 1 );
	}
	iLastBlock = iLastPage /spi_inio.param.erase_size 
 		 + ((iLastPage % spi_inio.param.erase_size ==0) ? 0: 1)  ;

	iSpareBlock = spi_inio.param.total_size / spi_inio.param.erase_size -1 - iLastBlock;

	if (AK_NULL != pCfgBuf)
		Fwl_Free(pCfgBuf);
	
	return iSpareBlock;
}

//往最后一块写数据, 保证不破坏烧录的数据
T_BOOL Fwl_Write_Last_Block_SafeLy(T_U8 * pData, T_U32 count)
{
    T_SFLASH_STRUCT spi_inio;
	T_U32 iLastBlock,iFirstPage;
	T_U32 iPageCount;
	T_S32 iSpareBlock;

    spi_inio = Fwl_SPIFlash_GetSPI_Inio();
    if (count > spi_inio.param.erase_size )
    {
    	AkDebugOutput("Fwl_Write_Last_Block:write count=[%d] is larger than on block \n",count);
    	return AK_FALSE;
    }
    
    iSpareBlock = Fwl_SPI_Get_Spare_Block_Num();
    if (iSpareBlock <=0)
    {
    	AkDebugOutput("Fwl_Write_Last_Block:no enough spare block to write \n");
    	return AK_FALSE;
    }
    
    iLastBlock = spi_inio.param.total_size / spi_inio.param.erase_size -1;
    if (SF_SUCCESS !=Fwl_SPIFlash_EraseBlock(iLastBlock))
    	return AK_FALSE;
    	
    iFirstPage = iLastBlock  * spi_inio.param.erase_size /spi_inio.param.page_size;

    if (count % spi_inio.param.page_size==0)
    	iPageCount = count / spi_inio.param.page_size ;
    else
      	iPageCount = count / spi_inio.param.page_size +1;

//	AkDebugOutput("first page=%d,pagecount=%d\n",iFirstPage, iPageCount);
    if ( SF_SUCCESS != Fwl_SPIFlash_WritePage(iFirstPage, pData, iPageCount))
		return AK_FALSE;
		
    return AK_TRUE;
}

T_BOOL Fwl_Read_Last_Block(T_U8 * pData, T_U32 count)
{
    T_SFLASH_STRUCT spi_inio;
	T_U32 iLastBlock,iFirstPage;
	T_U32 iPageCount, iLastCount=0;
	T_U8 * pLastPage;

    spi_inio = Fwl_SPIFlash_GetSPI_Inio();
    if (count > spi_inio.param.erase_size )
    {
    	AkDebugOutput("Fwl_Read_Last_Block:read count=[%d] is larger than on block \n",count);
    	return AK_FALSE;
    }
    
    
    iLastBlock = spi_inio.param.total_size / spi_inio.param.erase_size -1;
    iFirstPage = iLastBlock  * spi_inio.param.erase_size /spi_inio.param.page_size;

	iLastCount = count % spi_inio.param.page_size;
   	iPageCount = count / spi_inio.param.page_size ;

//	AkDebugOutput("first page=%d,pagecount=%d\n",iFirstPage, iPageCount);

	//读整页
	if (iPageCount > 0)
	{
	    if ( SF_SUCCESS != Fwl_SPIFlash_ReadPage(iFirstPage, pData, iPageCount))
	    	return AK_FALSE;
    }

	//读非整页
	if (iLastCount >0)
	{
		pLastPage = Fwl_Malloc(spi_inio.param.page_size);
		
		if (SF_SUCCESS !=Fwl_SPIFlash_ReadPage(iFirstPage + iPageCount , pLastPage, 1))	
		{
			Fwl_Free(pLastPage);
			return AK_FALSE;
		}		
		memcpy(pData+iPageCount*spi_inio.param.page_size, pLastPage, iLastCount);
		Fwl_Free(pLastPage);
	}
    return AK_TRUE;

}








#ifdef SPI_TEST
spi_read_test()
{
    T_FILE_CURRENT *test;
    T_U8 *buf;
    T_U8 *buf2;
    T_U32 a,b,c,d,e,i;
    T_U32 buf_size = 256;
    test = Fwl_SPI_FileOpen("BIOS");
    AkDebugOutput("file leng:%d\n",Fwl_SPI_GetFileLen(test));
    buf_size=test->fileCfg.file_length;
    buf = (T_U8*)Fwl_Malloc(buf_size);
    Fwl_SPI_FileSeek(test,0,SPI_FSEEK_SET);
#if 1
        *(volatile T_U32 *)0x08000078 &= ~(1<<10);
        *(volatile T_U32 *)0x0800007C &= ~(1<<13);//gpio13 dir is output
        *(volatile T_U32 *)0x08000080 &= ~(1<<13);//gpio13 output 0 
#endif
    a=get_tick_count();
    Fwl_SPI_FileRead(test,buf,buf_size);
    AkDebugOutput("T:%d\n",get_tick_count()-a);
#if 1    
    *(volatile T_U32 *)0x0800007C &= ~(1<<13);//gpio13 dir is output
    *(volatile T_U32 *)0x08000080 |= (1<<13);//gpio13 output 1 
    while(1);
#endif

#if 0    
    AkDebugOutput("\n 1 \n");
    Fwl_SPI_FileRead(test,buf,50);
    Fwl_SPI_FileSeek(test,0,SPI_FSEEK_SET);
    AkDebugOutput("\n 2 \n");
    Fwl_SPI_FileRead(test,buf,256);
    Fwl_SPI_FileSeek(test,0,SPI_FSEEK_SET);
    AkDebugOutput("\n 3 \n");
    Fwl_SPI_FileRead(test,buf,500);
    Fwl_SPI_FileSeek(test,0,SPI_FSEEK_SET);
    AkDebugOutput("\n 4 \n");
    Fwl_SPI_FileRead(test,buf,50);
    AkDebugOutput("\n 5 \n");
    Fwl_SPI_FileRead(test,buf,256);
    AkDebugOutput("\n 6 \n");
    Fwl_SPI_FileRead(test,buf,260);

    Fwl_SPI_FileSeek(test,0,SPI_FSEEK_SET);
    Fwl_SPI_FileSeek(test,50,SPI_FSEEK_SET);
    AkDebugOutput("\n 7 \n");
    Fwl_SPI_FileRead(test,buf,207);
    AkDebugOutput("\n 8 \n");
    Fwl_SPI_FileSeek(test,0,SPI_FSEEK_SET);    
    AkDebugOutput("\n 9 \n");
    Fwl_SPI_FileSeek(test,5,SPI_FSEEK_SET);    
    AkDebugOutput("###### %d\n",test->offset);
    Fwl_SPI_FileRead(test,buf,10);
#endif    
#if 0
    AkDebugOutput("SPI test read\n");
    a=Fwl_SPI_GetFileLen(test);
    Fwl_SPI_FileSeek(test,0,SPI_FSEEK_SET);
    b=0;
    c=0;
    while(a-b)
    {
        c +=1;
        if (500 == c)
        {
            c=1;
        }
        d = Fwl_SPI_FileRead(test,buf,c);
        if (d != c)
        {
            AkDebugOutput("read fail %d,%d\n",c,d);
            while(1);
        }
        else
        {
            b=b+d;
        }
    }
    AkDebugOutput("sdfsdfsdf %d,%d,%d,%d\n",a,b,c,d);
    Fwl_SPI_FileSeek(test,0,SPI_FSEEK_SET);
    AkDebugOutput("set:%d\n",test->offset);
    while(!((a-test->offset)<10))
    {
        Fwl_SPI_FileSeek(test,5,SPI_FSEEK_SET);
        e++;
    }
    AkDebugOutput("sadfgghhj:%d,%d,%d\n",e,a/5,test->offset);
#endif    
#if 0
    AkDebugOutput("SPI test read and seek\n");
    buf2 = (T_U8*)Fwl_Malloc(buf_size);
    a=Fwl_SPI_GetFileLen(test);
    Fwl_SPI_FileSeek(test,0,SPI_FSEEK_SET);
    b=0;
    c=buf_size-300;
    while(a-b)
    {
        c +=1;
        if (buf_size == c)
        {
            c=1;
        }
        memset(buf,0,buf_size);
        memset(buf2,0,buf_size);
        d = Fwl_SPI_FileRead(test,buf,c);
        if (((test->offset+c)>a)&&(d!=0)) c=d;
        if (d == c)
        {
            e = test->offset;
            Fwl_SPI_FileSeek(test,0,SPI_FSEEK_SET);
            Fwl_SPI_FileSeek(test,(e-c),SPI_FSEEK_SET);
            d = Fwl_SPI_FileRead(test,buf2,c);
            if (memcmp(buf,buf2,c) != 0)
            {
                for(i=0;i<c;i++)
                {
                    AkDebugOutput("B1:0x%x ",buf[i]);
                    AkDebugOutput(",B2:0x%x\n",buf2[i]);
                }
                AkDebugOutput("\n******%d,%d,%d,%d,%d,%d\n",a,b,c,d,e,test->offset);
                while(1);
            }
            b=b+d;
            if ((a-b)<=0) break;
        }
        else
        {
            AkDebugOutput("#####33read###3\n");
            while(1);
        }
    }
    AkDebugOutput("\nOK OK\n");
#endif
#if 0
{    
    T_U32 j=0;
    T_U32 start_add_dst=0;
    buf2 = (T_U8*)Fwl_Malloc(buf_size*2);
    memset(buf2,0,buf_size*2);
    for(i=0;i<2;i++)
    {
        memset(buf,0,buf_size);
        Fwl_SPIFlash_ReadPage(test->fileCfg.start_page+i,buf,1);
        memcpy(&buf2[start_add_dst],buf,256);
        start_add_dst += 256;
    }
    for (a=0;a<256*2;a++)
    {
        AkDebugOutput("%d:0x",j);
        if (0x10 > buf2[a]) {AkDebugOutput("0");}
            AkDebugOutput("%x ",buf2[a]);
        if ((j%16) == 0)AkDebugOutput("\n");
        j++;
    }
    
    memset(buf2,0,buf_size*2);
    Fwl_SPI_FileSeek(test,0,SPI_FSEEK_SET);
    Fwl_SPI_FileRead(test,buf2,256*2);
    AkDebugOutput("\nluheshan\n");
    for (a=0;a<256*2;a++)
    {
        AkDebugOutput("%d:0x",j);
        if (0x10 > buf2[a]) {AkDebugOutput("0");}
            AkDebugOutput("%x ",buf2[a]);
        if ((j%16) == 0)AkDebugOutput("\n");
        j++;
    }
    
    while(1);
    memset(buf,0,buf_size);
    Fwl_SPI_FileSeek(test,0,SPI_FSEEK_SET);
    Fwl_SPI_FileRead(test,buf,test->fileCfg.file_length);
    AkDebugOutput("\n\n\nluheshan\n");

    for(a=0;a<test->fileCfg.file_length;a++)
    {
        if (0x10 > buf[a]) {AkDebugOutput("0");}
        AkDebugOutput("%x ",buf[a]);
    }

}
#endif

    Fwl_SPI_FileClose(test);
    

}
#endif

#ifdef SPI_test

#define SFLASH_TEST_SUCCESS                (0)
#define SFLASH_TEST_INIT_FAIL              (-1)
#define SFLASH_TEST_READID_FAIL            (-2)
#define SFLASH_TEST_ERASE_FAIL             (-3)
#define SFLASH_TEST_READ_FAIL              (-4)
#define SFLASH_TEST_WRITE_FAIL             (-5)
#define SFLASH_TEST_ERROR_ID               (-6)
#define SFLASH_TEST_COMPARE_FAIL           (-7)
#define SFLASH_TEST_MALLOC_FAIL            (-8)
#define SFLASH_TEST_FAIL                   (-9)
#define SFLASH_TEST_NOT_TAKE               (-127)

typedef struct
{
    T_S32 result;
    T_U8 msg[64];
}
T_SFLASH_PRINT_INFO;

/***********************************
 * case naming
 * sflash_func_case1 : normal function case
 * sflash_abn_case1  : abnormal function case
 * sflash_pfm_case1  : performance case
 * sflash_cmpt_case1 : compatible case
 * sflash_strs_case1 : stress case
 **********************************/
T_VOID sflash_test();

T_VOID sflash_func_case();
static T_VOID sflash_func_case1();
static T_VOID sflash_func_case2();
static T_VOID sflash_func_case3();
static T_VOID sflash_func_case4();

T_VOID sflash_abn_case();
static T_VOID sflash_abn_case1();
static T_VOID sflash_abn_case2();
static T_VOID sflash_abn_case3();

T_VOID sflash_pfm_case();
static T_VOID sflash_pfm_case1();
static T_VOID sflash_pfm_case2();
static T_VOID sflash_pfm_case3();

static T_VOID cmd_sflash_result();

static T_VOID sflash_print_result(T_U32 case_num, T_S32 result);
static T_S32 sflash_probe(T_SFLASH_STRUCT *pSflash);
static T_S32 sflash_erase_all(T_SFLASH_STRUCT *pSflash, T_U32 bPrint);
static T_S32 sflash_write_all(T_SFLASH_STRUCT *pSflash, T_U32 step, T_U32 bCompare);
static T_S32 sflash_read_all(T_SFLASH_STRUCT *pSflash, T_U32 step, T_BOOL bCompare);


/***********************************
 * spi flash test result
 *
 **********************************/

#define SFLASH_FUNC_CASE_BASE 0
#define SFLASH_ABN_CASE_BASE 4
#define SFLASH_PFM_CASE_BASE 7

#define SFLASH_FUNC_CASE(x)    (SFLASH_FUNC_CASE_BASE+x-1)
#define SFLASH_ABN_CASE(x)    (SFLASH_ABN_CASE_BASE+x-1)
#define SFLASH_PFM_CASE(x)    (SFLASH_PFM_CASE_BASE+x-1)


T_S32 sflash_test_result[] = 
{
//func case
SFLASH_TEST_NOT_TAKE,
SFLASH_TEST_NOT_TAKE,
SFLASH_TEST_NOT_TAKE,
SFLASH_TEST_NOT_TAKE,

//abn case
SFLASH_TEST_NOT_TAKE,
SFLASH_TEST_NOT_TAKE,
SFLASH_TEST_NOT_TAKE,

//pfm case
SFLASH_TEST_NOT_TAKE,
SFLASH_TEST_NOT_TAKE,
SFLASH_TEST_NOT_TAKE
};

T_U32 sflash_pfm[3] = 
{
0,  //erase
0,  //write
0   //read
};

/***********************************
 * print message
 *
 **********************************/
T_SFLASH_PRINT_INFO sflash_test_print[] = 
{
    {SFLASH_TEST_SUCCESS, "SUCCESS"},
    {SFLASH_TEST_INIT_FAIL, "FAIL(init)"},
    {SFLASH_TEST_READID_FAIL, "FAIL(read id)"},
    {SFLASH_TEST_ERASE_FAIL, "FAIL(erase)"},
    {SFLASH_TEST_READ_FAIL, "FAIL(read)"},
    {SFLASH_TEST_WRITE_FAIL, "FAIL(write)"},
    {SFLASH_TEST_ERROR_ID, "ERROR(id)"},
    {SFLASH_TEST_COMPARE_FAIL, "FAIL(compare)"},
    {SFLASH_TEST_MALLOC_FAIL, "FAIL(malloc)"},
    {SFLASH_TEST_NOT_TAKE, "NO TEST"}
};


T_VOID sflash_test()
{
    sflash_func_case();
    sflash_abn_case();
    sflash_pfm_case();
}


/***********************************
 * spi flash static fuction
 *
 **********************************/

static T_VOID sflash_print_result(T_U32 case_num, T_S32 result)
{
    T_U32 i;
    T_U8 error_str[] = "UNKOWN ERROR";
    T_U8 *pStr = error_str;

    //print error message
    for(i = 0; i < sizeof(sflash_test_print)/sizeof(T_SFLASH_PRINT_INFO); i++)
    {
        if(sflash_test_print[i].result == result)
        {
            pStr = sflash_test_print[i].msg;
            break;
        }
    }

    if(case_num >= SFLASH_PFM_CASE_BASE)
    {
        AkDebugOutput("pfm case[%d]: %s\n", case_num - SFLASH_PFM_CASE_BASE + 1, pStr);
    }
    else if(case_num >= SFLASH_ABN_CASE_BASE)
    {
        AkDebugOutput("abn case[%d]: %s\n", case_num - SFLASH_ABN_CASE_BASE + 1, pStr);
    }
    else
    {
        AkDebugOutput("func case[%d]: %s\n", case_num + 1, pStr);
    }
}
 
static T_VOID sflash_handle_result(T_U32 case_num, T_S32 result)
{
    
    //save result 
    if(result >= 0) result = SFLASH_TEST_SUCCESS;
    sflash_test_result[case_num] = result;

    //print result
    sflash_print_result(case_num, result);
}

static T_S32 sflash_probe(T_SFLASH_STRUCT *pSflash)
{
    T_U32 id;
    T_U32 i;
    
    //initial
    //we asume all the spi flash mounted on spi0
    if(!spi_flash_init(SPI_ID0))
    {
        return SFLASH_TEST_INIT_FAIL;
    }

    //readid
    id = spi_flash_getid();
    if(0 == id || 0xFFFFFF == id)
    {
        return SFLASH_TEST_READID_FAIL;
    }

    //look up in sflash list
    for(i = 0; i < sizeof(sflash_list)/sizeof(T_SFLASH_STRUCT); i++)
    {
        if(id == sflash_list[i].id)
        {
            memcpy(pSflash, &sflash_list[i], sizeof(T_SFLASH_STRUCT));

            //set param
            spi_flash_set_param(&pSflash->param);
            
            return SFLASH_TEST_SUCCESS;
        }
    }

    return SFLASH_TEST_ERROR_ID;
}

static T_S32 sflash_erase_all(T_SFLASH_STRUCT *pSflash, T_U32 bPrint)
{
    T_U32 sector_count;
    T_U32 i;
    T_U32 t1, t2;

    //erase all sector
    sector_count = pSflash->capacity / pSflash->param.erase_size;

    AkDebugOutput("start to erase: %d\n", sector_count);

    t1 = get_tick_count();
    
    for(i = 0; i < sector_count; i++)
    {
        if(bPrint)
        {
            AkDebugOutput("<%d>", i);
        }
        
        if(!spi_flash_erase(i))
        {
            return SFLASH_TEST_ERASE_FAIL;
        }
    }

    t2 = get_tick_count();
    
    AkDebugOutput("\nerase end\n");

    return (t2 - t1);
}

static T_S32 sflash_write_all(T_SFLASH_STRUCT *pSflash, T_U32 step, T_U32 bCompare)
{
    T_U32 count, size;
    T_U32 i;
    T_U32 t1, t2;
    
    T_U8 *data;

    //alloc memory
    size = step * pSflash->param.page_size;
    data = (T_U8 *)drv_malloc(size);
    if(AK_NULL == data)
    {
        return SFLASH_TEST_MALLOC_FAIL;
    }

    //write all pages
    count = pSflash->capacity / size;

    AkDebugOutput("start to write: %d\n", count);

    t1 = get_tick_count();

    for(i = 0; i < count; i++)
    {
        if(bCompare)
        {
            AkDebugOutput("<%d>", i);
            memset(data, i&0xFF, size);
        }
        
        //write
       if(!spi_flash_write(i*step, data, step))
        {
            drv_free(data);
            return SFLASH_TEST_WRITE_FAIL;
        }
    }

    t2 = get_tick_count();

    AkDebugOutput("\nwrite end\n");

    drv_free(data);
    
    return (t2 - t1);
}

static T_S32 sflash_read_all(T_SFLASH_STRUCT *pSflash, T_U32 step, T_BOOL bCompare)
{
    T_U32 count, size;
    T_U32 start;
    T_U32 i, j;
     T_U32 t1, t2;
   
    T_U8 *data;

    //alloc memory
    size = step * pSflash->param.page_size;
    data = (T_U8 *)drv_malloc(size);
    if(AK_NULL == data)
    {
        return SFLASH_TEST_MALLOC_FAIL;
    }

    //read and compre pages
    count = pSflash->capacity / size;

    AkDebugOutput("start to read: %d\n", count);

    t1 = get_tick_count();

    for(i = 0; i < count; i++)
    {
        if(bCompare)
        {
            AkDebugOutput("<%d>", i);
            memset(data, 0, size);
        }
        
        //read
        if(!spi_flash_read(i*step, data, step))
        {
            drv_free(data);
            return SFLASH_TEST_READ_FAIL;
        }

        if(bCompare)
        {
            //compare
            for(j = 0; j < size; j++)
            {
                if(data[j] != (i&0xFF))
                {
                    drv_free(data);
                    return SFLASH_TEST_COMPARE_FAIL;
                }
            }
        }
    }

    t2 = get_tick_count();
    
    AkDebugOutput("\nread end\n");

    drv_free(data);
    return (t2 - t1);
}

/***********************************
 * spi flash function case
 *
 **********************************/

T_VOID sflash_func_case()
{
    //cmd_handle_group("func_case");
   
sflash_func_case1();
sflash_func_case2();
sflash_func_case3();
sflash_func_case4();
}

static T_VOID sflash_func_case1()
{
    T_S32 rlt = SFLASH_TEST_SUCCESS;

    //init
    if(!spi_flash_init(SPI_ID0))
    {
        rlt = SFLASH_TEST_INIT_FAIL;
    }

    //handle result
    sflash_handle_result(SFLASH_FUNC_CASE(1), rlt);
}

static T_VOID sflash_func_case2()
{
    T_S32 rlt = SFLASH_TEST_SUCCESS;
    T_SFLASH_STRUCT sflash;

    //read id
    rlt = sflash_probe(&sflash);

    //handle result
    sflash_handle_result(SFLASH_FUNC_CASE(2), rlt);
}

static T_VOID sflash_func_case3()
{
    T_S32 rlt = SFLASH_TEST_SUCCESS;
    T_SFLASH_STRUCT sflash;

    //probe
    rlt = sflash_probe(&sflash);
    if(rlt != SFLASH_TEST_SUCCESS)
    {
        goto CASE3_END;
    }

    rlt = sflash_erase_all(&sflash, AK_TRUE);

CASE3_END:
    //handle result
    sflash_handle_result(SFLASH_FUNC_CASE(3), rlt);
}

static T_VOID sflash_func_case4()
{
    T_S32 rlt = SFLASH_TEST_SUCCESS;
    T_SFLASH_STRUCT sflash;
    T_U32 step = 32*4;

    do
    {
        //probe
        rlt = sflash_probe(&sflash);
        if(rlt < 0) break;

        //erase
        rlt = sflash_erase_all(&sflash, AK_TRUE);
        if(rlt < 0) break;

        //write
        rlt = sflash_write_all(&sflash, step, AK_TRUE);
        if(rlt < 0) break;

        //read and compare
        rlt = sflash_read_all(&sflash, step, AK_TRUE);
    }
    while(0);

    sflash_handle_result(SFLASH_FUNC_CASE(4), rlt);
}

/***********************************
 * spi flash test abnormal case 
 *
 **********************************/
T_VOID sflash_abn_case()
{
    sflash_abn_case1();
    sflash_abn_case2();
    sflash_abn_case3();
}

static T_VOID sflash_abn_case1()
{
    T_S32 rlt = SFLASH_TEST_SUCCESS;
    T_U8 id = 3;
    
    if(spi_flash_init(id))
    {
        rlt = SFLASH_TEST_FAIL;
    }

    sflash_handle_result(SFLASH_ABN_CASE(1), rlt);
}

static T_VOID sflash_abn_case2()
{
    T_S32 rlt = SFLASH_TEST_SUCCESS;
    T_U8 data[256];

    do
    {
        if(spi_flash_read(0, data, 1))
        {
            rlt = SFLASH_TEST_FAIL;
            break;
        }

        if(spi_flash_write(0, data, 1))
        {
            rlt = SFLASH_TEST_FAIL;
            break;
        }
    }
    while(0);
    
    sflash_handle_result(SFLASH_ABN_CASE(2), rlt);
}

static T_VOID sflash_abn_case3()
{
    T_S32 rlt = SFLASH_TEST_SUCCESS;
    T_U32 pll = 260;
    T_U32 asic_prev, asic_change, tmp;
    T_SFLASH_STRUCT sflash;
    T_U32 step = 32*4;
    T_U32 i;

    //set pll
    set_pll_value(pll);

    asic_prev = (pll*1000000)/2;
    asic_change = (pll*1000000)/8;

    for(i = 0; i < 2; i++)
    {
         //set asic
        AkDebugOutput("================asic = %d===========\n", asic_prev);
        set_asic_freq(asic_prev);
        
       //probe
        rlt = sflash_probe(&sflash);
        if(rlt < 0) break;

        //change asic freq
        AkDebugOutput("================asic = %d===========\n", asic_change);
        set_asic_freq(asic_change);

        //erase
        rlt = sflash_erase_all(&sflash, AK_TRUE);
        if(rlt < 0) break;

        //write
        rlt = sflash_write_all(&sflash, step, AK_TRUE);
        if(rlt < 0) break;

        //read and compare
        rlt = sflash_read_all(&sflash, step, AK_TRUE);

        //switch asic value
        tmp = asic_prev;
        asic_prev = asic_change;
        asic_change = tmp;
    }
    
    sflash_handle_result(SFLASH_ABN_CASE(3), rlt);
}

/***********************************
 * spi flash test performance case 
 *
 **********************************/
T_VOID sflash_pfm_case()
{
    sflash_pfm_case1();
    sflash_pfm_case2();
    sflash_pfm_case3();
}

static T_VOID sflash_pfm_case1()
{
    T_S32 rlt = SFLASH_TEST_SUCCESS;
    T_SFLASH_STRUCT sflash;

    sflash_pfm[0] = 0;
    do
    {
        //probe
        rlt = sflash_probe(&sflash);
        if(rlt < 0) break;

        //erase all
        rlt = sflash_erase_all(&sflash, AK_FALSE);
        if(rlt < 0) break;
        
        sflash_pfm[0] = ((sflash.capacity*1000)/1024) / rlt;
    }
    while(0);

    sflash_handle_result(SFLASH_PFM_CASE(1), rlt);
}

static T_VOID sflash_pfm_case2()
{
    T_S32 rlt = SFLASH_TEST_SUCCESS;
    T_SFLASH_STRUCT sflash;

    sflash_pfm[1] = 0;
    do
    {
        //probe
        rlt = sflash_probe(&sflash);
        if(rlt < 0) break;

        //erase all
        rlt = sflash_erase_all(&sflash, AK_FALSE);
        if(rlt < 0) break;

        //write all
        rlt = sflash_write_all(&sflash, 32*4, AK_FALSE);
        if(rlt < 0) break;
        
        sflash_pfm[1] = ((sflash.capacity*1000)/1024) / rlt;
    }
    while(0);

    sflash_handle_result(SFLASH_PFM_CASE(2), rlt);
}

static T_VOID sflash_pfm_case3()
{
    T_S32 rlt = SFLASH_TEST_SUCCESS;
    T_SFLASH_STRUCT sflash;

    sflash_pfm[2] = 0;
    do
    {
        //probe
        rlt = sflash_probe(&sflash);
        if(rlt < 0) break;

        //read all
        rlt = sflash_read_all(&sflash, 32*4, AK_FALSE);
        if(rlt < 0) break;
               
        sflash_pfm[2] = ((sflash.capacity*1000)/1024) / rlt;
    }
    while(0);

    sflash_handle_result(SFLASH_PFM_CASE(3), rlt);
}

#endif
#endif

