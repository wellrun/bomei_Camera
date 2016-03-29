#include "akdefine.h"

#ifdef  SPIBOOT
#include <string.h>
#include "Eng_DataConvert.h"
#include "fwl_spiflash.h"
#include "Gbl_Resource.h"
#include "Fwl_osMalloc.h"
#include "Eng_Debug.h"
#include "akos_api.h"
#include "update.h"
#include "drv_api.h"
#include "fha_asa.h"

#ifdef _UNICODE
#define _TEXT(str)  _T(str)
#define _Utl_StrCpyN Utl_UStrCpyN
#define _Utl_StrCpy Utl_UStrCpy
#define _Utl_UStrCat Utl_UStrCat
#define _Utl_UStrLen  Utl_UStrLen
#define _File_Open File_OpenUnicode
#define _File_Close Fwl_FileClose
#define _File_Exist File_Exist
#define _File_IsFolder File_IsFolder
#else
#define _TEXT(str)  
#define _Utl_StrCpyN Utl_StrCpyN
#define _Utl_StrCpy Utl_StrCpy
#define _Utl_UStrCat Utl_StrCat
#define _Utl_UStrLen  Utl_StrLen
#define _File_Open File_OpenAsc
#define _File_Close Fwl_FileClose
#define _File_Exist File_Exist
#define _File_IsFolder File_IsFolder
#endif

#define FILE_BUF_SIZE_MAX            (12*1024)
#define UPDATE_TASK_STACK_SIZE_MIN   (FILE_BUF_SIZE_MAX + 12*1024)


#define READ_CONFIG_LINE_LEN     1024

#define CONFIG_FILE_NAME		"config.txt"

#define MACADDR_NAME        	"MACADDR"
#define MACADDR_LEN_MAX     	30


extern T_SFLASH_STRUCT pSflash;


static T_BURNER_PARAM_CONFIG  g_burner_config;


static T_CHR *g_strChipName[] = {
   ("AK_3224"), ("AK_322L"), ("AK_36XX"), ("AK_780X"), ("AK_880X"),  ("AK_10X6"), ("AK_3631"), 
   ("AK_3671"), ("AK_980X"), ("AK_3671A"), ("AK_1080A"), ("AK_37XX"),("RESERVER"), AK_NULL
};

#if 0
static T_U32 reg_addr_sdram_sun3[] = 
{
   0x08000078,
   0x2002d004, // ac row
   0x2002d000, // no operation, open mclk , 0x2002e010
   0x2002d000, // precharge all banks
   0x2002d000, // 8 auto refresh1
   0x2002d000, // 8 auto refresh2
   0x2002d000, // 8 auto refresh3
   0x2002d000, // 8 auto refresh4
   0x2002d000, // 8 auto refresh5
   0x2002d000, // 8 auto refresh6
   0x2002d000, // 8 auto refresh7
   0x2002d000, // 8 auto refresh8
   0x2002d000,  // load mode register
   0x2002d000,  // no operation
   0x88888888
};

static T_U32 reg_value_sdram_sun3[] = 
{ 
   0x18401F7E,
   0x115889cb, // ac row
   0xc0170000, // no operation 
   0xc0120400, // precharge all banks
   0xc0110000,  // 8 auto refresh1
   0xc0110000,  // 8 auto refresh2
   0xc0110000,  // 8 auto refresh3
   0xc0110000,  // 8 auto refresh4
   0xc0110000,  // 8 auto refresh5
   0xc0110000,  // 8 auto refresh6
   0xc0110000,  // 8 auto refresh7
   0xc0110000,  // 8 auto refresh8
   0xc0100033, // lode mode register
   0xe0170000, // no operation
   0x00000000
};
#endif



static T_TCHR *pFolderPath = AK_NULL;
static T_hTask  hTask = AK_INVALID_TASK;




#if 0
static void modify_ram_size(T_U32 *reg_value)
{
	T_U32 ram_size = g_burner_config.RamInfo.size;

	*reg_value &= ~0x7;

	if(4 == ram_size)
	{
		*reg_value |= 0x1;
	}
	else if(8 == ram_size)
	{
		*reg_value |= 0x2;
	}
	else if(16 == ram_size)
	{
		*reg_value |= 0x3;
	}
	else if(32 == ram_size)
	{
		*reg_value |= 0x4;
	}
	else if(64 == ram_size)
	{
		*reg_value |= 0x5;
	}	
  
}

