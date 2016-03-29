
#ifndef __WINE_WINE_UNICODE_H
#define __WINE_WINE_UNICODE_H

typedef unsigned short      WCHAR;

//#define WC_DISCARDNS         0x0010
//#define WC_SEPCHARS          0x0020
//#define WC_DEFAULTCHAR       0x0040
#ifdef OS_ANYKA
#ifndef WC_COMPOSITECHECK
#define WC_COMPOSITECHECK    0x0200
#endif
//#define WC_NO_BEST_FIT_CHARS 0x0400

#ifndef MB_PRECOMPOSED
#define MB_PRECOMPOSED              0x01
#endif
#endif
//#define MB_COMPOSITE                0x02
//#define MB_USEGLYPHCHARS            0x04
//#define MB_ERR_INVALID_CHARS        0x08


/* code page info common to SBCS and DBCS */
struct cp_info
{
    unsigned int          codepage;          /* codepage id */
    unsigned int          char_size;         /* char size (1 or 2 bytes) */
    WCHAR                 def_char;          /* default char value (can be double-byte) */
    WCHAR                 def_unicode_char;  /* default Unicode char value */
    //const char           *name;              /* code page name */
    char           name[100];              /* code page name */
};

struct sbcs_table
{
    struct cp_info        info;
    WCHAR          *cp2uni;            /* code page -> Unicode map */
    unsigned char  *uni2cp_low;        /* Unicode -> code page map */
    unsigned short *uni2cp_high;
};

struct dbcs_table
{
    struct cp_info        info;
    WCHAR          *cp2uni;            /* code page -> Unicode map */
    unsigned char  *cp2uni_leadbytes;
    unsigned short *uni2cp_low;        /* Unicode -> code page map */
    unsigned short *uni2cp_high;
    unsigned char         lead_bytes[12];    /* lead bytes ranges */
};

union cptable
{
    struct cp_info    info;
    struct sbcs_table sbcs;
    struct dbcs_table dbcs;
};

typedef enum _CODEPAGE_ {
    CP_NULL = 0,
    CP_037 = 37,
    CP_424 = 424,
    CP_437 = 437,
    CP_500 = 500,
    CP_737 = 737,
    CP_775 = 775,
    CP_850 = 850,
    CP_852 = 852,
    CP_855 = 855,
    CP_856 = 856,
    CP_857 = 857,
    CP_860 = 860,
    CP_861 = 861,
    CP_862 = 862,
    CP_863 = 863,
    CP_864 = 864,
    CP_865 = 865,
    CP_866 = 866,
    CP_869 = 869,
    CP_874 = 874,
    CP_875 = 875,
    CP_878 = 878,
    CP_932 = 932,
    CP_936 = 936,
    CP_949 = 949,
    CP_950 = 950,
    CP_1006 = 1006,
    CP_1026 = 1026,
    CP_1250 = 1250,
    CP_1251 = 1251,
    CP_1252 = 1252,
    CP_1253 = 1253,
    CP_1254 = 1254,
    CP_1255 = 1255,
    CP_1256 = 1256,
    CP_1257 = 1257,
    CP_1258 = 1258,
    CP_10000 = 10000,
    CP_10006 = 10006,
    CP_10007 = 10007,
    CP_10029 = 10029,
    CP_10079 = 10079,
    CP_10081 = 10081,
    CP_20127 = 20127,
    CP_20866 = 20866,
    CP_20932 = 20932,
    CP_21866 = 21866,
    CP_28591 = 28591,
    CP_28592 = 28592,
    CP_28593 = 28593,
    CP_28594 = 28594,
    CP_28595 = 28595,
    CP_28596 = 28596,
    CP_28597 = 28597,
    CP_28598 = 28598,
    CP_28599 = 28599,
    CP_28600 = 28600,
    CP_28603 = 28603,
    CP_28604 = 28604,
    CP_28605 = 28605,
    CP_28606 = 28606
} T_CODE_PAGE;

const union cptable *wine_cp_get_table( unsigned int codepage );
int wine_cp_wcstombs( const union cptable *table, int flags,
                      const WCHAR *src, int srclen,
                      char *dst, int dstlen, const char *defchar, int *used );
int wine_cp_mbstowcs( const union cptable *table, int flags,
                      const char *s, int srclen,
                      WCHAR *dst, int dstlen , const unsigned short *defchar);

#endif
