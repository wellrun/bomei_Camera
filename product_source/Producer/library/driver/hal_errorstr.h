/*******************************************************************************
 * @file    hal_errorstr.h
 * @brief   define the error string in every module
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.12.6
 * @version 1.0
*******************************************************************************/
#ifndef __HAL_ERRORSTR_H__
#define __HAL_ERRORSTR_H__


#ifndef __FILE__
    #define __FILE__    ""
#endif
#ifndef __LINE__
    #define __LINE__    0
#endif


#define ERROR_HAL_L2        "A "
#define ERROR_CAM           "C "
#define ERROR_LCD           "D "
#define ERROR_DETECTOR      "H "
#define ERROR_INT_MSG       "I "
#define ERROR_KEYPAD        "K "
#define ERROR_L2            "L "
#define ERROR_MMU           "M "
#define ERROR_NAND          "N "
#define ERROR_RTC           "R "
#define ERROR_SHARE_PIN     "S "
#define ERROR_TIMER_COUNTER "T "
#define ERROR_UART          "U "

#define NAND_RR_INFO        "NF_RR"
#define NAND_RANDOMIZER_INFO "NF_RAND"
#define NAND_DATA_LEN_INFO  "NF_DL:"
#define NAND_REREAD_INFO    "NF_R F: "
#define NAND_REREAD_INFO1   " R:"
#define NAND_REREAD_INFO2   " C:"
#define NAND_STATUS_ERROR   "NF_S:"
#define NAND_READ_FAIL      "NF_F"
#define NAND_NFC_TIMEOUT    "NF_CT!"
#define NAND_ECC_TIMEOUT    "NF_ET!"
#define NAND_ECC_ERROR      "NF_EE:"


#endif //__HAL_ERRORSTR_H__

