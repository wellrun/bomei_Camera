#include "drv_api.h"
#include "drv_gpio.h"

#define C1 1    /*Fatal error message*/
#define C2 2    /*Error message*/
#define C3 3    /*Common message*/

#define AK_ASSERT_VAL(_bool_, _msg_, _retval_)    if (!(_bool_)) { AkAssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return (_retval_); }
#define AK_ASSERT_PTR(_ptr_, _msg_, _retval_)    if (!AkAssertCheckPointer(_ptr_)) { AkAssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return (_retval_); }



typedef T_VOID (*T_fRTC_CALLBACK)(T_VOID);

typedef T_VOID (*US_DELAY_PT)(T_U32 us);
typedef T_VOID (*MINI_DELAY_PT)(T_U32 minisecond);
typedef T_VOID (*T_fTIMER_CALLBACK)(T_TIMER timer_id, T_U32 delay);

typedef T_BOOL (*ADC_SETLINEIN_PT)(T_BOOL enable);
typedef T_BOOL (*ANALOG_SETGAIN_LINEIN_PT)(T_U8 gain);
typedef T_BOOL (*ANALOG_SETSIGNAL_PT)(ANALOG_SIGNAL_INPUT analog_in, ANALOG_SIGNAL_OUTPUT analog_out, ANALOG_SIGNAL_STATE    state);

typedef T_U32 (*GET_ASIC_FREQ_PT)(T_VOID);
typedef T_VOID (*STORE_ALL_INT_PT)(T_VOID);
typedef T_VOID (*RESTORE_ALL_INT_PT)(T_VOID);
typedef T_VOID (*DETECTOR_ENABLE_PT)(T_U32 pin);

typedef int (*AKPRINTF_PT)(T_U8 level, T_pCSTR mStr, T_pCSTR s, ...);
typedef int (*PRINTF_PT)(const char *s, ...);

typedef T_BOOL (*PWM_SET_DUTY_CYCLE_PT)(T_U32 pwm_freq, T_U16 duty_cycle);
typedef T_BOOL (*PWM2_SET_DUTY_CYCLE_PT)(T_U32 pwm_freq, T_U16 duty_cycle);

typedef T_U32 (*GET_TICK_COUNT_PT)(T_VOID);


typedef T_VOID (*TS_ADCCHANGEFAST_PT)(T_VOID);
typedef T_VOID (*TS_ADCREGINIT_PT)(T_VOID);

typedef T_VOID (*AK_DRV_PROTECT_PT)(T_VOID);
typedef T_VOID (*AK_DRV_UNPROTECT_PT)(T_VOID);
typedef T_VOID (*AK_SLEEP_PT)(T_U32 ticks);

typedef T_BOOL (*SPI_INIT_PT)(T_eSPI_ID spi_id, T_eSPI_MODE mode, T_eSPI_ROLE role, T_U32 clk_div);
typedef T_VOID (*SPI_CLOSE_PT)(T_eSPI_ID spi_id);
typedef T_BOOL (*SPI_MASTER_READ_PT)(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS);
typedef T_BOOL (*SPI_MASTER_WRITE_PT)(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS);

typedef struct 
{
	US_DELAY_PT						us_delay;
	MINI_DELAY_PT					mini_delay;
	ADC_SETLINEIN_PT				ADC_SetLinein;
	ANALOG_SETGAIN_LINEIN_PT		analog_setgain_linein;
	ANALOG_SETSIGNAL_PT				analog_setsignal;
	GET_ASIC_FREQ_PT				get_asic_freq;
	AKPRINTF_PT						akprintf;
	//PRINTF_PT						printf;
	PWM_SET_DUTY_CYCLE_PT			pwm_set_duty_cycle;
	STORE_ALL_INT_PT				store_all_int;
	RESTORE_ALL_INT_PT				restore_all_int;
	GET_TICK_COUNT_PT				get_tick_count;
	AK_DRV_PROTECT_PT				AK_Drv_Protect;
	AK_DRV_UNPROTECT_PT				AK_Drv_Unprotect;
	AK_SLEEP_PT						AK_Sleep;
	SPI_INIT_PT						spi_init;
	SPI_CLOSE_PT					spi_close;
	SPI_MASTER_READ_PT				spi_master_read;
	SPI_MASTER_WRITE_PT				spi_master_write;
} T_DRV_CALLBACK_FUNC;


T_VOID Drv_Init_CB_Func(T_DRV_CALLBACK_FUNC *handler);


