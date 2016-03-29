/**
 * @file i2c.c
 * @brief I2C interface driver, define I2C interface APIs.
 *
 * This file provides I2C APIs: I2C initialization, write data to I2C & read data from I2C.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @version 1.0
 * @ref AK3210M technical manual.
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "drv_gpio.h"
#include "drv_module.h"


#ifdef OS_ANYKA

/** Use this flag to set I2C transmit freq, if I2C_DEFAULT_DELAY_UNIT is become more, the default freq is become less. */
#define  I2C_DEFAULT_DELAY_UNIT        24

static T_U32 I2C_SCL = INVALID_GPIO;    /* I2C serial interface clock input */
static T_U32 I2C_SDA = INVALID_GPIO;    /* I2C serial interface data I/O */

#define init_i2c_pin(scl, sda) \
    do{ \
        gpio_set_pin_level(scl, GPIO_LEVEL_HIGH); \
        gpio_set_pin_level(sda, GPIO_LEVEL_HIGH); \
		gpio_set_pin_dir(scl, GPIO_DIR_OUTPUT); \
        gpio_set_pin_dir(sda, GPIO_DIR_OUTPUT); \
    }while(0);

#define release_i2c_pin(scl, sda) \
    do{ \
        gpio_set_pin_level(scl, GPIO_LEVEL_LOW); \
        gpio_set_pin_level(sda, GPIO_LEVEL_LOW); \
        gpio_set_pin_dir(scl, GPIO_DIR_OUTPUT); \
        gpio_set_pin_dir(sda, GPIO_DIR_OUTPUT); \
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
    
#if 0    
static T_VOID set_i2c_pin(T_U32 pin);
static T_VOID clr_i2c_pin(T_U32 pin);
static T_U8   get_i2c_pin(T_U32 pin);
#endif    
static T_VOID i2c_delay(T_U32 time);

static T_VOID i2c_begin(T_VOID);
static T_VOID i2c_end(T_VOID);
static T_VOID i2c_write_ask(T_U8 flag);
static T_BOOL i2c_read_ack(T_VOID);
static T_U8   i2c_read_byte(T_VOID);
static T_BOOL i2c_write_byte(T_U8 data);

static T_U32 i2c_delay_unit = I2C_DEFAULT_DELAY_UNIT;


T_BOOL i2c_write_bytes(T_U8 *addr, T_U32 addrlen, T_U8 *data, T_U32 size)
{
    T_U32 i;     
      
    // start transmite
    i2c_begin();
    
    // write address to I2C device, first is device address, second is the register address
    for (i=0; i<addrlen; i++)
    {
        if (!i2c_write_byte(addr[i]))
        {
            i2c_end();
            return AK_FALSE;
        }
    }   
    
    // transmite data
    for (i=0; i<size; i++)
    {
        if (!i2c_write_byte(data[i]))
        {
            i2c_end();
            return AK_FALSE;
        }
    }
    
    // stop transmited
    i2c_end();
    
    return AK_TRUE;
}

T_BOOL i2c_read_bytes(T_U8 *addr, T_U32 addrlen, T_U8 *data, T_U32 size)
{
    T_U32 i;     
 
    // start transmite
    i2c_begin();
    
    // write address to I2C device, first is device address, second is the register address
    for (i=0; i<addrlen; i++)
    {
        if (!i2c_write_byte(addr[i]))
        {
            i2c_end();
            return AK_FALSE;
        }
    }

    i2c_end();
    
    // restart transmite
    i2c_begin();
    
    // send message to I2C device to transmite data
    if (!i2c_write_byte((T_U8)(addr[0] | 1)))
    {
        i2c_end();
        return AK_FALSE;
    }
    
    // transmite data
    for(i=0; i<size; i++)
    {
        data[i] = i2c_read_byte();
        (i<size-1) ? i2c_write_ask(0) : i2c_write_ask(1);
    }
    
    // stop transmite
    i2c_end();
    
    return AK_TRUE;
}

T_U32 i2c_set_cycle_delay(T_U32 delay)
{
    i2c_delay_unit = (delay >= 1) ? delay : 1;
    
    return i2c_delay_unit;
}

/**
 * @brief receive one byte from I2C interface function
 * receive one byte data from I2C bus
 * @author Junhua Zhao
 * @date 2005-04-05
 * @param T_VOID
 * @return T_U8: received the data
 * @retval
 */
static T_U8 i2c_read_byte(T_VOID)
{
    T_U8 i;
    T_U8 ret = 0;

    set_i2c_data_in(I2C_SDA);
    
    for (i=0; i<8; i++)
    {           
        i2c_delay(i2c_delay_unit << 2);
        set_i2c_pin(I2C_SCL);   
        i2c_delay(i2c_delay_unit << 2);
        ret = ret<<1;
        if (get_i2c_pin(I2C_SDA))
            ret |= 1;
        i2c_delay(i2c_delay_unit << 2);
        clr_i2c_pin(I2C_SCL);
        i2c_delay(i2c_delay_unit << 1);
        if (i==7)
        {
            set_i2c_data_out(I2C_SDA);
        }
        i2c_delay(i2c_delay_unit << 1);
    }       

    return ret;
}

/**
 * @brief write one byte to I2C interface function
 * write one byte data to I2C bus
 * @author Junhua Zhao
 * @date 2005-04-05
 * @param T_U8 data: send the data
 * @return T_BOOL: return write success or failed
 * @retval AK_FALSE: operate failed
 * @retval AK_TRUE: operate success
 */
static T_BOOL i2c_write_byte(T_U8 data)
{
    T_U8 i;

    for (i=0; i<8; i++)
    {
        i2c_delay(i2c_delay_unit << 2);
        if (data & 0x80)
            set_i2c_pin(I2C_SDA);
        else
            clr_i2c_pin(I2C_SDA);
        data <<= 1;

        i2c_delay(i2c_delay_unit << 2);
        set_i2c_pin(I2C_SCL);
        i2c_delay(i2c_delay_unit << 3);
        clr_i2c_pin(I2C_SCL);       
    }   
    
    return i2c_read_ack();
}

/**
 * @brief I2C interface initialize function
 * setup I2C interface
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID i2c_init(T_U32 pin_scl, T_U32 pin_sda)
{
    I2C_SCL = pin_scl;
    I2C_SDA = pin_sda;
	
	DrvModule_Protect(DRV_MODULE_SDMMC); 
	
    gpio_set_pin_as_gpio(pin_scl);
    gpio_set_pin_as_gpio(pin_sda);
    
    init_i2c_pin(I2C_SCL, I2C_SDA);
	
    DrvModule_UnProtect(DRV_MODULE_SDMMC); 
	
    i2c_delay_unit = I2C_DEFAULT_DELAY_UNIT;
}

T_VOID i2c_release(T_U32 pin_scl, T_U32 pin_sda)
{
    DrvModule_Protect(DRV_MODULE_SDMMC); 
    
    release_i2c_pin(pin_scl, pin_sda);

    DrvModule_UnProtect(DRV_MODULE_SDMMC); 
    
    i2c_delay_unit = I2C_DEFAULT_DELAY_UNIT;
}

/**
 * @brief delay function
 * delay the time
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param T_U32 time: delay time
 * @return T_VOID
 * @retval
 */
static T_VOID i2c_delay(T_U32 time)
{
    while(time--) 
    {   
        ;
    }
}

#if 0 //replace by macro definition
static T_VOID init_i2c_pin(T_U32 pin_scl, T_U32 pin_sda)
{
    REG32(GPIO_DIR_REG1) |= pin_scl;
    REG32(GPIO_DIR_REG1) |= pin_sda;
    
    REG32(GPIO_OUT_REG1) |= pin_scl;
    REG32(GPIO_OUT_REG1) |= pin_sda;

}

/**
 * @brief set  I2C input function
 * set I2C input: 1
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param T_U32 pin: pin number
 * @return T_VOID
 * @retval
 */
static T_VOID set_i2c_pin(T_U32 pin)
{
    REG32(GPIO_OUT_REG1) |= pin;
}

/**
 * @brief clear I2C input function
 * set I2C input: 0
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param T_U32 pin: pin number
 * @return T_VOID
 * @retval
 */
static T_VOID clr_i2c_pin(T_U32 pin)
{
    REG32(GPIO_OUT_REG1) &= (~pin);
}

/**
 * @brief get I2C output function
 * get I2C output data
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param T_U32 pin: pin number
 * @return T_U8: get I2C output data
 * @retval
 */
static T_U8 get_i2c_pin(T_U32 pin)
{
    T_U8 ret;
    T_U32 value;

    value = REG32(GPIO_IN_REG1);
    if ((value & pin) == 0)
        ret = 0;
    else
        ret = 1;

    return ret;
}

/**
 * @brief set I2C input function
 * set I2C input: 0
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param T_U32 pin: pin number
 * @return T_VOID
 * @retval
 */
static T_VOID set_i2c_data_in()
{
    REG32(GPIO_DIR_REG1) &= (~I2C_SDA);
}

/**
 * @brief set I2C output function
 * set I2C input: 0
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param T_U32 pin: pin number
 * @return T_VOID
 * @retval
 */
static T_VOID set_i2c_data_out()
{
    REG32(GPIO_DIR_REG1) |= I2C_SDA;
}
#endif//if 0

/**
 * @brief I2C interface start function
 * start I2C transmit
 * @author Junhua Zhao
 * @date 2004-04-05
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
static T_VOID i2c_begin(T_VOID)
{
	DrvModule_Protect(DRV_MODULE_SDMMC); 

    gpio_set_pin_as_gpio(I2C_SCL);
    gpio_set_pin_as_gpio(I2C_SDA);
    i2c_delay(i2c_delay_unit << 2);
    set_i2c_pin(I2C_SDA);   
    i2c_delay(i2c_delay_unit << 2);
    set_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 3);
    clr_i2c_pin(I2C_SDA);   
    i2c_delay(i2c_delay_unit << 3);
    clr_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 4);
}

/**
 * @brief I2C interface stop function
 * stop I2C transmit
 * @author Junhua Zhao
 * @date 2004-05-04
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
static T_VOID i2c_end(T_VOID)
{
    i2c_delay(i2c_delay_unit << 2);
    clr_i2c_pin(I2C_SDA);
    i2c_delay(i2c_delay_unit << 2);
    set_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 3);
    set_i2c_pin(I2C_SDA);
    i2c_delay(i2c_delay_unit << 4);
	
	DrvModule_UnProtect(DRV_MODULE_SDMMC); 
}

/**
 * @brief I2C interface send asknowlege function
 * send a asknowlege to I2C bus
 * @author Junhua Zhao
 * @date 2005-04-05
 * @param T_U8
 *   0:send bit 0
 *   not 0:send bit 1
 * @return T_VOID
 * @retval
 */
static T_VOID i2c_write_ask(T_U8 flag)
{
    if(flag)
        set_i2c_pin(I2C_SDA);
    else
        clr_i2c_pin(I2C_SDA);
    i2c_delay(i2c_delay_unit << 2);
    set_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 3);
    clr_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 2);
    set_i2c_pin(I2C_SDA);
    i2c_delay(i2c_delay_unit << 2);
}