static T_BOOL BootCodeConfig(T_U8 *pBootCode, T_U32 LenBootCode)
{
	T_U32  *pAddrRamConfig = AK_NULL;
	T_U32  *pRamAddr = AK_NULL;
	T_U32  *pRamData = AK_NULL;
	T_U32   DataCount;
	T_U32   i;

	
		
	Fwl_Print(C2, M_UPSYS, "g_burner_config.ChipType=%d,CHIP_37XX=%d"
		,g_burner_config.ChipType,CHIP_37XX);
	switch(g_burner_config.ChipType)
	{
		case CHIP_37XX:						
			//password
			if (CHIP_3771_L ==drv_get_chip_version())
				memcpy(pBootCode+0x04, "SUPERS3L", 8);
			else
				memcpy(pBootCode+0x04, "ANYKA373", 8);
			
			//code copy address, only for 37L
			if (CHIP_3771_L ==drv_get_chip_version())
			{
				const T_U32 BOOT_RAM_ADDR= 0x30770200 ; 
				memcpy(pBootCode+0x18, &BOOT_RAM_ADDR , 4);
			}
			//ramconfigaddr
			if (CHIP_3771_L ==drv_get_chip_version())
			{
				pAddrRamConfig = pBootCode+0x20;
			}else
			{
				pAddrRamConfig = pBootCode+0x18;
			}
			if(((T_U32)pAddrRamConfig & 0x3) != 0)
			{
				Fwl_Print(C2, M_UPSYS, "pAddrRamConfig & 0x3 failed\n");
				return AK_FALSE;
			}

			pRamAddr = reg_addr_sdram_sun3;
			pRamData = reg_value_sdram_sun3;
			DataCount = 
				sizeof(reg_addr_sdram_sun3)/sizeof(reg_addr_sdram_sun3[0]);

		    for(i=0; i<DataCount; i++)
	        {
	            if (0x2002d004 == pRamAddr[i])
	            {
	                modify_ram_size(&pRamData[i]);
	            }
	        }
			
			break;
		default:						
			break;
	}

	LenBootCode = LenBootCode -  512;
	memcpy(pBootCode+0x0C, &LenBootCode, 4);

	pBootCode[0x10] = 0x02;
	pBootCode[0x11] = 0;

	memset(pBootCode+0x14, 0, 4);
	pBootCode[0x14] = 0x06;

	if((AK_NULL == pRamAddr) || (AK_NULL == pRamData))
	{
		return AK_FALSE;
	}

	for(i=0; i < DataCount; ++i)
	{
		*(pAddrRamConfig++) = pRamAddr[i];		
		*(pAddrRamConfig++) = pRamData[i];		
	}	

	for(i=512; i<1024-4; ++i)
	{
		if(('S' == pBootCode[i]) &&
		   ('P' == pBootCode[i+1]) &&
		   ('I' == pBootCode[i+2]) &&
		   ('P' == pBootCode[i+3]))
		{
			if(!SetBootCodeSpiParam(&pBootCode[i+4], 1024-(i+4)))
			{
				Fwl_Print(C2, M_UPSYS, "SetBootCodeSpiParam failed\n");
				return AK_FALSE;
			}
		}
	}

	if(i > 1024-4)
	{
		Fwl_Print(C2, M_UPSYS, "WARNING: BootCode have no SPIP FLAG\n");
	}


	return AK_TRUE;
	
}
#endif



static T_VOID deleteStrTab(T_CHR** str)
{
	if(AK_NULL ==*str)
		return;
	
	while(1)
	{
		if('\t' == **str)
		{
			(*str)++;
		}
		else
		{
			break;
		}
	}

	return;
}

static T_U32 hex2int(T_CHR *str)
{
    T_S32 i;
    T_U32 number=0; 
    T_U32 order=1;
    T_CHR ch;

    for(i=Utl_StrLen(str)-1; i>=0; i--)
    {
		ch=str[i];
		if(ch=='x' || ch=='X')break;
		
		if(ch>='0' && ch<='9')
		{
			number+=order*(ch-'0');
			order*=16;
		}
		if(ch>='A' && ch<='F')
		{
			number+=order*(ch-'A'+10);
			order*=16;
		}
		if(ch>='a' && ch<='f')
		{
			number+=order*(ch-'a'+10);
			order*=16;
		}
    }
    return number;
}


static T_U32 UnicodeStr2AnsiStr(const T_U16 *pUniStr, T_U32 UniStrLen, T_CHR *pAnsibuf, T_U32 AnsiBufLen)
{
	T_U32  i;

	if((AK_NULL == pUniStr) || (AK_NULL == pAnsibuf))
	{
		return 0;
	}

	for(i = 0; (i < UniStrLen) && (i < AnsiBufLen-1) ; ++i)
	{
		if(0 == pUniStr[i])
			break;	
		pAnsibuf[i] = (T_CHR)(pUniStr[i] & 0x0FF);	
	}

	pAnsibuf[i] = 0;

	return i;
}


