/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKWnd.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKWND_H__
#define __AKWND_H__

#include "AKComponent.h"
#include "AKInterface.h"

typedef struct IWnd IWnd;

typedef enum
{
	AK_DISP_RGB,
	AK_DISP_YUV,
	AK_DISP_NUM
} T_DISP_TYPE;

typedef struct
{
    T_DISP_TYPE type;
    T_LEN width;
    T_LEN height;
    T_U8  *dispbuf;    
}T_WND_INITPARAM;


/*************************************************************************/
/**
                       <---- DESCRIPTION ---->
#define AK_INHERIT_IWND(name) \
        AK_INHERIT_IUNKNOWN(name); \
        //Add your function pointer list
        //of virtual table here...
If your extention doest inherit from IUnknown, you can use the Inherit 
macro AK_INHERIT_IYyy instead of AK_INHERIT_IUNKNOWN. For ezample:

#define AK_INHERIT_ICAT(name) \
        AK_INHERIT_IANIMAL(name); \
        ...    ...

*/
/*************************************************************************/
#define AK_INHERIT_IWND(name) \
      AK_INHERIT_IUNKNOWN(name)

AK_INTERFACE(IWnd)
{
    AK_INHERIT_IWND(IWnd);
};

struct IWnd
{
    AK_DEFINEVT(IWnd);
    T_VOID *pData;
};

T_S32 CWnd_New(IWnd **pp, T_WND_INITPARAM *param);

#define IWnd_AddRef(p)                  AK_GETVT(IWnd, p)->AddRef(p)
#define IWnd_Release(p)                 AK_GETVT(IWnd, p)->Release(p)

#endif //__AKWND_H__

