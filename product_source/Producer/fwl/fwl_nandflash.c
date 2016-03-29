/**
* @FILENAME nandflash.c
* @BRIEF    并口NAND FLASH控制器驱
* Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR   dengjian
* @MODIFY   zhaojiahuan   chenyanyan
* @DATE     2007-1-10
* @VERSION  1.0
* @REF      Please refer to…
* @NOTE     1.只支持目前市面上大量生产的samsung 和hynix的驱动（这样运行速度会快一些
*        果要支持比较特殊 的nandflash驱动，需要调整程序和宏定义
*       2. 多片选择的相关细节目前没有在驱动中体现，大多数都是针对单片flash编写。
*/
#ifdef OS_ANYKA

#include "anyka_types.h"
#include "nandflash.h"
#include "nand_list.h"
#include "arch_nand.h"
#include "fha.h"
#include "fwl_nandflash.h"

#define NAND_BOOT0_DATA_SIZE_37XX   486
#define NAND_BOOT0_DATA_SIZE        472
#define NAND_DATA_SIZE_P512B        512
#define NAND_DATA_SIZE_P1KB         1024

#define NAND_PARITY_SIZE_MODE0      7
#define NAND_PARITY_SIZE_MODE1      14

//***********************************************************************************
static E_NANDERRORCODE Nand_MultiWriteSector(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page,const T_U8 data[], T_U8* SpareTbl,T_U32 oob_len);
static E_NANDERRORCODE Nand_MultiReadSector(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page,T_U8 data[], T_U8* SpareTbl,T_U32 oob_len);
static E_NANDERRORCODE Nand_MultiCopyBack(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 PlaneNum, T_U32 SourceBlock, T_U32 DestBlock, T_U32 page);
static E_NANDERRORCODE Nand_MultiEraseBlock(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 planeNum, T_U32 block);
static E_NANDERRORCODE Nand_ReadFlag_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8 *spare_tbl,T_U32 oob_len);
static E_NANDERRORCODE Nand_WriteSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, const T_U8 data[], T_U8* spare_tbl, T_U32 oob_len);
static E_NANDERRORCODE Nand_ReadSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8 data[], T_U8* spare_tbl, T_U32 oob_len);
static E_NANDERRORCODE Nand_MultiWriteSector_Ex(T_PNANDFLASH nand, T_U32 chip,  T_U32 block, T_U32 page, const T_U8 data[], T_U8* spare_tbl, T_U32 oob_len);
static E_NANDERRORCODE Nand_MultiReadSector_Ex(T_PNANDFLASH nand, T_U32 chip,  T_U32 block, T_U32 page, T_U8 data[], T_U8* spare_tbl, T_U32 oob_len);

static E_NANDERRORCODE Nand_ReadSector_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block, T_U32 page, T_U8 data[], T_U8* spare_tbl, T_U32 oob_len, T_U32 page_num);
static E_NANDERRORCODE Nand_WriteSector_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block, T_U32 page, const T_U8 data[], T_U8* spare_tbl, T_U32 oob_len, T_U32 page_num);
static E_NANDERRORCODE Nand_EraseBlock_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block);

static E_NANDERRORCODE Nand_WriteFlag(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block, T_U32 page, T_U8* oob,T_U32 oob_len);
static E_NANDERRORCODE Nand_ReadFlag(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block, T_U32 page, T_U8* oob,T_U32 oob_len);

static T_VOID Nand_Config_Data(T_PNAND_ECC_STRU data_ctrl,  T_PNAND_ECC_STRU spare_ctrl, T_U8 *data, T_U32 datalen, T_U8 *oob, T_U32 oob_len, T_U8 ecctype);

static T_PNANDFLASH m_pNF_Info = AK_NULL;

static T_U32 m_true_block_per_die = 2048;
static T_U32 m_fake_block_per_die = 4096;

extern T_U32 g_burn_erase;
extern T_U32 g_burn_mode;


//此代码的功能是实现低格的功能

T_BOOL Fwl_Erase_block(viod)
{
    T_U32 i, j;
    T_BOOL is_used = AK_FALSE;
    T_U8 *pBuf = AK_NULL;
    T_U32 badblock_page_cnt = 0;
    T_U32 spare = 0;
    T_U32 chip = m_pNF_Info->PlaneCnt/m_pNF_Info->PlanePerChip;
    T_U32 blcok_num = m_pNF_Info->PlaneCnt*m_pNF_Info->BlockPerPlane;
    T_U32 page_size = m_pNF_Info->SectorPerPage*m_pNF_Info->BytesPerSector;
    T_U32 byte_loc, byte_offset;
    
     // EraseBlock when in the MODE_ERASE 

    badblock_page_cnt = (blcok_num - 1) / (page_size * 8) + 1;
    
    pBuf = Fwl_Malloc(badblock_page_cnt*page_size);
    if(AK_NULL == pBuf)
    {
        printf("Fwl_Erase_block malloc fail\r\n");
        return AK_FALSE;
    }
    //判断是否使用过。
    //printf("*************************************************\r\n");
    if(FHA_check_Nand_isused(pBuf, badblock_page_cnt*page_size))
    {
        is_used = AK_TRUE;
        //printf("is_used :%d\r\n", is_used);
    }
    memset(pBuf, 0, badblock_page_cnt*page_size);
    //printf("chip:%d\r\n", chip);
    //此nand没有被使用过
    if(AK_FALSE == is_used)
    {
        //printf("Nandflash format\r\n");
        for(j = chip; j >= 1; j--)
        {
            for (i = 0; i < blcok_num; i++)
            {
                //printf("chip:%d,  block:%d\r\n", chip-j, i);
                
                if(FHA_Nand_check_block(chip - j, i))
                {
                    printf("eb:%d\r\n", i);
                    Nand_EraseBlock(m_pNF_Info, chip-j, i);
                }
                else
                {
                    printf("factory bad block:%d\r\n", i);
                }
            }
        }
        return AK_TRUE;
    }

   //判断第一个块是否出厂坏块
   if(!FHA_Get_factory_badblock_Buf(pBuf, badblock_page_cnt))
   {
        printf("FHA_Get_factory_badblock_Buf return fail\r\n");
        return AK_FALSE;
   }
    
    //此nand已使用过
    if(AK_TRUE == is_used)
    {
        for(j = chip; j >= 1; j--)
        {
            for (i = 0; i < blcok_num; i++)
            {
                //判断是否出厂坏块
                byte_loc = i / 8;
                byte_offset = 7 - i % 8;

                if(pBuf[byte_loc] & (1 << byte_offset))
                {
                    printf("bad block:%d\n", i);
                    continue;
                }
                //擦块
                printf("eb:%d, %d\n", chip-j, i);
                Nand_EraseBlock(m_pNF_Info, chip-j, i);
            }
        }
    }
    printf("erase blk_num: %d, %d\n", i, blcok_num);
    return AK_TRUE;
}



