#include "akdefine.h"
#include "drv_gpio.h"

#include "platform_hd_config.h"
#include "camera_SIV100B.h"

#ifdef USE_CAMERA_SIV100B

#define CAM_EN_LEVEL		    0
#define CAM_RESET_LEVEL			0


/* Camera's SCCB interface address */
#ifndef CAMERA_SCCB_ADDR
#define CAMERA_SCCB_ADDR            0x66
#endif

#define CAMERA_TYPE_SIV100B         0xC
#define CAMERA_MCLK_DIV     3    //6
#define SIV100B_CAMERA_MCLK  24

static T_CAMERA_TYPE camera_SIV100B_type = CAMERA_P3M;
static T_BOOL camera_set_param(const unsigned char tabParameter[]);
static T_VOID camera_setup(const T_U8 para_table[]);
static T_VOID camera_setbit(T_U8 reg, T_U8 bit, T_U8 value);
static T_VOID cam_set_VGAToSXGA(T_VOID);
static T_CAMERA_MODE s_SIV100B_CurMode = CAMERA_MODE_VGA;

static T_VOID delay1ms(T_U32 t)
{
    T_U32 i, j, k;

    k = 0;
    for (i=0; i<t; i++)
        for (j=0; j<1000; j++)
            k++;
    i = k;
}

static T_VOID camera_setup(const T_U8 para_table[])
{
    int i;
    T_U8 reg, cmd;
    for (i=0; ;i++) {
        reg = para_table[2*i];
        cmd = para_table[2*i+1];
        if (reg == END_PARAMETER)
            break;
        sccb_write_data(CAMERA_SCCB_ADDR, reg, &cmd, 1);
    }
}

static T_VOID camera_setbit(T_U8 reg, T_U8 bit, T_U8 value)
{
    T_U8 tmp;
    tmp = sccb_read_data(CAMERA_SCCB_ADDR, reg);
    if (value == 1)
        tmp |= 0x1<<bit;
    else
        tmp &= ~(0x1<<bit);
    sccb_write_data(CAMERA_SCCB_ADDR, reg, &tmp, 1);
}

T_VOID lala_read(T_U8 reg)
{
    unsigned char value;

    value = sccb_read_data(CAMERA_SCCB_ADDR, reg);

    akprintf(C3, M_DRVSYS, "i2c read reg 0x%x is 0x%x\n", reg, value);
}

static T_BOOL camera_set_param(const unsigned char tabParameter[])
{
    int i = 0;
    unsigned char value;

    while (1)
    {
        if (tabParameter[i] == END_PARAMETER)
            break;

        sccb_write_data(CAMERA_SCCB_ADDR, tabParameter[i], (unsigned char *)&tabParameter[i + 1], 1);

        value = 0xff;
        value = sccb_read_data(CAMERA_SCCB_ADDR, tabParameter[i]);


        if (value != tabParameter[i + 1])
            {
                akprintf(C1, M_DRVSYS, "set parameter error!\r\n");
                akprintf(C1, M_DRVSYS, "%x", i);
                akprintf(C1, M_DRVSYS, " reg=0x%x, ", tabParameter[i]);
                akprintf(C1, M_DRVSYS, " data=0x%x, ",tabParameter[i + 1]);
                akprintf(C1, M_DRVSYS, " read=0x%x\n",value);

                return AK_FALSE;
            }

        i += 2;
    }

    return AK_TRUE;
}

/**
 * @brief: set camera mode from VGA to SXGA
 * @author:
 * @date 2004-09-22
 * @return T_VOID
 * @retval
 */
static T_VOID cam_set_VGAToSXGA(T_VOID)
{
}


/**
 * @brief: Set camera mode to specify image quality, SXGA/VGA/CIF/ etc
 * @author:
 * @date 2004-09-22
 * @param[in] mode: mode value
 * @return T_VOID
 * @retval
 */

