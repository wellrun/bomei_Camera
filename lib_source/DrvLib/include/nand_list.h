/**
 * @file nand_list.h
 * @brief this is the information from nandflash
 *
 * This file provides nandflash basic information,so we can
 * search this information in order software fit hardware quickly
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Dengjian
 * @MODIFY  zhaojiahuan
 * @DATE    2006-7-17
 * @MODIFY Yang Yiming
 * @DATE    2012-12-24
 * @version 1.1
 * @
 */

#ifndef _NAND_LIST_H_
#define _NAND_LIST_H_

//bit31 reserved
//bit30 reserved
//bit29表示是否前后die，1表示有前后die
//bit28表示是否奇偶plane，1表示有奇偶plane
//bit27 reserved
//bit26 reserved
#define FLAG_HIGH_ID    (1 << 25)   //bit25表示是否有high id,    1表示支持
#define FLAG_ENHANCE_SLC    (1 << 24)   //bit24表示是否支持Enhanced SLC,    1表示支持
#define FLAG_READ_CACHE     (1 << 23)   //bit23表示是否支持multi-cache read，1表示支持     
#define FLAG_PROG_CACHE     (1 << 22)   //bit22表示是否支持multi-cache program，1表示支持  
#define FLAG_READ_2PCACHE   (1 << 21)   //bit21表示是否支持single-cache read，1表示支持    
#define FLAG_PROG_2PCACHE   (1 << 20)   //bit20表示是否支持single-cache program，1表示支持 
#define FLAG_READ_RETRY     (1 << 19)   //bit19表示是否支持Read-Retry功能，1表示支持                                                                                            
#define FLAG_STEP_BY_STEP   (1 << 18)   //bit18表示在同一个block内是否需要严格按1为步长顺序写page，1表示需要。象Micron MT29F32G08CBACA等MLC有此要求，起始page不一定要求是0      
#define FLAG_RANDOMIZER     (1 << 17)   //bit17表示是否需要randomizer，1表示需要                                                                                                
#define FLAG_COPY           (1 << 16)   //bit16表示是否支持single-plane copyback，1表示支持                                                                                     
#define FLAG_COPY_2P        (1 << 15)   //bit15表示是否支持multi-plane copyback，1表示支持                                                                                
#define FLAG_READ_2P        (1 << 14)   //bit14表示是否支持multi-plane read，1表示支持        
#define FLAG_ERASE_2P       (1 << 13)   //bit13表示是否支持multi-plane erase，1表示支持       
#define FLAG_PROG_2P        (1 << 12)   //bit12表示是否支持multi-plane program，1表示支持     
#define FLAG_LUN_GAP        (1 << 11)   //bit11表示plane_blk_num被用2^10规整过，下一个die的起始block地址 等于 group_blk_num * 2                                                                                                                                                 
#define FLAG_BLOCK_GAP      (1 << 10)   //bit10表示page number是否需要向上规整，1表示需要。如TLC是192page/block，为了对齐下一个block则需要规整为256page/block给驱动                                                                                                             
#define FLAG_SPARE_MASK     (0x3 << 8)  //bit9 表示spare区域大小的高位                                                                                                                                                                                                          )
                                        //bit8 表示spare区域大小的高位，单位是256 Bytes。因spare_size仅为T_U8，不足以表示新型nand的400多个字节的spare大小                                                                                                                    
#define FLAG_ECC_POS        (4)         //bit7 表示ECC类型                                                                                                                 
                                        //bit6 表示ECC类型                                                                                                                 
                                        //bit5 表示ECC类型                                                                                                                 
                                        //bit4 表示ECC类型，0为4 bit/512B，1为8 bit/512B，2为12 bit/512B，3为16 bit/512B，4为24 bit/1024B，5为32 bit/1024B，6为40 bit/1024B，7为44 bit/1024B，8为60 bit/1024B，9为72 bit/1024B
#define FLAG_TARGET_ALLOW   (1 << 3)    //bit3 表示如果有多个target片选，则各target相同位置的块（在LUN合并之后）合并成一个大块；为'0'不作合并。                                                  
#define FLAG_LUN_ALLOW      (1 << 2)    //bit2 表示如果有多个LUN，则各LUN相同位置的块（在plane合并之后）合并成一个大块；为'0'不作合并。                                                          
#define FLAG_PLANE_ALLOW    (1 << 1)    //bit1 表示如果multi plane，则各plane相同位置的块合并成一个大块；为'0'不作合并。                                                                         

//注意,Nand的参数ECC TYPE与AK芯片的ECC 模式,并不一定一一对应.
typedef enum 
{
    ECC_TYPE_4BITS = 0,
    ECC_TYPE_8BITS,
    ECC_TYPE_12BITS,
    ECC_TYPE_16BITS,
    ECC_TYPE_24BITS,
    ECC_TYPE_32BITS,
    ECC_TYPE_40BITS,
    ECC_TYPE_44BITS,
    ECC_TYPE_60BITS,
    ECC_TYPE_72BITS 
}E_NAND_TYPE;
#define ECC_TYPE(flag)  ((flag & (0xF << FLAG_ECC_POS)) >> FLAG_ECC_POS)

//currently there are 7 types, more types might be added when new nand come out
//说明：括号里前一个是page号,后一个是page中的位置, 如果这些位置不为0xFF则该block是出厂坏快
typedef enum
{
    NAND_BAD_FLAG_SAMSUNG = 0,//NAND_TYPE_SAMSUNG:        0x1 小页SLC([0,1],[517]),   大页SLC([0,1],[2048]),          MLC([127], [2048/4096])
    NAND_BAD_FLAG_HYNIX,//NAND_TYPE_HYNIX:          0x2 小页SLC([0,1],[517]),   大页SLC([0,1],[2048]),          MLC([125,127], [2048/4096])
    NAND_BAD_FLAG_TOSHIBA,//NAND_TYPE_TOSHIBA:        0x3 小页SLC([0,1],[0,512]), 大页SLC([0,1],[0,2048]),        MLC([127], [0,2048/4096])
    NAND_BAD_FLAG_TOSHIBA_EXT,//NAND_TYPE_TOSHIBA_EXT:    0x4 小页SLC(),              大页SLC(),                      MLC([0,127/191/255], [0,2048/4096/8192])
    NAND_BAD_FLAG_MICRON,//NAND_TYPE_MICRON:         0x5 小页SLC([0,1],[512]),   大页SLC([0,1],[2048]),          MLC([0,1], [2048/4096])
    NAND_BAD_FLAG_ST,//NAND_TYPE_ST:             0x6 小页SLC([0,1],[517]),   大页SLC([0],[2048,2053]),       MLC([127], [0])
    NAND_BAD_FLAG_MICRON_4K//NAND_TYPE_MICRON_4K       0x7 小页SLC(),              大页SLC(),                      MLC([0], [4096 ~ 4096+218])
}E_NAND_BAD_FLAG;


/**
* @BRIEF    Nandflash info define
* @AUTHOR   zhaojiahuan
* @DATE     2006-7-17
* @MODIFY Yang Yiming
* @DATE    2012-12-24
*/
typedef struct Nand_phy_info{
    T_U32  chip_id;//chip id
    T_U16  page_size; //page size
    T_U16  page_per_blk; //page of one block
    T_U16  blk_num;//total block number
    T_U16  group_blk_num;//the same concept as die, according to nand's struture
    T_U16  plane_blk_num;
    T_U8   spare_size;//spare区域大小的低位，不超过255 Byte
    T_U8   col_cycle;//column address cycle
    T_U8   lst_col_mask;//last column  addrress cycle mask bit
    T_U8   row_cycle;//row address cycle
    T_U8   delay_cnt;//Rb delay, unit is 1024 asic clock, default value corresponds to 84MHz
    T_U8   custom_nd;//nand type flag, used to detect the original invilid block
    T_U32  flag;//character bits
    T_U32  cmd_len;//nandflash command length
    T_U32  data_len;//nandflash data length
    T_U8   *des_str;//descriptor string
}T_NAND_PARAM, T_NAND_PHY_INFO;

#define ERROR_CHIP_ID   0//0xFFFFFFFF

#endif

