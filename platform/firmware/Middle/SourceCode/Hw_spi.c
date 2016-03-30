/*******************************************************************************
Copyright(R) 2015-2025, Bomei Co., Ltd
File Name: hw_spi.c
Author: Danel           Date: 2016-03-30             Version: 1.0
Discription:

Function:

History:
1. Version: 1.0        Date: 2016-03-30         Author: Karsn
    Modification:
      Creation.
*******************************************************************************/

#include "drv_in_callback.h"
#include "Eng_Debug.h"
#include "drv_api.h"

#define HW_BOMEI_SPI (T_U8)0
#define HW_BOMEI_SPI_BR 6000000 //8MHz
#define HW_BOMEI_SPI_CS 43			//GPIO43ÎªÆ¬Ñ¡½Å

T_VOID  Bomei_HwspiInit(T_VOID)
{
	gpio_set_pin_dir(HW_BOMEI_SPI_CS,GPIO_DIR_OUTPUT); //SPI_CS Output
	gpio_set_pin_level(HW_BOMEI_SPI_CS,GPIO_LEVEL_HIGH);
	
	if(!gb_drv_cb_fun.spi_init(HW_BOMEI_SPI, 0, 1, HW_BOMEI_SPI_BR))
    {
    	AkDebugOutput("nrfHWInit() failed!\n"); //only for debug
    }
}

T_VOID BomeiHWSpiCtrl(T_BOOL nStatus)
{
	gpio_set_pin_dir(HW_BOMEI_SPI_CS,GPIO_DIR_OUTPUT); //Config CS Output
	
	if(nStatus == AK_TRUE)
 	{
 		gpio_set_pin_level(HW_BOMEI_SPI_CS,GPIO_LEVEL_LOW);
 	}
 	else
 	{
 		gpio_set_pin_level(HW_BOMEI_SPI_CS,GPIO_LEVEL_HIGH);
 	}
}


T_BOOL BomeiHWSpiRead(T_U8 *pBuf, T_U8 nLen)
{
	T_BOOL lu16v_Status;
	
    lu16v_Status = gb_drv_cb_fun.spi_master_read(HW_BOMEI_SPI, pBuf, nLen, AK_FALSE);
    
	return lu16v_Status;
}


T_BOOL BomeiHWSpiWrite(T_U8 const *pBuf, T_U8 nLen)
{
	T_BOOL  lu16v_Status;
	lu16v_Status = gb_drv_cb_fun.spi_master_write(HW_BOMEI_SPI, pBuf, nLen, AK_FALSE); 
	return lu16v_Status;
}
