/**
 * @file Ctl_Icon.h
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */

#ifndef __CTL_ICON_H__
#define __CTL_ICON_H__
 
#include "akdefine.h"
#include "Ctl_SlipMgr.h"
 
 
#ifdef __cplusplus
 extern "C" {
#endif


 /**
 * @brief Creat a Icon control
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_RECT rect:
 * @param in T_U32 alignmode:
 * @return T_SLIP_ICON * the Icon handle
 * @retval
 */
 T_SLIP_ICON *Icon_Creat(T_RECT rect, T_U32 alignmode);

 /**
 * @brief Destroy a Icon control
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_ICON *pSlipIcon:the Icon handle
 * @return AK_NULL
 * @retval
 */
 T_VOID *Icon_Destroy(T_SLIP_ICON *pSlipIcon);

 /**
 * @brief Set Icon data
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_ICON *pSlipIcon:the Icon handle
 * @param in const T_pDATA pIcon:the icon data
 * @return T_BOOL
 * @retval
 */
 T_BOOL Icon_SetIcon(T_SLIP_ICON *pSlipIcon, T_pCDATA pIcon);

 /**
 * @brief Show Icon data to a buf
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_ICON *pSlipIcon:the Icon handle
 * @param in T_U8 *pbuf:the buf to show
 * @param in T_U32 imgwidth:the width of the buf
 * @param in T_U32 imgheight:the height of the buf
 * @return T_BOOL
 * @retval
 */
 T_BOOL Icon_Show(T_SLIP_ICON *pSlipIcon, T_U8 *pbuf, T_U32 imgwidth, T_U32 imgheight);


 
#ifdef __cplusplus
 }
#endif
 
#endif

