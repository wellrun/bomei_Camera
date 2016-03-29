/**@file arch_nand.h
 * @brief AK322x nand controller
 *
 * This file describe how to control the AK322x nandflash driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan, chenyanyan
 * @date    2007-1-10
 * @version 1.0
 */
#ifndef __ARCH_NAND_H__
#define __ARCH_NAND_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "anyka_types.h"
#include "nand_list.h"
#include "hal_errorstr.h"

/** @defgroup  NAND NAND
 *    @ingroup Drv_Lib
 */
/*@{*/
#define DRV_SUPPORT_NAND 4
#define NAND_SUPPORT_RR 1
#define ENHANCED_SLC_PROGRAM 1
#define NAND_SMALL_PAGE 1
#define NAND_LARGE_PAGE 1
//#define akerror(a,b,c) printf(a)


#define NAND_DEVICE_NUM 2

#define NFC_SUPPORT_CHIPNUM        4

#define NAND_MAX_ADD_LEN    12
#define NAND_MAX_ADD_LEN_SMALL 8

/* @brief some macro used to call nfc_cycle
*/

/*@{*/

typedef enum
{
    COMMAND_C = 1,
    ADDRESS_C,
    READYB_C,
    DELAY_C,
    NULL_C,
    END_C,
    WDATA_C,
    ARRAY_C
}NAND_CYLCE_TYPE;

#define NAND_CYCLE_FLAG_POS        (8)
#define NAND_CYCLE_FLAG_MASK       (0xFF << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_VALUE_MASK      (0xFF)
#define GET_NAND_CYCLY_TYPE(n)      ((n >> NAND_CYCLE_FLAG_POS) & 0xFF)
#define GET_NAND_CYCLY_VALUE(n)     (n & NAND_CYCLE_VALUE_MASK)
#define NAND_CYCLE_CMD_FLAG         (COMMAND_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_ADDR_FLAG       (ADDRESS_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_RB_FLAG           (READYB_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_DELAY_FLAG     (DELAY_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_NULL_FLAG        (NULL_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_END_FLAG          (END_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_WDATA_FLAG       (WDATA_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_ARR_FLAG         (ARRAY_C << NAND_CYCLE_FLAG_POS)

#define CMD_CYCLE(x)            (NAND_CYCLE_CMD_FLAG | (x))
#define ADDR_CYCLE(x)           (NAND_CYCLE_ADDR_FLAG | ((x)&0xff))
#define RB_CYCLE                   NAND_CYCLE_RB_FLAG
#define DELAY_CYCLE(x)          (NAND_CYCLE_DELAY_FLAG | (x))
#define NULL_CYCLE              NAND_CYCLE_NULL_FLAG
#define END_CYCLE               NAND_CYCLE_END_FLAG
#define WDATA_CYCLE(x)          (NAND_CYCLE_WDATA_FLAG | (x))
/*@}*/


/* @brief the bits definition in T_NAND_RET
*/

/*@{*/
#define NAND_FAIL_STATUS          (1UL << 31)
#define NAND_FAIL_NFC_TIMEOUT     (1 << 30)
#define NAND_FAIL_PARAM           (1 << 29)
#define NAND_FAIL_ECC             (1 << 28)
#define NAND_WARN_ONCE_FAIL       (1 << 27)
#define NAND_WARN_STRONG_DANGER   (1 << 26)
#define NAND_WARN_WEAK_DANGER     (1 << 25)
#define NAND_FAIL_L2_TRANSFER     (1 << 24)

#define NAND_GOODPAGE_POS         (16)
#define NAND_MAXFLIP_POS          (8)
#define NAND_GOODSECT_POS         (0)
#define NAND_FAIL_MASK            (NAND_FAIL_STATUS |NAND_FAIL_NFC_TIMEOUT | NAND_FAIL_PARAM | NAND_FAIL_ECC | NAND_FAIL_L2_TRANSFER)
#define NAND_WARN_MASK            (NAND_WARN_ONCE_FAIL | NAND_WARN_STRONG_DANGER | NAND_WARN_WEAK_DANGER)
#define NAND_FLIPBIT_MASK         0xFF
/*@}*/

