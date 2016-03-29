/**
 * @FILENAME: lcd_st7781.c
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

#ifdef USE_LCD_MPU_ST7781

#define LCD_ST7781_ID 0x7783


#define DELAY_FLAG        0xFFFE   // first parameter is 0xfffe, then 2nd parameter is delay time count
#define END_FLAG          0xFFFF   // first parameter is 0xffff, then parameter table is over 


static const T_U16 init_cmdset[][2] = 
{ 
	//************* Start Initial Sequence **********//
	{0x0001, 0x0000},	
	{0x0002, 0x0700},	
	{0x0003, 0x1030},	
	{0x0008, 0x0302},		// set the back porch and front porch
	{0x0009, 0x0000},		// set non-display area refresh cycle ISC[3:0]
	{0x000A, 0x0008},		// FMARK function
	
	{DELAY_FLAG, 100},
  
  {0x0010, 0x0000},				  // SAP, BT[3:0], AP, DSTB, SLP, STB
	{0x0011, 0x0005},				  // DC1[2:0], DC0[2:0], VC[2:0]
	{0x0012, 0x0000},				  // VREG1OUT voltage
	{0x0013, 0x0000},				  // VDV[4:0] for VCOM amplitude
	
	{DELAY_FLAG, 200},

  {0x0010, 0x12b0},				  // SAP, BT[3:0], AP, DSTB, SLP, STB
	{0x0011, 0x0005},				  // DC1[2:0], DC0[2:0], VC[2:0]
	{0x0012, 0x008c},   			  // VREG1OUT voltage
	{0x0013, 0x1300},          //0x1300), //0x1800),				// VDV[4:0] for VCOM amplitude
	{0x0029, 0x0010},          //0x0004), //0x0005),				// VCM[4:0] for VCOMH

        //++++++++++++++++++++++++davis add,屏上多出一条线
	{0x0075, 0x0000},		     // Frame Rate and Color Control-----16M_EN, Dither, FR_SEL[1:0] 

	{DELAY_FLAG, 200},
	// ---------- Gamma Control  ---------- //
	{0x0030, 0x0000},
	{0x0031, 0x0106},
	{0x0032, 0x0101},
	{0x0035, 0x0106},
	{0x0036, 0x0203},
	{0x0037, 0x0000},
	{0x0038, 0x0707},
	{0x0039, 0x0204},
	{0x003C, 0x0106},
	{0x003D, 0x0103},
	
	// ---------- Window Address Area  ---------- //
       {DELAY_FLAG, 200},
	{0x0050, 0x0000},		// Horizontal GRAM Start Address-----HSA[7:0]
	{0x0051, 0x00ef},		// Horizontal GRAM End Address-----HEA[7:0]
	{0x0052, 0x0000},		// Vertical GRAM Start Address-----VSA[8:0]
	{0x0053, 0x013f},		// Vertical GRAM Start Address-----VEA[8:0]
	

	// ---------- Gate Scan Control  ---------- //
	{0x0060, 0x2700},		// GS, NL[5:0], SCN[5:0]
	{0x0061, 0x0001},		// NDL,VLE, REV
	{0x0090, 0x0030},		// VL[8:0]
	{DELAY_FLAG, 200},
	{0x0007, 0x0133},		// Display Control 1-----262K color and display ON

  {END_FLAG, END_FLAG}
};

static const T_U16 turnon_cmdset[][2] = 
{
	{0x0011, 0x0005}, 
	{DELAY_FLAG, 40}, 
	{0x0010, 0x12B0}, 
	{DELAY_FLAG, 50}, 
	{0x0011, 0x0007},
	{DELAY_FLAG, 50}, 
	{0x0012, 0x008C},
	{DELAY_FLAG, 50},
	{0x0013, 0x1700},
	{DELAY_FLAG, 50},
	{0x0007, 0x0133}, 
	
	{END_FLAG, END_FLAG}
};

static const T_U16 turnoff_cmdset[][2] = 
{
	{0x0007, 0x0131}, 
	{DELAY_FLAG, 30}, 
	{0x0007, 0x0020}, 
	{DELAY_FLAG, 60}, 
	{0x0010, 0x0082},
	{DELAY_FLAG, 40}, 

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

static T_VOID lcd_out(T_eLCD lcd, T_U16 reg_index, T_U16 reg_data)
{
	index_out(lcd, reg_index);
	vtimer_delayns(1);
	data_out(lcd, reg_data);	
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
			lcd_out(lcd, pCmdSet[i * 2], pCmdSet[i * 2 + 1]);
		}
	}
}


static T_U32 lcd_st7781_read_id(T_eLCD lcd)
{
	T_U32 tmp = 0;
	T_U16 reg_index = 0x00;

	//--select MPU interface--
//	lcd_set_mode(IF_SEL, DISPLY_COLOR_SEL, BUS_SEL, W_LEN1, W_LEN2);
		
	//send id_reg index
	index_out(lcd, reg_index);

	//readback  
	tmp = lcd_readback(lcd);
	
	//return tmp&0xffff;//ID is 0x1205
	return LCD_ST7781_ID;
}

static T_VOID lcd_st7781_init(T_eLCD lcd)
{
    //reset LCD
/*
    gpio_set_pin_level(GPIO_LCD_RST, 1);
    us_delay(1000);
    gpio_set_pin_level(GPIO_LCD_RST, 0);
    us_delay(10000);
    gpio_set_pin_level(GPIO_LCD_RST, 1);
    us_delay(50000);
*/    
	//--select MPU interface--
	send_cmd(lcd, (const T_U16 *)init_cmdset);
}

static T_VOID lcd_st7781_turn_on(T_eLCD lcd)
{
	send_cmd(lcd, (const T_U16 *)turnon_cmdset);

}
static T_VOID lcd_st7781_turn_off(T_eLCD lcd)
{
	send_cmd(lcd, (const T_U16 *)turnoff_cmdset);

}
static T_VOID lcd_st7781_set_disp_address(T_eLCD lcd, T_U32 x1, T_U32 y1, T_U32 x2, T_U32 y2)
{
	T_eLCD_DEGREE current_lcd_degree;
	T_U32 HardwareX1, HardwareX2, HardwareY1, HardwareY2;
	T_U32 lcd_hardware_w, lcd_hardware_h;

//	lcd_set_mode(IF_SEL, DISPLY_COLOR_SEL, BUS_SEL, W_LEN1, W_LEN2);


	//akprintf(C3, M_DRVSYS,"size:%d,%d,%d,%d\n",x1,x2,y1,y2);

	lcd_hardware_w = lcd_get_hardware_width(lcd);
	lcd_hardware_h = lcd_get_hardware_height(lcd);


	// translate the rectangle x1, x2, y1, y2 
	// to the lcd x1, x2, y1, y2 in hardware 
	// by the degree between buffer and lcd   
	current_lcd_degree = lcd_degree(lcd);
	switch (current_lcd_degree)
	{
		case LCD_0_DEGREE:
			HardwareX1 = x1;
			HardwareX2 = x2;

			HardwareY1 = y1;
			HardwareY2 = y2;
			lcd_out(lcd, 0x0020, HardwareX1);                 //start x
			lcd_out(lcd, 0x0021, HardwareY1);    
			break;
		case LCD_90_DEGREE:
			HardwareX1 = lcd_hardware_w - y2 - 1;
			HardwareX2 = lcd_hardware_w - y1 - 1;  

			HardwareY1 = x1;
			HardwareY2 = x2;
			lcd_out(lcd, 0x0020, HardwareX1);                 //start x
			lcd_out(lcd, 0x0021, HardwareY2);    

			break;
		case LCD_180_DEGREE:
			HardwareX1 = x1;//lcd_hardware_w - x2 - 1;
			HardwareX2 = x2;//lcd_hardware_w - x1 - 1;

			HardwareY1 = lcd_hardware_h - y2 - 1;
			HardwareY2 = lcd_hardware_h - y1 - 1;
			lcd_out(lcd, 0x0020, HardwareX2);                 //start x
			lcd_out(lcd, 0x0021, HardwareY2);     
			break;
		case LCD_270_DEGREE:
			HardwareX1 = y1;
			HardwareX2 = y2;

			HardwareY1 = lcd_hardware_h - x2 - 1;
			HardwareY2 = lcd_hardware_h - x1 - 1;
			lcd_out(lcd, 0x0020, HardwareX1);                 //start x
			lcd_out(lcd, 0x0021, HardwareY2);      
			break;
		default:
			akprintf(C2, M_DRVSYS, "lcd_st7781_set_disp_address error degree\r\n");
			break;
	}

#if 0
	akprintf(C3, M_DRVSYS,"lcd_hardware_x1:%d\n",HardwareX1);
	akprintf(C3, M_DRVSYS,"lcd_hardware_x2:%d\n",HardwareX2);	
	akprintf(C3, M_DRVSYS,"lcd_hardware_y1:%d\n",HardwareY1);
	akprintf(C3, M_DRVSYS,"lcd_hardware_y2:%d\n",HardwareY2);	
#endif

	//set window address
	lcd_out(lcd, 0x0050, HardwareX1);                         //start of x
	lcd_out(lcd, 0x0051, HardwareX2);                         //end of x	

	lcd_out(lcd, 0x0052, HardwareY1);                         //start of y
	lcd_out(lcd, 0x0053, HardwareY2);                         //end of y

//	lcd_out(lcd, 0x0050,0x0000);    
//	lcd_out(lcd, 0x0051,lcd_hardware_w - 1);	   
//	lcd_out(lcd, 0x0052,0x0000);    
//	lcd_out(lcd, 0x0053,lcd_hardware_h - 1);	 

	
}

static T_VOID lcd_st7781_start(T_eLCD lcd)
{
	index_out(lcd, 0x0022);
}


static T_BOOL lcd_st7781_rotate(T_eLCD lcd, T_eLCD_DEGREE degree)
{
//	lcd_set_mode(IF_SEL, DISPLY_COLOR_SEL, BUS_SEL, W_LEN1, W_LEN2);
	
	switch (degree)
	{
		case LCD_0_DEGREE:
			lcd_out(lcd, 0x0003, 0x1030);
			break;

		case LCD_90_DEGREE:
			lcd_out(lcd, 0x0003, 0x1028);
			break;

		case LCD_180_DEGREE:
			lcd_out(lcd, 0x0003, 0x1000);
			break;

		case LCD_270_DEGREE:
			lcd_out(lcd, 0x0003, 0x1018);
			break;
		
		default:
			akprintf(C2, M_DRVSYS, "wrong degree %d\n", degree);
			return AK_FALSE;
	}

	return AK_TRUE; 

}

static T_LCD_FUNCTION_HANDLER st7781_function_handler =
{
	240,			///< pannel size width
	320,			///< pannel size height
  9000000,   		///< Clock cycle, Unit Hz
  16,       		///< data bus width(8,9,16,18 or 24)  
  0x01,			///< interface type(0x01 MPU, 0x02 RGB, 0x03 TV-out)
  0x01,			///< 0x00 4K color, 0x01 64K/256K color
	0x0,			///< no use
	AK_NULL,	///< refer to T_RGBLCD_INFO define
	
	lcd_st7781_read_id, 		///< only for MPU interface 
	lcd_st7781_init,		///< init for lcd driver 
	lcd_st7781_turn_on,		
	lcd_st7781_turn_off,			
	lcd_st7781_set_disp_address,
	lcd_st7781_rotate,
	lcd_st7781_start	
};


static int lcd_st7781_reg(void)
{
	lcd_reg_dev(LCD_ST7781_ID, &st7781_function_handler, AK_TRUE);
	return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(lcd_st7781_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif





