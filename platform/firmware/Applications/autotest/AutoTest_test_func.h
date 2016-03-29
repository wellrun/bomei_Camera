#ifndef _AUTO_TEST_H_
#define _AUTO_TEST_H_

#ifdef SUPPORT_AUTOTEST


#include "anyka_types.h"
#include "Fwl_osFS.h"


#define FILE_MAX_PATH_LEN          259
#define AUTOTEST_THREAD_PRI	       70 //”≈œ»º∂
#define AUTOTEST_THRD_STACK_SIZE   (256*1024)
#define AUTOTEST_THREAD_TIMESLICE  (2)

#define AUTOTEST_DIR_APTH          DRI_D"autotest"

typedef enum {
	eFS_AUTOTEST_ING = 0,
	eFS_AUTOTEST_Success,
	eFS_AUTOTEST_Fail,
	eFS_AUTOTEST_OtherState,
}e_AUTOTEST_STATE;

typedef T_BOOL (*F_pGetkeyIDback)(T_VOID *pData, T_U8 keyID, T_U8 presstype);
typedef e_AUTOTEST_STATE (*F_GekeyinfoState)(e_AUTOTEST_STATE state);
typedef T_U32   (*ThreadAutoTestFunPTR)(T_pVOID pData);


typedef struct
{
	T_hTask thread;
	T_U8 Stack[AUTOTEST_THRD_STACK_SIZE];
} T_AUTO_CTRL,*T_PAUTO_CTRL;

/**
 * @brief   creat a thread to start auto test
 *
 * @author  Lixingjian
 * @date    2012-04-10
 * @param   [in] autotest
 * @return  T_BOOL
 * @retval   successful
 * @retval   error
 */
T_BOOL  AutoTest_Test_CreatThread(T_U32 filenameflag, F_GekeyinfoState	SetState);


/**
 * @brief   kill a thread to stop auto test
 *
 * @author  Lixingjian
 * @date    2012-04-10
 * @param   [in] autotest
 * @return  T_VOID
 * @retval   
 * @retval  
 */
T_VOID  AutoTest_Test_KillThead(T_PAUTO_CTRL **ThreadHandle);


#endif


#endif /* _AUTO_TEST_H_ */
