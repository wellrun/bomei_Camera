
#include "rtc.h"
#include "rtc_algo.h"
//#include "Eng_Debug.h"
#include "gpio_config.h"
#include "drv_in_callback.h"
#include "i2c_core.h"

#define SYSTEM_DEFAULT_YEAR    2008 

#define I2C_READ_FLAG	1
#define I2C_WRITE_FLAG	0

#define RTC_S35390A_ADDR 0x60	//( (0110b) <<4 )
#define RTC_S35390A_ADDR_MIX 0x60
#define RTC_S35390A_ADDR_MAX 0x67

/* S35390A regs number define */
#define REG_STATUS1	0
#define REG_STATUS2	1
#define REG_DATE	2
#define REG_TIME	3
#define REG_ALARM1	4
#define REG_FREQ1	4
#define REG_ALARM2	5
#define REG_FREQ2	5
#define REG_ADJUST	6
#define REG_FREE	7

/* S35390A regs len define */
#define STATUS1_LEN	1
#define STATUS2_LEN	1
#define DATE_LEN	7
#define TIME_LEN	3
#define ALARM1_LEN	3
#define FREQ1_LEN	1
#define ALARM2_LEN	3
#define FREQ2_LEN	5
#define ADJUST_LEN	1
#define FREE_LEN	1
#define RTC_MAX_REG_LEN 7


static int 
write_reg(T_U8 reg_num, T_U8 *reg_data, T_U8 reg_len){

	T_U8 wr_buf[RTC_MAX_REG_LEN + 1];
//	int i;
 //   int nRet = 0;

	//printf("reg_data[0] = 0x%x.\n",*reg_data);
	wr_buf[0] = RTC_S35390A_ADDR | (reg_num << 1);
	//memcopy(wr_buf+1, reg_data, reg_len);
	my_memcopy(reg_data, wr_buf+1, reg_len);

	/*printf("reg num: %d.\n", reg_num);
	for (i = 0; i < (reg_len+1); i++){
		printf("wr_buf[%i] = 0x%x.\n", i, wr_buf[i]);
	}*/

#if 0
    for(i = 0; i < reg_len+1; i++  )
    {
        AK_DEBUG_OUTPUT("write_reg wr_buf[%d]=%d",i,wr_buf[i]);
    }
#endif
    
	return i2c_cmd(wr_buf, reg_len+1, 0, 0);
//    AK_DEBUG_OUTPUT("write_reg nRet =%d",nRet);
    
}

static int 
read_reg(T_U8 reg_num, T_U8 *reg_data, T_U8 reg_len){
	T_U8 addr;
	//T_U8 rd_buf[RTC_MAX_REG_LEN];
	//int ret;

 //   int i =0;

     
	addr = RTC_S35390A_ADDR | (reg_num << 1) | I2C_READ_FLAG;

//    AK_DEBUG_OUTPUT("read_reg add=%d", addr);
    
	return i2c_cmd(&addr, 1, reg_data, reg_len);

//    AK_DEBUG_OUTPUT("read_reg ret=%d",ret);

}

/* defaut time: 2008.08.08   4  10:35:58 */   // friday
static const default_date[DATE_LEN] = {8,8,8,4,10,35,58};

#define RTC_POC	(1 << 0)
#define RTC_BLD (1 << 1)
#define RTC_24H	(1 << 6)
#define RTC_RST (1 << 7)

#define RTC_TEST (1 << 0)


int rtc_SetPowerFlag(T_U8 num)
{
    int ret = 10;
    T_U8 buf[RTC_MAX_REG_LEN];

    buf[0] = (T_U8)(num);
	if ( (ret = write_reg(REG_FREE, buf, 1)) != 0 )
	{
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "rtc_SetPowerFlag write reset command error.\n");
		}
		return ret;
	}

    return 0;
}

T_U8 rtc_CheckPower(void)
{
    int ret = 10;
    T_U8 buf[RTC_MAX_REG_LEN];
    
	if ( (ret = read_reg(REG_FREE, buf, 1)) != 0 )
	{
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "rtc_CheckPower error_1. ret=%d\n", ret);
		}

		return (T_U8)(-1);
	}
	if (AK_NULL != gb_drv_cb_fun.akprintf)
	{
		gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "rtc_CheckPower  buf[0] =%d , \n", buf[0]);
	}


    return buf[0];
    
}
/* 
** default 24 hours format, and no power on loop.
*/
int rtc_reset(T_U32 pin_scl, T_U32 pin_sda){
	int ret,i;
	
	T_U8 buf[RTC_MAX_REG_LEN];
	//T_U8 *time;

    //T_U8 tmpData = 0;
	
	i2c_pin_cfg(pin_scl, pin_sda);

//loop_rtc_power_on:
	//rtc_power_on();
	//wait_500ms();
	/* read status1 reg */
	ret = read_reg(REG_STATUS1, buf, 1);
	if (ret)
	{
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "read status1 error_1.\n");
		}

		return ret;
	}
    
	/* check poc flag is ok,else goto rtc power on */
	if ( !(buf[0] & RTC_POC) ){
		/* note here may be goto bead loop, you can limit retry times */
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "POC bit don't equal 1.\n");
		}

		//return -1;
		//goto loop_rtc_power_on;	
}

//write_reset:
	buf[0] = RTC_RST;
	/* reset rtc module by set status1 reg's bit7 */
	ret = write_reg(REG_STATUS1, buf, 1);
	if (ret)
	{
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "write reset command error.\n");
		}

		return ret;
	}
        
	/* check poc flag and bld flag are ok,else goto rtc reset */
	ret = read_reg(REG_STATUS1, buf, 1);
	if (ret)
	{
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "read status1 error_2.\n");
		}

		return ret;
	}

//chekc_poc_bld:
	/* check poc flag is ok,else goto rtc power on */
	if ( (buf[0] & RTC_POC) || (buf[0] & RTC_BLD) ){
		/* note here may be goto bead loop, you can limit retry times */
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "POC or BLD don't equal 0.\n");
		}

		return -1; //goto loop_rtc_power_on;	
	}

	/* set am/pm format, default am */
	/* read status reg 1 and make sure,but how to make sure */
	buf[0] |= RTC_24H;
	ret = write_reg(REG_STATUS1, buf, 1);
	if(ret)
	{
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "set am/pm error.\n");
		}

		return -1;
	}
    
	if ( !(buf[0] & RTC_24H) ){
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "set am/pm failt.\n");
		}

		return -1; //goto check_poc_bld;
	}
		
	/* now rtc module reset finish.
	set date/time
	read status reg2 and check test bit flag is ok? why */
	if (AK_NULL != gb_drv_cb_fun.akprintf)
	{
		gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "default date is:\n");
	}

	for (i = 0; i < DATE_LEN; i++){
		buf[i] = hex2bcd(default_date[i]);
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "default_date[i] =%d,bcd[i]=%d \n ", default_date[i],buf[i]);
		}
	}
	
	if (AK_NULL != gb_drv_cb_fun.akprintf)
	{
		gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "\n");
	}

	ret = write_reg(REG_DATE, buf, DATE_LEN);
	if(ret)
	{
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "set date error.\n");
		}
		return -1;
	}

	mem_clr(buf,DATE_LEN);

   	ret = read_reg(REG_DATE, buf, DATE_LEN);
	if (ret)
	{
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "read date reg error.\n");
		}
		return -1;
	}

	if (AK_NULL != gb_drv_cb_fun.akprintf)
	{
		gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "now date is:\n");
		for (i = 0; i < DATE_LEN; i++){
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "hex[i]=%d ,bcd[i]=%d\n",bcd2hex(buf[i]),buf[i]);
		}
		gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "\n");
	}
	

	ret = read_reg(REG_STATUS2, buf, 1);
	if(ret)
	{
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "read status2 reg error.\n");
		}

		return -1;
	}
	if (buf[0] & RTC_TEST){
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "test bit don't equal 0, rtc reset fail.\n");
		}

		return -1; // goto loop_rtc_power_on
	}
   
	return AK_TRUE;
}

static int rtc_read_rtctime(RTC_TIME *pt_rtc_time){
	int ret,i;
	//T_U8 buf[RTC_MAX_REG_LEN];
	T_U8 *buf;
	if (!pt_rtc_time){
		return -1;
	}

	buf = (T_U8 *)pt_rtc_time;
	ret = read_reg(REG_DATE, buf, DATE_LEN);
	if (ret)
	{
		return -1;
	}
	buf[4]&=0xfc;
	for (i = 0; i < sizeof(RTC_TIME); i++){
		buf[i] = bcd2hex(buf[i]);	
	}
	/*printf("now date is:\n");
	for (i = 0; i < DATE_LEN; i++){
		//printf("hex:0x%x,bcd:%x\n",bcd2hex(buf[i]),buf[i]);
		//printf("%d ", bcd2hex(buf[i]));
		printf("%d ", pt_rtc_time[i]);
	}
	printf("\n");*/
	return 0;
}

static int rtc_write_rtctime(RTC_TIME *pt_rtc_time){
	int i;
	T_U8 *buf;
	
	if (!pt_rtc_time){
		return -1;
	}

	buf = (T_U8 *)pt_rtc_time;
	for (i = 0; i < sizeof(RTC_TIME); i++){
		buf[i] = hex2bcd(buf[i]);
	}
	
	if ( write_reg(REG_DATE, buf, DATE_LEN) ){
		return -1;
	}
	return 0;
}

static void trans2systime(const RTC_TIME *rtc_time, T_SYSTIME *sys_time){
	sys_time->year = rtc_time->year + 2000;
	sys_time->month = rtc_time->month;
	sys_time->day = rtc_time->day;
	sys_time->week = rtc_time->week;
	sys_time->hour = rtc_time->hour;
	sys_time->minute = rtc_time->minute;
	sys_time->second = rtc_time->second;
}

static void trans2rtctime(RTC_TIME *rtc_time, const T_SYSTIME *sys_time){
	if(sys_time->year < 2000)
		{
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "sys_time->year<SYSTEM_DEFAULT_YEA\r\n");
		}

  	       rtc_time->year = default_date[0];
            	rtc_time->month =  default_date[1];
            	rtc_time->day =  default_date[2];
            	rtc_time->week =  default_date[3];
            	rtc_time->hour =  default_date[4];
            	rtc_time->minute =  default_date[5];
            	rtc_time->second =  default_date[6];
		}
          else
          {
          	rtc_time->year = sys_time->year - 2000;
	rtc_time->month = sys_time->month;
	rtc_time->day = sys_time->day;
	rtc_time->week = sys_time->week;
	rtc_time->hour = sys_time->hour;
	rtc_time->minute = sys_time->minute;
	rtc_time->second = sys_time->second;
            }
}

/* return value: 0 means success, else fail */
int rtc_read_systime(T_SYSTIME *pt_sys_time){
	RTC_TIME rtc_time;
	
	if (!pt_sys_time){
		return -1;
	}

	if ( rtc_read_rtctime(&rtc_time) ){
		return -1;
	}
	
	trans2systime(&rtc_time, pt_sys_time);
	return 0;
}

/* return value: 0 means success, else fail */
int rtc_write_systime(T_SYSTIME *pt_sys_time){
	RTC_TIME rtc_time;
	
	if (!pt_sys_time){
		return -1;
	}

	trans2rtctime(&rtc_time, pt_sys_time);
	return rtc_write_rtctime(&rtc_time);
}

#define BIT_ALARM1	(1 << 0)
#define BIT_ALARM2	(1 << 1)

/* use alarm num for index */
static const T_U8 alarm_marks[] = {BIT_ALARM1, BIT_ALARM2};

static T_U8 alarm_open_flag = 0;
/* 0x00 means all alarms close,
** 0x01 means alarm1 is open
** 0x02 means alarm2 is open
** 0x03 means alarm1 and alarm2 are open
*/

static int rtc_open_alarm(T_U8 alarm_num){
	T_U8 status2;

	if ( read_reg(REG_STATUS2, &status2, STATUS2_LEN) ){
		return -1;
	}

	switch (alarm_num){
		case 0:
			status2 = (status2 & 0x0f) | 0x20;
			break;

		case 1:
			status2 = (status2 & 0xf0) | 0x02;
			break;

		default:
			return -1;
			break;
	}

	if ( write_reg(REG_STATUS2, &status2, STATUS2_LEN) ){
		return -1;
	}

	gb_drv_cb_fun.store_all_int();
	alarm_open_flag |= alarm_marks[alarm_num];
	gb_drv_cb_fun.restore_all_int();
	
	return 0;
}

/*
int rtc_close_alarm(T_U8 alarm_num){
	T_U8 status2;

	if ( read_reg(REG_STATUS2, &status2, STATUS2_LEN) ){
		return -1;
	}

	switch (alarm_num){
		case 0:
			status2 &= 0x0f;
			break;

		case 1:
			status2 &= 0xf0;
			break;

		default:
			return -1;
			break;
	}
	
	if ( write_reg(REG_STATUS2, &status2, STATUS2_LEN) ){
		return -1;
	}

	store_all_int();
	alarm1_open_flag &= alarm_marks[alarm_num];
	restore_all_int();
	
	return 0;
}
*/

int rtc_set_alarm(T_U8 alarm_num,T_SYSTIME *sys_time){
	T_U8 reg_num;
	T_U8 buf[ALARM1_LEN];

    T_U8  nTmpData = 0;

	if (rtc_open_alarm(alarm_num)){
			return -1;
	}
	
	switch (alarm_num){
		case 0:
			reg_num = REG_ALARM1;
			break;

		case 1:
			reg_num = REG_ALARM2;
			break;

		default:
			return -1;
			break;
	}

    if(sys_time->hour >= 12)
    {
        nTmpData = 2;
    }
    else
    {
        nTmpData = 0;

    }
    
	buf[0] = hex2bcd(sys_time->week) | 1;
	buf[1] = hex2bcd(sys_time->hour) | 1 | nTmpData;
	buf[2] = hex2bcd(sys_time->minute) | 1;

//    AK_DEBUG_OUTPUT("rtc_set_alarm  alarm_num=%d,week=%d,hour=%d,minute=%d",alarm_num, buf[0],buf[1],buf[2]);
    
	if ( write_reg(reg_num, buf, ALARM1_LEN) ){
		return -1;
	}

	return 0;
	
}

