#ifdef OS_WIN32

#include "anyka_types.h"
#include "hal_sysdelay.h"
#include   <windows.h>

/**
 * @brief millisecond delay
 * @author guoshaofeng
 * @date 2007-04-23
 * @param[in] minisecond minisecond delay number
 * @return T_VOID
 */
T_VOID mini_delay(T_U32 minisecond)
{
    Sleep(minisecond);
}

/**
 * @brief microsecond delay
 * @author guoshaofeng
 * @date 2007-04-23
 * @param[in] us microsecond delay number
 * @return T_VOID
 */
T_VOID us_delay(T_U32 us)
{

}
#endif
