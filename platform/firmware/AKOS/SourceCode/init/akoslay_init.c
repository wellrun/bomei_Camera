#include "drv_init_callback.h"

#ifdef OS_ANYKA
extern T_VOID gpio_pin_init(T_VOID);

T_VOID AKOSLay_init(T_VOID)
{
	Drv_Init_Callback();
	gpio_pin_init();
}

#endif