static T_BOOL get_sub_str(T_CHR** str_sub, T_CHR** str_parent)
{
    T_S32 nPos; 
    T_CHR* strFind = ",";
    
    nPos = Utl_StrFnd(*str_parent, strFind, 0);
    
    if(nPos < 0)
    {
        return AK_FALSE;
    }

    *str_sub = *str_parent;

    (*str_sub)[nPos] = '\0';

    *str_parent = *str_parent + nPos + 1;

    *str_sub = Utl_StrTrim(*str_sub);
    *str_parent = Utl_StrTrim(*str_parent);

	deleteStrTab(str_sub);
	deleteStrTab(str_parent);
    
    return AK_TRUE;
}



static T_BOOL get_download_nand(T_U32 index, T_CHR* subRight)
{
	T_CHR *pSubRight = subRight;
	T_CHR* strTmp = AK_NULL;

	if(index >= g_burner_config.NandBinCnt)
	{
		return AK_FALSE;
	}

    Fwl_Print(C3, M_UPSYS, "bin[%d] \n ", index);

    //bCompare
    if(!get_sub_str(&strTmp, &pSubRight))
    {
		return AK_FALSE;
	}
    g_burner_config.pNandBinParam[index].bCompare = Utl_Atoi(strTmp);
    Fwl_Print(C3, M_UPSYS, " compare:%d ", g_burner_config.pNandBinParam[index].bCompare);

/*
    //bupdateself
    if(!get_sub_str(&strTmp, &pSubRight))
    {
		return AK_FALSE;
	}
    g_burner_config.pNandBinParam[index].bUpdateSelf= Utl_Atoi(strTmp);
    AK_DEBUG_OUTPUT(" bupdateself:%d ", g_burner_config.pNandBinParam[index].bUpdateSelf);
*/

    //对spring没有配置，也没有自升级
    g_burner_config.pNandBinParam[index].bUpdateSelf= 0;
    Fwl_Print(C3, M_UPSYS, " bupdateself:%d ", g_burner_config.pNandBinParam[index].bUpdateSelf);

    
	//pc path
	if(!get_sub_str(&strTmp, &pSubRight))
    {
		return AK_FALSE;
	}
    Utl_StrCpy(g_burner_config.pNandBinParam[index].binPath, strTmp);
    Fwl_Print(C3, M_UPSYS, " path:%s ", g_burner_config.pNandBinParam[index].binPath);

	//ld addr
	if(!get_sub_str(&strTmp, &pSubRight))
    {
		return AK_FALSE;
	}
    g_burner_config.pNandBinParam[index].ld_addr = hex2int(strTmp);
    Fwl_Print(C3, M_UPSYS, " ld_addr:0x%x ", g_burner_config.pNandBinParam[index].ld_addr);

	//file name
	if(!get_sub_str(&strTmp, &pSubRight))
    {
		Utl_StrCpy(g_burner_config.pNandBinParam[index].fileName, pSubRight);
	}
	else
	{
		Utl_StrCpy(g_burner_config.pNandBinParam[index].fileName, strTmp);
	}

    Fwl_Print(C3, M_UPSYS, " filename:%s ", g_burner_config.pNandBinParam[index].fileName);

    Fwl_Print(C3, M_UPSYS, "\r\n");
    
	return AK_TRUE;
}




