
#include "anyka_types.h"

#include "gpio_config.h"
#include "drv_in_callback.h"

//#include "eng_debug.h"

#define init_i2c_pin(scl, sda) \
    do{ \
        gpio_set_pin_dir(scl, GPIO_DIR_OUTPUT); \
        gpio_set_pin_dir(sda, GPIO_DIR_OUTPUT); \
        gpio_set_pin_level(scl, GPIO_LEVEL_HIGH); \
        gpio_set_pin_level(sda, GPIO_LEVEL_HIGH); \
    }while(0);

#define release_i2c_pin(scl, sda) \
    do{ \
        gpio_set_pin_level(scl, GPIO_LEVEL_LOW); \
        gpio_set_pin_level(sda, GPIO_LEVEL_LOW); \
        gpio_set_pin_dir(scl, GPIO_DIR_INPUT); \
        gpio_set_pin_dir(sda, GPIO_DIR_INPUT); \
    }while(0);
#define set_i2c_pin(pin) \
    gpio_set_pin_level(pin, GPIO_LEVEL_HIGH)

#define clr_i2c_pin(pin) \
    gpio_set_pin_level(pin, GPIO_LEVEL_LOW)

#define get_i2c_pin(pin) \
    gpio_get_pin_level(pin)

#define set_i2c_data_in(pin) \
    gpio_set_pin_dir(pin, GPIO_DIR_INPUT)

#define set_i2c_data_out(pin) \
    gpio_set_pin_dir(pin, GPIO_DIR_OUTPUT)

static T_U32 I2C_SCL = INVALID_GPIO;    /* I2C serial interface clock input */
static T_U32 I2C_SDA = INVALID_GPIO;    /* I2C serial interface data I/O */
#define I2C_ACK		0
#define I2C_NACK	1

#define I2C_SCL_DEFAULT_SPEED	4 /* unit is us */
static T_U32 qtr_scl_cyc_delay = I2C_SCL_DEFAULT_SPEED;	/* scl default speed is 1000000/(4*2) = 125KHz */

#define US_TIMER	50
static void udelay(volatile T_U32 nus){

	int i;
	volatile T_U32 j;

//    AK_DEBUG_OUTPUT("qtr_scl_cyc udelay nus =%d",nus);
    
	for (i = 0; i < nus; i++){
		j = US_TIMER;
		while (j--);
	}
}

/* 
** scl_speed unit is us. you may not set scl speed by use default speed 125kHz.
*/
void i2c_set_scl_speed(T_U32 scl_speed){
	qtr_scl_cyc_delay = scl_speed / 4;
//    AK_DEBUG_OUTPUT("i2c_set_scl_speed  scl_speed=%d,qtr_scl_cyc_delay=%d",scl_speed,qtr_scl_cyc_delay);
}
/*
static void i2c_delay(void){
	udelay(qtr_scl_cyc_delay);
}*/

void i2c_pin_cfg(T_U32 pin_scl, T_U32 pin_sda){

    	I2C_SCL = pin_scl;
    	I2C_SDA = pin_sda;
    	init_i2c_pin(I2C_SCL, I2C_SDA);

	i2c_set_scl_speed(I2C_SCL_DEFAULT_SPEED);
}

static void sda_in(void){
    gpio_set_pin_dir(I2C_SDA, GPIO_DIR_INPUT);
}

static void sda_out(void){
    gpio_set_pin_dir(I2C_SDA, GPIO_DIR_OUTPUT);
}

static T_BOOL get_sda(void){
    return gpio_get_pin_level(I2C_SDA);
}

static void clr_sda(void){
	gpio_set_pin_level(I2C_SDA, GPIO_LEVEL_LOW);
}

static void set_sda(void){
	gpio_set_pin_level(I2C_SDA, GPIO_LEVEL_HIGH);
}
static void clr_scl(void){
	gpio_set_pin_level(I2C_SCL, GPIO_LEVEL_LOW);
}

static void set_scl(void){
	gpio_set_pin_level(I2C_SCL, GPIO_LEVEL_HIGH);
}

#define I2C_DELAY_NUS	2
static void qtr_scl_cyc(void){
	udelay(qtr_scl_cyc_delay);
}

static void halt_scl_cyc(void){
	qtr_scl_cyc();
	qtr_scl_cyc();
}

/*
** make sure i2c_bus is idle, before call start().
** after call start(), start hold time is satisfy. you can trans data,and don't need delay.
*/
static void start(void){
	///* if you make sure i2c bus is idle,don't need next lines.
	sda_out(); // if sda is output,this line don't need.
	set_sda(); // if sda is output and output high,this two lines don't need. 
	qtr_scl_cyc();

	set_scl(); // if scl is output high
	qtr_scl_cyc();
	//*/

	clr_sda();
	qtr_scl_cyc();
	clr_scl();
}

/*
** before call stop(), scl is low level.
*/
static void stop(void){
	sda_out();
	clr_sda();
	set_scl();
	qtr_scl_cyc();
	set_sda();
	qtr_scl_cyc();
}

/*
** before call write_ack(),scl is low level.
*/
static void write_ack(T_U8 flag){
	qtr_scl_cyc();
	sda_out();
	if (flag == I2C_ACK){
		clr_sda();
	}else{
		set_sda();
	}
	qtr_scl_cyc();
	set_scl();
	halt_scl_cyc();
	clr_scl();
}

/*
** before call read_ack(), scl is low level.
** retval: 0 means recieve ack, 1 means no recieve ack
*/
static T_BOOL read_ack(void){
	T_BOOL ack;

	sda_in();
	halt_scl_cyc();
	set_scl();
	qtr_scl_cyc();
	ack = get_sda();
	qtr_scl_cyc();
	clr_scl();

	if (ack == 0){
		return I2C_ACK;
	}else{
		return I2C_NACK;
	}
}

/*
** before call write_byte(). scl is low level.
*/
static void write_byte(T_U8 data){
    	T_U8 i;

	sda_out();
	for (i=0; i<8; i++)
    	{
		(data & 0x80) ? set_sda() : clr_sda();
        	data <<= 1;
		halt_scl_cyc();
		set_scl();
		halt_scl_cyc();
		clr_scl();
    	}
}
/*
** before call read_byte(), scl is low level.
*/
static T_U8 read_byte(void){
    	T_U8 i;
    	T_U8 ret = 0;

   	sda_in(); 
	for (i=0; i<8; i++)
	{       
		halt_scl_cyc();    
        	set_scl();
		qtr_scl_cyc();
		ret = ret << 1;
		if ( get_sda() ){
			ret |= 1;
		}
		qtr_scl_cyc();
		clr_scl();
	}
       
	return ret;
}

int i2c_cmd(T_U8 *wr_data, int wr_len, T_U8 *rd_data, int rd_len){

	int ret;

	/*
	int i;
	for (i = 0; i < wr_len; i++){
		printf("write data[%d] = 0x%x\n",i,wr_data[i]);
	}
	*/
	gb_drv_cb_fun.store_all_int();
	start();
	while(wr_len--){
		write_byte(*wr_data++);
		if ( I2C_NACK == read_ack() ){
			/*stop();
			printf("i2c write no ack.\n");
			return -1;*/
			ret = -1;
			goto exit_i2c_cmd;
		}
		//wr_data++;
	}

	while (rd_len--){
		*rd_data++ = read_byte();
		if (rd_len){
			write_ack(I2C_ACK);
		}else{
			write_ack(I2C_NACK);
		}
	}
	ret = 0;
	
exit_i2c_cmd:
	stop();
	gb_drv_cb_fun.restore_all_int();
	return ret;
}
