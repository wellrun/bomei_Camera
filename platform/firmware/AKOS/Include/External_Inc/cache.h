#ifndef        _CACHE_H_
#define        _CACHE_H_

#define  MAX_SECTOR_COUNT  64

#ifndef OS_WIN32

#define    NOP        1
#define    IN         2
#define    OUT        3
#endif

struct CACHE_INFO
{
    T_PMEDIUM medium;
    T_U32 SectorCnt1;          // 总容量，扇区个数    
    T_U32 SectorCnt2;
    T_U8  *buffer1;            // Cache缓冲区
    T_U8  *buffer2;
    T_U32 Map1[MAX_SECTOR_COUNT];    // Cache的扇区地址映射表
    T_U32 Map2[MAX_SECTOR_COUNT];
    T_U32 Index1[MAX_SECTOR_COUNT];  // 排序后， Cache中各扇区对应的数据ID
    T_U32 Index2[MAX_SECTOR_COUNT];
    T_U32 status1;
    T_U32 status2;
    T_BOOL byPass;             // 是否使用cache
};


typedef struct CACHE_INFO  T_CACHE_INFO;
typedef struct CACHE_INFO * T_PCACHE_INFO;

T_PMEDIUM Cache_Create(T_PMEDIUM medium, T_U32 sector_count);
T_BOOL  Cache_Flush(T_PMEDIUM medium);
T_U32   Cache_Write(T_PMEDIUM medium, const T_U8 *buf, T_U32 start, T_U32 size);
T_U32   Cache_Read(T_PMEDIUM medium, T_U8* buf, T_U32 start, T_U32 size);
T_VOID  Cache_SetbyPass(T_PMEDIUM medium, T_BOOL byPass);
T_pVOID Cache_Destroy(T_PMEDIUM medium);

#endif