static T_BOOL ReadConfig(T_TCHR *path)
{
    T_hFILE hFile = FS_INVALID_HANDLE;
	T_U16 lineBuf_unicode[READ_CONFIG_LINE_LEN] = {0};
    T_U8 lineBuf[READ_CONFIG_LINE_LEN] = {0};
    T_U32 read_len=1;
    T_U32 index=0;

    Fwl_Print(C3, M_UPSYS, "********config start:%s***********\r\n", _TEXT(path));
    hFile = Fwl_FileOpen(path,_FMODE_READ, _FMODE_READ);

    if(FS_INVALID_HANDLE == hFile)
    {
		UnicodeStr2AnsiStr(path, READ_CONFIG_LINE_LEN, lineBuf, 
			READ_CONFIG_LINE_LEN);
        Fwl_Print(C1, M_UPSYS, "open file fail,path:  %s\r\n", lineBuf);
        return AK_FALSE;
    }
    
    while(read_len>0)
    {
        T_S32  equ_pos;
        T_CHR ch = 0;
		T_CHR ch_uni[2];
        T_CHR* pstrline = AK_NULL;
        T_CHR* pequ_right = AK_NULL;
        T_CHR* strequ = "=";

        index = 0;
        while(read_len > 0 && '\n' != ch && '\r' != ch)
        {
            read_len = Fwl_FileRead(hFile, &ch_uni, 2);
            lineBuf_unicode[index++] = *((T_U16*)ch_uni);
			ch = ch_uni[0];
        }

		lineBuf_unicode[--index] = '\0';

		UnicodeStr2AnsiStr(lineBuf_unicode, READ_CONFIG_LINE_LEN, lineBuf, 
			READ_CONFIG_LINE_LEN);

//		AK_DEBUG_OUTPUT("Line: %s\n", lineBuf);

		pstrline = lineBuf;

        if('\r' == pstrline[0] || '\n' == pstrline[0])
        {
            continue;
        }
    
        equ_pos = Utl_StrFnd(pstrline, strequ, 0);  //strchr return soure addr

        if(equ_pos<0)
        {
            continue;
        }
        
        pequ_right = pstrline + equ_pos + 1;
        if(AK_NULL == pequ_right)
        {
            continue;
        }
        pstrline[equ_pos] = '\0';

        pstrline = Utl_StrTrim(pstrline);
        pequ_right = Utl_StrTrim(pequ_right);

		//delete tab
		deleteStrTab(&pstrline);
		deleteStrTab(&pequ_right);
		
     //   AK_DEBUG_OUTPUT("strline:%s, strright:%s\r\n", pstrline, pequ_right);
		if(0 == Utl_StrCmp(pstrline, "ram type")) //add by luheshan
		{
			g_burner_config.RamInfo.type = Utl_Atoi(pequ_right);
			Fwl_Print(C3, M_UPSYS, "ram type:%d\r\n", g_burner_config.RamInfo.type);
		}
        else if(0 == Utl_StrCmp(pstrline, "ram size")) 
        {
            g_burner_config.RamInfo.size = Utl_Atoi(pequ_right);
            Fwl_Print(C3, M_UPSYS, "ram size:%d\r\n", g_burner_config.RamInfo.size);
        }
        else if(0 == Utl_StrCmp(pstrline, "ram bank"))
        {
            g_burner_config.RamInfo.banks = Utl_Atoi(pequ_right);
            Fwl_Print(C3, M_UPSYS, "ram bank:%d\r\n", g_burner_config.RamInfo.banks);
        }
        else if(0 == Utl_StrCmp(pstrline, "ram row"))
        {
            g_burner_config.RamInfo.row = Utl_Atoi(pequ_right);
            Fwl_Print(C3, M_UPSYS, "ram row:%d\r\n", g_burner_config.RamInfo.row);
        }
        else if(0 == Utl_StrCmp(pstrline, "ram column"))
        {
            g_burner_config.RamInfo.column= Utl_Atoi(pequ_right);
            Fwl_Print(C3, M_UPSYS, "ram column:%d\r\n", g_burner_config.RamInfo.column);
        }
        else if(0 == Utl_StrCmp(pstrline, "fs nonfs reserve size"))
        {
            g_burner_config.ResvSize = Utl_Atoi(pequ_right);
            Fwl_Print(C3, M_UPSYS, "resv size:%d\r\n", g_burner_config.ResvSize);
        }
        else if(0 == Utl_StrCmp(pstrline, "fs reserver block"))
        {
            g_burner_config.fs_resv_block_num = Utl_Atoi(pequ_right);
            Fwl_Print(C3, M_UPSYS, "fs resv block:%d\r\n", g_burner_config.fs_resv_block_num);
        }
		else if(0 == Utl_StrCmp(pstrline, "chip clock"))
        {
            g_burner_config.m_freq = Utl_Atoi(pequ_right);
            Fwl_Print(C3, M_UPSYS, "m_freq:%d\r\n", g_burner_config.m_freq);
        }        
        else if(0 == Utl_StrCmp(pstrline, "chip type"))
        {
			int i = 0;
			g_burner_config.ChipType = CHIP_RESERVER;	

			Fwl_Print(C3, M_UPSYS, "%s\n",pequ_right);
			
			while(g_strChipName[i])
			{
				if(!Utl_StrCmp(g_strChipName[i], pequ_right))
				{
					g_burner_config.ChipType = (E_CHIP_TYPE)i;
					break;
				}
				
				i++;
			}	            
            Fwl_Print(C3, M_UPSYS, "chip type:%d\r\n", g_burner_config.ChipType);
        }        
        else if(0 == Utl_StrCmpN(pstrline, "download_to_nand", 16))
        {
             if(0 == Utl_StrCmp(pstrline, "download_to_nand count"))
            {
                g_burner_config.NandBinCnt = Utl_Atoi(pequ_right);
                Fwl_Print(C3, M_UPSYS, "NandBinCnt:%d\r\n", g_burner_config.NandBinCnt);

                if(g_burner_config.NandBinCnt>0)
                {
                    g_burner_config.pNandBinParam = Fwl_Malloc(g_burner_config.NandBinCnt * sizeof(T_BURN_BIN_CONFIG));

                    if(AK_NULL == g_burner_config.pNandBinParam)
                    {
                        return AK_FALSE;
                    }
                }
            }
            else
            {
                T_U32 bin_index;
                bin_index = Utl_Atoi(Utl_StrTrim(pstrline+16));

                bin_index--;

                if(!get_download_nand(bin_index, pequ_right))
                {
                    return AK_FALSE;
                }    
            }
        }          
        else 
        {
        	const T_CHR * BOOT_PATH="path nandboot";
        	const T_CHR * L_BOOT_PATH="path nandboot new";
        	
        	if (CHIP_3771_L ==drv_get_chip_version())
			{
		        if(0 == Utl_StrCmpN(pstrline, L_BOOT_PATH, Utl_StrLen(L_BOOT_PATH)))
		        {
		            Utl_StrCpy(g_burner_config.nandbootpath, pequ_right);			
					Fwl_Print(C3, M_UPSYS, "nandboot:%s\r\n", g_burner_config.nandbootpath);
		        }
	        }else
	        {
		        if(0 == Utl_StrCmpN(pstrline, BOOT_PATH, Utl_StrLen(BOOT_PATH))
		        	&& 0 != Utl_StrCmpN(pstrline, L_BOOT_PATH, Utl_StrLen(L_BOOT_PATH)))
		        {
		            Utl_StrCpy(g_burner_config.nandbootpath, pequ_right);			
					Fwl_Print(C3, M_UPSYS, "nandboot:%s\r\n", g_burner_config.nandbootpath);
		        
		        }
	        	
	        }
        }
    }

    Fwl_FileClose(hFile);
    Fwl_Print(C3, M_UPSYS, "********config end***********\r\n");
    return AK_TRUE;
}


