/***************************************************************************
* @FILENAME: l2_cache.h
* @BRIEF     l2 cache driver head file
* Copyright  (C) 2010 Anyka (Guang zhou) Software Technology Co., LTD
* @AUTHOR   caodonghua 
* @DATA     2010-8-11
* @VERSION  2.0
* @REF please refer to...
*****************************************************************************/


/****************************************************************************
The following is an example to use l2 cache controller driver APIs

*****************************************************************************/

#ifndef __L2_CACHE_H__
#define __L2_CACHE_H__

#include "anyka_types.h"

#define L2_CACHE_MINI_ADDR                  0x4000

#define L2_CACHE_SECTION_EN(Index)          (1 << Index)

#define L2_CACHE_RAM_BUSY(AHB_Index)        (1 << (AHB_Index + 16))

#define L2_CACHE_BUSY                       (1 << 24)
#define L2_CACHE_INITIAL_STATUS             (1 << 25)
#define L2_CACHE_ENTRY_CLR                  (1 << 26)
#define L2_LINE_BUFFER_CLR_NON              (1 << 27)

#define L2_CACHE_SECTION_NUM            8

#define L2_CACHE_SECTION_ENABLE         1
#define L2_CACHE_SECTION_DISABLE        0


enum L2_CACHE_SECTION_INDEX
{
    L2_CACHE_SECTION0 = 0,  
    L2_CACHE_SECTION1,
    L2_CACHE_SECTION2,
    L2_CACHE_SECTION3,
    L2_CACHE_SECTION4,
    L2_CACHE_SECTION5,
    L2_CACHE_SECTION6,
    L2_CACHE_SECTION7
};

typedef struct
{
    T_U32 start_addr;
    T_U32 end_addr;
    T_U8 status;
    T_U8 enable;
}
T_L2CACHE_SECTION_INFO;

/*********************************************************************
* @BRIEF set the l2 cache initial
* @AUTHOR caodonghua
* @DATE   2010-8-11
* @PARAM T_VOID
* @RETURN T_BOOL: open l2 cache section open ok, return AK_TRUE, open failed,
     return AK_FALSE
* @RETVAL set ok, return AK_TRUE, set failed, return AK_FALSE
* @NOTE: the section start address reg bit[13:0] all must be 0,
     the end address reg bit[13:0] all must be 1.  
*********************************************************************/
T_BOOL l2_cache_initial(T_VOID);


/*********************************************************************
* @BRIEF wait for the l2 cache clean finish
* @AUTHOR caodonghua
* @DATE   2010-8-11
* @PARAM T_VOID
* @RETURN T_BOOL: l2 cache clean finish ok, return AK_TRUE, clean failed,
     return AK_FALSE
* @RETVAL 2 cache clean finish ok, return AK_TRUE, clean failed,
     return AK_FALSE
* @NOTE:   
*********************************************************************/
T_BOOL l2_cache_clean_finish(T_VOID);


/*********************************************************************
* @BRIEF invalidate l2 cache and wait for finish
* @AUTHOR caodonghua
* @DATE   2010-8-11
* @PARAM T_VOID
* @RETURN T_BOOL: l2 cache invalidate finish ok, return AK_TRUE, invalidate failed,
     return AK_FALSE
* @RETVAL l2 cache invalidate finish ok, return AK_TRUE, invalidate failed,
     return AK_FALSE
* @NOTE:   
*********************************************************************/
T_BOOL l2_cache_invalidate(T_VOID);

/*********************************************************************
* @BRIEF set the l2 cache section map start and end address
* @AUTHOR caodonghua
* @DATE 2010-8-11
* @PARAM T_U32 StartAddr : set extern dram address
* @PARAM T_U32 EndAddr : set extern dram address
* @PARAM T_U8 SectionIdx : select section index
* @RETURN T_BOOL: set ok, return AK_TRUE, set failed, return AK_FALSE
* @RETVAL set ok, return AK_TRUE, set failed, return AK_FALSE
* @NOTE: the section start address reg bit[13:0] all must be 0,
     the end address reg bit[13:0] all must be 1.  
*********************************************************************/
T_BOOL l2_cache_map(T_U32 StartAddr, T_U32 EndAddr, T_U8 SectionIdx);


#endif


