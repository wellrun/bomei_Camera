#include <string.h>
#include "akdefine.h"
#include "fha_asa.h"

#define SERIAL_NAME        "SERADDR"
#define SERIAL_LEN_MAX     512 //sd boot need 512 size. 

#ifdef OS_ANYKA
T_U8 *Serial_GetSysNum(T_U8 **serial_buf, T_U32 *serial_len)
{
    T_U8 data_tmp[SERIAL_LEN_MAX];
    T_U32 data_len = 0;
    T_U8 *pBuf = AK_NULL;

    if (((AK_NULL != serial_buf) && (AK_NULL == *serial_buf)) || (AK_NULL == serial_len))
    {
        return AK_NULL;
    }
    
    memset(data_tmp, 0, SERIAL_LEN_MAX);
    
    if (ASA_FILE_SUCCESS != FHA_asa_read_file(SERIAL_NAME, data_tmp, SERIAL_LEN_MAX))
    {
        *serial_len = 0;
        return AK_NULL;
    }
    
    memcpy(&data_len, data_tmp, sizeof(T_U32));

    if (AK_NULL != serial_buf)
    {
        if (*serial_len < data_len)
        {
            return AK_NULL;
        }
        
        pBuf = *serial_buf;
        memcpy(pBuf, data_tmp + sizeof(T_U32), data_len);
    }

    *serial_len = data_len + 1; // add 1 is the '\0'
        
    return pBuf;
}
#endif

