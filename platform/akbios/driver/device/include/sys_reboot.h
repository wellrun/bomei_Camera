#ifndef _SYS_REBOOT_H_
#define _SYS_REBOOT_H_

#include "anyka_types.h"
#include "eng_string.h"
#include "mmi_config.h"
#include "drv_api.h"
#ifdef SUPPORT_NAND_TRACE
#include "eng_nandtrace.h"
#endif	// end of #ifdef SUPPORT_NAND_TRACE



#ifdef SUPPORT_PANNIC_REBOOT
#ifdef OS_ANYKA

#define	SIM_STATE_FLAG			*(volatile T_U32*)(&SYS_STATE_FLAG-1)


#define NAND_REBOOT_ZONE_ROOM 		(2 * 1024 * 1024)
#define BASE_FS_START_ADDR          (2 * 1024 * 1024)
#define BLOCK_MAX                   (NAND_REBOOT_ZONE_ROOM / (32 * 512))


// the mmi file name must identical with burn tool
#define MMI_PATH		    	(_T("a:/t500_elfd.bin"))

#define ASSERT_TYPE     2
#define WATCHDOG_TYPE     3



/**
 * @brief read MMI RW region from MMI bin file, then write to NANDFLSH reserve zone
 *
 * @author LiChenjie
 * @date 2007-10-31
 * @param None
 * @return T_BOOL
 * @retval AK_TRUE: sucess
 * @retval AK_FALSE: Failure
 */
T_BOOL bakupMmiRw(T_TCHR *path);



/**
 * @brief reset AK322X 
 *
 * @author LiChenjie
 * @date 2007-11-19
 * @param None
 * @return None
 */
T_fCExceptCallback System_Start(T_U8 type);


#endif	// end of #ifdef OS_ANYKA
#endif  // end of #ifdef SUPPORT_PANNIC_REBOOT
#endif	// end of #ifndef _SYS_REBOOT_H_