//***********************************************************************************
T_PNANDFLASH Nand_Init(T_NAND_PHY_INFO *nand_info)
{
    T_U32 nand_id, i, ChipCnt;
    T_PNandflash_Add pNF_Add;
    T_PNANDFLASH pNF_Info;
    pNF_Info = (T_PNANDFLASH)Fwl_Malloc(sizeof(T_NANDFLASH) + sizeof(T_Nandflash_Add));
	
    if(AK_NULL == pNF_Info)
    {
        printf("no more memory!\n");
        while(1);
    }
    ChipCnt = 0;
    pNF_Info->NandType = 0;
    pNF_Info->PlaneCnt = 0;
    pNF_Info->PlanePerChip = 0;
    pNF_Info->Fake2Real = AK_NULL;

    pNF_Add = (T_PNandflash_Add)(&pNF_Info[1]);

    for( i = 0; i < NFC_SUPPORT_CHIPNUM; i++ )
        pNF_Add->ChipPos[i] = 0xFF; //init for no connect flash

    for( i = 0; i < NFC_SUPPORT_CHIPNUM; i++ )
    {
	 //   nand_id = nand_read_chipID(i);
        printf("id = %x\r\n", nand_id);
		
        if( ERROR_CHIP_ID != nand_id )
        {
	        if(nand_info->chip_id == nand_id)
	        {
				pNF_Info->ChipCharacterBits = nand_info->flag;
            //    nand_calctiming(nand_info->data_len);
              //  nand_changetiming(get_asic_freq());

                pNF_Info->NandType = nand_info->custom_nd;
				pNF_Info->BytesPerSector = nand_info->page_size;
				pNF_Info->BlockPerPlane = nand_info->plane_blk_num;
				pNF_Info->PlanePerChip = nand_info->blk_num / pNF_Info->BlockPerPlane;
				pNF_Info->PagePerBlock = nand_info->page_per_blk;
				pNF_Info->SectorPerPage = nand_info->page_size / pNF_Info->BytesPerSector;
			
				pNF_Add->ChipPos[ChipCnt] = i;        //logic CE to phy CE
				pNF_Add->RowCycle = nand_info->row_cycle;
				pNF_Add->ColCycle= nand_info->col_cycle;
				pNF_Add->PageSize = nand_info->page_size;
                pNF_Add->PagesPerBlock = nand_info->page_per_blk;
				pNF_Add->EccType = (pNF_Info->ChipCharacterBits >> 4) & 0xF;
                
	            ChipCnt ++;
	            printf("chip id =%x CE=%x\r\n", nand_id, i);

                m_true_block_per_die = nand_info->group_blk_num;

                //if fake_block_per_die should be upscaled, double it
                if (1 == ((pNF_Info->ChipCharacterBits >> 11) & 0x1))
                {
                    m_fake_block_per_die = m_true_block_per_die * 2;
                    printf("block_per_die %d upscale to %d!!\n", m_true_block_per_die, m_fake_block_per_die);
                }
                else
                {
                    m_fake_block_per_die = m_true_block_per_die;
                }

                if(1 == ((pNF_Info->ChipCharacterBits >> 17) & 0x1))
                {
                  //  nand_enable_randomizer(pNF_Add);
                }       

               
            }
			else
			{
                printf("unkonw chip 0x%x!!\r\n", nand_id);
                continue;
			}
        }
    }

    if( ChipCnt > 0 )
    {
        pNF_Info->WriteSector = Nand_WriteSector;
        pNF_Info->ReadSector = Nand_ReadSector;
		pNF_Info->ReadFlag = Nand_ReadFlag;
		pNF_Info->WriteFlag = Nand_WriteFlag;
        pNF_Info->CopyBack = Nand_CopyBack;

        pNF_Info->EraseBlock = Nand_EraseBlock;
	    pNF_Info->IsBadBlock = Nand_IsBadBlock;
        pNF_Info->SetBadBlock  = Nand_SetBadBlock;

        pNF_Info->MultiEraseBlock = Nand_MultiEraseBlock;
        pNF_Info->MultiCopyBack = Nand_MultiCopyBack;
        pNF_Info->MultiWrite = Nand_MultiWriteSector;
        pNF_Info->MultiRead = Nand_MultiReadSector;

        pNF_Info->ExReadFlag = Nand_ReadFlag_Exnftl;
        pNF_Info->ExWrite = Nand_WriteSector_Exnftl;
        pNF_Info->ExRead = Nand_ReadSector_Exnftl;
        pNF_Info->ExEraseBlock = Nand_EraseBlock_Exnftl;
        pNF_Info->ExIsBadBlock = Nand_IsBadBlock;
        pNF_Info->ExSetBadBlock = Nand_SetBadBlock;
        
        if(pNF_Info->BytesPerSector == 8192)
        {
			pNF_Add->ChipType = NAND_8K_PAGE;
        }
        else if(pNF_Info->BytesPerSector == 4096)
        {
			pNF_Add->ChipType = NAND_4K_PAGE;
        }
        else if(pNF_Info->BytesPerSector == 2048)
        {
			pNF_Add->ChipType = NAND_2K_PAGE;
        }
		else
	    {
            pNF_Add->ChipType = NAND_512B_PAGE;
		}
        
        pNF_Info->PlaneCnt = pNF_Info->PlanePerChip * ChipCnt;

    }
    else
    {
        printf("Cannot find chip!\r\n");
    }
    printf("find chip=%d page size=%d\r\n", ChipCnt, pNF_Info->BytesPerSector);
    printf("planes=%d plane size=%d\r\n", pNF_Info->PlaneCnt, pNF_Info->BlockPerPlane);

    m_pNF_Info = pNF_Info;

    //此代码的功能是实现低格的功能
    #if 0       // EraseBlock when in the MODE_ERASE 
    {
        if (g_burn_erase && MODE_NEWBURN == g_burn_mode)   // ERASE
        {
            T_U32 i;

            printf("Nandflash format\r\n");
            for (i=0; i<nand_info->blk_num; i++)
            {
                printf(".\r\n");
                Nand_EraseBlock(pNF_Info, 0, i);
            }
            printf("erase blk_num: %d, %d\n", i, nand_info->blk_num);
        }
    }  
    #endif
    
    return pNF_Info;       
}

//*********************************************************************************
T_BOOL Fwl_NandHWInit(T_U32 gpio_ce2, T_U32 gpio_ce3, T_U32* ChipID, T_U32* ChipCnt)
{
    T_U32 chip_id_tmp = ERROR_CHIP_ID;
	T_U8  chip_count = 0;
	T_U32 i;
    T_BOOL ret = AK_TRUE;
	
	printf("nand HWinit\r\n");
	
  //  nand_HWinit();

    //Get chip0 ID
//	*ChipID = nand_read_chipID(0);

    printf("chip id0: 0x%x\r\n", *ChipID);

	if(ERROR_CHIP_ID != *ChipID)
	{
        chip_count++; 

        for(i = 1; i < NFC_SUPPORT_CHIPNUM; i++)
	    {
   	   //     chip_id_tmp = nand_read_chipID(i);
            printf("chip id%d: 0x%x\r\n", i, chip_id_tmp);

            //ONLY support the same chip id
            if(*ChipID == chip_id_tmp)
    		{
       			chip_count++;
    		}
            else if(0 == chip_id_tmp || 0xFFFFFFFF == chip_id_tmp)
            {
                break;
            }
            else
            {
                //ret = FHA_FAIL;
                continue;
            }
        }

        *ChipCnt = chip_count;
	}
	else
	{	
        ret = AK_FALSE;
	}

    printf("chip cnt:%d\r\n", chip_count);

    return ret;
}

//***********************************************************************************
T_BOOL Nand_IsBadBlock(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block )
{
   	 T_U32 blk_per_chip, phyBlk;
	 
	 blk_per_chip = pNF_Info->BlockPerPlane * pNF_Info->PlanePerChip;
	 phyBlk = chip * blk_per_chip + block;

     return FHA_check_bad_block(phyBlk);  
}

//***********************************************************************************