#if 0
static T_VOID BackupFile(T_TCHR * ppBackupFileList[])
{

	
}

#endif






#if 0
void ReadSpiBoot(void)
{
	T_U8   buf[1024];
	T_U32  i,j;

	AK_DEBUG_OUTPUT("\n===============================================\n");

	for(i=120; i < 124; ++i)
	{	
		if(!spi_flash_read(i, buf,1))
		{
			AK_DEBUG_OUTPUT("Read Error, %d\n", i);
		}

		for(j=0; j < 256; ++j)
		{
			AK_DEBUG_OUTPUT("%02X,%s",buf[j],(j+1)&0x0F ? " ":"\n");
		}		
	}

	AK_DEBUG_OUTPUT("\n===============================================\n");	
}
#endif



static T_VOID SPI_UpdateFile(T_U32 argc , CBFUNC_UPDATE_FILE pFunc)
{
    T_hFILE           hFile = FS_INVALID_HANDLE;
	T_TCHR            FolderPath[MAX_PATH];
	T_TCHR            FilePath[MAX_PATH];	
	T_U8              cFileBuf[FILE_BUF_SIZE_MAX];
	T_U32             i;
	T_U32             LenReaded;
	T_S32             LenFile;
	T_S32             LenTotalFile;
	T_S32             LenOddFile;
	T_BOOL            bRet = AK_TRUE;
	//T_BOOL            bBootConfig = AK_TRUE;
	T_U8			  *pDATA = AK_NULL;
	T_U8 			  data_tmp[MACADDR_LEN_MAX] = {0};
	T_BOOL			  bMacFlag = AK_FALSE;

	pDATA = (T_U8*)Fwl_Malloc(SPIFLASH_PAGE_SIZE * SPI_CFG_PAGE);
	
	if (AK_NULL == pDATA)
	{
		pFunc(OTHER_ERROR, 0);			
		Fwl_Print(C2, M_UPSYS, "pDATA malloc failed! \n"); 
		return ;
	}


	memset(pDATA, 0, SPIFLASH_PAGE_SIZE * SPI_CFG_PAGE);
	
	if (SF_SUCCESS != Fwl_SPIFlash_ReadPage(0, pDATA, SPI_CFG_PAGE))
	{
		pFunc(OTHER_ERROR, 0);			
		Fwl_Print(C2, M_UPSYS, "Fwl_SPIFlash_ReadPage failed! \n"); 
		Fwl_Free(pDATA);
		return ;
	}
#if 0
	hFile = _File_Open(AK_NULL, pFolderPath, _FMODE_READ);
	if(AK_NULL == hFile)
	{
		pstUpdateRes->RetCode = OTHER_ERROR;
		return AK_FALSE;		
	}

	if(!(_File_Exist() && _File_IsFolder()))
	{
		pstUpdateRes->RetCode = FOLDER_NO_EXIST;
		return AK_FALSE;		
	}

	_File_Close(hFile);
#endif	
	
	_Utl_StrCpyN(FolderPath, pFolderPath, sizeof(FolderPath)/sizeof(FolderPath[0]));
	Fwl_Free(pFolderPath);
	pFolderPath = AK_NULL;

//	i = UnicodeStr2AnsiStr(FolderPath, MAX_PATH, cFileBuf, FILE_BUF_SIZE_MAX);
//	AK_DEBUG_OUTPUT("PATH1: %s, len = %d\n", cFileBuf,i);

	i = _Utl_UStrLen(FolderPath);	
	if(('\\' != FolderPath[i-1]) && ('/' != FolderPath[i-1]))
	{
		FolderPath[i] = '/';
		FolderPath[i+1] = 0;
		
	}
	_Utl_StrCpy(FilePath, FolderPath);

//	i = UnicodeStr2AnsiStr(FilePath, MAX_PATH, cFileBuf, FILE_BUF_SIZE_MAX);
	
//	AK_DEBUG_OUTPUT("FILE PATH1: %s, len = %d\n", cFileBuf,i);
	
	_Utl_UStrCat(FilePath,_TEXT(CONFIG_FILE_NAME));

//	UnicodeStr2AnsiStr(FilePath, MAX_PATH, cFileBuf, FILE_BUF_SIZE_MAX);
//	AK_DEBUG_OUTPUT("PATH2: %s\n", cFileBuf);

	
	if(!ReadConfig(FilePath))
	{
		pFunc(FILE_NO_EXIST, 0);
		Fwl_Free(pDATA);
		return ;
	}

	LenTotalFile = LenOddFile = 0;
	
	for(i = 0; i < g_burner_config.NandBinCnt; ++i)
	{
		_Utl_StrCpy(FilePath, FolderPath);
		_Utl_UStrCat(FilePath, _TEXT(g_burner_config.pNandBinParam[i].binPath));		
		hFile = Fwl_FileOpen(FilePath,_FMODE_READ, _FMODE_READ);		
		if(FS_INVALID_HANDLE == hFile)
	    {
	        pFunc(FILE_NO_EXIST,0);
			Fwl_Print(C2, M_UPSYS, "%s Not Exist \n", 
						_TEXT(FilePath));
			Fwl_Free(pDATA);
	        return ;
		}

		LenTotalFile += Fwl_GetFileLen(hFile);
		
		Fwl_FileClose(hFile);
	}

#if 0
	_Utl_StrCpy(FilePath, FolderPath);
	_Utl_UStrCat(FilePath, _TEXT(g_burner_config.nandbootpath));		
	hFile = Fwl_FileOpen(FilePath,_FMODE_READ, _FMODE_READ);
	if(FS_INVALID_HANDLE == hFile)
	{	
		pFunc(FILE_NO_EXIST, 0);			
		Fwl_Print(C2, M_UPSYS, "%s Not Exist \n", 
			_TEXT(FilePath)); 
		Fwl_Free(pDATA);
		return ;
	}

	LenTotalFile += Fwl_GetFileLen(hFile);
	Fwl_FileClose(hFile);

#endif
	Fwl_Print(C3, M_UPSYS, "LenTotalFile = %d\n", LenTotalFile);
	

//    BackupFile(ppBackupFileList);



	if(!SpiInitPreUpdate())
	{
		pFunc(OTHER_ERROR, 0);		
		Fwl_Print(C1, M_UPSYS, "SpiInitPreUpdate fail\n");
		Fwl_Free(pDATA);
		return ;
	}

	if (ASA_FILE_SUCCESS == FHA_asa_read_file(MACADDR_NAME, data_tmp, MACADDR_LEN_MAX))
    {
    	Fwl_Print(C2, M_UPSYS, "Read MAC addr ok!\n"); 
		bMacFlag = AK_TRUE;
    }
	else
	{
		Fwl_Print(C2, M_UPSYS, "Read MAC addr failed!\n"); 
	}


	for(i = 0; i < g_burner_config.NandBinCnt; ++i)
	{		
		_Utl_StrCpy(FilePath, FolderPath);
		_Utl_UStrCat(FilePath, _TEXT(g_burner_config.pNandBinParam[i].binPath));		
		hFile = Fwl_FileOpen(FilePath,_FMODE_READ, _FMODE_READ);
		if(FS_INVALID_HANDLE == hFile)
		{
			//need some sleep ?
			
			hFile = Fwl_FileOpen(FilePath,_FMODE_READ, _FMODE_READ);
			if(FS_INVALID_HANDLE == hFile)
			{
				pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);				
				Fwl_Print(C1, M_UPSYS, "Open %s fail\n", 
					g_burner_config.pNandBinParam[i].binPath);
                Spi_FHA_close();
				Fwl_Free(pDATA);
				return ;
			}
		}

		LenFile = Fwl_GetFileLen(hFile);

		if(AK_TRUE != Spi_write_bin_begin(
			LenFile, 
			g_burner_config.pNandBinParam[i].ld_addr,
			g_burner_config.pNandBinParam[i].fileName,
			0,
			g_burner_config.pNandBinParam[i].bCompare,
			g_burner_config.pNandBinParam[i].bUpdateSelf))
		{
			pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);	
			Fwl_Print(C1, M_UPSYS, "Spi_write_bin_begin fail\n"); 
			Fwl_FileClose(hFile);
            Spi_FHA_close();
			Fwl_Free(pDATA);
			return ;
		}


		while(LenFile > 0)
		{
			LenReaded = Fwl_FileRead(hFile, cFileBuf, FILE_BUF_SIZE_MAX);
			if(LenReaded > 0)
			{
	//			AK_DEBUG_OUTPUT("LenOddFile = %d, LenReaded = %d",LenOddFile,  LenReaded);
				if(!Spi_write_bin(cFileBuf, (T_U32)LenReaded))
				{
					pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);
					bRet = AK_FALSE;					
					Fwl_Print(C1, M_UPSYS, "Spi_write_bin fail 1\n"); 
					break ;						
				}
				LenOddFile += LenReaded;

				pFunc(UPDATE_SUCCESSFUL , LenOddFile*100/LenTotalFile);
			}
			else if(LenReaded == 0)
			{
				pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);
				bRet = AK_FALSE;				
				Fwl_Print(C1, M_UPSYS, "Fwl_FileRead fail 2 \n"); 
				break ; 					
			}

			LenFile -= LenReaded;

#if 1 //maybe not need			
			if((FILE_BUF_SIZE_MAX != LenReaded) && (0 != LenFile))
			{
				pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);
				bRet = AK_FALSE;				
				Fwl_Print(C1, M_UPSYS, "Fwl_FileRead fail 3 \n"); 
				break ;														
			}
#endif
			
		}
		


		Fwl_FileClose(hFile);

		if(!bRet)
		{
            Spi_FHA_close();
			Fwl_Free(pDATA);
			return ;
		}
		
	}
#if 0
	_Utl_StrCpy(FilePath, FolderPath);
	_Utl_UStrCat(FilePath, _TEXT(g_burner_config.nandbootpath));		
	hFile = Fwl_FileOpen(FilePath,_FMODE_READ, _FMODE_READ);
	if(FS_INVALID_HANDLE == hFile)
	{
		//need some sleep ?
		
		hFile = Fwl_FileOpen(FilePath,_FMODE_READ, _FMODE_READ);
		if(FS_INVALID_HANDLE == hFile)
		{
			pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);			
			Fwl_Print(C1, M_UPSYS, "Open %s fail \n", 
				_TEXT(FilePath)); 
            Spi_FHA_close();
			return ;
		}
	}

	LenFile = Fwl_GetFileLen(hFile);

	Fwl_Print(C3, M_UPSYS, "Open %s  \n", 
		_TEXT(FilePath)); 
	
	if(AK_TRUE != Spi_write_boot_begin((T_U32)LenFile))
	{
		pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);
		Fwl_FileClose(hFile);		
		Fwl_Print(C1, M_UPSYS, "Spi_write_boot_begin fail 4 \n"); 
        Spi_FHA_close();
		return ;		
	}
	
	while(LenFile > 0)
	{
		LenReaded = Fwl_FileRead(hFile, cFileBuf, FILE_BUF_SIZE_MAX);
		if(LenReaded > 0)
		{
			if(bBootConfig)
			{				
				bBootConfig = AK_FALSE;
				
				if(LenReaded < 1024)
				{
					pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);
					bRet = AK_FALSE;					
					Fwl_Print(C1, M_UPSYS, "bBootConfig && LenReaded < 1024 \n"); 
					break ;	
				}	

				if(!BootCodeConfig(cFileBuf, LenFile))
				{
					pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);
					bRet = AK_FALSE;					
					Fwl_Print(C1, M_UPSYS, "BootCodeConfig Fail\n"); 
					break ;	
				}
#if 0
				AK_DEBUG_OUTPUT("Check after modify:\n");
				for(i=0; i < 256; ++i)
				{
					AK_DEBUG_OUTPUT("%02X,%s",cFileBuf[i],(i+1)&0x0F ? " ":"\n");
				}	
#endif				
				
			}
			
			if(!Spi_write_boot(cFileBuf, (T_U32)LenReaded))
			{
				pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);
				bRet = AK_FALSE;					
				Fwl_Print(C1, M_UPSYS, "Spi_write_boot fail 5 \n"); 
				break ;	
			}				
			
			pFunc(UPDATE_SUCCESSFUL , LenOddFile*100/LenTotalFile);
		}
		else if(LenReaded == 0)
		{
			pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);
			bRet = AK_FALSE;			
			Fwl_Print(C1, M_UPSYS, "Fwl_FileRead fail 6 \n"); 
			break ;	
		}

		LenFile -= LenReaded;

