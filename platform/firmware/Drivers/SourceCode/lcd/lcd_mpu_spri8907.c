/**
 * @FILENAME: lcd_mpu_spri8907.c
 * @BRIEF LCD driver file
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR LiuHuadong
 * @DATE 2010-12-22
 * @VERSION 1.0
 * @REF
 * @Note:    
 *            LCD Module : 240X320 RGB
 *            Screen Size: 
 *            Interface: 16 bits
 */ 
#include "anyka_types.h"
#include "platform_hd_config.h"
#include "drv_lcd.h"

#ifdef USE_LCD_MPU_SPRING_8907

#define LCD_T8907A_ID 0x1205



#define DELAY_FLAG        0xFFFE   // first parameter is 0xfffe, then 2nd parameter is delay time count
#define END_FLAG          0xFFFF   // first parameter is 0xffff, then parameter table is over 



static const T_U16 init_cmdset[][2] = 
{

    {0x00A4,0x0001},
    {DELAY_FLAG, 20},
    {0x00AB,0x0010},
    {0x0010,0x0000},
    {0x0011,0x0007},
    {0x0012,0x0000},
    {0x0013,0x0000},
    {DELAY_FLAG, 50},
    
    {0x0010,0x0430},
    {0x0011,0x0237},
    {DELAY_FLAG, 10},
    {0x0012,0x01BF},
    {DELAY_FLAG, 10},
    {0x0013,0x0600},//vcom adjust
    {0x002A,0x0080},
    {0x0029,0x0030},//30
    {DELAY_FLAG, 10},
    {0x0000,0x0001},
    {0x0001,0x0100},
    {0x0002,0x0700},
    {0x0003,0x1030},//D030
    {0x0004,0x0000},
    {0x0008,0x0808},
    {0x0009,0x0000},
    {0x000A,0x0008},
    {0x000C,0x0000},
    {0x000D,0x0000},
    {0x000E,0x0030},
    {0x000F,0x0000},
    {0x0030,0x1700},
    {0x0031,0x3306},
    {0x0032,0x0603},
    {0x0033,0x0017},
    {0x0034,0x0000},
    {0x0035,0x1700},
    {0x0036,0x3306},
    {0x0037,0x0603},
    {0x0038,0x0017},
    {0x0039,0x0000},
    {0x0050,0x0000},
    {0x0051,0x00EF},
    {0x0052,0x0000},
    {0x0053,0x013F},
    {0x0060,0x2700},
    {0x0061,0x0001},
    {0x006A,0x0000},
    {0x0090,0x0016},
    {0x0091,0x0006},
    {0x0092,0x0400},
    {0x0093,0x0404},
    {0x0093,0x0004},
    {0x0007,0x0001},
    {DELAY_FLAG, 10},
    {0x0007,0x0061},
    {0x0007,0x0173},
    {END_FLAG, END_FLAG}//over send flag



};

static const T_U16 turnoff_cmdset[][2] = 
{
    {0x0007, 0x0072}, {DELAY_FLAG, 10}, {0x0007, 0x0001}, 
    {DELAY_FLAG, 10}, {0x0007, 0x0000}, {DELAY_FLAG, 10}, 
    {DELAY_FLAG, 10}, {0x0012,0x018F},
    {END_FLAG, END_FLAG}
};


