/**@file arch_nand.h
 * @brief AK880x nand controller
 *
 * This file describe how to control the AK880x nandflash driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author yiruoxiang, jiangdihui
 * @date 2007-1-10
 * @version 1.0
 */
#ifndef __ARCH_NAND_H__
#define __ARCH_NAND_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "read_retry.h"

/** @defgroup NandFlash Nandflash group
 *  @ingroup Drv_Lib
 */
/*@{*/
#define NFC_FIFO_SIZE            512
#define NFC_LOG_SPARE_SIZE       16

#define NAND_512B_PAGE      0   // 512 bytes per page
#define NAND_2K_PAGE        1
#define NAND_4K_PAGE        2
#define NAND_8K_PAGE        3

#define NFC_SUPPORT_CHIPNUM        4

//when flip bits are more than the following defines, be careful of the data
#define WEAK_DANGER_BIT_NUM_MODE0 3  //4bit nand ecc's weak danger flip bit number among 512 Bytes
#define WEAK_DANGER_BIT_NUM_MODE1 5  //8 bit nand ecc's weak danger flip bit number among 512 Bytes
#define WEAK_DANGER_BIT_NUM_MODE2 8  //12 bit nand ecc's weak danger flip bit number among 512 Bytes
#define WEAK_DANGER_BIT_NUM_MODE3 12  //16 bit nand ecc's weak danger flip bit number among 512 Bytes
#define WEAK_DANGER_BIT_NUM_MODE4 18  //24 bit nand ecc's weak danger flip bit number among 1024 Bytes
#define WEAK_DANGER_BIT_NUM_MODE5 26  //32 bit nand ecc's weak danger flip bit number among 1024 Bytes

//when flip bits are more than the following defines, don't use current block any more
#define STRONG_DANGER_BIT_NUM_MODE0 4  //4bit nand ecc's strong danger flip bit number among 512 Bytes
#define STRONG_DANGER_BIT_NUM_MODE1 7  //8 bit nand ecc's strong danger flip bit number among 512 Bytes
#define STRONG_DANGER_BIT_NUM_MODE2 10  //12 bit nand ecc's strong danger flip bit number among 512 Bytes
#define STRONG_DANGER_BIT_NUM_MODE3 14  //16 bit nand ecc's strong danger flip bit number among 512 Bytes
#define STRONG_DANGER_BIT_NUM_MODE4 22  //24 bit nand ecc's strong danger flip bit number among 1024 Bytes
#define STRONG_DANGER_BIT_NUM_MODE5 30  //32 bit nand ecc's strong danger flip bit number among 1024 Bytes

typedef enum
{
    FINAL_FAIL_BIT            = 31, //this bit indicates the whole operation finally failed or not, 1=failed finally
    ONCE_OPERATION_FAILED_BIT = 30, //this bit indicates whether read/write/erase/copyback fail has ever happened or not, 1=fail
    L2_ALLOC_FAIL_BIT         = 29, //this bit indicates whether L2_alloc failed or not, 1=alloc fail
    TIME_OUT_BIT              = 28, //this bit indicates operation time out, 1=time out
    FLIP_NUM_BIT              = 0   //this bit indicates how many flip bit per ecc_length(e.g. 512, 1024...)
}eNAND_OPERATION_RETURN_BIT_MAP;

typedef enum
{
    ECC_4BIT_P512B  = 0,    //4bit ecc requirement per 512 bytes
    ECC_8BIT_P512B  = 1,    //8 bit ecc requirement per 512 bytes
    ECC_12BIT_P512B = 2,    //12 bit ecc requirement per 512 bytes
    ECC_16BIT_P512B = 3,    //16 bit ecc requirement per 512 bytes
    ECC_24BIT_P1KB  = 4,    //24 bit ecc requirement per 1024 bytes
    ECC_32BIT_P1KB  = 5     //32 bit ecc requirement per 1024 bytes
}ECC_TYPE;

typedef enum
{
    BOARD_TYPE_PLATFORM,    //platform board of AK98xx & AK37xx
    BOARD_TYPE_DAGONGMO,    //大公模客户板
    BOARD_TYPE_UNKNOWN
}eBOARD_TYPE;