#if 1 //maybe not need			
		if((FILE_BUF_SIZE_MAX != LenReaded) && (0 != LenFile))
		{
			pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);
			bRet = AK_FALSE;			
			Fwl_Print(C1, M_UPSYS, "Fwl_FileRead fail 7 \n"); 
			break ;	
		}
#endif
		
	}
	
	Fwl_FileClose(hFile);
#else
	if (SF_SUCCESS != Fwl_SPIFlash_WritePage(0, pDATA, SPI_CFG_PAGE))
	{
		pFunc(OTHER_ERROR, LenOddFile*100/LenTotalFile);		
		Fwl_Print(C2, M_UPSYS, "Fwl_SPIFlash_WritePage failed! \n"); 
		Spi_FHA_close();
		Fwl_Free(pDATA);
		return ;
	}
#endif

	if (bMacFlag)
	{
		if (ASA_FILE_SUCCESS == FHA_asa_write_file(MACADDR_NAME, data_tmp, Utl_StrLen(data_tmp), ASA_MODE_CREATE))
	    {
	    	Fwl_Print(C2, M_UPSYS, "Write MAC addr ok!\n"); 
	    }	
		else
		{
			Fwl_Print(C2, M_UPSYS, "Write MAC addr failed!\n"); 
		}
	}

	
	if(!Spi_FHA_close())
	{
		Fwl_Print(C1, M_UPSYS, "Call Spi_FHA_close fail\n");
	}
	
	Fwl_Print(C3, M_UPSYS, "Update finish\n");
	pFunc(UPDATE_SUCCESSFUL, 100);	
	Fwl_Free(pDATA);

}