int rtc_get_alarm(T_U8 alarm_num,T_SYSTIME *sys_time){
	T_U8 reg_num;
	T_U8 buf[ALARM1_LEN];
       //T_SYSTIME ret;	
	gb_drv_cb_fun.store_all_int();
	if ( !(alarm_open_flag & alarm_marks[alarm_num]) ){
		gb_drv_cb_fun.restore_all_int();
		return -1;
	}
	gb_drv_cb_fun.restore_all_int();
	
	switch (alarm_num){
		case 0:
			reg_num = REG_ALARM1;
			break;

		case 1:
			reg_num = REG_ALARM2;
			break;

		default:
			return -1;
			break;
	}

	if ( read_reg(reg_num, buf, ALARM1_LEN) ){
		return -1;
	}

//    AK_DEBUG_OUTPUT("rtc_get_alarm  alarm_num=%d,week=%d,hour=%d,minute=%d",alarm_num, bcd2hex(buf[0]),bcd2hex(buf[1]),bcd2hex(buf[2]));
	sys_time->week = bcd2hex( buf[0] & 0xfe );
	sys_time->hour = bcd2hex( buf[1] & 0xfc );
	sys_time->minute = bcd2hex( buf[2] & 0xfe );
	return 0;

}
/* 一次alarm1到达后，int1脚变为高电平直到调用这个函数手动清除 */
int rtc_clear_alarm(T_U8 alarm_num){
	T_U8 status;

	/* read status1 to clear  INT1 and INT2 flag */
	if ( read_reg(REG_STATUS1, &status, STATUS1_LEN) ){
		return -1;
	}

	if ( read_reg(REG_STATUS2, &status, STATUS2_LEN) ){
		return -1;
	}

	switch (alarm_num){
		case 0:
			status &= 0x0f;
			break;

		case 1:
			status &= 0xf0;
			break;

		default:
			return -1;
			break;
	}

	if ( write_reg(REG_STATUS2, &status, STATUS2_LEN) ){
		return -1;
	}

	gb_drv_cb_fun.store_all_int();
	alarm_open_flag &= ~alarm_marks[alarm_num];
	gb_drv_cb_fun.restore_all_int();
	
	return 0;
}

/*
static T_BOOL rtc_probe_device(void){
	
	T_U8 rtc_dev_addr;
	
	rtc_dev_addr = RTC_S35390A_ADDR;
	if ( (ret = write_reg(REG_STATUS2, &rtc_dev_addr, STATUS_LEN)) ){
		return -1;
	}

static T_RTC_FUNCTION_HANDLE rtc_s35390a_handle = 
{
    RTC_S35390A_ADDR,
    rtc_probe_device,
    rtc_s35390a_reset,
    rtc_read_time,
    rtc_write_time,
    AK_NULL    
};

#define RTC_S35390A_ID  1

static int rtc_s35390a_reg(void)
{
    rtc_reg_dev(RTC_S35390A_ID, &rtc_s35390a_handle);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(rtc_s35390a_reg)
#ifdef __CC_ARM
#pragma arm section
#endif
*/

/********************** the next functions only for test now ******************/
int cmd_rtc_read_status1(void){

	T_U8 status1 = 0;
	T_U8 addr;
	int ret;
 
	addr = RTC_S35390A_ADDR | 1;
	
	ret = i2c_cmd(&addr, 1, &status1, 1);   	
	if (ret == 0){
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "status1_reg = %d\n", status1);
		} 
	}
	return AK_TRUE;
}

int cmd_rtc_read_status2(void){

	T_U8 status2 = 0;
	T_U8 addr;
	int ret;
 
	addr = RTC_S35390A_ADDR | (1 << 1) |1;
	
	ret = i2c_cmd(&addr, 1, &status2, 1);   	
	if (ret == 0){
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "status2_reg = %d\n", status2);
		}
	}
	return AK_TRUE;
}


int cmd_rtc_write_free_reg(void){
	T_U8 free;
	int ret;
	
	free = 0x55;
	ret = write_reg(REG_FREE, &free, FREE_LEN);
	if (ret == 0){
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "write free reg no error.\n");
		}   
	}
	return AK_TRUE;
}

int cmd_rtc_read_free_reg(void){
	T_U8 free_reg = 0;
	
	int ret;
	
	ret = read_reg(REG_FREE, &free_reg, FREE_LEN);
	if (ret == 0){
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "free reg = %d\n", free_reg);
		} 
	}

	return AK_TRUE;
}