T_VOID cam_SIV100B_set_mode(T_CAMERA_MODE mode)
{
    s_SIV100B_CurMode = mode;

    switch(mode)
    {
        case CAMERA_MODE_VGA:
            camera_setup(VGA_MODE_TAB);
            break;
        case CAMERA_MODE_CIF:
            camera_setup(CIF_MODE_TAB);
            break;
        case CAMERA_MODE_QVGA:
            camera_setup(QVGA_MODE_TAB);
            break;
        case CAMERA_MODE_QCIF:
            camera_setup(QCIF_MODE_TAB);
            break;
        case CAMERA_MODE_QQVGA:
            camera_setup(QQVGA_MODE_TAB);
            break;
        default:
            s_SIV100B_CurMode = CAMERA_MODE_VGA;
            akprintf(C1, M_DRVSYS, "parameter error!\n");
            return;
    }
}


/**
 * @brief: Set camera exposure mode
 * @author:
 * @date 2004-09-22
 * @param[in] exposure: exposure mode
 * @return T_VOID
 * @retval
 */
T_VOID cam_SIV100B_set_exposure(T_CAMERA_EXPOSURE exposure)
{
	switch(exposure)
    {
        case EXPOSURE_WHOLE:
            camera_setup(EXPOSURE_WHOLE_TAB);
            break;
        case EXPOSURE_CENTER:
            camera_setup(EXPOSURE_CENTER_TAB);
            break;
        case EXPOSURE_MIDDLE:
            camera_setup(EXPOSURE_MIDDLE_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "parameter error!\n");
            break;
    }
}

/**
 * @brief: Set camera brightness level
 * @author:
 * @date 2004-09-22
 * @param[in] brightness: brightness value
 * @return T_VOID
 * @retval
 */
T_VOID cam_SIV100B_set_brightness(T_CAMERA_BRIGHTNESS brightness)
{
    switch(brightness)
    {
    case CAMERA_BRIGHTNESS_0:
        camera_setup(BRIGHTNESS_0_TAB);
        break;
        case CAMERA_BRIGHTNESS_1:
            camera_setup(BRIGHTNESS_1_TAB);
            break;
        case CAMERA_BRIGHTNESS_2:
            camera_setup(BRIGHTNESS_2_TAB);
            break;
        case CAMERA_BRIGHTNESS_3:
            camera_setup(BRIGHTNESS_3_TAB);
            break;
        case CAMERA_BRIGHTNESS_4:
            camera_setup(BRIGHTNESS_4_TAB);
            break;
        case CAMERA_BRIGHTNESS_5:
            camera_setup(BRIGHTNESS_5_TAB);
            break;
        case CAMERA_BRIGHTNESS_6:
            camera_setup(BRIGHTNESS_6_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "parameter error!\n");
            break;
    }
}


/**
 * @brief: Set camera contrast level
 * @author:
 * @date 2004-09-22
 * @param[in] contrast: contrast value
 * @return T_VOID
 * @retval
 */
T_VOID cam_SIV100B_set_contrast(T_CAMERA_CONTRAST contrast)
{
    switch(contrast)
    {
        case CAMERA_CONTRAST_1:
            camera_setup(CONTRAST_1_TAB);
            break;
        case CAMERA_CONTRAST_2:
            camera_setup(CONTRAST_2_TAB);
            break;
        case CAMERA_CONTRAST_3:
            camera_setup(CONTRAST_3_TAB);
            break;
        case CAMERA_CONTRAST_4:
            camera_setup(CONTRAST_4_TAB);
            break;
        case CAMERA_CONTRAST_5:
            camera_setup(CONTRAST_5_TAB);
            break;
        case CAMERA_CONTRAST_6:
            camera_setup(CONTRAST_6_TAB);
            break;
        case CAMERA_CONTRAST_7:
            camera_setup(CONTRAST_7_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "parameter error!\n");
            break;
    }
}


/**
 * @brief: Set camera saturation level
 * @author:
 * @date 2004-09-22
 * @param[in] saturation: saturation value
 * @return T_VOID
 * @retval
 */
T_VOID cam_SIV100B_set_saturation(T_CAMERA_SATURATION saturation)
{
    switch(saturation)
    {
        case CAMERA_SATURATION_1:
            camera_setup(SATURATION_1_TAB);
            break;
        case CAMERA_SATURATION_2:
            camera_setup(SATURATION_2_TAB);
            break;
        case CAMERA_SATURATION_3:
            camera_setup(SATURATION_3_TAB);
            break;
        case CAMERA_SATURATION_4:
            camera_setup(SATURATION_4_TAB);
            break;
        case CAMERA_SATURATION_5:
            camera_setup(SATURATION_5_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "parameter error!\n");
            break;
    }
}


/**
 * @brief: Set camera sharpness level
 * @author:
 * @date 2004-09-22
 * @param[in] sharpness: sharpness value
 * @return T_VOID
 * @retval
 */
T_VOID cam_SIV100B_set_sharpness(T_CAMERA_SHARPNESS sharpness)
{
    switch(sharpness)
    {
        case CAMERA_SHARPNESS_0:
            camera_setup(SHARPNESS_0_TAB);
            break;
        case CAMERA_SHARPNESS_1:
            camera_setup(SHARPNESS_1_TAB);
            break;
        case CAMERA_SHARPNESS_2:
            camera_setup(SHARPNESS_2_TAB);
            break;
        case CAMERA_SHARPNESS_3:
            camera_setup(SHARPNESS_3_TAB);
            break;
        case CAMERA_SHARPNESS_4:
            camera_setup(SHARPNESS_4_TAB);
            break;
        case CAMERA_SHARPNESS_5:
            camera_setup(SHARPNESS_5_TAB);
            break;
        case CAMERA_SHARPNESS_6:
            camera_setup(SHARPNESS_6_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "parameter error!\n");
            break;
    }
}

T_VOID cam_SIV100B_set_night_mode(T_NIGHT_MODE mode)
{
	T_U8 day_param[] =
		{
			0x33,0x10,
			0x40,0x84,
			0xD7,0x20,
			END_PARAMETER, END_PARAMETER
		};

	T_U8 night_param[]=
		{
			0x33,0x08,
			0x40,0x80,
			0xD7,0x00,
			END_PARAMETER, END_PARAMETER
		};
	
	switch(mode)
	{

		case CAMERA_NIGHT_MODE:
			camera_set_param(night_param);
			
			break;
		case CAMERA_DAY_MODE:
		default:
			camera_set_param(day_param);
			break;

	}
}

/**
 * @brief: Set camera mirror mode
 * @author:
 * @date 2004-09-22
 * @param[in] mirror: mirror mode
 * @return T_VOID
 * @retval
 */
T_VOID cam_SIV100B_set_mirror(T_CAMERA_MIRROR mirror)
{
    switch(mirror)
    {
        case CAMERA_MIRROR_V:
            camera_setbit(0x04, 1, 1);
            break;
        case CAMERA_MIRROR_H:
            camera_setbit(0x04, 0, 1);
            break;
        case CAMERA_MIRROR_NORMAL:
            camera_setbit(0x04, 1, 0);
            camera_setbit(0x04, 0, 0);
            break;
        case CAMERA_MIRROR_FLIP:
            camera_setbit(0x04, 1, 1);
            camera_setbit(0x04, 0, 1);
            break;
        default:
            akprintf(C1, M_DRVSYS, "parameter error!\n");
            break;
    }
}


/**
 * @brief: Set camera effect mode
 * @author:
 * @date 2004-09-22
 * @param[in] effect: effect mode
 * @return T_VOID
 * @retval
 */
T_VOID cam_SIV100B_set_effect(T_CAMERA_EFFECT effect)
{
    switch(effect)
    {
        case CAMERA_EFFECT_NORMAL:
            camera_setup(EFFECT_NORMAL_TAB);
            break;
        case CAMERA_EFFECT_SEPIA:
            camera_setup(EFFECT_SEPIA_TAB);
            break;
        case CAMERA_EFFECT_ANTIQUE:
            camera_setup(EFFECT_ANTIQUE_TAB);
            break;
        case CAMERA_EFFECT_BLUE:
            camera_setup(EFFECT_BLUISH_TAB);
            break;
        case CAMERA_EFFECT_GREEN:
            camera_setup(EFFECT_GREENISH_TAB);
            break;
        case CAMERA_EFFECT_RED:
            camera_setup(EFFECT_REDDISH_TAB);
            break;
        case CAMERA_EFFECT_NEGATIVE:
            camera_setup(EFFECT_NEGATIVE_TAB);
            break;
        case CAMERA_EFFECT_BW:
            camera_setup(EFFECT_BW_TAB);
            break;
        case CAMERA_EFFECT_BWN:
            camera_setup(EFFECT_BWN_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "parameter error!\n");
            break;
    }
}

/**
 * @brief: Set camera windows
 * @author:
 * @date 2004-09-22
 * @param[in] width: window width
 * @param[in] height: window height
 * @return int
 * @retval 0 error mode; 1 success; -1 fail
 */
T_S32 cam_SIV100B_set_focus(T_U32 width, T_U32 height)
{
	unsigned int hrefstart = 0, vrefstart = 0;

	unsigned char hbit = 0;
	unsigned char Camera_window_table[] =
	{
		0xf0, 0,
		0xf1, 0,
		0xf2, 0,
		0xf3, 0,
		0xf4, 0,
		END_PARAMETER, END_PARAMETER
	};

//akprintf(C1, M_DRVSYS, "cam_SIV100B_set_focus %d %d %d\n",width,height,Cammode);
	if(s_SIV100B_CurMode == CAMERA_MODE_VGA )//VGA_MODE
	{
	    hrefstart = (640 - width)/2;
	    vrefstart = (480 - height)/2;
	    }
	else if(s_SIV100B_CurMode == CAMERA_MODE_QVGA )//QVGA_MODE
	{
	    hrefstart = (320 - width)/2;
	    vrefstart = (240 - height)/2;
	}
	else if(s_SIV100B_CurMode == CAMERA_MODE_QQVGA )//QQVGA_MODE
	{
	    hrefstart = (160 - width)/2;
	    vrefstart = (120 - height)/2;
	}
	else
	{
		return 0;
	}

	Camera_window_table[3]=hrefstart&0xff;
	Camera_window_table[5]=width&0xff;
	Camera_window_table[7]=vrefstart&0xff;
	Camera_window_table[9]=height&0xff;

	Camera_window_table[1] = ((hrefstart&0x300)>>2)|((width&0x300)>>4)|((vrefstart&0x100)>>5)|((height&0x100)>>6);//Horizontal Frame start high 2-bit


	if (camera_set_param(Camera_window_table)  == AK_TRUE)
	    return 1;
	else
		return -1;

}


/**
 * @brief: Set camera AWB mode
 * @author:
 * @date 2004-09-22
 * @param[in] awb: AWB mode
 * @return T_VOID
 * @retval
 */
T_VOID cam_SIV100B_set_AWB(T_CAMERA_AWB awb)
{
    switch(awb)
    {
        case AWB_AUTO:
            cam_SIV100B_set_contrast(CAMERA_CONTRAST_4);
            cam_SIV100B_set_saturation(CAMERA_SATURATION_4);
            cam_SIV100B_set_brightness(CAMERA_BRIGHTNESS_3);
            camera_setup(AWB_AUTO_TAB);
            break;
        case AWB_SUNNY:
            cam_SIV100B_set_contrast(CAMERA_CONTRAST_5);
            cam_SIV100B_set_saturation(CAMERA_SATURATION_5);
            cam_SIV100B_set_brightness(CAMERA_BRIGHTNESS_3);
            camera_setup(AWB_SUNNY_TAB);
            break;
        case AWB_CLOUDY:
            cam_SIV100B_set_contrast(CAMERA_CONTRAST_5);
            cam_SIV100B_set_saturation(CAMERA_SATURATION_4);
            cam_SIV100B_set_brightness(CAMERA_BRIGHTNESS_3);
            camera_setup(AWB_CLOUDY_TAB);
            break;
        case AWB_OFFICE:
            cam_SIV100B_set_contrast(CAMERA_CONTRAST_4);
            cam_SIV100B_set_saturation(CAMERA_SATURATION_4);
            cam_SIV100B_set_brightness(CAMERA_BRIGHTNESS_3);
            camera_setup(AWB_OFFICE_TAB);
            break;
        case AWB_HOME:
            cam_SIV100B_set_contrast(CAMERA_CONTRAST_4);
            cam_SIV100B_set_saturation(CAMERA_SATURATION_3);
            cam_SIV100B_set_brightness(CAMERA_BRIGHTNESS_3);
            camera_setup(AWB_HOME_TAB);
            break;
        case AWB_NIGHT:
            cam_SIV100B_set_contrast(CAMERA_CONTRAST_4);
            cam_SIV100B_set_saturation(CAMERA_SATURATION_2);
            cam_SIV100B_set_brightness(CAMERA_BRIGHTNESS_5);
            camera_setup(AWB_NIGHT_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "parameter error!\n");
            break;
    }
}


T_BOOL cam_SIV100B_init()
{
    if( !camera_set_param(RESET_TAB) )
	{
		gpio_set_pin_level( GPIO_CAMERA_CHIP_ENABLE, !CAM_EN_LEVEL );
		gpio_set_pin_level( GPIO_CAMERA_AVDD, 0 );    // avdd
		return AK_FALSE;
	}
	return AK_TRUE;
}

T_U32 cam_SIV100B_read_id()
{
    T_U8 value=0x0;
    T_U32 id=0;

//    akprintf(C1, M_DRVSYS, "init sccb\n");

    sccb_init(GPIO_I2C_SCL, GPIO_I2C_SDA); //init sccb first here!!

	gpio_set_pin_dir( GPIO_CAMERA_AVDD, 1 );
	gpio_set_pin_level( GPIO_CAMERA_AVDD, 1 );    // avdd

	gpio_set_pin_dir( GPIO_CAMERA_CHIP_ENABLE, 1 );
	gpio_set_pin_level( GPIO_CAMERA_CHIP_ENABLE, CAM_EN_LEVEL);		/* modify by LiuB 20061027 */

	gpio_set_pin_dir( GPIO_CAMERA_RESET, 1 );
	gpio_set_pin_level( GPIO_CAMERA_RESET, CAM_RESET_LEVEL );
	mini_delay(10);
	gpio_set_pin_level( GPIO_CAMERA_RESET, !CAM_RESET_LEVEL );

	mini_delay(500);

	value = sccb_read_data(CAMERA_SCCB_ADDR, 0x01);
	id = value;
	return id;
}

T_BOOL	cam_SIV100B_set_to_cap(T_U32 width, T_U32 height)
{
	float Mclk;
    T_CAMERA_MODE Cammode;

	Mclk = (float)(get_asic_freq() / 1000000) / CAMERA_MCLK_DIV;

    if ((width <= 160) && (height <= 120))
    {
        Cammode = CAMERA_MODE_QQVGA;
    }
    else if ((width <= 176) && (height <= 144))
    {
        Cammode = CAMERA_MODE_QCIF;
    }
    else if ((width <= 320) && (height <= 240))
    {
        Cammode = CAMERA_MODE_QVGA;
    }
    else if ((width <= 352) && (height <= 288))
    {
        Cammode = CAMERA_MODE_CIF;
    }
    else if ((width <= 640) && (height <= 480))
    {
        Cammode = CAMERA_MODE_VGA;
    }
    else
    {
        akprintf(C1, M_DRVSYS, "SIV100B unsupport %d & %d mode!\n", width, height);
        return AK_FALSE;
    }
	if(Cammode < CAMERA_MODE_VGA)
	{
		akprintf(C1, M_DRVSYS, "30W camera dose not support such mode");
		return AK_FALSE;
	}

	cam_SIV100B_set_mode(Cammode);
	cam_SIV100B_set_focus(width, height);
	return AK_TRUE;
}

T_BOOL	cam_SIV100B_set_to_prev(T_U32 width, T_U32 height)
{
	cam_SIV100B_set_mode(CAMERA_MODE_VGA);
	cam_SIV100B_set_focus(width, height);
	return AK_TRUE;
}

static T_BOOL cam_SIV100B_set_to_record(T_U32 width, T_U32 height)
{	
	cam_SIV100B_set_mode(CAMERA_MODE_CIF);
	cam_SIV100B_set_focus(width, height);
	return AK_TRUE;
}

T_BOOL	cam_SIV100B_close(T_VOID)
{
	gpio_set_pin_level( GPIO_CAMERA_CHIP_ENABLE, !CAM_EN_LEVEL );
	gpio_set_pin_level( GPIO_CAMERA_AVDD, 0 );    // avdd
	gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_INPUT);

	gpio_set_pin_dir(GPIO_I2C_SCL, GPIO_DIR_OUTPUT);
	gpio_set_pin_level(GPIO_I2C_SCL, GPIO_LEVEL_LOW);
	gpio_set_pin_dir(GPIO_I2C_SDA, GPIO_DIR_OUTPUT);
	gpio_set_pin_level(GPIO_I2C_SDA, GPIO_LEVEL_LOW);
	
	return AK_TRUE;
}

T_VOID cam_SIV100B_open(T_VOID)
{
	gpio_set_pin_dir( GPIO_CAMERA_AVDD, 1 );
	gpio_set_pin_level( GPIO_CAMERA_AVDD, 1 );    // avdd
	
	gpio_set_pin_dir( GPIO_CAMERA_CHIP_ENABLE, 1 );
	gpio_set_pin_level( GPIO_CAMERA_CHIP_ENABLE, CAM_EN_LEVEL);		/* modify by LiuB 20061027 */

	gpio_set_pin_dir( GPIO_CAMERA_RESET, 1 );
	gpio_set_pin_level( GPIO_CAMERA_RESET, CAM_RESET_LEVEL );
	mini_delay(10);
	gpio_set_pin_level( GPIO_CAMERA_RESET, !CAM_RESET_LEVEL );

    mini_delay(500);
}


T_CAMERA_TYPE cam_SIV100B_get_type(T_VOID)
{
	return camera_SIV100B_type;
}

static T_CAMERA_FUNCTION_HANDLER SIV100B_HANDLER = 
{
	SIV100B_CAMERA_MCLK,
	cam_SIV100B_open,
	cam_SIV100B_close,
	cam_SIV100B_read_id,
	cam_SIV100B_init,
	cam_SIV100B_set_mode,
	cam_SIV100B_set_exposure,
	cam_SIV100B_set_brightness,
	cam_SIV100B_set_contrast,
	cam_SIV100B_set_saturation,
	cam_SIV100B_set_sharpness,
	cam_SIV100B_set_AWB,
	cam_SIV100B_set_mirror,
	cam_SIV100B_set_effect,
	cam_SIV100B_set_focus,
	cam_SIV100B_set_night_mode,
	cam_SIV100B_set_to_cap,
	cam_SIV100B_set_to_prev,
	cam_SIV100B_set_to_record,
	cam_SIV100B_get_type	
};

static int camera_SIV100B_reg(void)
{
	camera_reg_dev(CAMERA_TYPE_SIV100B, &SIV100B_HANDLER);
	return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_SIV100B_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif
