/**
 * @FILENAME: lcd_mpu_ili9341.c
 * @BRIEF LCD driver file.
 * @DTATSHEET IC AM283P33-37AJ-TPA21-08, LCD NAME ILI9341_DS_V1.02_20101206
 * @Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR LuHeShan
 * @DATE 2012-12-20
 * @VERSION 1.0
 * @REF
 * @Note:    MODE:       NAME(AF240S21-37A V1.0)
 *            LCD Module : 240X320 RGB  2.4inch
 *            Screen Size: 
 *            Interface: 8 bits
 */ 
#include "anyka_types.h"
#include "platform_hd_config.h"
#include "arch_init.h"
#include "drv_lcd.h"
#include "Gpio_config.h"

#ifdef USE_LCD_MPU_ILI9341V

#define LCD_ILI9341V_ID 0x9341

#define DATA                    0xFFFC
#define CMD                     0xFFFD

#define DELAY_FLAG        0xFFFE   // first parameter is 0xfffe, then 2nd parameter is delay time count
#define END_FLAG          0xFFFF   // first parameter is 0xffff, then parameter table is over 


static const T_U16 init_cmdset[][2] = 
{   
#if 1
    {CMD, 0x11},
    {DELAY_FLAG, 150},

    {CMD, 0xF6},
    {DATA, 0x01},
    {DATA, 0x33},
    
    {CMD, 0xB5},
    {DATA, 0x04},
    {DATA, 0x04},
    {DATA, 0X0A},
    {DATA, 0X14},

    {CMD, 0xCF},
    {DATA, 0x00},
    {DATA, 0xEA},
    {DATA, 0XF0},
    
    {CMD, 0xED},
    {DATA, 0x64},
    {DATA, 0x03},
    {DATA, 0X12},
    {DATA, 0X81},
    {CMD, 0xE8},
    {DATA, 0x85},
    {DATA, 0x00},
    {DATA, 0x78},
    {CMD, 0xCB},
    {DATA, 0x39},
    {DATA, 0x2C},
    {DATA, 0x00},
    {DATA, 0x33},
    {DATA, 0x06},
    {CMD, 0xF7},
    {DATA, 0x20},
    {CMD, 0xEA},
    {DATA, 0x00},
    {DATA, 0x00},
    {CMD, 0xC0},
    {DATA, 0x30},//32
    {CMD, 0xC1},
    {DATA, 0x12},//32
    {CMD, 0xC5},
    {DATA, 0x29},//26//16
    {DATA, 0x16},//12//14
    
    {CMD, 0xC7},
    {DATA, 0xd0},//0xc0
    
    {CMD, 0x36},
    {DATA, 0x08},//04
    
    {CMD, 0xB1},
    {DATA, 0x00},
    {DATA, 0x16},
    
    {CMD, 0xB6},
    {DATA, 0x0A},
    {DATA, 0x82},
    {CMD, 0xF2},
    {DATA, 0x00},
    {CMD, 0x26},
    {DATA, 0x01},
    {CMD, 0x3a},
    {DATA, 0x55},
    {CMD, 0x2a},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0xef},
    {CMD, 0x2b},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x01},
    {DATA, 0x3f},
    
    {CMD, 0xE0},//SetGamma 
    {DATA, 0x0e},
    {DATA, 0x23},
    {DATA, 0x1F},
    {DATA, 0x0B},
    {DATA, 0x0E},
    {DATA, 0x08},
    {DATA, 0x4B},
    {DATA, 0x64},//a8
    {DATA, 0x3B},
    {DATA, 0x0A},
    {DATA, 0x14},
    {DATA, 0x06},
    {DATA, 0x10},
    {DATA, 0x09},
    {DATA, 0x00},
    {CMD, 0XE1},//SetGamma
    
    {DATA, 0x01},
    {DATA, 0x1c},//1c
    {DATA, 0x20},
    {DATA, 0x04},
    {DATA, 0x11},
    {DATA, 0x07},
    {DATA, 0x34},
    {DATA, 0x9b},//8
    {DATA, 0x44},//44
    {DATA, 0x05},
    {DATA, 0x0B},
    {DATA, 0x09},
    {DATA, 0x2F},
    {DATA, 0x36},//36
    {DATA, 0x0F},
    {CMD, 0x29},
    {CMD, 0x2c},
#else
    //************* Start Initial Sequence **********// 
    {CMD, 0x3A},  
    {DATA, 0x55}, 
     
    {CMD, 0xF6},  
    {DATA, 0x01}, 
    {DATA, 0x33}, 
     
    {CMD, 0xB5},  
    {DATA, 0x04}, 
    {DATA, 0x04}, 
    {DATA, 0x0A}, 
    {DATA, 0x14}, 
     
    {CMD, 0x35},  
    {DATA, 0x00}, 
     
    {CMD, 0xCF},  
    {DATA, 0x00}, 
    {DATA, 0xEA}, 
    {DATA, 0XF0}, 
     
    {CMD, 0xED},  
    {DATA, 0x64}, 
    {DATA, 0x03}, 
    {DATA, 0X12}, 
    {DATA, 0X81}, 
    
    {CMD, 0xE8},  
    {DATA, 0x85}, 
    {DATA, 0x00}, 
    {DATA, 0x78}, 
     
    {CMD, 0xCB},  
    {DATA, 0x39}, 
    {DATA, 0x2C}, 
    {DATA, 0x00}, 
    {DATA, 0x33}, 
    {DATA, 0x06}, 
     
    {CMD, 0xF7},  
    {DATA, 0x20}, 
     
    {CMD, 0xEA},  
    {DATA, 0x00}, 
    {DATA, 0x00}, 
     
    {CMD, 0xC0},    //Power control 
    {DATA, 0x21},   //VRH[5:0] 
     
    {CMD, 0xC1},    //Power control 
    {DATA, 0x10},   //BT[3:0] 
     
    {CMD, 0xC5},    //VCM control 
    {DATA, 0x4F}, 
    {DATA, 0x38}, 
     
    {CMD, 0x36},    // Memory Access Control 
    {DATA, 0x08}, 
     
    {CMD, 0xB1},   
    {DATA, 0x00},   
    {DATA, 0x13}, 
     
    {CMD, 0xB6},    // Display Function Control 
    {DATA, 0x0A}, 
    {DATA, 0xA2}, 
     
    {CMD, 0xF2},    // 3Gamma Function Disable 
    {DATA, 0x02}, 
     
    {CMD, 0xE0},    //Set Gamma 
    {DATA, 0x0F}, 
    {DATA, 0x27}, 
    {DATA, 0x24}, 
    {DATA, 0x0C}, 
    {DATA, 0x10}, 
    {DATA, 0x08}, 
    {DATA, 0x55}, 
    {DATA, 0X87}, 
    {DATA, 0x45}, 
    {DATA, 0x08}, 
    {DATA, 0x14}, 
    {DATA, 0x07}, 
    {DATA, 0x13}, 
    {DATA, 0x08}, 
    {DATA, 0x00}, 
     
    {CMD, 0xE1},    //Set Gamma 
    {DATA, 0x00}, 
    {DATA, 0x0F}, 
    {DATA, 0x12}, 
    {DATA, 0x05}, 
    {DATA, 0x11}, 
    {DATA, 0x06}, 
    {DATA, 0x25}, 
    {DATA, 0x34}, 
    {DATA, 0x37}, 
    {DATA, 0x01}, 
    {DATA, 0x08}, 
    {DATA, 0x07}, 
    {DATA, 0x2B}, 
    {DATA, 0x34}, 
    {DATA, 0x0F}, 
    {CMD, 0x11},    //Exit Sleep 
    {DELAY_FLAG, 120}, 
    {CMD, 0x29},    //Display on 
#endif
    {END_FLAG, END_FLAG}
};

static const T_U16 turnon_cmdset[][2] = 
{        
	{CMD, 0x0011}, //Exit Sleep
	{DELAY_FLAG, 10},
	{CMD, 0x0011},//Exit Sleep
	{DELAY_FLAG, 120},
	{CMD, 0x0029}, //display on
	{END_FLAG, END_FLAG}
};

static const T_U16 turnoff_cmdset[][2] = 
{
	{CMD, 0x0028}, //display off
	{DELAY_FLAG, 50},
	{CMD, 0x0010}, //enter Sleep
    {END_FLAG, END_FLAG}
};

/*
static T_VOID vtimer_delayns(T_S32 delay)
{
    delay = delay << 4;
    while(delay)
    {
        delay--;
    }
}
*/
static T_VOID index_out(T_eLCD lcd, T_U16 reg_index)
{
    lcd_MPU_ctl(lcd, LCD_MPU_CMD, reg_index);
    us_delay(10);
}

static T_VOID data_out(T_eLCD lcd, T_U16 reg_data)
{
    lcd_MPU_ctl(lcd, LCD_MPU_DATA, reg_data);
    us_delay(10);
}

