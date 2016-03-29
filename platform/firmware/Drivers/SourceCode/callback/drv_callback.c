#include    <string.h>
#include "drv_callback.h"


T_DRV_CALLBACK_FUNC gb_drv_cb_fun;

T_VOID Drv_Init_CB_Func(T_DRV_CALLBACK_FUNC *handler)
{
	
	memset(&gb_drv_cb_fun, 0, sizeof(T_DRV_CALLBACK_FUNC));
	
	memcpy(&gb_drv_cb_fun, handler, sizeof(T_DRV_CALLBACK_FUNC));
}

