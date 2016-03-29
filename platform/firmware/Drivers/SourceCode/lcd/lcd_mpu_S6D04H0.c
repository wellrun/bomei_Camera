/**
 * @FILENAME: lcd_S6D04H0.c
 * @BRIEF LCD driver file
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR LianGenhui
 * @DATE 2010-07-12
 * @VERSION 1.0
 * @REF
 * @Note:	MODE: 
 *			LCD Module : 400X272 RGB
 *			Screen Size: 
 *			Interface: 18 bits
 */ 
#include "anyka_types.h"
#include "drv_api.h"//"akdefine.h"
#include "platform_hd_config.h"
#include "drv_lcd.h"

#ifdef USE_LCD_MPU_S6D04H0

#define LCD_S6D04H0_ID 0x6D04


#define DATA                    0xFFFC
#define CMD                     0xFFFD

#define DELAY_FLAG        0xFFFE   // first parameter is 0xfffe, then 2nd parameter is delay time count
#define END_FLAG          0xFFFF   // first parameter is 0xffff, then parameter table is over 


static const T_U16 init_cmdset[][2] = {
    {CMD,  0x11},       //sleep out
    {DELAY_FLAG,260},       //wait at least 100 ms
    {CMD,  0x13},       // NORMAL DISPLAY MODE ON
    {DELAY_FLAG,260},
            
    {CMD,  0xf3},       //  MANPWRSEQ
    {DATA, 0x01},
    {DATA, 0xff},
    {DATA, 0x1f},
    {DATA, 0x00},
    {DATA, 0x03},
               
    {CMD,  0xf2},       //DISCTL
    {DATA, 0x28},
    {DATA, 0x64},
    {DATA, 0x7f},
    {DATA, 0x08},
    {DATA, 0x08},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x15},
    {DATA, 0x48},
    {DATA, 0x00},
    {DATA, 0x07},
    {DATA, 0x01},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x94},
    {DATA, 0x08},
    {DATA, 0x08},
               
    {CMD,  0xf4},       //PWRCTL
    {DATA, 0x0b},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x21},
    {DATA, 0x47},
    {DATA, 0x01},
    {DATA, 0x02},
    {DATA, 0x2A},
    {DATA, 0x5D},
    {DATA, 0x07},
    {DATA, 0x2a},
    {DATA, 0x00},
    {DATA, 0x07},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
               
    {CMD,  0xf5},       //VCMCTL
    {DATA, 0x00},
    {DATA, 0x42},
    {DATA, 0x3a},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x12},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x0d},
    {DATA, 0x01},
    {DATA, 0x00},
    {DATA, 0x00},
               
    {CMD,  0xf6},       //SRCCTL 
    {DATA, 0x01},
    {DATA, 0x01},
    {DATA, 0x07},
    {DATA, 0x00},
    {DATA, 0x02},
    {DATA, 0x0c},
    {DATA, 0x02},
    {DATA, 0x08},
    {DATA, 0x03},
                
    {CMD,  0xf7},    //  IFCTL (Interface Control)
    {DATA, 0x01},
    {DATA, 0x00},
    {DATA, 0x10},
    {DATA, 0x00},
               
    {CMD,  0xf8},   //PANELCTL (Panel Control)
    {DATA, 0x99},      
    {DATA, 0x00},
    {DATA, 0x00},
                
    {CMD,  0xf9},  //GAMMASEL(Gamma Selection) 
    {DATA, 0x01},
                
    {CMD,  0xfa},    //PGAMMACTL (Positive Gamma Control Register)
    {DATA, 0x00},
    {DATA, 0x17},
    {DATA, 0x0c},
    {DATA, 0x18},
    {DATA, 0x23},
    {DATA, 0x36},
    {DATA, 0x2b},       //
    {DATA, 0x2c},
    {DATA, 0x2a},
    {DATA, 0x28},
    {DATA, 0x25},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
                
    {CMD,  0xfb},    //NGAMMACTL (Negative Gamma Control Register)
    {DATA, 0x00},
    {DATA, 0x0a},
    {DATA, 0x26},
    {DATA, 0x23},
    {DATA, 0x23},
    {DATA, 0x26},
    {DATA, 0x1A},      
    {DATA, 0x3B},
    {DATA, 0x27},
    {DATA, 0x1E},
    {DATA, 0x0D},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
                
    {CMD,  0xF9},     //GAMMASEL(Gamma Selection) 
    {DATA, 0x02},
                
    {CMD,  0xfa},    //PGAMMACTL (Positive Gamma Control Register)
    {DATA, 0x00},
    {DATA, 0x17},
    {DATA, 0x0c},
    {DATA, 0x18},
    {DATA, 0x23},
    {DATA, 0x36},
    {DATA, 0x2b},      
    {DATA, 0x2c},
    {DATA, 0x2a},
    {DATA, 0x28},
    {DATA, 0x25},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
                
    {CMD,  0xfb},     //NGAMMACTL (Negative Gamma Control Register)
    {DATA, 0x00},
    {DATA, 0x0a},
    {DATA, 0x26},
    {DATA, 0x23},
    {DATA, 0x23},
    {DATA, 0x26},
    {DATA, 0x1A},       
    {DATA, 0x3B},
    {DATA, 0x27},
    {DATA, 0x1E},
    {DATA, 0x0D},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
                
    {CMD,  0xF9},    //GAMMASEL(Gamma Selection)
    {DATA, 0x04},
                
    {CMD,  0xfa},    //PGAMMACTL (Positive Gamma Control Register)
    {DATA, 0x00},
    {DATA, 0x17},
    {DATA, 0x0c},
    {DATA, 0x18},
    {DATA, 0x23},
    {DATA, 0x36},
    {DATA, 0x2b},       
    {DATA, 0x2c},
    {DATA, 0x2a},
    {DATA, 0x28},
    {DATA, 0x25},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
               
    {CMD,  0xfb},     ////NGAMMACTL (Negative Gamma Control Register)
    {DATA, 0x00},
    {DATA, 0x0a},
    {DATA, 0x26},
    {DATA, 0x23},
    {DATA, 0x23},
    {DATA, 0x26},
    {DATA, 0x1A},      
    {DATA, 0x3B},
    {DATA, 0x27},
    {DATA, 0x1E},
    {DATA, 0x0D},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
                
    {CMD,  0x26},   //GAMSET (Gamma Set) 
    {DATA, 0x01},
                
    {CMD,  0x35},   //TEON (Tearing Effect Line ON)     
    {DATA, 0x00},
                
    {CMD,  0x36},   //MADCTL (Memory Data Access Control)    
    {DATA, 0x48},
                
    {CMD,  0x3A},   //COLMOD (Interface Pixel Format) 
    {DATA, 0x55},
                
    {CMD,  0x2A},  //set windows
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0xEF},
    {CMD,  0x2B},
    {DATA, 0x00},
    {DATA, 0x00},
    {DATA, 0x01},
    {DATA, 0x3F},
               
    {CMD,  0x29},  // dispaly on
    {DELAY_FLAG,260},  
               
    //{CMD,  0x2C};  // write GRAM
    //{DELAY_FLAG,260};

    
    {END_FLAG, END_FLAG}
};


