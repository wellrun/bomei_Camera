/**
 * @author: Bennis Zhang
 */
#include "anyka_types.h"

#ifdef SUPPORT_UVC
#ifdef OS_ANYKA


#include "akos_api.h"
#include "eng_debug.h"
#include "fwl_pfdisplay.h"
#include "gpio_config.h"
#include "Lib_event.h"

#include "hal_camera.h"

#include "hal_usb_s_uvc.h"
#include "hal_usb_s_state.h"

#include "log_uvc_cam.h"
#include "lib_image_api.h"
#include "Eng_ImgDec.h"
#include "fwl_osmalloc.h"
#include "drv_gpio.h"
#include "hal_gpio.h"
#include "hal_sysdelay.h"
#include "Lib_state_api.h"
#include "Fwl_pfcamera.h"
#include "Fwl_ImageLib.h"
#include "Fwl_sys_detect.h"



#define UVC_CAM_WIN_W   (640)
#define UVC_CAM_WIN_H   (480)
#define UVC_CAM_VIDO_W  (160)
#define UVC_CAM_VIDO_H  (120)

#ifdef TASK_STACK_SIZ
#undef TASK_STACK_SIZ
#endif
#define TASK_STACK_SIZ			(20*1024)

#define EVENT_USB_SEND_FINISH   0x00000002
#define EVENT_CHANGE_SIZE		0x00000004
#define EVENT_USB_SUSPEND       0x00000010
#define EVENT_USB_TASK_FINISHED 0x00000020
#define EVENT_USB_DETECT		0x00000040

#define __MYMALLOC(s)			Fwl_Malloc(s)
#define __MYFREE(p)				Fwl_Free(p)
#define __DBG_PRINT(s)			AK_DEBUG_OUTPUT("\n%s ... Line: %d, Message: %s.\n", __FILE__, __LINE__, (s))
#define __DBG_PRINT_VAR(v)		AK_DEBUG_OUTPUT("\n%s --- Line: %d, Variable: %s, Value: %d.\n", __FILE__, __LINE__, #v, (v))

#define __CHECK_PARAM_RET_NULL(con)  \
{  \
	if (con)  \
	{  \
		AK_DEBUG_OUTPUT("\n%s ... Line: %d, Condition: %s.\n", __FILE__, __LINE__, #con);  \
		return;  \
	}  \
}

#define __CHECK_PARAM_RET(con,ret)  \
{  \
	if (con)  \
	{  \
		AK_DEBUG_OUTPUT("\n%s ... Line: %d, Condition: %s.\n", __FILE__, __LINE__, #con);  \
		return (ret);  \
	}  \
}

typedef union
{
	struct
	{
		T_U8 * y;
		T_U8 * u;
		T_U8 * v;
		T_U32  siz;
	} dat;
	T_U8 * yuv;
} T_U_YUV;

typedef struct
{
	T_hEventGroup	eventGroup;
	T_UVCCAM_INI	cfg;
	T_U_YUV			yuv0;
	T_U_YUV			yuv1;
	T_U_YUV			yuv2;
	T_U8			* jpeg;
	T_U8			* payload0;
	T_U8			* payload1;
} T_UVCCAM_RUNTIME_DATA;

typedef struct
{
	T_UVCCAM_INI			args;

	T_hTask					hTask;
	T_U32					stack[TASK_STACK_SIZ / 4];

	T_UVCCAM_RUNTIME_DATA	runtime;
} T_UVCCOM_DESC;


typedef struct
{
	T_hSemaphore	sem;
	T_UVCCOM_DESC	*current;
    T_BOOL          bUserExit;
} T_UVCCOM_CTRL;

static T_UVCCOM_CTRL m_UVCCtrl = {-1, AK_NULL, AK_FALSE};

#if (SDRAM_MODE >= 16)
static T_eUVC_STREAM_FORMAT stream_format = UVC_STREAM_YUV;
#else
static T_eUVC_STREAM_FORMAT stream_format = UVC_STREAM_MJPEG;
#endif

T_CAMERA_BUFFER yuv0, yuv1, yuv2;

static T_UVCCOM_DESC* Uvc_Open(T_UVCCAM_INI * args);
static T_S32 Uvc_Close(T_UVCCOM_DESC * pDesc);
static T_VOID Uvc_TaskHandle(T_U32 argc, T_VOID *argv);
static T_S32 Uvc_NewYuv(T_U_YUV * p, T_U32 width, T_U32 height, T_E_UVCCAM_YUVTYPE ty);
static T_S32 Uvc_DelYuv(T_U_YUV * p);
static T_S32 Uvc_ChangeSize(T_UVCCAM_RUNTIME_DATA * runtm);
static T_VOID Uvc_DataNotifyCB(T_VOID);
static T_VOID Uvc_VCCtrlCB(T_eUVC_CONTROL dwCtrl, T_U32 val1, T_U32 val2);
static T_VOID Uvc_VSCtrlCB(T_eUVC_CONTROL dwCtrl, T_U32 val1, T_U32 val2);
static T_VOID Uvc_SentDataCB(T_VOID);
static T_S32 Uvc_UsbErr(T_VOID);

T_S32 UVCCam_Init(T_VOID)
{
	if (m_UVCCtrl.sem == -1)
	{
		m_UVCCtrl.sem = AK_Create_Semaphore(1, AK_PRIORITY);
		__CHECK_PARAM_RET((AK_IS_INVALIDHANDLE(m_UVCCtrl.sem)), -1);
		m_UVCCtrl.current = AK_NULL;
	}
	__DBG_PRINT("UVCCam_Init OK");
	return -1;
}



T_BOOL  UVCCam_InitStruct(T_UVCCAM_INI * args)
{
	if(AK_NULL == args)
		return AK_FALSE;
	
	args->camWinWidth  = UVC_CAM_WIN_W;
	args->camWinHeight = UVC_CAM_WIN_H;
	args->videoWidth   = UVC_CAM_VIDO_W;
	args->videoHeight  = UVC_CAM_VIDO_H;
	args->zoom_level = 0;
	
	args->YUVType = E_UVCCAM_YUVTYPE_420;
	
	args->USBType = USB_MODE_20 | USB_MODE_CPU;// USB2.0; USB_MODE_11|USB_MODE_CPU USB1.1

	return AK_TRUE;
}

T_HANDLE UVCCam_Open(T_UVCCAM_INI * args)
{	
	__CHECK_PARAM_RET(AK_NULL == args, 0);

	if (AK_SUCCESS == AK_Obtain_Semaphore(m_UVCCtrl.sem, AK_SUSPEND))
	{
		if (AK_NULL != m_UVCCtrl.current)
		{
			AK_Release_Semaphore(m_UVCCtrl.sem);
			__DBG_PRINT("UVCCam_Open failed");
			return (T_HANDLE)0;
		}

		if (AK_NULL == (m_UVCCtrl.current = Uvc_Open(args)))
		{
			AK_Release_Semaphore(m_UVCCtrl.sem);
			__DBG_PRINT("UVCCam_Open failed");
			return (T_HANDLE)0;
		}

		AK_Release_Semaphore(m_UVCCtrl.sem);

		__DBG_PRINT("UVCCam_Open is OK");
	}

	return m_UVCCtrl.current;
}

T_S32 UVCCam_Close(T_HANDLE hdl)
{
	T_UVCCOM_DESC * pDesc;
	
	__CHECK_PARAM_RET(0 == hdl, -1);

	pDesc = (T_UVCCOM_DESC *)hdl;

	if (0 != Uvc_Close(pDesc))
	{
		__DBG_PRINT("UVCCam_Close failed");
		return -1;
	}

	m_UVCCtrl.current = AK_NULL;

	//GPIO_I2C_SDA and GPIO_KEYAPD_COLUMN1 use the same gpio
	gpio_set_pin_level(GPIO_I2C_SDA, GPIO_LEVEL_HIGH); 

	__DBG_PRINT("UVCCam_Close OK");

	return 0;
}

T_VOID UVCCam_UserExit(T_BOOL bFlag)
{	
	m_UVCCtrl.bUserExit = bFlag;
}

static T_VOID Uvc_FreeRuntmYuv(T_UVCCAM_RUNTIME_DATA *runtm)
{
	Uvc_DelYuv(&runtm->yuv2);
	Uvc_DelYuv(&runtm->yuv1);
	Uvc_DelYuv(&runtm->yuv0);
}

static T_VOID Uvc_FreeRuntm(T_UVCCAM_RUNTIME_DATA *runtm)
{
	__MYFREE(runtm->payload1);
	__MYFREE(runtm->payload0);
	__MYFREE(runtm->jpeg);
	Uvc_FreeRuntmYuv(runtm);
}

static T_VOID Uvc_SetCamBuf(T_CAMERA_BUFFER *yuv0, T_CAMERA_BUFFER *yuv1, T_CAMERA_BUFFER *yuv2, T_UVCCAM_RUNTIME_DATA *runtm)
{
	yuv0->dY = runtm->yuv0.dat.y;
	yuv0->dU = runtm->yuv0.dat.u;
	yuv0->dV = runtm->yuv0.dat.v;
	yuv1->dY = runtm->yuv1.dat.y;
	yuv1->dU = runtm->yuv1.dat.u;
	yuv1->dV = runtm->yuv1.dat.v;
	yuv2->dY = runtm->yuv2.dat.y;
	yuv2->dU = runtm->yuv2.dat.u;
	yuv2->dV = runtm->yuv2.dat.v;

}

static T_BOOL Uvc_InitRuntmData(T_UVCCOM_DESC *pDesc, T_UVCCAM_INI *args)
{
	T_U32 siz;
	
	memcpy((T_U8 *)&pDesc->args, (T_U8 *)args, sizeof(T_UVCCAM_INI));
	memcpy((T_U8 *)&pDesc->runtime.cfg, (T_U8 *)&pDesc->args, sizeof(T_UVCCAM_INI));

	if (-1 == Uvc_NewYuv(&pDesc->runtime.yuv0, pDesc->runtime.cfg.camWinWidth, pDesc->runtime.cfg.camWinHeight, pDesc->runtime.cfg.YUVType))
	{
		__DBG_PRINT_VAR(pDesc->runtime.yuv0);
		return AK_FALSE;
	}
	
	if (-1 == Uvc_NewYuv(&pDesc->runtime.yuv1, pDesc->runtime.cfg.camWinWidth, pDesc->runtime.cfg.camWinHeight, pDesc->runtime.cfg.YUVType))
	{
		Uvc_DelYuv(&pDesc->runtime.yuv0);
		__DBG_PRINT_VAR(pDesc->runtime.yuv1);
		return AK_FALSE;
	}
	
	if (-1 == Uvc_NewYuv(&pDesc->runtime.yuv2, pDesc->runtime.cfg.camWinWidth, pDesc->runtime.cfg.camWinHeight, pDesc->runtime.cfg.YUVType))
	{
		Uvc_DelYuv(&pDesc->runtime.yuv1);
		Uvc_DelYuv(&pDesc->runtime.yuv0);
		__DBG_PRINT_VAR(pDesc->runtime.yuv2);
		return AK_FALSE;
	}
	
	siz = pDesc->runtime.cfg.camWinWidth * pDesc->runtime.cfg.camWinHeight;

	if(UVC_STREAM_MJPEG ==stream_format)
	{
		siz >>= 1;
	}
	else
	{
		siz *= 3;
	}
	__DBG_PRINT_VAR(siz);

#if 0	
	pDesc->runtime.jpeg = __MYMALLOC(siz);	
	if (AK_NULL == pDesc->runtime.jpeg)
	{
		Uvc_FreeRuntmYuv(&pDesc->runtime);
		__DBG_PRINT_VAR(pDesc->runtime.jpeg);
		return AK_FALSE;
	}
#endif

	pDesc->runtime.payload0 = __MYMALLOC(siz);
	if (AK_NULL == pDesc->runtime.payload0)
	{
		__MYFREE(pDesc->runtime.jpeg);
		Uvc_FreeRuntmYuv(&pDesc->runtime);
		__DBG_PRINT_VAR(pDesc->runtime.payload0);
		return AK_FALSE;
	}
	
	pDesc->runtime.payload1 = __MYMALLOC(siz);
	if (AK_NULL == pDesc->runtime.payload1)
	{
		__MYFREE(pDesc->runtime.payload0);
		__MYFREE(pDesc->runtime.jpeg);
		Uvc_FreeRuntmYuv(&pDesc->runtime);
		__DBG_PRINT_VAR(pDesc->runtime.payload1);
		return AK_FALSE;
	}
	
	pDesc->runtime.eventGroup = AK_Create_Event_Group();
	
	if (AK_IS_INVALIDHANDLE(pDesc->runtime.eventGroup ))
	{
		Uvc_FreeRuntm(&pDesc->runtime);
		__DBG_PRINT("AK_Create_Event_Group failed");
		return AK_FALSE;
	}
	
	Uvc_SetCamBuf(&yuv0, &yuv1, &yuv2, &pDesc->runtime);

	return AK_TRUE;
}

static T_VOID Uvc_FreeDesc(T_UVCCOM_DESC *pDesc)
{
	AK_Delete_Event_Group(pDesc->runtime.eventGroup);
	Uvc_FreeRuntm(&pDesc->runtime);
	__MYFREE(pDesc);
}

static T_UVCCOM_DESC* Uvc_Open(T_UVCCAM_INI * args)
{
	T_UVCCOM_DESC * pDesc;
	
	pDesc = __MYMALLOC(sizeof(T_UVCCOM_DESC));
	__CHECK_PARAM_RET(AK_NULL == pDesc, 0);
	memset((T_U8 *)pDesc, 0, sizeof(T_UVCCOM_DESC));
	
	if(!Uvc_InitRuntmData(pDesc, args))
	{
		__MYFREE(pDesc);
		return AK_NULL;
	}
	
	if (!Fwl_CameraInit())
	{
		Uvc_FreeDesc(pDesc);
		__DBG_PRINT("cam_open error");
		return AK_NULL;
	}
	
	//cam_set_feature(CAM_FEATURE_MIRROR, CAMERA_MIRROR_FLIP);	
	cam_set_to_prev(pDesc->runtime.cfg.camWinWidth, pDesc->runtime.cfg.camWinHeight);
	cam_set_feature(CAM_FEATURE_BRIGHTNESS, CAMERA_BRIGHTNESS_3);
	
	mini_delay(1000);

	if (!camstream_init( pDesc->runtime.cfg.videoWidth, pDesc->runtime.cfg.videoHeight, &yuv0, &yuv1, &yuv2))
	{
		Uvc_FreeDesc(pDesc);
		__DBG_PRINT("camstream_init error");
		return AK_NULL;
	}
	
	camstream_set_callback (Uvc_DataNotifyCB);

	if (!uvc_init(USB_MODE_20, stream_format))
	{
		camstream_stop();
		Uvc_FreeDesc(pDesc);
		__DBG_PRINT("UVC_Init error");
		return AK_NULL;
	}

	//	Setting Control
	uvc_set_ctrl(UVC_CTRL_BRIGHTNESS, 0, (CAMERA_BRIGHTNESS_NUM - 1), 3, 1); 
	uvc_set_ctrl(UVC_CTRL_CONTRAST,	  0, (CAMERA_CONTRAST_NUM - 1),	  4, 1);
	uvc_set_ctrl(UVC_CTRL_SATURATION, 0, (CAMERA_SATURATION_NUM - 1), 2, 1);
	//	Setting frame
	uvc_set_callback(Uvc_VCCtrlCB, Uvc_VSCtrlCB, Uvc_SentDataCB);
	
	uvc_init_desc();
	AK_Sleep(100);	
	
	memset(pDesc->stack, 0, TASK_STACK_SIZ);
	
	pDesc->hTask = AK_Create_Task((T_VOID *)Uvc_TaskHandle, "UVCCam", 1, (T_VOID *)&pDesc->runtime,
		(T_VOID *)pDesc->stack, TASK_STACK_SIZ,
		100, 5, AK_PREEMPT, AK_START);
	
	if (AK_IS_INVALIDHANDLE(pDesc->hTask) )
	{
		uvc_stop();
		camstream_stop();
		Uvc_FreeDesc(pDesc);
		__DBG_PRINT("AK_Create_Task failed");
		return AK_NULL;
	}
	
	uvc_start();
	AK_Set_Events(pDesc->runtime.eventGroup, EVENT_USB_SEND_FINISH, AK_OR);

	return pDesc;
}

static T_S32 Uvc_Close(T_UVCCOM_DESC *pDesc)
{
	T_S32 status;
	T_U32 tmp_event;
	
	status = AK_Set_Events(pDesc->runtime.eventGroup, EVENT_USB_SUSPEND, AK_OR);
	if (AK_SUCCESS != status)
	{
		__DBG_PRINT_VAR(status);
	}
	
	status = AK_Retrieve_Events(pDesc->runtime.eventGroup, EVENT_USB_TASK_FINISHED, AK_OR_CONSUME, &tmp_event, AK_SUSPEND);
	if (AK_SUCCESS != status)
	{
		__DBG_PRINT_VAR(status);
	}
	
	AK_Sleep(10);
	status = AK_Delete_Task(pDesc->hTask);
	if (AK_SUCCESS != status)
	{
		__DBG_PRINT_VAR(status);
	}

	status = AK_Delete_Event_Group(pDesc->runtime.eventGroup);
	if (AK_SUCCESS != status)
	{
		__DBG_PRINT_VAR(status);
	}

	uvc_stop();
	camstream_stop();
	cam_close();

	Uvc_FreeDesc(pDesc);
	pDesc = AK_NULL;

	return 0;
}

static T_VOID Uvc_TaskHandle(T_U32 argc, T_VOID *argv)
{
	T_CAMERA_BUFFER * pYuv;
	T_UVCCAM_RUNTIME_DATA * runtm;
	T_U32 siz, payloadSize;
	T_U32 tmp_event, saved_event;

	if ((1 != argc) && (AK_NULL == argv))
	{
		__DBG_PRINT_VAR(argc);
		__DBG_PRINT_VAR(argv);
		__DBG_PRINT("exit task -1");
		return;
	}

	__DBG_PRINT("start USB camera task");

	runtm = (T_UVCCOM_DESC *)argv;
	saved_event = 0;
	
	AK_Set_Events(runtm->eventGroup, EVENT_USB_DETECT, AK_OR);

	while (1)
	{
		//AK_DEBUG_OUTPUT(".");

#if  0
		if(!Usb_IsConnected())
		{
			break;
		}

		if (camstream_ready())
		{
			pYuv = camstream_get();
		}		
		else
		{
			continue;
		}

		if (AK_NULL == pYuv)
		{
			continue;
		}


		if(UVC_STREAM_MJPEG == stream_format)
		{					
			siz = runtm->cfg.camWinWidth * runtm->cfg.camWinHeight / 2;
		
			if (!Img_YUV2JPEG(pYuv->dY, pYuv->dU, pYuv->dV, runtm->payload1, &siz,
								runtm->cfg.videoWidth, runtm->cfg.videoHeight, 200))
			{
				__DBG_PRINT("Img_YUV2JPEG failed\n");
				continue;
			}
		}
		else
		{
			siz = uvc_parse_yuv(runtm->payload1, pYuv->dY, pYuv->dU, pYuv->dV,	
				runtm->cfg.videoWidth, runtm->cfg.videoHeight, YUV_FORMAT_420);
			if(runtm->cfg.videoWidth * runtm->cfg.videoHeight * 2 != siz)
			{
				__DBG_PRINT("uvc_parse_yuv failed\n");
				continue;
			}
		}

		if (AK_SUCCESS == AK_Retrieve_Events(runtm->eventGroup,
			EVENT_USB_SEND_FINISH | EVENT_CHANGE_SIZE | EVENT_USB_SUSPEND,
			AK_OR_CONSUME, &tmp_event, AK_SUSPEND))
		{
			if (tmp_event & EVENT_CHANGE_SIZE)
			{
				Uvc_ChangeSize(runtm);
				
				AK_DEBUG_OUTPUT("UVC: CHANGE SIZE EVENT.\n");				
			}	

			if (tmp_event & EVENT_USB_SUSPEND)
			{
				AK_DEBUG_OUTPUT("UVC:	EVT_Z09COM_SYS_RESET.\n");
				m_triggerEvent(M_EVT_Z09COM_SYS_RESET, AK_NULL);
				//break;  //sleep may be OK. or continue may OK too,
			}

			if(!(tmp_event & EVENT_USB_SEND_FINISH))
			{
				continue;
			}
		}


		
		payloadSize = uvc_payload(runtm->payload0, runtm->payload1, siz);
		
		while (!uvc_send(runtm->payload0, payloadSize));


#else 
        
        if (m_UVCCtrl.bUserExit)
        {  
            AK_DEBUG_OUTPUT("UVC: User Exit.\n"); 
            AK_Set_Events(runtm->eventGroup, EVENT_USB_SUSPEND, AK_OR);
            m_triggerEvent(M_EVT_Z09COM_SYS_RESET, AK_NULL);
            break;
        }
        
		if (AK_SUCCESS == AK_Retrieve_Events(runtm->eventGroup,
				EVENT_USB_SEND_FINISH | EVENT_CHANGE_SIZE | EVENT_USB_SUSPEND | EVENT_USB_DETECT,
				AK_OR_CONSUME, &tmp_event, AK_SUSPEND))
		{
			if (tmp_event & EVENT_USB_SEND_FINISH)
				saved_event |= EVENT_USB_SEND_FINISH;

			if (tmp_event & EVENT_USB_SUSPEND)
			{
				AK_DEBUG_OUTPUT("UVC:	EVT_Z09COM_SYS_RESET.\n");
				//[sheldon : ] m_triggerEvent(M_EVT_Z09COM_SYS_RESET, AK_NULL);
				//[sheldon : ]  据驱动部的说法:usbspend时不应该退出PC camera状态
				//[sheldon : ]  解决vista/win7系统下会自动退出的问题
				//break;
			}
            
			if (tmp_event & EVENT_USB_DETECT)
			{
				AK_Set_Events(runtm->eventGroup, EVENT_USB_DETECT, AK_OR);

                if (!usb_is_connected())
                {
                    __DBG_PRINT("usb cable out");
                    break;
                }
			
				if (Uvc_UsbErr() == USB_ERROR)
				{					
					if (!usb_is_connected())
					{
						AK_DEBUG_OUTPUT("UVC:	EVT_Z09COM_SYS_RESET.\n");
						m_triggerEvent(M_EVT_Z09COM_SYS_RESET, AK_NULL);
						break;
					}			
					continue;
				}

				if (Uvc_UsbErr() == USB_SUSPEND)
				{
					AK_DEBUG_OUTPUT("UVC:	EVT_Z09COM_SYS_RESET1.\n");
					//[sheldon : ]  m_triggerEvent(M_EVT_Z09COM_SYS_RESET, AK_NULL);
					//[sheldon : ]  据驱动部的说法:usbspend时不应该退出PC camera状态
					//[sheldon : ]  break;
				}
				
			}
			
			if (tmp_event & EVENT_CHANGE_SIZE)
			{
				Uvc_ChangeSize(runtm);
				
				AK_DEBUG_OUTPUT("UVC: CHANGE SIZE EVENT.\n");				
			}	
			
			if (saved_event & EVENT_USB_SEND_FINISH)
			{
				pYuv = AK_NULL;

				if (camstream_ready())
				{
					pYuv = camstream_get();
				}
				
				else
				{
					AK_Set_Events(runtm->eventGroup, EVENT_USB_SEND_FINISH, AK_OR);
					continue;
				}

				if (AK_NULL == pYuv)
				{
					AK_Set_Events(runtm->eventGroup, EVENT_USB_SEND_FINISH, AK_OR);
					continue;
				}

				saved_event = 0;

				if(UVC_STREAM_MJPEG == stream_format)
				{					
					siz = runtm->cfg.camWinWidth * runtm->cfg.camWinHeight / 2;
					if (!Img_YUV2JPEG_Stamp_Mutex(pYuv->dY, pYuv->dU, pYuv->dV, runtm->payload1, &siz,
										runtm->cfg.videoWidth, runtm->cfg.videoHeight, 200,AK_NULL	))
					{
						__DBG_PRINT("Img_YUV2JPEG_Stamp_Mutex failed\n");
						continue;
					}
					
				}
				else
				{
					siz = uvc_parse_yuv(runtm->payload1, pYuv->dY, pYuv->dU, pYuv->dV,	
						runtm->cfg.videoWidth, runtm->cfg.videoHeight, YUV_FORMAT_420);
					if(runtm->cfg.videoWidth * runtm->cfg.videoHeight * 2 != siz)
					{
						__DBG_PRINT("uvc_parse_yuv failed\n");
						continue;
					}
				}

				payloadSize = uvc_payload(runtm->payload0, runtm->payload1, siz);
				
				while (!uvc_send(runtm->payload0, payloadSize))
				{
					__DBG_PRINT_VAR(payloadSize);

					AK_Sleep(100);
					
					if (Uvc_UsbErr() != USB_OK || m_UVCCtrl.bUserExit)
					{
						AK_Set_Events(runtm->eventGroup, EVENT_USB_SEND_FINISH, AK_OR);
						break;
					}
				}

			//	Fwl_InvalidateRectYUV(pYuv->dY, pYuv->dU, pYuv->dV, runtm->cfg.videoWidth, runtm->cfg.videoHeight, 0, 0, MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT);
			}			
			// tmp_event = 0;
			if ((!usb_is_connected()) || (Uvc_UsbErr() == USB_SUSPEND))
			{
				AK_Set_Events(runtm->eventGroup, EVENT_USB_SUSPEND, AK_OR);
			}
			
		}
#endif

	}
	
	if (AK_SUCCESS != AK_Set_Events(runtm->eventGroup, EVENT_USB_TASK_FINISHED, AK_OR))
	{
		__DBG_PRINT("exit task AK_Set_Events failed");
	}
	
	__DBG_PRINT("exit task 0");
}

static T_S32 Uvc_NewYuv(T_U_YUV * p, T_U32 width, T_U32 height, T_E_UVCCAM_YUVTYPE ty)
{
	T_U32 siz;

	__CHECK_PARAM_RET(AK_NULL == p, -1);

	siz = width * height;

	__CHECK_PARAM_RET(0 == siz, -1);
	
	if (E_UVCCAM_YUVTYPE_422 == ty) // YUV422
	{
		p->dat.siz = (siz << 1) + 192;
		p->dat.y = (T_U8 *)__MYMALLOC(p->dat.siz);
		__CHECK_PARAM_RET(AK_NULL == p->dat.y, -1);
		p->dat.u = p->dat.y + siz;
		p->dat.v = p->dat.u + (siz >> 1);
	}
	
	else if (E_UVCCAM_YUVTYPE_420 == ty) // YUV420
	{
		p->dat.siz = (siz * 3 / 2) + 192;
		p->dat.y = (T_U8 *)__MYMALLOC(p->dat.siz);
		__CHECK_PARAM_RET(AK_NULL == p->dat.y, -1);
		p->dat.u = p->dat.y + siz;
		p->dat.v = p->dat.u + (siz >> 2);
	}
	
	else
	{
		return -1;
	}
	
	return 0;
}

static T_S32 Uvc_DelYuv(T_U_YUV * p)
{
	__CHECK_PARAM_RET(AK_NULL == p, -1);

	if (AK_NULL != p->yuv)
	{
		__MYFREE(p->yuv);
	}

	p->dat.y = AK_NULL;
	p->dat.u = AK_NULL;
	p->dat.v = AK_NULL;
	p->dat.siz = 0;

	return 0;
}

static T_S32 Uvc_ChangeSize(T_UVCCAM_RUNTIME_DATA *runtm)
{
	T_CAMERA_BUFFER yuv0, yuv1, yuv2;

	if (AK_SUCCESS == AK_Obtain_Semaphore(m_UVCCtrl.sem, AK_SUSPEND))
	{
		if (AK_NULL == m_UVCCtrl.current)
		{
			AK_Release_Semaphore(m_UVCCtrl.sem);
			return -1;
		}

		runtm->cfg.videoWidth = m_UVCCtrl.current->args.videoWidth;
		runtm->cfg.videoHeight = m_UVCCtrl.current->args.videoHeight;

		AK_Release_Semaphore(m_UVCCtrl.sem);
	}

	__DBG_PRINT_VAR(runtm->cfg.camWinWidth);
	__DBG_PRINT_VAR(runtm->cfg.camWinHeight);
	__DBG_PRINT_VAR(runtm->cfg.videoWidth);
	__DBG_PRINT_VAR(runtm->cfg.videoHeight);

	Uvc_SetCamBuf(&yuv0, &yuv1, &yuv2, runtm);

	camstream_change(runtm->cfg.videoWidth, runtm->cfg.videoHeight,
		&yuv0,
		&yuv1,
		&yuv2);

	return 0;
}

static T_VOID Uvc_DataNotifyCB(T_VOID)
{
	;
}

static T_VOID Uvc_VCCtrlCB(T_eUVC_CONTROL dwCtrl, T_U32 val1, T_U32 val2)
{
    __DBG_PRINT("control_CALLBACK");
	__DBG_PRINT_VAR(dwCtrl);
	__DBG_PRINT_VAR(val1);

	switch (dwCtrl)
	{
	case UVC_CTRL_BRIGHTNESS:
		cam_set_feature(CAM_FEATURE_BRIGHTNESS, val1);
		break;

	case UVC_CTRL_CONTRAST:
		cam_set_feature(CAM_FEATURE_CONTRAST, val1);
		break;

	case UVC_CTRL_SATURATION:
		cam_set_feature(CAM_FEATURE_SATURATION, val1);
		break;

	default:
		__DBG_PRINT_VAR(dwCtrl);
		break;
	}
}

static T_VOID Uvc_VSCtrlCB(T_eUVC_CONTROL dwCtrl, T_U32 val1, T_U32 val2)
{
	T_U32 ulformat, ulFrameId;
	T_UVC_FRAME_RES FrameRes;

	__DBG_PRINT("frame_CALLBACK");
	__DBG_PRINT_VAR(val1);
	__DBG_PRINT_VAR(val2);

	printf("vs ctrl cb 0x%x\n", dwCtrl);

	if (AK_SUCCESS == AK_Obtain_Semaphore(m_UVCCtrl.sem, AK_SUSPEND))
	{
		if (AK_NULL == m_UVCCtrl.current)
		{
			AK_Release_Semaphore(m_UVCCtrl.sem);
			return;
		}		

   		if (UVC_CTRL_RESOLUTION == dwCtrl)
    	{
        	ulformat = (val1>>16)&0xff;
        	ulFrameId = (val1>>24)&0xff;
        	AK_DEBUG_OUTPUT("ZZZZ:		ulFrameId-1: %d.\n", ulFrameId-1);
			uvc_get_frame_res(&FrameRes,ulFrameId);
			m_UVCCtrl.current->args.videoWidth = FrameRes.unWidth;
			m_UVCCtrl.current->args.videoHeight = FrameRes.unHeight;	
			AK_DEBUG_OUTPUT("ZZZZ:		SetVidwoW: %d, SetVideoH: %d.\n", m_UVCCtrl.current->args.videoWidth, m_UVCCtrl.current->args.videoHeight);
			AK_Set_Events(m_UVCCtrl.current->runtime.eventGroup, EVENT_CHANGE_SIZE, AK_OR);
    	}
    	else
    	{
        	printf("unsupported UVC vs control\n");
    	}		

		AK_Release_Semaphore(m_UVCCtrl.sem);
	}
}

static T_VOID Uvc_SentDataCB(T_VOID)
{
	// __DBG_PRINT("sendDataFinish_CallBack");

	if (AK_SUCCESS == AK_Obtain_Semaphore(m_UVCCtrl.sem, AK_SUSPEND))
	{
		if (AK_NULL == m_UVCCtrl.current && m_UVCCtrl.current->runtime.eventGroup <= 0)
		{
			AK_Release_Semaphore(m_UVCCtrl.sem);
			return;
		}
		
		AK_Set_Events(m_UVCCtrl.current->runtime.eventGroup, EVENT_USB_SEND_FINISH, AK_OR);

		AK_Release_Semaphore(m_UVCCtrl.sem);
	}
}

static T_S32 Uvc_UsbErr(T_VOID)
{
	T_U8 state;

	state = usb_slave_getstate();
	
	if (USB_OK != state)
		__DBG_PRINT_VAR(state);
	
	return state;
}

#endif
#endif

/** End of File */