static const T_U16 turnon_cmdset[][2] = 
{
	{DELAY_FLAG, 10},
	{0x0007, 0x0021}, {DELAY_FLAG, 10},
    {0x0007, 0x0061}, {DELAY_FLAG, 10}, 
    {DELAY_FLAG, 10}, {0x0007, 0x0173},
    {DELAY_FLAG, 10}, {0x0012,0x01BF},
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


static T_U32 lcd_t8907a_read_id(T_eLCD lcd)
{
    T_U32 tmp = 0;
    T_U16 reg_index = 0x00;

    //--select MPU interface--
//    lcd_set_mode(IF_SEL, DISPLY_COLOR_SEL, BUS_SEL, W_LEN1, W_LEN2);
        
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

static T_VOID lcd_t8907a_set_disp_address(T_eLCD lcd,T_U32 x_start, T_U32 y_start, T_U32 x_end, T_U32 y_end)
{
//    T_U32 real_x_start, real_y_start, real_x_end, real_y_end;
    T_U32 HardwareX1, HardwareX2, HardwareY1, HardwareY2;
    T_U32 lcd_hardware_w, lcd_hardware_h;
    T_eLCD_DEGREE current_lcd_degree;
    current_lcd_degree = lcd_degree(lcd);
    lcd_hardware_w = lcd_get_hardware_width(lcd);
    lcd_hardware_h = lcd_get_hardware_height(lcd);    
    switch(current_lcd_degree)
    {
        case LCD_0_DEGREE:
            //akprintf(C3, M_DRVSYS,"0 degree!\n");
            //LCD_WriteCmd(0x20);
            //LCD_WriteData(x_start);//x start
            
            lcd_out(lcd, 0x0020, x_start);
            
           // LCD_WriteCmd(0x21); //y start
           // LCD_WriteData(y_start);      
            lcd_out(lcd, 0x0021, y_start); 
           
            HardwareX1 = x_start;
            HardwareX2 = x_end;
            HardwareY1 = y_start;
            HardwareY2 = y_end;
            
            break;
            
        case LCD_90_DEGREE:
            //akprintf(C3, M_DRVSYS,"90 degree!\n");
            //LCD_WriteCmd(0x20);
            //LCD_WriteData(y_start);
            lcd_out(lcd, 0x0020, y_start);
            lcd_out(lcd, 0x0021, lcd_hardware_h - x_start - 1);
            
           // LCD_WriteCmd(0x21);
           // LCD_WriteData(lcd_hardware_h - x_start - 1);
            HardwareX1 = y_start;
            HardwareX2 = y_end;
            HardwareY1 = lcd_hardware_h - x_start - 1;
            HardwareY2 = lcd_hardware_h - x_end - 1;
            
            break;
            
        case LCD_180_DEGREE:
            //akprintf(C3, M_DRVSYS,"180 degree\n");
            lcd_out(lcd, 0x0020, lcd_hardware_w - x_start -1);
            
            lcd_out(lcd, 0x0021, lcd_hardware_h - y_start -1);
            //LCD_WriteCmd(0x20);
            //LCD_WriteData(lcd_hardware_w - x_start -1);//x start
                                   
            //LCD_WriteCmd(0x21); //y start
            //LCD_WriteData(lcd_hardware_h - y_start -1);      
            
            HardwareX1 = lcd_hardware_w - x_start -1;
            HardwareX2 = lcd_hardware_w - x_end -1;
            HardwareY1 = lcd_hardware_h - y_start -1;
            HardwareY2 = lcd_hardware_h - y_end -1;                           
                              
            break;
        
            
        case LCD_270_DEGREE:
            //akprintf(C3, M_DRVSYS,"270 degree\n");
            
            lcd_out(lcd, 0x0020, lcd_hardware_w - y_start -1);
            //LCD_WriteCmd(0x20);
            //LCD_WriteData(lcd_hardware_w - y_start -1);//x start
             lcd_out(lcd, 0x0021, x_start);                      
          //  LCD_WriteCmd(0x21); //y start
           // LCD_WriteData(x_start);      
                     
            HardwareX1 = lcd_hardware_w - y_start -1;
            HardwareX2= lcd_hardware_w - y_end -1;
            HardwareY1= x_start;
            HardwareY2 = x_end;                                                   
                                                      
            break;
            
        default :
            akprintf(C3, M_DRVSYS,"The rotate is wrong !\n");
            while(1);
    }
    
    if(HardwareX2 > HardwareX1)                                           //set x range
    {   
        lcd_out(lcd, 0x0050, HardwareX1);                         //start of x
        lcd_out(lcd, 0x0051, HardwareX2);
    }
    else
    {
        lcd_out(lcd, 0x0050, HardwareX2);                         //start of x
        lcd_out(lcd, 0x0051, HardwareX1);    
    }
    
    if(HardwareY2 > HardwareY1)                                           //set y range
    {       
        lcd_out(lcd, 0x0052, HardwareY1);                         //start of x
        lcd_out(lcd, 0x0053, HardwareY2);
    }
    else
    {
        lcd_out(lcd, 0x0052, HardwareY2);                         //start of x
        lcd_out(lcd, 0x0053, HardwareY1);
    }
}


static T_VOID lcd_t8907a_start(T_eLCD lcd)
{
    index_out(lcd, 0x0022);
}


static T_BOOL lcd_t8907a_rotate(T_eLCD lcd, T_eLCD_DEGREE degree)
{
//    lcd_set_mode(IF_SEL, DISPLY_COLOR_SEL, BUS_SEL, W_LEN1, W_LEN2);
    
    switch (degree)
    {
        case LCD_0_DEGREE:
            lcd_out(lcd, 0x0003, 0x1030);
            break;

        case LCD_90_DEGREE:
            lcd_out(lcd, 0x0003, 0x1018);
            break;

        case LCD_180_DEGREE:
            lcd_out(lcd, 0x0003, 0x1000);
            break;

        case LCD_270_DEGREE:
            lcd_out(lcd, 0x0003, 0x1028);
            break;
        
        default:
            akprintf(C2, M_DRVSYS, "wrong degree %d\n", degree);
            return AK_FALSE;
    }

    return AK_TRUE; 

}

static T_LCD_FUNCTION_HANDLER t8907a_function_handler =
{
    240,            ///< pannel size width
    320,            ///< pannel size height
    10000000,      ///< Clock cycle, Unit Hz
    16,             ///< data bus width(8,9,16,18 or 24)  
    0x01,           ///< interface type(0x01 MPU, 0x02 RGB, 0x03 TV-out)
    0x01,           ///< 0x00 4K color, 0x01 64K/256K color
    0x0,            ///< no use
    AK_NULL,        ///< refer to T_RGBLCD_INFO define
    
    lcd_t8907a_read_id,         ///< only for MPU interface 
    lcd_t8907a_init,        ///< init for lcd driver 
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