/**
 * @brief I2C receive anknowlege
 * receive anknowlege from i2c bus
 * @author Junhua Zhao
 * @date 2005-04-05
 * @param T_VOID
 * @return T_BOOL: return received anknowlege bit
 * @retval AK_FALSE: 0
 * @retval AK_TRUE: 1
 */
static T_BOOL i2c_read_ack(T_VOID)
{   
    T_BOOL ret;

    set_i2c_data_in(I2C_SDA);
    
    i2c_delay(i2c_delay_unit << 3);
    set_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 2);
    if (!get_i2c_pin(I2C_SDA))
    {
        ret = AK_TRUE;
    }
    else
    {
        ret = AK_FALSE;
    }

    i2c_delay(i2c_delay_unit << 2);
    clr_i2c_pin(I2C_SCL);
    i2c_delay(i2c_delay_unit << 2);

    set_i2c_data_out(I2C_SDA);
    
    i2c_delay(i2c_delay_unit << 2); 
    
    return ret;
}

T_BOOL i2c_write_data(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size)
{
    T_U8 addr[2];
    
    addr[0] = daddr;
    addr[1] = raddr;
    
    return i2c_write_bytes(addr, 2, data, size);
}

T_BOOL i2c_write_data2(T_U8 daddr, T_U16 raddr, T_U8 *data, T_U32 size)
{
    T_U8 addr[3];

    addr[0] = daddr;
    addr[1] = (T_U8)(raddr >> 8);   //hight 8bit
    addr[2] = (T_U8)(raddr);	      //low 8bit

    return i2c_write_bytes(addr, 3, data, size);
}

T_BOOL i2c_write_data3(T_U8 daddr, T_U16 raddr, T_U16 *data, T_U32 size)
{
    T_U32 i;
    T_U8 high_8bit,low_8bit;

    high_8bit = (T_U8)(raddr >> 8);   //hight 8bit
    low_8bit = (T_U8)(raddr);            //low 8bit

    i2c_begin();
    if (!i2c_write_byte(daddr))
    {
        i2c_end();
        return AK_FALSE;
    }
    if (!i2c_write_byte(high_8bit))
    {
        i2c_end();
        return AK_FALSE;
    }
    if (!i2c_write_byte(low_8bit))
    {
        i2c_end();
        return AK_FALSE;
    }

    for(i=0; i<size; i++)
    {
        low_8bit = (T_U8)(*data);
        high_8bit = (T_U8)((*data) >> 8);

        if (!i2c_write_byte(high_8bit))
        {
            i2c_end();
            return AK_FALSE;
        }
        if (!i2c_write_byte(low_8bit ))
        {
            i2c_end();
            return AK_FALSE;
        }		
        data++;
    }
    i2c_end();

    return AK_TRUE;
}

T_BOOL i2c_write_data4(T_U8 daddr, T_U8 *data, T_U32 size)
{ 
    return i2c_write_bytes(&daddr, 1, data, size);
}

T_BOOL i2c_read_data(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size)
{
    T_U8 addr[2];
    
    addr[0] = daddr;
    addr[1] = raddr;
    
    return i2c_read_bytes(addr, 2, data, size);
}

T_BOOL i2c_read_data2(T_U8 daddr, T_U16 raddr, T_U8 *data, T_U32 size)
{
    T_U8 addr[3];

    addr[0] = daddr;
    addr[1] = (T_U8)(raddr >> 8);   //hight 8bit
    addr[2] = (T_U8)(raddr);	      //low 8bit

    return i2c_read_bytes(addr, 3, data, size);
}

T_BOOL i2c_read_data3(T_U8 daddr, T_U16 raddr, T_U16 *data, T_U32 size)
{
    T_U32 i;
    T_U8 high_8bit,low_8bit;

    high_8bit = (T_U8)(raddr >> 8);   //hight 8bit
    low_8bit = (T_U8)(raddr);            //low 8bit

    i2c_begin();
    if (!i2c_write_byte(daddr))
    {
        i2c_end();
        return AK_FALSE;
    }
    if (!i2c_write_byte(high_8bit))
    {
        i2c_end();
        return AK_FALSE;
    }
    if (!i2c_write_byte(low_8bit))
    {
        i2c_end();
        return AK_FALSE;
    }

    i2c_begin();

    if (!i2c_write_byte((T_U8)(daddr | 1)))
    {
        i2c_end();
        return AK_FALSE;
    }
    for(i=0; i<size; i++)
    {
        high_8bit = i2c_read_byte();
        i2c_write_ask(0);		
        low_8bit = i2c_read_byte();		
        (i<size-1)?i2c_write_ask(0):i2c_write_ask(1);

        *data = (T_U16)(high_8bit) << 8 | low_8bit;
        data++;
    }

    i2c_end();

    return AK_TRUE;
}

T_BOOL i2c_read_data4(T_U8 daddr, T_U8 *data, T_U32 size)
{  
    return i2c_read_bytes(&daddr, 1, data, size);
}

#endif

/* end of file */