#define SET_GOODSECT_CNT(nRet,nSet)     (nRet |= (nSet << NAND_GOODSECT_POS))
#define GET_GOODSECT_CNT(nRet)          ((nRet >> NAND_GOODSECT_POS) & 0xFF)
#define SET_MAXFLIP_CNT(nRet,nFlip)      (nRet |= (nFlip << NAND_MAXFLIP_POS))
#define GET_MAXFLIP_CNT(nRet)           ((nRet >> NAND_MAXFLIP_POS) & 0xFF)
#define SET_GOODPAGE_CNT(nRet,nPage)    (nRet |= (nPage << NAND_GOODPAGE_POS))

typedef T_U32 T_NAND_RET;

typedef enum
{
    OP_INIT,                ///>initial nandflash
    OP_GET_ECC,             ///>get ecc strategy
    OP_ENABLE_SLC,          ///>enable slc mode
    OP_GET_LOWER_PAGE,      ///>get lower page
    OP_CHECK_BAD            ///>check bad blok
}E_DEV_OP;

typedef struct NAND_ECC_CTL
{
    T_U16 nMainLen;             ///>main data section length
    T_U16 nSectOffset;          ///>main data section length + the ecc length
    T_U8   nMainEcc;            ///>main data ecc mode
    T_U8   nAddEcc;             ///>additional data ecc mode
    T_U8   nAddLen;             ///>additional data section length
    T_BOOL   bSeperated;        ///>wheather the main data and additional data is seperated
    T_U8    nMainSectCnt;       ///>main data section count
}T_NAND_ECC_CTRL;

typedef struct NAND_ADDR
{
    T_U16 nSectAddr;            ///>from which section in the page
    T_U16 nTargetAddr;          
    T_U32 nLogAbsPageAddr;      
}T_NAND_ADDR;

typedef struct NAND_DATA
{
    T_U16    nSectCnt;//indicate the bytes to be read in ecc bypass mode
    T_U16    nPageCnt;
    T_U8    *pMain;
    T_U8    *pAdd;
    T_NAND_ECC_CTRL *pEccCtrl;
}T_NAND_DATA;

typedef struct  NAND_DEVICE_INFO T_NAND_DEVICE_INFO;
typedef T_BOOL (*f_device_ctrl)(E_DEV_OP eOp, T_U8 nArgc, T_VOID * pArgv);
typedef T_NAND_RET  (*f_eraseN)(T_NAND_DEVICE_INFO * pDevice, T_NAND_ADDR * pAddr, T_U16 nBlockCnt);
typedef T_NAND_RET  (*f_progN)(T_NAND_DEVICE_INFO *pDevice, T_NAND_ADDR *pAddr, T_NAND_DATA *pData);
typedef T_NAND_RET  (*f_readN)(T_NAND_DEVICE_INFO * pDevice, T_NAND_ADDR * pAddr, T_NAND_DATA * pData);

typedef struct NAND_FUNCTIONS
{
    f_device_ctrl  ctrl ;
    f_eraseN erase;
    f_progN program;
    f_readN read;
}T_NAND_FUNCTIONS;

typedef struct NAND_LOGIC_INFO
{
    T_U16   nLogicBPP;
    T_U16   nLogicPPB;
    T_U16   nLogicBPC;
    T_U8    nBlockShift;
    T_U8    nPageShift;
}T_NAND_LOGIC_INFO;

struct NAND_DEVICE_INFO
{
    T_U32   nID[2];
    T_U8    nChipCnt;
    T_NAND_FUNCTIONS FunSet;
    T_NAND_LOGIC_INFO LogInfo;
    T_NAND_ECC_CTRL   **ppEccCtrl;    
    T_VOID  *pPhyInfo;
};
typedef struct _NAND_READ_RETRY_PARAM
{
    T_U32 nNandID[2];
    T_U32 nRetryTime;
    T_U32 nDefaultScale;
    T_U16 aCycles[12][20];    
}NAND_READ_RETRY_PARAM;


/**
 * @brief   set gpios as nand Ce pin, and get the chip id and chip count detected
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [out]pChipID the 8 bytes ID of nandflash detected
 * @param       [out]pChipCnt the amount of nandflash detected
 * @param       [in]pCePos the GPIOs connected to nandflash Ce, 0xFF for default pin 
 * @param       [in]nCeCnt the GPIOs amount to be set, not more than 4
 * @return      T_VOID
 */
T_VOID nand_setCe_getID(T_U32 pChipID[2], T_U32 *pChipCnt, 
    T_U8 *pCePos, T_U32 nCeCnt);

/**
 * @brief       initial the nandflash with pNandParam and get T_NAND_DEVICE_INFO
 *                  function nand_setCe_getID  should be called before calling this
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]pNandParam a anyka nandflash param structure 
 * @param       [in]nCeCnt the chipcnt to be initialed, not more than 4
 * @return      T_NAND_DEVICE_INFO
 */
T_NAND_DEVICE_INFO* nand_get_device(T_NAND_PARAM *pNandParam, T_U32 nChipCnt);

/**
 * @brief       register a nand device
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]pDevice a structure including the chip ID etc.
 * @return      T_BOOL
 */
T_BOOL nand_reg_device(T_NAND_DEVICE_INFO *pDevice);

/**
 * @brief       initial nand flash controller
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]pCePos the GPIOs connected to nandflash #Ce, 0xFF for default pin 
 * @param       [in]nCeCnt the GPIOs amount to be set, not more than 4 
 * @return      T_VOID
 */
T_VOID  nfc_init(T_U8 *pCePos, T_U8 nCeCnt);

/**
 * @brief       initial randomizer
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]nRandPageSize the physical page size of nandflash
 * @return      T_BOOL
 */
T_BOOL nfc_init_randomizer(T_U32 nRandPageSize);

/**
 * @brief       configure randomizer
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]nPageAddr the absoult page address to be read/written
 * @param       [in]nColumnAddr the column address in the target page
 * @param       [in]bEnable AK_TRUE to enble randomizer, AK_FALSE to disable
 * @param       [in]bWrite AK_TRUE to write, AK_FALSE to read
 * @return      T_VOID
 */
T_VOID nfc_config_randomize(T_U16 nPageAddr, T_U16 nColumnAddr, T_BOOL bEnable, T_U8 bWrite);

/**
 * @brief       select/deselect a nandflash target
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]nTarget target nandflash
 * @param       [in]bSelect AK_TRUE to select, AK_FALSE to deselect
 * @return      T_BOOL
 */
T_BOOL nfc_select(T_U8 nTarget, T_BOOL bSelect);

/**
 * @brief       change the tRC manually
 *                  if the board signal environment was quite good, 
 *                  try to decrease the tRC to improve performance.
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]nTrc the time for read/write cycle
 * @param       [in]nDelay for some reason, user can increase the tRC by nDelay with nTrc equals zero
 * @return      T_VOID
 */
T_VOID nfc_configtRC(T_U8 nTrc, T_U8 nDelay);

/**
 * @brief       interface provided for clock module to adjust nandtiming with a new asic
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]nAsic the new asic, unit Hz
 * @return      T_VOID
 */
T_VOID  nfc_timing_adjust(T_U32 nAsic);

/**
 * @brief       issue nand cycles including command cycle, address cycle, RB cycle etc.
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]nCmdSeq the cycle sequence
 * @return      T_BOOL
 */
T_BOOL nfc_cycle(T_U32 nCmdSeq,...);

/**
 * @brief           issue read status command cycle to check nandflash status
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]nStatuscmd the read status command
 * @param       [in]nExpectbits a bitmap
 * @return      T_BOOL
 */
T_BOOL nfc_waitstatus(T_U8 nStatuscmd, T_U8 nExpectbits);

/**
 * @brief           get ecc strategy from nand controller
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [out]ppEccCtrl the ecc strategy
 * @param       [in]nPageSize the physical page size of nandflash
 * @param       [in]nEccType ecc requirement 
 * @return      T_VOID
 */
T_VOID nfc_get_ecc(T_NAND_ECC_CTRL **ppEccCtrl, T_U32 nPageSize, T_U8 nEccType);

/**
 * @brief           transfer data out to nandflash
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]pMain the main data buffer
 * @param       [in]pAdd the additonal data buffer
 * @param       [in]nSectCnt the main data section count
 * @param       [in]pEccCtrl ecc strategy got from nfc_get_ecc
 * @return      T_NAND_RET
 */
T_U32 nfc_write(T_U8 *pMain, T_U8 *pAdd, T_U16 nSectCnt, T_NAND_ECC_CTRL *pEccCtrl);

/**
 * @brief           transfer data in from nandflash
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]pMain the main data buffer
 * @param       [in]pAdd the additonal data buffer
 * @param       [in]nSectCnt the main data section count
 * @param       [in]pEccCtrl ecc strategy got from nfc_get_ecc
 * @return      T_NAND_RET
 */
T_U32 nfc_read(T_U8 *pMain, T_U8 *pAdd, T_U16 nSectCnt, T_NAND_ECC_CTRL *pEccCtrl);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif //__ARCH_NAND_H__