static T_VOID send_cmd(T_eLCD lcd, const T_U16 *pCmdSet)
{
    int i = 0;

    for(i=0; AK_TRUE; i++)
    {
        if ((END_FLAG == pCmdSet[i * 2]) && (END_FLAG == pCmdSet[i * 2 + 1]))
        {
            break;
        }
        else if (DELAY_FLAG == pCmdSet[i * 2])
        {
            mini_delay(pCmdSet[i * 2 + 1]);
        }
        else
        {
			if(CMD == pCmdSet[i * 2])
            {
                index_out(lcd, pCmdSet[i * 2 + 1]);
            }
            else if(DATA == pCmdSet[i * 2])
            {
                data_out(lcd, pCmdSet[i * 2 + 1]);
            }
        }
    }
}

static T_U32 lcd_ili9341v_read_id(T_eLCD lcd)
{
    // return ili9343v id
    return LCD_ILI9341V_ID;
}

static T_VOID lcd_ili9341v_init(T_eLCD lcd)
{
    // initialize ili9341v lcd
   //reset LCD

    gpio_set_pin_level(GPIO_LCD_RST, 0);
    us_delay(2000);//低电平保持时间，最小1毫秒
    gpio_set_pin_level(GPIO_LCD_RST, 1);
    us_delay(2000); //复位等待时间，最小1毫秒
    
    send_cmd(lcd, (const T_U16 *)init_cmdset);
}

static T_VOID lcd_ili9341v_turn_on(T_eLCD lcd)
{
    // turn on ili9341v lcd
    send_cmd(lcd, (const T_U16 *)turnon_cmdset);
}
static T_VOID lcd_ili9341v_turn_off(T_eLCD lcd)
{
    // turn off ili9341v lcd
    send_cmd(lcd, (const T_U16 *)turnoff_cmdset);
}

static T_VOID lcd_ili9341v_set_disp_address(T_eLCD lcd,T_U32 x_start, T_U32 y_start, T_U32 x_end, T_U32 y_end)
{
	//set column address
	index_out(lcd,0x2A);
	data_out(lcd,(x_start >> 8)&0xFF);
	data_out(lcd,x_start&0xFF);
	data_out(lcd,(x_end >> 8)&0xFF);
	data_out(lcd,x_end&0xFF);
	
	//set row address
	index_out(lcd,0x2B);
	data_out(lcd,(y_start >> 8)&0xFF);
	data_out(lcd,y_start&0xFF);
	data_out(lcd,(y_end >> 8)&0xFF);
	data_out(lcd,y_end&0xFF);
}


static T_VOID lcd_ili9341v_start(T_eLCD lcd)
{
    // start ili9341v lcd
    index_out(lcd, 0x002C);
}

static T_BOOL lcd_ili9341v_rotate(T_eLCD lcd, T_eLCD_DEGREE degree)
{
    // rotate ili9341v lcd
	switch (degree)
    {
        case LCD_0_DEGREE:
			index_out(lcd, 0x36);
			data_out(lcd,  0x48);
            break;

        case LCD_90_DEGREE:
			index_out(lcd, 0x36);
			data_out(lcd,  0xE8);
            break;

        case LCD_180_DEGREE:
			index_out(lcd, 0x36);
			data_out(lcd,  0x88);
            break;

        case LCD_270_DEGREE:
			index_out(lcd, 0x36);
			data_out(lcd,  0x28);
            break;
        
        default:
            //akprintf(C2, M_DRVSYS, "wrong degree %d\n", degree);
            return AK_FALSE;
    }

    return AK_TRUE; 
}

static T_LCD_FUNCTION_HANDLER ili9341_function_handler =
{
    240,            ///< pannel size width
    320,            ///< pannel size height
    9000000,      ///< Clock cycle, Unit Hz
    8,             ///< data bus width(8,9,16,18 or 24)  
    0x01,           ///< interface type(0x01 MPU, 0x02 RGB, 0x03 TV-out)
    0x01,           ///< 0x00 4K color, 0x01 64K/256K color
    0x0,            ///< no use
    AK_NULL,        ///< refer to T_RGBLCD_INFO define
    
    lcd_ili9341v_read_id,         ///< only for MPU interface 
    lcd_ili9341v_init,        ///< init for lcd driver 
    lcd_ili9341v_turn_on,        // turn on
    lcd_ili9341v_turn_off,            // turn off
    lcd_ili9341v_set_disp_address,    // set display address
    lcd_ili9341v_rotate,             // rotate display
    lcd_ili9341v_start               // start lcd
};


static int lcd_ili9341v_reg(void)
{   // reg dev
    lcd_reg_dev(LCD_ILI9341V_ID, &ili9341_function_handler, AK_TRUE);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(lcd_ili9341v_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif


