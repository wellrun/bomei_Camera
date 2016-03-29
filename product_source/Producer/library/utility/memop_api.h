#ifndef __MEMOP_API_H__
#define __MEMOP_API_H__

T_BOOL  Utl_MemCpy(T_pVOID strDest, T_pCVOID strSour, T_U16 count);
T_BOOL  Utl_MemSet(T_pVOID strDest, T_U8 chr, T_U16 count);
T_S8	Utl_MemCmp(T_pCVOID data1, T_pCVOID data2, T_U16 count);

T_BOOL  Utl_MemCpy32(T_pVOID strDest, T_pCVOID strSour, T_U32 count);
T_BOOL  Utl_MemSet32(T_pVOID strDest, T_U8 chr, T_U32 count);
T_S8    Utl_MemCmp32(T_pCVOID data1, T_pCVOID data2, T_U32 count);


#endif

