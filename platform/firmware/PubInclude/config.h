/**
 * @file
 * @brief ANYKA software
 * 
 */

#ifndef __CONFIG_H__
/**
 * @def __CONFIG_H__
 *
 */
#define __CONFIG_H__

/*############WIN32*/#ifdef OS_WIN32/*##########################*/
#ifndef DEBUG_TRACE_SERIAL		// serial trace
	#define DEBUG_TRACE_SERIAL
#endif
/* parameters related to battery and hardware */
#define NEED_BAT_VOLATGE					3480   	//battery shutdowm voltage
#endif/*##############END WIN32###############################*/


/***************************************************************************
**@the following macro had been defined by platform_hd_config.h
****************************************************************************/

//the camera macro should be consistent with camera type, the CI8802 use USE_CAMERA_OV9650
#define CAMERA_H_SHOT					1		//∫·∆¡ªπ « ˙∆¡
#define CAMERA_1P3M_TO_2M               1       //1.3mega camera to 2mega
//#define CAMERA_P3M_TO_1P3M                    1        //0.3mega camera to 1.3mega
#endif

