//nandflash common commands  refer <<ONFI_2_3a_Gold.pdf>>
//READ,Mandatory
#define NFLASH_READ_1               0x00
#define NFLASH_READ_2               0x30
//Multi-plane READ,Optional
#define NFLASH_READ_MUL_1           0x00
#define NFLASH_READ_MUL_2           0x32
//CopyBack READ,Optional
#define NFLASH_READ_BCK_1           0x00
#define NFLASH_READ_BCK_2           0x35
//Change READ column, Mandatory
#define NFLASH_READ_CHGC_1          0x05
#define NFLASH_READ_CHGC_2          0xE0
//Change READ column enHanced, Optional
#define NFLASH_READ_CHGC_EH_1       0x06
#define NFLASH_READ_CHGC_EH_2       0xE0
//READ Cache Random,Optional
#define NFLASH_READ_CACHE_R_1       0x00
#define NFLASH_READ_CACHE_R_2       0x31
//READ Cache Sequential,Optional
#define NFLASH_READ_CACHE_S_1       0x31
//READ Cache End,Optional
#define NFLASH_READ_CACHE_E_1       0x3F
//block ERASE,  Mandatory
#define NFLASH_ERASE_1              0x60
#define NFLASH_ERASE_2              0xD0
//Multi-plane ERESE,Optional
#define NFLASH_ERASE_MUL_1          0x60
#define NFLASH_ERASE_MUL_2          0xD1
//read STATUS,  Mandatory
#define NFLASH_STATUS               0x70
//read STATUS Enhanced,Optional
#define NFLASH_STATUS_EH            0x78
//page PROGram, Mandatory
#define NFLASH_PROG_1               0x80
#define NFLASH_PROG_2               0x10
//Multi-plane page PROGram,Optional
#define NFLASH_PROG_MUL_1           0x80
#define NFLASH_PROG_MUL_2           0x11
//page Cache PROGram,Optional
#define NFLASH_PROG_CACHE_1         0x80
#define NFLASH_PROG_CACHE_2         0x15
//copyBack PROGram,Optional
#define NFLASH_PROG_BCK_1           0x85
#define NFLASH_PROG_BCK_2           0x10
//Multi-plane copyBack PROGram,Optional
#define NFLASH_PROG_MUL_BCK_1       0x85
#define NFLASH_PROG_MUL_BCK_2       0x11
//small data move 0x85 0x11
//chanGe PROGram Column , Mandatory
#define NFLASH_PROG_CHGC            0x10
//chanGe PROGram Row,Optional
#define NFLASH_PROG_GHGR            0x10
//read ID, Mandatory
#define NFLASH_ID                   0x90
//read Parameter Page, Mandatory
#define NFLASH_PP                   0xEC
//read Unique ID,Optional
#define NFLASH_UID                  0xED
//Get Feature,Optional
#define NFLASH_FEATURE_GET          0xEE
//Set Feature,Optional
#define NFLASH_FEATURE_SET          0xEF
//RESET LUN,Optional
#define NFLASH_RESET_LUN            0xFA
//SYNChronous RESET,Optional
#define NFLASH_RESET_SYNC           0xFA
//RESET, Mandatory
#define NFLASH_RESET                0xFF

///////////slc extern cmd
#define NFLASH_READ22               0x50
#define NFLASH_READ1_HALF           0x01


//Status Register Definition for 70h Command
#define NFLASH_STATUS_NWP_BIT       (1 << 7) ///non protect
#define NFLASH_STATUS_READY_BIT     (1 << 6)
#define NFLASH_STATUS_FAIL_BIT0     (1 << 0)

