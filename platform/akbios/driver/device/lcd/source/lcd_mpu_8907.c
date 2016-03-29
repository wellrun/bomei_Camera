/**
 * @FILENAME: lcd_t8907a.c
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
#include "drv_lcd.h"
#include "lcd_mpu_t8907a.h"


#ifdef USE_LCD_MPU_8907A

#define LCD_T8907A_ID 0x1205


static T_VOID vtimer_delayns(T_S32 delay)
{
    delay = delay << 4;
    while(delay)
    {
        delay--;
    }
}

static T_U32 convert_to_8907(T_U32 u18)
{
	T_U32 ret;

	ret = u18;
	ret <<= 2;
	ret &= 0x3FC00;
	u18 <<= 1;
	u18 &= 0x1FE;
	ret |= u18;

	return ret;
}

static T_VOID index_out(T_eLCD lcd, T_U16 reg_index)
{
	lcd_MPU_ctl(lcd, LCD_MPU_CMD, convert_to_8907(reg_index));
}

static T_VOID data_out(T_eLCD lcd, T_U16 reg_data)
{
	lcd_MPU_ctl(lcd, LCD_MPU_DATA, convert_to_8907(reg_data));
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


static T_U32 lcd_t8907a_read_id(T_eLCD lcd)
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
	return LCD_T8907A_ID;
}

static T_VOID lcd_t8907a_init(T_eLCD lcd)
{
	//--select MPU interface--
	send_cmd(lcd, (const T_U16 *)init_cmdset);
}

static T_VOID lcd_t8907a_turn_on(T_eLCD lcd)
{
	send_cmd(lcd, (const T_U16 *)turnon_cmdset);

}
static T_VOID lcd_t8907a_turn_off(T_eLCD lcd)
{
	send_cmd(lcd, (const T_U16 *)turnoff_cmdset);

}
static T_VOID lcd_t8907a_set_disp_address(T_eLCD lcd, T_U32 x1, T_U32 y1, T_U32 x2, T_U32 y2)
{
	T_eLCD_DEGREE current_lcd_degree;
	T_U32 HardwareX1, HardwareX2, HardwareY1, HardwareY2;
	T_U32 lcd_hardware_w, lcd_hardware_h;

//	lcd_set_mode(IF_SEL, DISPLY_COLOR_SEL, BUS_SEL, W_LEN1, W_LEN2);


	//printf("size:%d,%d,%d,%d\n",x1,x2,y1,y2);

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
			akprintf(C2, M_DRVSYS, "lcd_t8907a_set_disp_address error degree\r\n");
			break;
	}

#if 0
    printf("lcd_hardware_x1:%d\n",HardwareX1);
    printf("lcd_hardware_x2:%d\n",HardwareX2);	
    printf("lcd_hardware_y1:%d\n",HardwareY1);
    printf("lcd_hardware_y2:%d\n",HardwareY2);	
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

static T_VOID lcd_t8907a_start(T_eLCD lcd)
{
	index_out(lcd, 0x0022);
}


static T_BOOL lcd_t8907a_rotate(T_eLCD lcd, T_eLCD_DEGREE degree)
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

static T_LCD_FUNCTION_HANDLER t8907a_function_handler =
{
	240,			///< pannel size width
	320,			///< pannel size height
    9000000,   		///< Clock cycle, Unit Hz
    18,       		///< data bus width(8,9,16,18 or 24)  
    0x01,			///< interface type(0x01 MPU, 0x02 RGB, 0x03 TV-out)
    0x01,			///< 0x00 4K color, 0x01 64K/256K color
	0x0,			///< no use
	AK_NULL,	///< refer to T_RGBLCD_INFO define
	
	lcd_t8907a_read_id, 		///< only for MPU interface 
	lcd_t8907a_init,		///< init for lcd driver 
	lcd_t8907a_turn_on,		
	lcd_t8907a_turn_off,			
	lcd_t8907a_set_disp_address,
	lcd_t8907a_rotate,
	lcd_t8907a_start	
};


static int lcd_t8907a_reg(void)
{
	lcd_reg_dev(LCD_T8907A_ID, &t8907a_function_handler, AK_TRUE);
	return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(lcd_t8907a_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif





