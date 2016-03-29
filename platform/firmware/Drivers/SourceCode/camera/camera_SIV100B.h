
#ifndef __CAMERA_SIV100B_H__
#define __CAMERA_SIV100B_H__

#ifndef END_PARAMETER
#define END_PARAMETER           0xff
#endif

static const T_U8 RESET_TAB[] = {
0x04, 0x00,
0x05, 0x07,
0x11, 0x04,
0x12, 0x0A,
0x13, 0x1F,
0x16, 0x89,
0x1B, 0x90,
0x1F, 0x52,

    /* SIV100B  60Hz - 30.00FPS(120/8) 24MHz */
0x20, 0x00,
0x21, 0x08,
0x22, 0x12,
0x34, 0xAD,

    /* SIV100B  50Hz - 25.00FPS(100/4) 24MHz */
0x23, 0x00,
0x24, 0x73,
0x25, 0x17,
0x35, 0x83,
0x33, 0x08,

    /* AE */
0x40, 0x80,
0x41, 0x8A,
0x42, 0x7F,
0x43, 0xC0,
0x44, 0x48,
0x45, 0x28,
0x46, 0x08,
0x47, 0x15,
0x48, 0x1E,
0x49, 0x13,
0x4A, 0x63,
0x4B, 0xC4,
0x4C, 0x3C,
0x4E, 0x17,
0x4F, 0x8A,
0x50, 0x94,
0x5A, 0x00,

    /* Auto White Balance  0105 */
0x60, 0xC8,
0x61, 0x88,
0x62, 0x01,
0x63, 0x80,
0x64, 0x80,
0x65, 0xD0,
0x66, 0x8C,
0x67, 0xC8,
0x68, 0x8B,
0x69, 0x8A,
0x6A, 0x73,
0x6B, 0x95,
0x6C, 0x70,
0x6D, 0x88,
0x6E, 0x77,
0x6F, 0x46,
0x70, 0xEA,
0x71, 0x60,
0x72, 0x05,
0x73, 0x02,
0x74, 0x0C,
0x75, 0x0F,
0x76, 0x20,
0x77, 0xB7,
0x78, 0x97,

    /* IDP */
0x80, 0xAF,
0x81, 0x0D,
0x82, 0x1D,
0x83, 0x00,
0x86, 0xA1,
0x87, 0x04,
0x88, 0x28,
0x89, 0x0F,
0x92, 0x44,
0x93, 0x10,
0x94, 0x20,
0x95, 0x40,
0x96, 0x10,
0x97, 0x20,
0x98, 0x30,
0x99, 0x29,
0x9A, 0x50,

    /* Shading  0106 */
0xA4, 0xFF,
0xA5, 0xFF,
0xA6, 0xFF,
0xA7, 0xED,
0xA8, 0xCB,
0xA9, 0x55,
0xAA, 0x55,
0xAB, 0x55,
0xAC, 0x55,
0xAE, 0x55,
0xAD, 0x55,
0xAF, 0x98,
0xB0, 0x90,

    /* Gamma  0126 */
0xB1, 0x00,
0xB2, 0x08,
0xB3, 0x10,
0xB4, 0x23,
0xB5, 0x45,
0xB6, 0x62,
0xB7, 0x78,
0xB8, 0x8A,
0xB9, 0x9B,
0xBA, 0xAA,
0xBB, 0xB8,
0xBC, 0xCE,
0xBD, 0xE2,
0xBE, 0xF3,
0xBF, 0xFB,
0xC0, 0xFF,

    /* Color Matrix */
0xC1, 0x3D,
0xC2, 0xC6,
0xC3, 0xFD,
0xC4, 0x10,
0xC5, 0x21,
0xC6, 0x10,
0xC7, 0xF3,
0xC8, 0xBD,
0xC9, 0x50,

    /* Edge */
0xCA, 0x90,
0xCB, 0x18,
0xCC, 0x20,
0xCD, 0x06,
0xCE, 0x06,
0xCF, 0x10,
0xD0, 0x20,
0xD1, 0x2A,
0xD2, 0x86,
0xD3, 0x00,

    /* Contrast */
0xD4, 0x10,
0xD5, 0x14,
0xD6, 0x14,
0xD7, 0x00,
0xD8, 0x00,
0xD9, 0x00,
0xDA, 0x00,
0xDB, 0xFF,
0xDC, 0x0A,
0xDD, 0xFF,
0xDE, 0x00,
0xDF, 0xFF,
0xE0, 0x00,

    /* Saturation */
0xE1, 0x29,
0xE2, 0x58,

   /* Windowing */
0xF0, 0x24,
0xF1, 0x00,
0xF2, 0x80,
0xF3, 0x00,
0xF4, 0xE0,
0x03, 0xC5,
0x7A, 0x90,
0x7B, 0xB0,
0x7C, 0x80,
END_PARAMETER,END_PARAMETER,
};

static const T_U8 VGA_TO_SXGA_TAB[] =
{
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 SXGA_MODE_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 VGA_MODE_TAB[] = {
0x05, 0x07,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 CIF_MODE_TAB[] = {
0x05, 0x03,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 QVGA_MODE_TAB[] = {
0x05, 0x05,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 QCIF_MODE_TAB[] = {
0x05, 0x01,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 QQVGA_MODE_TAB[] = {
0x05, 0x04,
    END_PARAMETER, END_PARAMETER,
};

//setup exposure mode:whole, center, middle
static const T_U8 EXPOSURE_WHOLE_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EXPOSURE_CENTER_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EXPOSURE_MIDDLE_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

//the following codes define the exposure level
static const T_U8 EP_ADD_4_TAB[] =
{
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EP_0_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EP_SUB_4_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

//the following codes define the AWB mode:
static const T_U8 AWB_AUTO_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 AWB_SUNNY_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 AWB_CLOUDY_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 AWB_OFFICE_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 AWB_HOME_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 AWB_NIGHT_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

/****************   Camera Brightness Table   ****************/
static const T_U8 BRIGHTNESS_0_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 BRIGHTNESS_1_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 BRIGHTNESS_2_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 BRIGHTNESS_3_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 BRIGHTNESS_4_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 BRIGHTNESS_5_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 BRIGHTNESS_6_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

/****************   Camera Contrast Table   ****************/
static const T_U8 CONTRAST_1_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 CONTRAST_2_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 CONTRAST_3_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 CONTRAST_4_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 CONTRAST_5_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 CONTRAST_6_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 CONTRAST_7_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

/****************   Camera Saturation Table   ****************/
static const T_U8 SATURATION_1_TAB[] = {

	END_PARAMETER, END_PARAMETER,
};

static const T_U8 SATURATION_2_TAB[] = {
	END_PARAMETER, END_PARAMETER,
};

static const T_U8 SATURATION_3_TAB[] = {
	END_PARAMETER, END_PARAMETER,
};

static const T_U8 SATURATION_4_TAB[] = {
	END_PARAMETER, END_PARAMETER,
};

static const T_U8 SATURATION_5_TAB[] = {
	END_PARAMETER, END_PARAMETER,
};

static const T_U8 SHARPNESS_0_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 SHARPNESS_1_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 SHARPNESS_2_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 SHARPNESS_3_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 SHARPNESS_4_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 SHARPNESS_5_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 SHARPNESS_6_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EFFECT_NORMAL_TAB[] = {
		0xD8, 0x00,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EFFECT_SEPIA_TAB[] = {
		0xD8, 0x80,
		0xD9, 0x60,
		0xDA, 0xA0,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EFFECT_ANTIQUE_TAB[] = {//Emboss
		0xD8, 0x08,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EFFECT_BLUISH_TAB[] = {
		0xD8, 0x80,
		0xD9, 0xC0,
		0xDA, 0x60,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EFFECT_GREENISH_TAB[] = {
		0xD8, 0x80,
		0xD9, 0x50,
		0xDA, 0x50,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EFFECT_REDDISH_TAB[] = {
		0xD8, 0x80,
		0xD9, 0x50,
		0xDA, 0x50,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EFFECT_NEGATIVE_TAB[] = {
		0xD8, 0x20,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EFFECT_BW_TAB[] = {
		0xD8, 0x40,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 EFFECT_BWN_TAB[] = {
		0xD8, 0x10,
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 PICTURE_BEGIN_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 PICTURE_SYN_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 PICTURE_END_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 PLL_1X_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 PLL_2X_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 PLL_3X_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

static const T_U8 PLL_4X_TAB[] = {
    END_PARAMETER, END_PARAMETER,
};

#endif
