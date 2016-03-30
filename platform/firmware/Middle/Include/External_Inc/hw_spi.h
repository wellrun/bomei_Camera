#ifndef _HW_SPI_H_
#define _HW_SPI_H_

#include "anyka_types.h"

extern T_VOID Bomei_HwspiInit(T_VOID);
extern T_VOID BomeiHWSpiCtrl(T_BOOL nStatus);
extern T_BOOL BomeiHWSpiRead(T_U8 *pBuf, T_U8 nLen);
extern T_BOOL nrfHWWrite(T_U8 const *pBuf, T_U8 nLen);

#endif 