T_VOID Nand_SetBadBlock(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block)
{
    T_U32 blk_per_chip, phyBlk;
         
    blk_per_chip = pNF_Info->BlockPerPlane * pNF_Info->PlanePerChip;
    phyBlk = chip * blk_per_chip + block;
    
    FHA_set_bad_block(phyBlk);
}

//***********************************************************************************
//分离的data and oob
T_VOID Nand_Config_Data(T_PNAND_ECC_STRU data_ctrl,  T_PNAND_ECC_STRU spare_ctrl, T_U8 *data, T_U32 datalen, T_U8 *oob, T_U32 oob_len, T_U8 ecctype)
{
    if (AK_NULL != data_ctrl)
    {
        data_ctrl->buf = data;
        data_ctrl->buf_len = datalen;
        data_ctrl->ecc_section_len = (ecctype > ECC_16BIT_P512B) ? NAND_DATA_SIZE_P1KB : NAND_DATA_SIZE_P512B;
        data_ctrl->ecc_type = ecctype;
    }

    if (AK_NULL != spare_ctrl)
    {
        spare_ctrl->buf = oob;
        spare_ctrl->buf_len = oob_len;
        spare_ctrl->ecc_section_len = oob_len;
        spare_ctrl->ecc_type = (ecctype > ECC_12BIT_P512B) ? ECC_12BIT_P512B : ecctype;
    }

} 

//不分离的data and oob
static T_VOID Nand_Config_Data_SmallPage(T_PNANDFLASH pNF_Info, T_PNAND_ECC_STRU data_ctrl,  T_PNAND_ECC_STRU spare_ctrl, T_U8* data, T_U8 *spare, T_U32 spare_len)
{
    T_PNandflash_Add pNF_Add = (T_PNandflash_Add)(&pNF_Info[1]);
    
    data_ctrl->buf = data;
    data_ctrl->buf_len = NAND_DATA_SIZE_P512B;
    data_ctrl->ecc_section_len = NAND_DATA_SIZE_P512B + spare_len;
    data_ctrl->ecc_type = pNF_Add->EccType;

    spare_ctrl->buf = spare;
    spare_ctrl->buf_len= spare_len;
    spare_ctrl->ecc_section_len = data_ctrl->ecc_section_len;
    spare_ctrl->ecc_type = pNF_Add->EccType;
} 

static E_NANDERRORCODE Nand_ChangeReturnValue(T_U32 ChipCharacterBits, T_U32 ret)
{
    T_U32 ECCType, WeakDangerBits, StrongDangerBits;
    
    if(ret & ((1UL << FINAL_FAIL_BIT) | (1UL << L2_ALLOC_FAIL_BIT) | (1UL << TIME_OUT_BIT)))
    {
        return NF_FAIL;
    }
    else if(ret & (1UL << ONCE_OPERATION_FAILED_BIT))
    {
        return NF_STRONG_DANGER;
    }
    
    ECCType = (ChipCharacterBits >> 4) & 0xF;
    if(ECC_4BIT_P512B == ECCType)
    {
        WeakDangerBits = WEAK_DANGER_BIT_NUM_MODE0;
        StrongDangerBits = STRONG_DANGER_BIT_NUM_MODE0;
    }
    else if(ECC_8BIT_P512B == ECCType)
    {
        WeakDangerBits = WEAK_DANGER_BIT_NUM_MODE1;
        StrongDangerBits = STRONG_DANGER_BIT_NUM_MODE1;
    }
    else if(ECC_12BIT_P512B == ECCType)
    {
        WeakDangerBits = WEAK_DANGER_BIT_NUM_MODE2;
        StrongDangerBits = STRONG_DANGER_BIT_NUM_MODE2;
    }
    else if(ECC_24BIT_P1KB == ECCType)
    {
        WeakDangerBits = WEAK_DANGER_BIT_NUM_MODE4;
        StrongDangerBits = STRONG_DANGER_BIT_NUM_MODE4;
    }
    else    //default is ECC_32BIT_P1KB
    {
        WeakDangerBits = WEAK_DANGER_BIT_NUM_MODE5;
        StrongDangerBits = STRONG_DANGER_BIT_NUM_MODE5;
    }
    ret = ret & 0xFF;
    if(ret < WeakDangerBits)
    {
        return NF_SUCCESS;
    }
    else if (ret < StrongDangerBits)
    {
        return NF_WEAK_DANGER;
    }
    else
    {
        return NF_STRONG_DANGER;
    }
}

//***********************************************************************************

/**
 * @brief   read 1 sector data from nandflash with ECC.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @update  2011-04-07 conjoin pSpareCtrl and RowAddr together to form a new spare
 *
 * @param   [in] pNand_Info the struct of nandflash.
 * @param   [in] chip which chip will be read.
 * @param   [in] block which block will be read.
 * @param   [in] sector which sector will be read.
 * @param   [in] data buffer for read sector, should be 512 bytes.
 * @param   [in] spare buffer for file system infomation, should be 4 bytes.
 * @return  E_NANDERRORCODE
 */
