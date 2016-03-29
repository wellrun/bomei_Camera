#ifndef  __VA_TYPES_H__
#define  __VA_TYPES_H__


typedef        unsigned char       T_U8;         // unsigned 8 bit integer
typedef        signed char         T_S8;         // signed 8 bit integer

typedef        unsigned short      T_U16;        // unsigned 16 bit integer
typedef        signed short        T_S16;        // signed 16 bit integer

typedef        unsigned long       T_U32;        // unsigned 32 bit integer
typedef        signed long         T_S32;        // signed 32 bit integer

typedef        char                T_CHAR;       // char
typedef        short               T_SHORT;      // short
typedef        int                 T_INT;        // int

typedef        float               T_FLOAT;      // float
typedef        double              T_DOUBLE;     // double
typedef        void                T_VOID;       // void
typedef        void *              T_pVOID;      // void *   

typedef        T_U8                T_BYTE;       // byte
typedef        T_U8               T_BOOL;       // bool

typedef        T_U32               T_COLOR;
typedef        T_S16               Fixed16; 
typedef        T_S32               Fixed32;  
typedef	       T_U8 *	             T_pDATA;	     // pointer of data

#define     AK_FALSE       0
#define     AK_TRUE        1
#define 	AK_NULL				((T_pVOID)(0))

#if 0
#if defined(_MSC_VER)
typedef _int64           T_S64;
typedef unsigned _int64  T_U64;
#elif defined(__GNUC__)
typedef long long        T_S64;
typedef unsigned long long T_U64;
#else
#error "can't define 64-bit base type!"
#endif
#endif


#endif
