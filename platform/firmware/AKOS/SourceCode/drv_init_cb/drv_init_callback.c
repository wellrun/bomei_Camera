#include "drv_callback.h"
#include "drv_gpio.h"
#include "hal_print.h"
#include "hal_sysdelay.h"
#include "arch_freq.h"
#include "arch_pwm.h"
#include "arch_interrupt.h"
#include "hal_timer.h"
#include "hal_ts.h"
#include "akos_api.h"


//wgtbupt
T_BOOL pwm2_set_duty_cycle(T_U32 pwm_freq, T_U16 duty_cycle);



#ifdef OS_ANYKA
extern T_BOOL spi_init(T_eSPI_ID spi_id, T_eSPI_MODE mode, T_eSPI_ROLE role, T_U32 clk_div);
extern T_VOID spi_close(T_eSPI_ID spi_id);
extern T_BOOL spi_master_write(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS);
extern T_BOOL spi_master_read(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS);
extern T_VOID gpio_pin_config_init(T_GPIO_SET *pGpioSetting);
extern T_U8 gpio_pin_get_ActiveLevel(T_U8 pin);
extern T_BOOL analog_setgain_linein(T_U8 gain);
extern T_BOOL analog_setsignal(ANALOG_SIGNAL_INPUT analog_in, ANALOG_SIGNAL_OUTPUT analog_out, ANALOG_SIGNAL_STATE    state);    



T_BOOL ADC_SetLinein(T_BOOL enable)
{
	if (enable)
	{
		analog_setsignal(INPUT_LINEIN, OUTPUT_HP, SIGNAL_CONNECT);
	}
	else
	{
		analog_setsignal(INPUT_LINEIN, OUTPUT_HP, SIGNAL_DISCONNECT);
	}

	return AK_TRUE;
}


static T_DRV_CALLBACK_FUNC drv_cb_func_handler = 
{
	us_delay,
	mini_delay,
	ADC_SetLinein,
	analog_setgain_linein,
	analog_setsignal,
	get_asic_freq,
	akprintf,
	//printf,
	pwm_set_duty_cycle,
	store_all_int,
	restore_all_int,
	get_tick_count, 
	AK_Drv_Protect,
	AK_Drv_Unprotect,
	AK_Sleep,
	spi_init,
	spi_close,
	spi_master_read,
	spi_master_write
};



T_VOID Drv_Init_Callback(T_VOID)
{
	Drv_Init_CB_Func(&drv_cb_func_handler);
}

#endif


