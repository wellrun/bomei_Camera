#ifndef __UTILS_H
#define __UTILS_H

//#include "gbl_macrodef.h"

int strcmp(const char *s1, const char *s2);
int strlen(const char * s);
void memset(T_U8 *s, T_U8 c, int n);
void memcpy(T_U8 *src, T_U8 *dst, int n);
unsigned long strtoul(char *s, char **endptr, int radix);
T_S8 Utl_StrCmpNC(T_pCSTR str1, T_pCSTR str2, T_U16 length);

#endif /* __UTILS_H */