E_NANDERRORCODE Nand_ReadSector(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block, T_U32 page, T_U8 data[], T_U8 *oob, T_U32 oob_len)
{
    T_U32   ret, rowAddr;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    E_NANDERRORCODE result;
    T_PNandflash_Add pNF_Add = (T_PNandflash_Add)(&pNF_Info[1]);
    T_U8 new_spare_buffer[20]; //conjoin pSpareCtrl and RowAddr together to form a new spare
  
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }

    rowAddr = block * pNF_Info->PagePerBlock + page;

    if (NAND_512B_PAGE == pNF_Add->ChipType)
    {
        Nand_Config_Data_SmallPage(pNF_Info, &data_ctrl, &spare_ctrl, data, oob, oob_len);
    }
    else
    {
        Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                 data, pNF_Info->BytesPerSector,
                 (T_U8 *)new_spare_buffer, oob_len + sizeof(rowAddr), pNF_Add->EccType);
    }
    
   // ret = nand_readpage_ecc(pNF_Add->ChipPos[chip], rowAddr, 0, pNF_Add, &data_ctrl, &spare_ctrl);

    if (NAND_512B_PAGE != pNF_Add->ChipType)
    {
        memcpy(oob, (T_U8 *)new_spare_buffer, oob_len);
    }
    
    result = Nand_ChangeReturnValue(pNF_Info->ChipCharacterBits, ret);

    ret = ret & 0xff;

    if (NF_FAIL == result)
    {
        printf("C=%d,B=%d,P=%d,mflip=%d,spare_0x%x_0x%x,RL\n", chip, block, page, ret & 0xff, *((T_U32 *)oob), *((T_U32 *)(oob + 4)));
    }
    else if (NF_WEAK_DANGER == result)
    {
        printf("C=%d,B=%d,P=%d,mflip=%d,wdanger!\n", chip, block, page, ret); //weak danger!
    }    
    else if (NF_STRONG_DANGER == result)
    {
        printf("C=%d,B=%d,P=%d,mflip=%d,sdanger!\n", chip, block, page, ret); //strong danger!!
    }

    return result;
}
//***********************************************************************************
E_NANDERRORCODE Nand_ReadFlag(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block, T_U32 page, T_U8 *oob, T_U32 oob_len)
{
    T_U32   rowAddr, columnAddr, ret;
    T_NAND_ECC_STRU spare_ctrl;
    T_PNandflash_Add pNF_Add = (T_PNandflash_Add)(&pNF_Info[1]);
    T_U32 ecc_type_limit = (pNF_Add->EccType > ECC_12BIT_P512B) ? ECC_12BIT_P512B : pNF_Add->EccType;
    T_U8 new_spare_buffer[20]; //conjoin pSpareCtrl and RowAddr together to form a new spare
      
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }

    rowAddr = block * pNF_Info->PagePerBlock + page;

    if (NAND_512B_PAGE == pNF_Add->ChipType) 
    {
	 	T_NAND_ECC_STRU data_ctrl;
        T_U8 data_tmp[NAND_DATA_SIZE_P512B];

        Nand_Config_Data_SmallPage(pNF_Info, &data_ctrl, &spare_ctrl, (T_U8*)data_tmp, oob, oob_len);
        
      //  ret = nand_readpage_ecc(pNF_Add->ChipPos[chip],rowAddr, 0, pNF_Add, &data_ctrl, &spare_ctrl);
        
    }
    else
    {
     	spare_ctrl.buf = new_spare_buffer;
        spare_ctrl.buf_len= oob_len + sizeof(rowAddr);
        spare_ctrl.ecc_section_len = oob_len + sizeof(rowAddr);
        spare_ctrl.ecc_type = ecc_type_limit;

        if(pNF_Add->EccType <= ECC_12BIT_P512B)
        {
            columnAddr = (NAND_DATA_SIZE_P512B + (pNF_Add->EccType + 1) * NAND_PARITY_SIZE_MODE0) * (pNF_Info->BytesPerSector / NAND_DATA_SIZE_P512B);
        }
        else
        {
            columnAddr = (NAND_DATA_SIZE_P1KB + (pNF_Add->EccType - 1) * NAND_PARITY_SIZE_MODE1) * (pNF_Info->BytesPerSector / NAND_DATA_SIZE_P1KB);
        } 
     
      //  ret = nand_readpage_ecc(pNF_Add->ChipPos[chip],rowAddr, columnAddr, pNF_Add, AK_NULL, &spare_ctrl);

        memcpy(oob, (T_U8 *)new_spare_buffer, oob_len);  
    }   
    
    if(ret & (1UL << FINAL_FAIL_BIT))
    {
        printf("C=%d,B=%d,P=%d,spare_0x%x_0x%x,RS\n", chip, block, page, *((T_U32 *)oob), *((T_U32 *)(oob + 4)));
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }

}
//***********************************************************************************
E_NANDERRORCODE Nand_WriteSector(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block,
                                        T_U32 page, T_U8 data[], T_U8 *oob, T_U32 oob_len)
{
    T_U32   rowAddr, ret; 
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_PNandflash_Add pNF_Add = (T_PNandflash_Add)(&pNF_Info[1]);
    T_U8 new_spare_buffer[20]; //conjoin pSpareCtrl and RowAddr together to form a new spare
                               //new spare =  oob(oob_len bytes)  |  rowaddr(3 bytes)  |  chip(1 byte)
    
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }

    rowAddr = block * pNF_Info->PagePerBlock + page;

    if (NAND_512B_PAGE == pNF_Add->ChipType)
    {
        Nand_Config_Data_SmallPage(pNF_Info, &data_ctrl, &spare_ctrl, data, oob, oob_len);
    }
    else
    {
        //1: form new spare buffer
        memcpy((T_U8 *)new_spare_buffer, oob, oob_len);
        memcpy((T_U8 *)new_spare_buffer + oob_len, (T_U8 *)(&rowAddr), sizeof(rowAddr) - 1);
        new_spare_buffer[oob_len + sizeof(rowAddr) - 1] = chip;
        //2: change length of new spare buffer
        oob_len = oob_len + sizeof(rowAddr);
    
        Nand_Config_Data(&data_ctrl, &spare_ctrl, 
             data, pNF_Info->BytesPerSector,
             (T_U8 *)new_spare_buffer, oob_len, pNF_Add->EccType);
    }

  //  ret = nand_writepage_ecc(pNF_Add->ChipPos[chip], rowAddr, 0, pNF_Add, &data_ctrl, &spare_ctrl);

    if(0 != ret)
    {
        printf("C=%d,B=%d,P=%d,spare_0x%x_0x%x,WL\n", chip, block, page, *((T_U32 *)oob), *((T_U32 *)(oob + 4)));
        return NF_FAIL;
    }    
    else
    {
        return NF_SUCCESS;
    }    
}

//***********************************************************************************
E_NANDERRORCODE Nand_WriteFlag(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block, T_U32 page, T_U8 *oob, T_U32 oob_len)
{
    T_U8 *data;
    E_NANDERRORCODE ret = 0;

    data = (T_U8 *)Fwl_Malloc(pNF_Info->BytesPerSector);
    memset(data, 0x5A, pNF_Info->BytesPerSector);
    ret = Nand_WriteSector(pNF_Info, chip, block, page, data, oob, oob_len);
    Fwl_Free(data);
    return ret;
}

//***********************************************************************************
E_NANDERRORCODE Nand_EraseBlock(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block)
{
    T_U32 blk_start_page, ret;
    T_PNandflash_Add pNF_Add = (T_PNandflash_Add)(&pNF_Info[1]);

    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }

    blk_start_page = block * pNF_Info->PagePerBlock;

    //ret = nand_eraseblock(pNF_Add->ChipPos[chip], blk_start_page, pNF_Add);
        
    if(0 != ret)
    {
        printf("C=%d,B=%d,EB\n", chip, block);
        return NF_FAIL;
    }    
    else
    {
        return NF_SUCCESS;
    }     
}
//***********************************************************************************
/**
 * @brief   copy one physical page to another one, with hardware.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNand_Info the struct of nandflash.
 * @param   [in] chip which chip will be operated.
 * @param   [in] SourceBlock read the source block.
 * @param   [in] DestBlock write to destination block.
 * @param   [in] page the page of the block will be copy.
 * @return  E_NANDERRORCODE
 */
E_NANDERRORCODE Nand_CopyBack(T_PNANDFLASH nand, T_U32 chip, T_U32 SourceBlock, T_U32 DestBlock, T_U32 page)
{
    T_U32   SouPhyPage, DesPhyPage, ret;
    T_PNandflash_Add pNand_Add = (T_PNandflash_Add)(&nand[1]);

    if ((m_fake_block_per_die != m_true_block_per_die))
    {
        if (SourceBlock >= m_true_block_per_die)
        {
            SourceBlock = (SourceBlock % m_true_block_per_die) + m_fake_block_per_die;
        }

        if (DestBlock >= m_true_block_per_die)
        {
            DestBlock = (DestBlock % m_true_block_per_die) + m_fake_block_per_die;
        }    
    }

    SouPhyPage = page + (SourceBlock * nand->PagePerBlock);
    DesPhyPage = page + (DestBlock * nand->PagePerBlock);

 //   ret = nand_copyback(pNand_Add->ChipPos[chip], SouPhyPage, DesPhyPage, pNand_Add);
    
    if(0 != ret)
    {
        printf("C=%d,SB=%d,DB=%d,P=%d,CB\n", chip, SourceBlock, DestBlock, page);
        return NF_FAIL;
    }    
    else
    {
        return NF_SUCCESS;
    }    
}


static E_NANDERRORCODE Nand_MultiWriteSector(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page,const T_U8 data[], T_U8* SpareTbl,T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_U32 ecc_len = 0;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 plane0_rowaddr = 0;   //row address of the block in plane 0
    T_U32 plane1_rowaddr = 0;   //row address of the block in plane 1
    T_U32 ret = 0;
    T_U8 new_spare_len = 0;
    T_U8 new_spare_buffer[30]; //conjoin spare_ctrl and RowAddr together to form a new spare
                               //new spare =  oob(oob_len bytes)  |  rowaddr(3 bytes)  |  chip(1 byte)
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);

    //1: calculate row address of two planes, and new spare length
    plane0_rowaddr = block * pNand_Add->PagesPerBlock + page;
    plane1_rowaddr = (block + 1) * pNand_Add->PagesPerBlock + page;
    new_spare_len = oob_len + sizeof(plane0_rowaddr);

    //2: form new spare buffer from row address and chip in plane 0
    memcpy((T_U8 *)new_spare_buffer, SpareTbl, oob_len);
    memcpy((T_U8 *)new_spare_buffer + oob_len, (T_U8 *)(&plane0_rowaddr), sizeof(plane0_rowaddr) - 1);
    new_spare_buffer[new_spare_len - 1] = chip;
    
    //3: form new spare buffer from row address and chip in plane 1
    memcpy((T_U8 *)new_spare_buffer + new_spare_len, SpareTbl + oob_len, oob_len);
    memcpy((T_U8 *)new_spare_buffer + new_spare_len + oob_len, (T_U8 *)(&plane1_rowaddr), sizeof(plane1_rowaddr) - 1);
    new_spare_buffer[(new_spare_len * 2) - 1] = chip;
    
    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                 data, nand->BytesPerSector,
                 (T_U8 *)new_spare_buffer, new_spare_len, pNand_Add->EccType);

 //   ret = nand_writepage_ecc_2plane(pNand_Add->ChipPos[chip], plane0_rowaddr, 0, 
  //                           pNand_Add, &data_ctrl, &spare_ctrl);
    
    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}
static E_NANDERRORCODE Nand_MultiReadSector(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page,T_U8 data[], T_U8* SpareTbl,T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 plane0_rowaddr = 0;  //row address of the block in plane 0
    T_U32 plane1_rowaddr = 0;  //row address of the block in plane 1
    T_U8 new_spare_buffer[30]; //conjoin pSpareCtrl and RowAddr together to form a new spare
    T_U32 ret;
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    
    plane0_rowaddr = block * pNand_Add->PagesPerBlock + page;
    plane1_rowaddr = (block + 1) * pNand_Add->PagesPerBlock + page;

    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                   data, nand->BytesPerSector,
                   (T_U8 *)new_spare_buffer, oob_len + sizeof(plane0_rowaddr), pNand_Add->EccType);

  //  ret = nand_readpage_ecc_2plane(pNand_Add->ChipPos[chip], plane0_rowaddr, 0, 
  //                             pNand_Add, &data_ctrl, &spare_ctrl);
  
    memcpy(SpareTbl, (T_U8 *)new_spare_buffer, oob_len);
    memcpy(SpareTbl + oob_len, (T_U8 *)new_spare_buffer + oob_len + sizeof(plane0_rowaddr), oob_len);

    return Nand_ChangeReturnValue(nand->ChipCharacterBits, ret);
}

static E_NANDERRORCODE Nand_MultiCopyBack(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 SourceBlock, T_U32 DestBlock, T_U32 page)
{
    T_U32   SouPhyPage, DesPhyPage, ret;
    T_PNandflash_Add pNand_Add = (T_PNandflash_Add)(&nand[1]);

    SouPhyPage = page + (SourceBlock * nand->PagePerBlock);
    DesPhyPage = page + (DestBlock * nand->PagePerBlock);

//    ret = nand_copyback_2plane(pNand_Add->ChipPos[chip], SouPhyPage, DesPhyPage, pNand_Add);

    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}

static E_NANDERRORCODE Nand_MultiEraseBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 planeNum, T_U32 block)
{
    T_U32 blk_start_page, ret;
    T_PNandflash_Add pNand_Add = (T_PNandflash_Add)(&nand[1]);

    blk_start_page = block * nand->PagePerBlock;

    //ret = nand_eraseblock_2plane(pNand_Add->ChipPos[chip], blk_start_page, pNand_Add);
    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}
static E_NANDERRORCODE Nand_WriteSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, const T_U8 data[], T_U8 *spare_tbl, T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_U32 ecc_len = 0;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 rowAddr = 0;
    T_U32 ret = 0;      

    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }
    
    rowAddr = block * pNand_Add->PagesPerBlock + page;
    if(NAND_512B_PAGE == pNand_Add->ChipType)
    {
        Nand_Config_Data_SmallPage(nand, &data_ctrl,
                &spare_ctrl, data, 
                 spare_tbl, oob_len - sizeof(page));
    }
    else
    {   
        Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                 data, nand->BytesPerSector,
                 spare_tbl, oob_len, pNand_Add->EccType);
        
    *(T_U32 *)(spare_tbl + oob_len - sizeof(page)) = page;
    }
  //  ret = nand_writepage_ecc(pNand_Add->ChipPos[chip], rowAddr, 0, 
  //                           pNand_Add, &data_ctrl, &spare_ctrl);
    
    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}

static E_NANDERRORCODE Nand_ReadSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8 data [ ], T_U8 * spare_tbl, T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    E_NANDERRORCODE fwl_ret;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 rowAddr = 0, drv_ret;
    T_U32 page_offset = 0;  //record page offset in a block
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }
    
    rowAddr = block * pNand_Add->PagesPerBlock + page;

    if(NAND_512B_PAGE == pNand_Add->ChipType)
    {
        Nand_Config_Data_SmallPage(nand, &data_ctrl,
                   &spare_ctrl, data, 
                    spare_tbl, oob_len - sizeof(page));
    }
    else
    {
        Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                     data, nand->BytesPerSector,
                     spare_tbl, oob_len , pNand_Add->EccType);
    }
    
 //   drv_ret = nand_readpage_ecc(pNand_Add->ChipPos[chip], rowAddr, 0, 
  //                             pNand_Add, &data_ctrl, &spare_ctrl);
    
    fwl_ret = Nand_ChangeReturnValue(nand->ChipCharacterBits, drv_ret);

    if(NF_SUCCESS == fwl_ret)
    {
        page_offset = *(T_U32 *)(spare_tbl + oob_len - sizeof(page));

        if ((0xffffffff != page_offset) && (page != page_offset))
        {
            printf("C%d_B%d_P%d != 0x%x,RL###\n", chip, block, page, page_offset);
        }
    }
    
    return fwl_ret;
}

static E_NANDERRORCODE Nand_MultiWriteSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, const T_U8 data [ ], T_U8 * spare_tbl, T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_U32 ecc_len = 0;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 plane0_rowaddr = 0;   //row address of the block in plane 0
    T_U32 ret = 0;   
    pNand_Add = (T_PNandflash_Add)(&nand[1]);

    //1: calculate row address of  plane0
    plane0_rowaddr = block * pNand_Add->PagesPerBlock + page;

    //2: form new spare buffer from row address in plane 0
    *(T_U32 *)(spare_tbl + oob_len - sizeof(page)) = page;

    //3: form new spare buffer from row address  in plane 1
    *(T_U32 *)(spare_tbl + (oob_len<<1) - sizeof(page)) = page;
    
    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                 data, nand->BytesPerSector,
                 spare_tbl, oob_len, pNand_Add->EccType);
    
 //   ret = nand_writepage_ecc_2plane(pNand_Add->ChipPos[chip], plane0_rowaddr, 0, 
  //                           pNand_Add, &data_ctrl, &spare_ctrl);
    
    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}

static E_NANDERRORCODE Nand_MultiReadSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page,T_U8 data[], T_U8* spare_tbl,T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    E_NANDERRORCODE fwl_ret;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 plane0_rowaddr = 0;  //row address of the block in plane 0
    T_U32 page_offset0;  //record page offset in a block 
    T_U32 page_offset1;  //record page offset in a block
    T_U32 drv_ret;
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    
    plane0_rowaddr = block * pNand_Add->PagesPerBlock + page;
    
    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                      data, nand->BytesPerSector,
                       spare_tbl, oob_len, pNand_Add->EccType);

//    drv_ret = nand_readpage_ecc_2plane(pNand_Add->ChipPos[chip], plane0_rowaddr, 0, 
 //                              pNand_Add, &data_ctrl, &spare_ctrl);

    fwl_ret = Nand_ChangeReturnValue(nand->ChipCharacterBits, drv_ret);
    
     if(NF_SUCCESS == fwl_ret)
     {

    page_offset0 =  *(T_U32 *)(spare_tbl + oob_len - sizeof(page));

    page_offset1 =  *(T_U32 *)(spare_tbl + (oob_len<<1) - sizeof(page));

    if (((0xffffffff != page_offset0) && (page != page_offset0))
        || ((0xffffffff != page_offset1) && (page != page_offset1)))
    {
        printf("C%d_B%d_P%d != 0x%x_0x%x,MRL***\n", chip, block, page, page_offset0, page_offset1);
        
    }
     }

    return fwl_ret;
}

static E_NANDERRORCODE Nand_ReadSector_Cache(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8 data [ ], T_U8 * spare_tbl, T_U32 oob_len, T_U32 page_num)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    E_NANDERRORCODE fwl_ret;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 rowAddr = 0, drv_ret;
    T_U32 i; 
    T_U32 page_offset = 0;  //record page offset in a block
    T_U32 spare_tbl_offset = 0;

    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }
    
    rowAddr = block * pNand_Add->PagesPerBlock + page;
    

    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                    data, nand->BytesPerSector,
                    spare_tbl, oob_len, pNand_Add->EccType);

   // drv_ret = nand_readpage_cache_ecc(pNand_Add->ChipPos[chip], rowAddr, 0,
   //                            page_num, pNand_Add, &data_ctrl, &spare_ctrl);
       
   fwl_ret = Nand_ChangeReturnValue(nand->ChipCharacterBits, drv_ret);
    
   if(NF_SUCCESS == fwl_ret)
   {
        spare_tbl_offset = (T_U32)spare_tbl - sizeof(page);
        for(i = 0; i < page_num; i++)
        {   
            spare_tbl_offset += oob_len;

            page_offset = *(T_U32 *)spare_tbl_offset;
            
            if((0xffffffff != page_offset) && ((page + i) != page_offset))
            {
                printf("C%d_B%d_P%d != 0x%x,CRL***\n", chip, block, page + i, page_offset);
            
            }
            
        }
    }
   
    return fwl_ret;
    
}
static E_NANDERRORCODE Nand_MultiReadSector_Cache(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page, T_U8 data [ ], T_U8 * spare_tbl, T_U32 oob_len, T_U32 page_num)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    E_NANDERRORCODE fwl_ret;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 plane0_rowaddr = 0;  //row address of the block in plane 0
    T_U32 drv_ret;
    T_U32 i; 
    T_U32 page_offset = 0;  //record page offset in a block
    T_U32 spare_tbl_offset = 0;
     
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    
    plane0_rowaddr = block * pNand_Add->PagesPerBlock + page;

  //  Nand_Config_Data(&data_ctrl, &spare_ctrl, 
  //                   data, nand->BytesPerSector,
  //                   spare_tbl, oob_len, pNand_Add->EccType);


   // drv_ret = nand_readpage_cache_ecc_2plane(pNand_Add->ChipPos[chip], plane0_rowaddr, 0, 
//                               (page_num >> 1), pNand_Add, &data_ctrl, &spare_ctrl);
   // fwl_ret = Nand_ChangeReturnValue(nand->ChipCharacterBits, drv_ret);
     
    if(NF_SUCCESS == fwl_ret)
    {

    spare_tbl_offset = (T_U32)spare_tbl - sizeof(page);
    for(i = 0; i < page_num; i++)
    {
        spare_tbl_offset += oob_len;
        page_offset = *(T_U32 *)spare_tbl_offset;

        if((0xffffffff != page_offset) && ((page + (i >> 1)) != page_offset))
        {
            printf("C%d_B%d_P%d != 0x%x,MCRL***\n", chip, block + (i & 0x1), page + (i >> 1), page_offset);
        }

    }
    }

    return fwl_ret;    
}

static E_NANDERRORCODE Nand_WriteSector_Cache(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, const T_U8 data [ ], T_U8 * spare_tbl, T_U32 oob_len , T_U32 page_num)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 rowAddr = 0;
    T_U32 ret = 0;
    T_U32 i; 
    T_U32 page_offset = 0;  //record page offset in a block
    T_U32 spare_tbl_offset = 0;
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }
    
    rowAddr = block * pNand_Add->PagesPerBlock + page;

    //Nand_Config_Data(&data_ctrl, &spare_ctrl, 
    //                data, nand->BytesPerSector,
    //                spare_tbl, oob_len, pNand_Add->EccType);
    


    spare_tbl_offset = (T_U32)spare_tbl - sizeof(page);
    page_offset = page;
    for(i = 0; i < page_num; i++)
    {
        spare_tbl_offset += oob_len;
        *(T_U32 *)(spare_tbl_offset) = page_offset;
        page_offset++;
    }


   // ret = nand_writepage_cache_ecc(pNand_Add->ChipPos[chip], rowAddr, 0, 
   //                          page_num, pNand_Add, &data_ctrl, &spare_ctrl);
    
    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }

}


static E_NANDERRORCODE Nand_MultiWriteSector_Cache(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page, const T_U8 data [ ], T_U8 * spare_tbl, T_U32 oob_len, T_U32 page_num)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 plane0_rowaddr = 0;   //row address of the block in plane 0
    T_U32 ret = 0;
    T_U32 i; 
    T_U32 page_offset = 0;  //record page offset in a block  
    T_U32 spare_tbl_offset = 0;
     
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    plane0_rowaddr = block * pNand_Add->PagesPerBlock + page;


  //  Nand_Config_Data(&data_ctrl, &spare_ctrl, 
  //          data, nand->BytesPerSector,
  //          spare_tbl, oob_len, pNand_Add->EccType);

    spare_tbl_offset = (T_U32)spare_tbl - sizeof(page);
    page_offset = page;

    for(i = 0; i < page_num; i++)
    {   
        spare_tbl_offset += oob_len;
        *(T_U32 *)spare_tbl_offset = page_offset;
        page_offset += (i & 0x1);//modify page offset
    }

  //  ret = nand_writepage_cache_ecc_2plane(pNand_Add->ChipPos[chip], plane0_rowaddr, 0, 
  //                           (page_num >> 1), pNand_Add, &data_ctrl, &spare_ctrl);
    
    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }    
}
static E_NANDERRORCODE Nand_ReadFlag_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8 *spare_tbl,T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU spare_ctrl;
    T_NAND_ECC_STRU data_ctrl;
    T_U32 rowAddr = 0, columnAddr = 0;
    T_U32 ret = 0;
    T_U32 page_offset;  //record chip and rowaddr information, = (rowAddr | (chip << 24))
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    


    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }
    rowAddr = block * pNand_Add->PagesPerBlock + page;


    if (NAND_512B_PAGE == pNand_Add->ChipType) 
    {
        T_NAND_ECC_STRU data_ctrl;
        T_U8 data_tmp[NAND_DATA_SIZE_P512B];

    //    Nand_Config_Data_SmallPage(nand, &data_ctrl, &spare_ctrl, (T_U8*)data_tmp, spare_tbl, oob_len - sizeof(page));
        
//        ret = nand_readpage_ecc(pNand_Add->ChipPos[chip],rowAddr, 0, pNand_Add, &data_ctrl, &spare_ctrl);
        
    }
    else
    {
        
      //  Nand_Config_Data(AK_NULL, &spare_ctrl, 
       //              AK_NULL, 0,
         //            spare_tbl, oob_len, pNand_Add->EccType);
    
        if(pNand_Add->EccType <= ECC_12BIT_P512B)
        {
            columnAddr = (NAND_DATA_SIZE_P512B + (pNand_Add->EccType + 1) * NAND_PARITY_SIZE_MODE0) * (nand->BytesPerSector / NAND_DATA_SIZE_P512B);
        }

        else
        {
            columnAddr = (NAND_DATA_SIZE_P1KB + (pNand_Add->EccType - 1) * NAND_PARITY_SIZE_MODE1) * (nand->BytesPerSector / NAND_DATA_SIZE_P1KB);
        } 

  //      ret = nand_readpage_ecc(pNand_Add->ChipPos[chip], rowAddr, columnAddr, 
  //                               pNand_Add, AK_NULL, &spare_ctrl);
        
        page_offset = *(T_U32 *)(spare_tbl + oob_len - sizeof(page));

        if ((0xffffffff != page_offset) && (page_offset != page))
        {
            printf("C%d_B%d_P%d != 0x%x,RS$$$\n", chip, block, page, page_offset);
          
        }
    }

    if(ret & (1UL << FINAL_FAIL_BIT))
    {
        printf("C=%d,B=%d,P=%d,spare_0x%x_0x%x,RS\n", chip, block, page, *((T_U32 *)spare_tbl), page_offset);
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}

/**
* @brief reading interface for exNFTL
* 
* @author yang_yiming
* @date 2011.11.14
*
* @param   [in] pNand_Info    the struct of nandflash.
* @param   [in] chip   which chip to be operated.
* @param   [in] plane_num  indicating the num of plane to be operated
* @param   [in] block   which block to be operated.
* @param   [in] page   which page to be operated.
* @param   [out] data   buffer for data read from nand
* @param   [out] spare_tbl   buffer for file system infomation
* @param   [in] page_num   the number of page to be read
* @return    E_NANDERRORCODE
*/

static E_NANDERRORCODE Nand_ReadSector_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block, T_U32 page,T_U8 data[], T_U8* spare_tbl,T_U32 oob_len, T_U32 page_num)
{
    E_NANDERRORCODE ret;
    T_U32 i;
    T_U8 *data_shift, *oob_shift;

    if(1 == plane_num)
    {
        if((1 == page_num) || (0 == ((nand->ChipCharacterBits) & 0x4))) //when page_num = 1 or the cache operation is not supported
        {
            for(i = 0; i < page_num; i++)
            {
                data_shift = &data[i * nand->BytesPerSector];
                oob_shift = (T_U32)spare_tbl + (i * oob_len);
                ret = Nand_ReadSector_Ex(nand, chip,  block, page + i, data_shift, oob_shift, oob_len);
                
                if(NF_FAIL == ret)
                {
                    printf("ExSingleReading failed at block %d, page %d\n", block, page + i);
                    goto EXITR;
                }
            }
        }
        else
        {
              ret = Nand_ReadSector_Cache(nand, chip, block, page, data, spare_tbl, oob_len, page_num);
        }
    }
    else
    {
        if((2 == page_num) || (0 == ((nand->ChipCharacterBits) & 0x4)))//when page_num = 2 or the cache operation is not supported
        {
            for(i = 0; i < page_num; i += 2)
            {
                data_shift = &data[i * nand->BytesPerSector];
                oob_shift = (T_U32)spare_tbl + (i * oob_len);
                ret = Nand_MultiReadSector_Ex(nand, chip,  block, page + (i >> 1), data_shift, oob_shift, oob_len);
                
                if(NF_FAIL == ret)
                {
                    printf("ExMultiReading failed at block %d, page %d\n", block, page + (i >> 1));
                    goto EXITR;
                }
            }
            
        }
        else
        {
            ret = Nand_MultiReadSector_Cache(nand, chip, plane_num, block, page, data, spare_tbl, oob_len, page_num);
        }
    }

EXITR:
    return ret;

}

/**
* @brief writing interface for exNFTL
* 
* @author yang_yiming
* @date 2011.11.14
*
* @param   [in] pNand_Info    the struct of nandflash.
* @param   [in] chip   which chip to be operated.
* @param   [in] plane_num  indicating if the chip was construted of odd-even planes ,1 for not, 2 for be
* @param   [in] block   which block to be operated.
* @param   [in] page   which page to be operated.
* @param   [in] data   buffer stored with the data to be writen
* @param   [in] spare_tbl   buffer stored with the file system infomation to be writen 
* @param   [in] page_num   the number of page to be writen
* @return    E_NANDERRORCODE
*/
static E_NANDERRORCODE Nand_WriteSector_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block, T_U32 page,const T_U8 data[], T_U8* spare_tbl,T_U32 oob_len, T_U32 page_num)
{
    E_NANDERRORCODE ret;
    T_U32 i;
    T_U8 *data_shift, *oob_shift;

    if(1 == plane_num)
    {
        if((1 == page_num) || (0 == ((nand->ChipCharacterBits) & 0x2))) //when page_num = 1 or the cache operation is not supported
        {
            for(i = 0; i < page_num; i++)
            {
                data_shift = &data[i * nand->BytesPerSector];
                oob_shift = (T_U32)spare_tbl + (i * oob_len);
                ret = Nand_WriteSector_Ex(nand, chip,  block, page + i, data_shift, oob_shift, oob_len);
                
                if(NF_FAIL == ret)
                {
                    printf("ExSingleWriting failed at block %d, page %d\n", block, page + i);
                    goto EXITW;
                }
            }
        }
        else
        {
              ret = Nand_WriteSector_Cache(nand, chip, block, page, data, spare_tbl, oob_len, page_num);
        }
    }
    else
    {
        if((2 == page_num) || (0 == ((nand->ChipCharacterBits) & 0x2)))//when page_num = 2 or the cache operation is not supported
        {
            for(i = 0; i < page_num; i += 2)
            {
                data_shift = &data[i * nand->BytesPerSector];
                oob_shift = (T_U32)spare_tbl + (i * oob_len);
                ret = Nand_MultiWriteSector_Ex(nand, chip,  block, page + (i >> 1), data_shift, oob_shift, oob_len);
                
                if(NF_FAIL == ret)
                {
                    printf("ExMultiWriting failed at block %d, page %d\n", block, page + (i >> 1));
                    goto EXITW;
                }
            }
            
        }
        else
        {
            ret = Nand_MultiWriteSector_Cache(nand, chip, plane_num, block, page, data, spare_tbl, oob_len, page_num);
        }
    }

EXITW:

    return ret;
 
}

/**
* @brief erasing interface for exNFTL
* 
* @author yang_yiming
* @date 2011.11.14
*
* @param   [in] pNand_Info    the struct of nandflash.
* @param   [in] chip   which chip to be operated.
* @param   [in] plane_num  indicating if the chip was construted of odd-even planes ,1 for not, 2 for be
* @param   [in] block   which block to be operated.
* @return    E_NANDERRORCODE
*/

static E_NANDERRORCODE Nand_EraseBlock_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block)
{
    E_NANDERRORCODE ret;
        
    if((1 == plane_num) || (0 == nand->ChipCharacterBits >> 13))
    {
        ret = Nand_EraseBlock(nand, chip, block);
    }
    else//only if the plane_num = 2 & multi-operation supported, we called Nand_MultiEraseBlock
    {
        ret = Nand_MultiEraseBlock(nand, chip, plane_num, block);
    }

    return ret;
}


T_BOOL Burn_WriteBootPage(T_U32 page, T_U8 data[])
{
    T_PNandflash_Add pNF_Add = (T_PNandflash_Add)(&m_pNF_Info[1]);
    T_NAND_ECC_STRU data_ctrl;
    T_U32 boot_page_size = (pNF_Add->PageSize > 4096) ? 4096 : pNF_Add->PageSize;
    T_U32 boot_param_len;
    
    //nand_close_randomizer();
    
    if(0 == page)
    {   
#ifndef CHIP_AK37XX 
        boot_param_len = NAND_BOOT0_DATA_SIZE;
        data_ctrl.ecc_type = ECC_32BIT_P1KB;
#else
        boot_param_len = NAND_BOOT0_DATA_SIZE_37XX;
        data_ctrl.ecc_type = ECC_24BIT_P1KB;
#endif
        data_ctrl.buf = data;
        data_ctrl.buf_len = boot_param_len;
        data_ctrl.ecc_section_len = boot_param_len;
       
  //      if(0 != nand_writepage_ecc(0, page, 0, pNF_Add, &data_ctrl, AK_NULL))
        {
            //nand_reopen_randomizer();
        
            return AK_FALSE;
        }    
    }
    else
    {
        data_ctrl.buf = data;
        data_ctrl.buf_len = boot_page_size;
        data_ctrl.ecc_section_len = 512;
        data_ctrl.ecc_type = pNF_Add->EccType;
      
	    //for Micron MT29F32G08CBACA, 4KB page. but spare is just 224B, not enough to save 24 bit ecc info
        //so nandboot can only use 12 bit ecc!!! added on 20110421
        if ((pNF_Add->EccType >= ECC_24BIT_P1KB) && (4096 == pNF_Add->PageSize))
        {
            data_ctrl.ecc_type = ECC_12BIT_P512B;
        }
        
  //      if(0 != nand_writepage_ecc(0, page, 0, pNF_Add, &data_ctrl, AK_NULL))
        {
            //nand_reopen_randomizer();
            
            return AK_FALSE;
        }    
    }    

   // nand_reopen_randomizer();
    
    return AK_TRUE;
    
}
T_BOOL Burn_ReadBootPage(T_U32 page, T_U8 data[])
{
    T_PNandflash_Add pNF_Add = (T_PNandflash_Add)(&m_pNF_Info[1]);
    T_NAND_ECC_STRU data_ctrl;
    T_U32 boot_page_size = (pNF_Add->PageSize > 4096) ? 4096 : pNF_Add->PageSize;
    T_U32 ret;
    T_U32 boot_param_len;
    
   // nand_close_randomizer();
    
    if(0 == page) 
    {           
#ifndef CHIP_AK37XX 
        boot_param_len = NAND_BOOT0_DATA_SIZE;
        data_ctrl.ecc_type = ECC_32BIT_P1KB;
#else
        boot_param_len = NAND_BOOT0_DATA_SIZE_37XX;
        data_ctrl.ecc_type = ECC_24BIT_P1KB;
#endif
        data_ctrl.buf = data;
        data_ctrl.buf_len = boot_param_len;
        data_ctrl.ecc_section_len = boot_param_len;

 //       ret = nand_readpage_ecc(0, page, 0, pNF_Add, &data_ctrl, AK_NULL);
        
        if(ret & (1UL << FINAL_FAIL_BIT))
        {
           // nand_reopen_randomizer();
            
            return AK_FALSE;
        }       

        memset(data + boot_param_len, 0, boot_page_size - boot_param_len);
    }
    else
    {
        data_ctrl.buf = data;
        data_ctrl.buf_len = boot_page_size;
        data_ctrl.ecc_section_len = 512;
        data_ctrl.ecc_type = pNF_Add->EccType;

        //for Micron MT29F32G08CBACA, 4KB page. but spare is just 224B, not enough to save 24 bit ecc info
        //so nandboot can only use 12 bit ecc!!! added on 20110421
        if ((pNF_Add->EccType >= ECC_24BIT_P1KB) && (4096 == pNF_Add->PageSize))
        {
            data_ctrl.ecc_type = ECC_12BIT_P512B;
        }

//        ret = nand_readpage_ecc(0, page, 0, pNF_Add, &data_ctrl, AK_NULL);
        
        if(ret & (1UL << FINAL_FAIL_BIT))
        {
            //nand_reopen_randomizer();

            return AK_FALSE;
        }    
    } 
     
   // nand_reopen_randomizer();
    
    return AK_TRUE;
} 

T_U32 FHA_Nand_EraseBlock(T_U32 nChip,  T_U32 nPage)
{
    T_U32 ret = FHA_SUCCESS;
    T_PNandflash_Add pNF_Add = (T_PNandflash_Add)(&m_pNF_Info[1]);
    
    //if(0 != nand_eraseblock(pNF_Add->ChipPos[nChip], nPage, pNF_Add))
    {
         ret = FHA_FAIL;
    }

    return ret;
}

T_U32 FHA_Nand_WritePage(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType)
{
    T_U32 ret = FHA_SUCCESS;
    T_U32 block, page;

    block = nPage / m_pNF_Info->PagePerBlock;
    page = nPage % m_pNF_Info->PagePerBlock;

    if (FHA_DATA_BOOT != eDataType)
    {
        if(NF_SUCCESS != m_pNF_Info->WriteSector(m_pNF_Info, nChip, block, page, pData, pOob, nOobLen))
        {
             ret = FHA_FAIL;
        }
    }
    else
    {
        if (!Burn_WriteBootPage(nPage, pData))
        {
            ret = FHA_FAIL;
        }    
    }    

    return ret;
}

T_U32 FHA_Nand_ReadPage(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType)
{
    T_U32 ret = FHA_SUCCESS;
    T_U32 block, page;

    block = nPage / m_pNF_Info->PagePerBlock;
    page = nPage % m_pNF_Info->PagePerBlock;

    if (FHA_DATA_BOOT != eDataType)
    {
        if(NF_FAIL == m_pNF_Info->ReadSector(m_pNF_Info, nChip, block, page, pData, pOob, nOobLen))
        {
             ret = FHA_FAIL;
        }
    }
    else
    {
        if (!Burn_ReadBootPage(nPage, pData))
        {
            ret = FHA_FAIL;
        }    
    }    

    return ret;
}


T_BOOL ASA_ReadBytes(T_U32 chip, T_U32 rowAddr, T_U32 columnAddr, T_U8 data[], T_U32 len)
{
    T_PNandflash_Add pNF_Add = (T_PNandflash_Add)(&m_pNF_Info[1]);
    T_U32 block = rowAddr / pNF_Add->PagesPerBlock;
    
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
        rowAddr = (block * pNF_Add->PagesPerBlock) + (rowAddr % pNF_Add->PagesPerBlock);
    }

    if(0)//0 == nand_readbytes(pNF_Add->ChipPos[chip], rowAddr, columnAddr, pNF_Add, data, len))
    {
        return FHA_SUCCESS;
    }
    else
    {
        return FHA_FAIL;
    }    
} 
#endif