T_BOOL  SPI_UpdateTask(T_TCHR *pFolder, T_TCHR * ppBackupFileList[],
						  CBFUNC_UPDATE_FILE  pFunc, T_VOID *TaskStackAddr, 
						  T_U32 StackSize)
{
	Fwl_Print(C3, M_UPSYS, "SPI_UpdateTask pFunc = 0x%x", pFunc);
	
	if((AK_NULL == pFolder) || (AK_NULL == pFunc) || (AK_NULL == TaskStackAddr))
	{
		return AK_FALSE;
	}

	if(StackSize < UPDATE_TASK_STACK_SIZE_MIN)
	{
		Fwl_Print(C1, M_UPSYS, "StackSize too mini!");
		return AK_FALSE;
	}

//	AK_DEBUG_OUTPUT(__func__ "   %s\n", sizeof(T_TCHR) == 2 ? "UNICODE":"ASIC");

	pFolderPath = Fwl_Malloc(MAX_PATH);
	if(AK_NULL == pFolderPath)
	{
		Fwl_Print(C1, M_UPSYS, "Molloc for FolderPath fail");
		return AK_FALSE;
	}	

	_Utl_StrCpy(pFolderPath, pFolder);
//	UnicodeStr2AnsiStr(pFolderPath, 1024, TaskStackAddr , 1024);
//	AK_DEBUG_OUTPUT("path : %s\n", TaskStackAddr);

	hTask = AK_Create_Task((T_VOID*)SPI_UpdateFile, "update file", 1, 
		(T_VOID*)pFunc, TaskStackAddr, StackSize, 200, 5, 
		AK_PREEMPT, AK_START);


	if(AK_IS_INVALIDHANDLE(hTask))
	{
		Fwl_Free(pFolderPath);
		pFolderPath = AK_NULL;
		Fwl_Print(C1, M_UPSYS, "AK_Create_Task Fail\n");		
		return AK_FALSE;
	}

	return AK_TRUE;
	
}

T_VOID UpdateTask_Close(T_VOID)
{
	if (AK_IS_VALIDHANDLE(hTask))
	{
		AK_Terminate_Task(hTask);
		AK_Delete_Task(hTask);
		
		hTask = AK_INVALID_TASK;
	}
}

#endif