typedef struct SNandEccStru
{
    T_U8 *buf;                         //data buffer, e.g. common data buffer or spare buffer
    T_U32 buf_len;                     //data total length, e.g. 4096 or 8192
    T_U32 ecc_section_len;             //ecc section length, e.g. 512, 512+4, or 1024, 1024+4
    ECC_TYPE ecc_type;                 //ecc type, e.g. ECC_4BIT or ECC_8BIT
}T_NAND_ECC_STRU, *T_PNAND_ECC_STRU;

typedef enum
{
    CFG_CMD,//命令节点类型
    CFG_ADD,//地址节点类型
    CFG_READ,//读若干数据节点
    CFG_WRITE,//写若干数据节点
    CFG_BUFADD,//读写数据存放的buff地址
    CFG_DELAY,//延时节点
    CFG_RB  //等待RB#信号节点
}E_NODE_TYPE;

typedef struct
{
E_NODE_TYPE e_type;//标记此节点的类型
T_U32 value;//命令的值
}T_NFC_NODE;


#define RETRY_NAND_MAX_SUPPORT        8

struct SNandflash_Add
{
    T_U8    ChipPos[NFC_SUPPORT_CHIPNUM];
    T_U8    RowCycle;
    T_U8    ColCycle;
    T_U8    ChipType;
    T_U8    EccType;
    T_U32   PageSize;
    T_U32   PagesPerBlock;
};

typedef struct SNandflash_Add* T_PNandflash_Add;
typedef struct SNandflash_Add T_Nandflash_Add;
//**********************************************************************
/**
 * @brief config gpio for additive ce2 or ce3, and consider INVALID_GPIO to be invalid chip choose  
 *
 * @author jiangdihui
 * @date 2010-11-30
 * @param[in] ce2 gpio for nand ce2
 * @param[in] ce3 gpio for nand ce3
 * @return T_VOID
 */
T_VOID nand_set_additive_ce_gpio(T_U32 ce2, T_U32 ce3);

/**
 * @brief initialization of nandflash hardware.
 *
 * @author yiruoxiang
 * @date 2006-11-02
 * @return  T_VOID
 */
T_VOID nand_HWinit(T_VOID);

/**
 * @brief config nand command and data cycle
 *
 * @author xuchang
 * @date 2007-12-27
 * @param[in] cmd_cycle the command cycle to config
 * @param[in] data_cycle the data cycle to config
 * @return T_VOID
 */
T_VOID nand_config_timeseq(T_U32 cmd_cycle, T_U32 data_cycle);

/**
 * @brief calculate each nand's timing under 62MHz & 124MHz
 *
 * @author yiruoxiang
 * @date 2007-12-27
 * @param[in] DefDataLen default data lenght
 * @return  T_VOID
 */
T_VOID nand_calctiming(T_U32 DefDataLen);

/**
 * @brief change nand timing when Freq has changed
 *
 * @author yiruoxiang
 * @date 2007-12-27
 * @param[in] Freq frequency
 * @return  T_VOID
 */
T_VOID nand_changetiming(T_U32 Freq);

/**
 * @brief set different board type
 * @note: 
 * 1. nand_set_board_type() should be called before calling nand_changetiming() in Nand_Init()
 * 2. it should only be called once.
 * 3. if using platform board, this function need not to be used!!!
 *
 * @author zhujianlin
 * @date 2011-07-07
 * @param[in] board_type different board type
 * @return  T_VOID
 */
T_VOID nand_set_board_type(eBOARD_TYPE board_type);

/**
 * @brief judge one Toshiba nand is PBA or not.
 * @note: nand_read_chipID(0) should be called before calling nand_is_PBA() !!!
 *
 * @author zhujianlin
 * @date 2011-04-21
 * @return  T_BOOL
 * @retval AK_TRUE means it is a TOSHIBA PBA nand, AK_FALSE means not
 */
T_BOOL nand_is_PBA(T_VOID);

/**
 * @brief mark one nand is using read_retry method or not.
 *
 * @author zhujianlin
 * @date 2011-07-26
 * @return  T_BOOL
 * @retval AK_TRUE means this nand  uses read_retry method, AK_FALSE means not
 */
T_BOOL nand_use_read_retry(T_VOID);

/**
 * @brief read nand flash chip ID.
 *
 * @author yiruoxiang
 * @date 2006-11-02
 * @param[in] Chip which chip will be read.
 * @return  T_U32
 * @retval  current nandflash ID
 */
T_U32 nand_read_chipID(T_U32 Chip);

/**
 * @brief read unique id from nandflash.
 *
 * @author zhujianlin
 * @date 2011-3-17
 * @param[out] pId_buf Data buffer for reading unique id, should not be less than 512 bytes!!!
 * @return T_U32
 * @retval length of nandflash's unique id, normally NAND_UNIQUE_ID_LENGTH = 32
 */
T_U32 nand_read_unique_ID(T_U8 *pId_buf);

/**
 * @brief reset nand flash.
 *
 * @author yiruoxiang
 * @date 2006-11-02
 * @param[in] chip which chip will be reset.
 * @return  T_VOID
 */
T_VOID nand_reset(T_U32 chip);

/**
 * @brief read data from nand flash with ECC .
 *
 * @author jiangdihui
 * @date 2010-07-23
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param pDataCtrl control reading data section: buffer, data lenght, ECC.
 * @param pSpareCtrl control reading spare section: buffer, data lenght, ECC.
 * @return  T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_readpage_ecc(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_PNAND_ECC_STRU  pDataCtrl,  T_PNAND_ECC_STRU pSpareCtrl);

/**
 * @brief read one page(page size>=2048) of data from nand flash with ECC .
 *
 * @author jiangdihui
 * @date 2010-07-23
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[out] Data buffer for read data, should be large than or equal to 2048 bytes.
 * @param[out] Spare buffer for file system info, should be 4 bytes.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_readsector_large(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_U8 Data[], T_U32 *Spare);

/**
 * @brief read one page(page size=512) of data from nand flash with ECC .
 *
 * @author jiangdihui
 * @date 2010-07-23
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[out] Data buffer for read data, should be 512 bytes.
 * @param[out] Spare buffer for file system info, should be 4 bytes.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_readsector(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_U8 Data[], T_U32 *Spare);

/**
 * @brief read file system info.
 *
 * @author jiangdihui
 * @date 2010-07-23
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[out] Spare buffer for file system info, should be 4 bytes.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_readspare(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_U32 *Spare);

/**
 * @brief read data from nandflash without ECC.
 *
 * @author yiruoxiang
 * @date 2006-11-02
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[out] Data buffer for read data.
 * @param[in] Len how many bytes read from nandflash
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_readbytes(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_U8 Data[], T_U32 Len);

/**
 * @brief write data to nand flash with ECC .
 *
 * @author jiangdihui
 * @date 2010-07-23
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[in] pDataCtrl control writting data section: buffer, data lenght, ECC.
 * @param[in] pSpareCtrl control writting spare section: buffer, data lenght, ECC.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_writepage_ecc(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_PNAND_ECC_STRU  pDataCtrl,  T_PNAND_ECC_STRU pSpareCtrl);

/**
 * @brief write one page(page size>=2048) of data to nand flash with ECC .
 *
 * @author jiangdihui
 * @date 2010-07-23
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[in] Data buffer for write data, should be large than or equal to 2048 bytes.
 * @param[in] Spare file system info.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_writesector_large(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_U8 Data[], T_U32 Spare);

/**
 * @brief write one page(page size=512) of data to nand flash with ECC .
 *
 * @author jiangdihui
 * @date 2010-07-23
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[in] Data buffer for write data, should be 512 bytes.
 * @param[in] Spare file system info.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_writesector_small(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_U8 Data[], T_U32 Spare);

/**
 * @brief write data to nandflash without ECC.
 *
 * @author yiruoxiang
 * @date 2006-11-02
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[in] Data buffer for write data.
 * @param[in] Len how many bytes write to nandflash
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_writebytes(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_U8 Data[], T_U32 Len);

/**
 * @brief erase one block of nandflash.
 *
 * @author yiruoxiang
 * @date 2006-11-02
 * @param[in] Chip which chip will be operated.
 * @param[in] BlkStartPage first page of the block.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_eraseblock(T_U32 Chip, T_U32 BlkStartPage, T_PNandflash_Add pNF_Add);

/**
 * @brief copy one physical page to another one.
 *
 * hardware copyback mode, there should be caches in nandflash, source and destation page should be in the same plane
 * @author yiruoxiang
 * @date 2006-11-02
 * @param[in] Chip which chip will be operated.
 * @param[in] SrcPhyPage the source page to read.
 * @param[in] DestPhyPage the destination page to write.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_copyback(T_U32 Chip, T_U32 SrcPhyPage, T_U32 DestPhyPage, T_PNandflash_Add pNF_Add);

/**
 * @brief multi-plane erase blocks of nandflash.
 *
 * @author jiangdihui
 * @date 2006-12-13
 * @param[in] Chip which chip will be operated.
 * @param[in] BlkStartPage first page of the block.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
 T_U32 nand_eraseblock_2plane(T_U32 Chip, T_U32 BlkStartPage, T_PNandflash_Add pNF_Add);

/**
 * @brief multi-plane write data to nand flash with ECC .
 *
 * @author jiangdihui
 * @date 2010-12-13
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[in] pDataCtrl control writting data section: buffer, data lenght, ECC.
 * @param[in] pSpareCtrl control writting spare section: buffer, data lenght, ECC.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_writepage_ecc_2plane(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_PNAND_ECC_STRU  pDataCtrl,  T_PNAND_ECC_STRU pSpareCtrl);

/**
 * @brief multi-plane read data from nand flash with ECC .
 *
 * @author jiangdihui
 * @date 2010-12-13
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param pDataCtrl control reading data section: buffer, data lenght, ECC.
 * @param pSpareCtrl control reading spare section: buffer, data lenght, ECC.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_readpage_ecc_2plane(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_PNAND_ECC_STRU  pDataCtrl,  T_PNAND_ECC_STRU pSpareCtrl);

/**
 * @brief multi-plane copy one physical page to another one.
 *
 * hardware copyback mode, there should be caches in nandflash, source and destation page should be in the same plane
 * @author jiangdihui
 * @date 2006-12-13
 * @param[in] Chip which chip will be operated.
 * @param[in] SrcPhyPage the source page to read.
 * @param[in] DestPhyPage the destination page to write.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
 T_U32 nand_copyback_2plane(T_U32 Chip, T_U32 SrcPhyPage, T_U32 DestPhyPage, T_PNandflash_Add pNF_Add);

/**
 * @brief set PBA mode
 *
 * @author jiangdihui
 * @date 2011-4-15
 * @param[in] Chip which chip to be set.
 * @param[in] Mode different mode of PBA
              normal read mode:1
              faster read mode:2
              pre-read mode:3
              slient read mode:4
 * @return  T_VOID
 */
T_VOID nand_set_pba_mode(T_U32 Chip, T_U32 Mode);

/**
 * @brief read data from PBA nandflash without ECC.
 *
 * @author jiangdihui
 * @date 2011-4-15
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[out] Data buffer for read data.
 * @param[in] DataLen how many bytes read from nandflash
 * @param[out] Spare buffer for oob info.
 * @param[in] SpareLen length of oob info
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_readbytes_pba(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_U8 Data[], T_U32 DataLen, T_U8 Spare[], T_U32 SpareLen);

/**
 * @brief write data to PBA nandflash without ECC.
 *
 * @author jiangdihui   
 * @date 2011-4-15
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[in] Data buffer for write data.
 * @param[in] DataLen how many bytes write to nandflash
 * @param[out] Spare buffer for oob info.
 * @param[in] SpareLen length of oob info 
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_writebytes_pba(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_U8 Data[], T_U32 DataLen, T_U8 Spare[], T_U32 SpareLen);

/**
 * @brief multi plane read data from PBA nandflash without ECC.
 *
 * @author jiangdihui
 * @date 2011-4-15
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[out] Data buffer for read data.
 * @param[in] DataLen how many bytes read from nandflash
 * @param[out] Spare buffer for oob info.
 * @param[in] SpareLen length of oob info
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_readbytes_pba_2plane(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_PNandflash_Add pNF_Add, T_U8 Data[], T_U32 DataLen, T_U8 Spare[], T_U32 SpareLen);

/**
* @brief enable the Randomizer function for read/write data but spare excluded
*
*@author yangyiming
*@date 2011-8-16
*@param[in] pNF_Add information of the nandflash characteristic.
*@return T_BOOL  AK_TRUE for success  AK_FALSE for failure
*/
T_BOOL nand_enable_randomizer(T_PNandflash_Add pNF_Add);

/**
* @brief disable the Randomizer function completely
*
*@author yangyiming
*@date 2011-8-16
*@return T_VOID
*/
T_VOID nand_disable_randomizer(T_VOID);

/**
*@brief  reopen randomizer funciton when randomizer closed by nand_close_randomizer()
*
*@author yangyiming 
*@date  2011-8-26
*@return T_VOID
*/
T_VOID nand_reopen_randomizer(T_VOID);


/**
*@brief  close randomizer function temporarily if randomizer have been enabled
*
*@author yangyiming 
*@date  2011-8-26
*@return T_VOID
*/
T_VOID nand_close_randomizer(T_VOID);

/**
* @brief restore default read retrial scale value of nand
*
* @author   yangyiming
* @date    2011-9-21
* @param  [in]chip the chip to be operated
* @retrun T_VOID
*/
T_VOID nand_restore_default_scale(T_U32 chip);

/**
 * @brief cache_read data from nand flash with ECC.
 *
 * @author jiangdihui
 * @date 2011-07-14
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] PageCnt counts of continuous page to be read.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[in/out] pDataCtrl control reading data section: buffer, data lenght, ECC.
 * @param[in/out] pSpareCtrl control reading spare section: buffer, data lenght, ECC.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_readpage_cache_ecc(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_U32 PageCnt, T_PNandflash_Add pNF_Add, T_PNAND_ECC_STRU  pDataCtrl,  T_PNAND_ECC_STRU pSpareCtrl);

/**
 * @brief multi-plane cache_read data from nand flash with ECC .
 *
 * @author jiangdihui
 * @date 2011-07-19
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] PageCnt counts of continuous page to be read.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[in/out] pDataCtrl control reading data section: buffer, data lenght, ECC.
 * @param[in/out] pSpareCtrl control reading spare section: buffer, data lenght, ECC.
 * @return  T_U32
 * @retval  values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_readpage_cache_ecc_2plane(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_U32 PageCnt, T_PNandflash_Add pNF_Add, T_PNAND_ECC_STRU  pDataCtrl,  T_PNAND_ECC_STRU pSpareCtrl);

/**
 * @brief cache_write data to nand flash with ECC .
 *
 * @author jiangdihui
 * @date 2011-07-19
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] PageCnt counts of continuous page to be write.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[in] pDataCtrl control writting data section: buffer, data lenght, ECC.
 * @param[in] pSpareCtrl control writting spare section: buffer, data lenght, ECC.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_writepage_cache_ecc(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_U32 PageCnt, T_PNandflash_Add pNF_Add, T_PNAND_ECC_STRU  pDataCtrl,  T_PNAND_ECC_STRU pSpareCtrl);

/**
 * @brief multi-plane cache write data to nand flash with ECC .
 *
 * @author jiangdihui
 * @date 2010-12-13
 * @param[in] Chip which chip will be read.
 * @param[in] RowAddr the row address of nandflash.
 * @param[in] ColumnAddr the column address of nandflash.
 * @param[in] PageCnt counts of continuous page to be write.
 * @param[in] pNF_Add information of the nandflash characteristic.
 * @param[in] pDataCtrl control writting data section: buffer, data lenght, ECC.
 * @param[in] pSpareCtrl control writting spare section: buffer, data lenght, ECC.
 * @return T_U32
 * @retval values of eNAND_OPERATION_RETURN_BIT_MAP
 */
T_U32 nand_writepage_cache_ecc_2plane(T_U32 Chip, T_U32 RowAddr, T_U32 ColumnAddr, T_U32 PageCnt, T_PNandflash_Add pNF_Add, T_PNAND_ECC_STRU  pDataCtrl,  T_PNAND_ECC_STRU pSpareCtrl);

/**
 * @brief excute nandflash command sequence
 *
 * @author yangyiming
 * @date 2012-05-15
 * @param[in] chip the target chip
 * @param[in] Seq the command sequence array
 * @param[in] Size the size of command sequence,unit node
 * @return T_BOOL
 * @retval AK_TRUE for success, failure for failure
 */
T_BOOL nfl_cmd_seq(T_U32 chip, T_NFC_NODE Seq[], T_U32 Size);

/**
 * @brief register Read-Retry algorithms for a nand with the given ID
 *
 * @author yangyiming
 * @date 2012-05-15
 * @param[in] IDL the lower 4 bytes id
 * @param[in] IDH the higher 2 bytes id
 * @param[in] Fun the Read-Retry algorithms
 * @return T_BOOL
 * @retval AK_TRUE for success, failure for failure
 */
T_BOOL nfl_retry_reg(T_U32 IDL, T_U32 IDH, T_NAND_RETRY_FUNCTION_SET * Fun);
    

/*@}*/
#ifdef __cplusplus
}
#endif

#endif //__ARCH_NAND_H__