static T_VOID vtimer_delayns(T_S32 delay)
{
    delay = delay << 4;
    while(delay)
    {
        delay--;
    }
}


static T_VOID index_out(T_eLCD lcd, T_U16 reg_index)
{
	lcd_MPU_ctl(lcd, LCD_MPU_CMD, reg_index);
}

static T_VOID data_out(T_eLCD lcd, T_U16 reg_data)
{
	lcd_MPU_ctl(lcd, LCD_MPU_DATA, reg_data);
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
			if(CMD == init_cmdset[i][0])
            {
                index_out(lcd, (init_cmdset[i][1]));
            }
            else if(DATA == init_cmdset[i][0])
            {
                data_out(lcd, (init_cmdset[i][1]));
            }
		}
	}
}


static T_U32 lcd_S6D04H0_read_id(T_eLCD lcd)
{
	T_U32 tmp = 0;
	T_U16 reg_index = 0x00;

	return LCD_S6D04H0_ID;
}

static T_VOID lcd_S6D04H0_init(T_eLCD lcd)
{
    
    T_U32 i;

    //reset LCD
    //gpio_set_pin_level(GPIO_LCD_RST, 0);
    //vtimer_delayns(5000);
    //gpio_set_pin_level(GPIO_LCD_RST, 1);
    //vtimer_delayns(1000);
    
    send_cmd(lcd, (const T_U16 *)init_cmdset);
}

static T_VOID lcd_S6D04H0_turn_on(T_eLCD lcd)
{	
	index_out(lcd,0x29);
	mini_delay(20);
	index_out(lcd,0x11);
}
static T_VOID lcd_S6D04H0_turn_off(T_eLCD lcd)
{
	index_out(lcd,0x28);
	mini_delay(50);
	index_out(lcd,0x10);
}

static T_VOID lcd_S6D04H0_set_disp_address(T_eLCD lcd, T_U32 x1, T_U32 y1, T_U32 x2, T_U32 y2)
{
    T_U8 temp1_low,temp1_hight;
    T_U8 temp2_low,temp2_hight;

    temp1_hight = (x1>>8)&0xFF;
    temp1_low = x1&0xFF;
    temp2_hight = (x2>>8)&0xFF;
    temp2_low = x2&0xFF;
    
    //set row address
    index_out(lcd,0x2A);

    data_out(lcd,temp1_hight);
    data_out(lcd,temp1_low);
    data_out(lcd,temp2_hight);
    data_out(lcd,temp2_low);

    temp1_hight = (y1>>8)&0xFF;
    temp1_low = y1&0xFF;
    temp2_hight = (y2>>8)&0xFF;
    temp2_low = y2&0xFF;

    //set column address
    index_out(lcd,0x2B);

    data_out(lcd,temp1_hight);
    data_out(lcd,temp1_low);
    data_out(lcd,temp2_hight);
    data_out(lcd,temp2_low);

	
}

static T_VOID lcd_S6D04H0_start(T_eLCD lcd)
{
	index_out(lcd, 0x2C);
}


static T_BOOL lcd_S6D04H0_rotate(T_eLCD lcd, T_eLCD_DEGREE degree)
{
//	lcd_set_mode(IF_SEL, DISPLY_COLOR_SEL, BUS_SEL, W_LEN1, W_LEN2);
	
	switch (degree)
	{
		case LCD_0_DEGREE:
			break;

		case LCD_90_DEGREE:
		   index_out(lcd, 0x36);//MADCTL (Memory Data Access Control)   
		   data_out(lcd,  0x28);
			break;

		case LCD_180_DEGREE:
			break;

		case LCD_270_DEGREE:
			break;
		
		default:
			akprintf(C2, M_DRVSYS, "wrong degree %d\n", degree);
			return AK_FALSE;
	}

	return AK_TRUE; 

}

static T_LCD_FUNCTION_HANDLER S6D04H0_function_handler =
{
	240,			///< pannel size width
	320,			///< pannel size height
  24000000,   		///< Clock cycle, Unit Hz
  16,       		///< data bus width(8,9,16,18 or 24)  
  0x01,			///< interface type(0x01 MPU, 0x02 RGB, 0x03 TV-out)
  0x01,			///< 0x00 4K color, 0x01 64K/256K color
	0x0,			///< no use
	AK_NULL,	///< refer to T_RGBLCD_INFO define
	
	lcd_S6D04H0_read_id, 		///< only for MPU interface 
	lcd_S6D04H0_init,		///< init for lcd driver 
	lcd_S6D04H0_turn_on,		
	lcd_S6D04H0_turn_off,			
	lcd_S6D04H0_set_disp_address,
	lcd_S6D04H0_rotate,
	lcd_S6D04H0_start	
};


static int lcd_S6D04H0_reg(void)
{
	lcd_reg_dev(LCD_S6D04H0_ID, &S6D04H0_function_handler, AK_TRUE);
	return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(lcd_S6D04H0_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif





