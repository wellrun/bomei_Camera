#ifndef __BIOS_H
#define __BIOS_H

struct Com_type
{
    char name[16];
    void (*function)(void);
    
};

#define     BIOS_VERSION                "V2.4.09"
#define     HD_VERSION                  "AK3771_MB_V1"


#define     MMI_NAME                    "a:/Sword37.bin"
#define     MMI_BACK_NAME               "c:/Sword37.bin"
//#define 	RES_NAME					"a:/AkResData.bin"
//#define 	RES_BACK_NAME				"c:/AkResData.bin"

#define     BIOS_AUTORUN                0
#define     BIOS_NORMAL                 1
#define     BIOS_WITH_USB               2

#define     DEFAULT_ROM_ADDRESS         0x10000000    // NOR flash rom
#define     DEFAULT_RAM_ADDRESS         0x30000000    // SRAM-like
#define 	DEFAULT_RES_ADDRESS			0x31a00000	//Resource address
#define 	DEFAULT_MMI_ADDRESS			0x30000000	// SRAM-like

#define     KEY_LIST_NUM                3
#define     ComNum                      20

//#define SYS_STATE_FLAG            *( volatile T_U32* )0x30fffffc
#ifdef NANDBOOT
    #ifdef SDRAM_8M
        #define SYS_STATE_FLAG          *( volatile T_U32* )0x807ffffc
    #else    
      #ifdef SDRAM_32M
        #define SYS_STATE_FLAG          *( volatile T_U32* )0x81fffffc
      #else
        #define SYS_STATE_FLAG          *( volatile T_U32* )0x80fffffc
      #endif
  #endif
#else
    #define SYS_STATE_FLAG              *( volatile T_U32* )0x807ffffc
#endif

#define SYS_STATE_POWER_KEY             1    //Switch on by power-key
#define SYS_STATE_CHARGING              2    //Switch on by charger
#define SYS_STATE_USB                   3    //Switch on by USB cable
#define SYS_STATE_ALARM                 4    //Switch on by Alarm

#define POWERON_PIC_WIDTH               480
#define POWERON_PIC_HEIGHT              272
#define POWERON_BACKGROUND_COLOR        COLOR_WHITE

#endif  /* __BIOS_H */
