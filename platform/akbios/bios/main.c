
#include "akdefine.h"
#include "gbl_global.h"
#include "bios.h"
#include "utils.h"
#include "gpio.h"
#include "lcd.h"
#include "Fwl_osMalloc.h"
#include "ramtofla.h"
#include "anyka_types.h"
#include "hal_timer.h"
#include "drv_gpio.h"
#include "hal_timer.h"
#include "fha.h"
#include "file.h"
#include "fs.h"


#define        BIOS_PIC_NAME    "a:/menubyakbmp2"
#define        MAX_PIC_SIZE    (320*240*3+4096)
static      T_U8 normal_or_bios;
void Welcome();
void download(void);
void go(void);
void reboot(void);
void help(void);
void dump (void);
void version(void);
void setvalue(void);
void usb_disk();
void loadmmi(T_U8* MMI_path, T_U8* RES_path);
void getid();
void test();
void storepassword();
void Parse_command (char * buf);
void load_mmi(void);

extern T_VOID lcd_device_init(T_VOID);
T_U32 battery_get_voltage_value(T_VOID);


T_BOOL gb_bSDAvail = AK_FALSE;

const unsigned char gb_Bmppoweron[] = {
    #include "bmp_poweron.txt"
};



static struct Com_type Command[]=
{
    {"u",usb_disk},
    {"usbdisk",usb_disk},
    {"version",version},
    {"download",download},
    {"go",go},
    {"reboot",reboot},
    {"dump",dump},
    {"setvalue",setvalue},
    {"help",help},
    {"loadmmi",load_mmi},    
    {"$get_id$",getid},
    {"$test$",    test},
    {"$store_passwd$",storepassword},
    {"" ,AK_NULL}
};

void help(void)
{
    printf("help        : help you to use this bios! :-)\n");
    printf("version     : show version\n");
    printf("setvalue    : set adress xxx 's value is xxx\n");
    printf("dump        £ºshow the data of memory\n");
    printf("go          £ºstart program for the address you input\n");
    printf("download    £ºdownload program(data)to address you input\n");
    printf("reboot      : reboot the system\n");
    printf("usbdisk     : enter usb disk function\n");
    printf("loadmmi     : load and run mmi\n");
     return;
}

void load_mmi(void)
{
    loadmmi((T_U8*)MMI_NAME, (T_U8*)MMI_BACK_NAME);
}

void version(void)
{
    printf("Current bios version is "BIOS_VERSION"\n");
}

void Parse_command (char * buf)
{
    int i;
    int len = strlen(buf);

    for (i = len -1 ;i >= 0 ; i--)
    {
        if(buf[i] == ' ')
            buf[i] = 0;
        else
            break;
    }
    if (len  == 0 )
        return;
    for(i = 0; i < ComNum ; i++)
    {
        if (!strcmp(Command[i].name,buf))
        {
              Command[i].function();
               return;
        }
    }

    printf("Command '%s' error!\nplease use help to view the command list!\n");
    return;
}

void hardware_jump_table()
{
    /* PC = PC + 8 + (signed_immed_24)<<2 */
    #define LOC_CODE(addr) (0xea000000 | ((addr-RAM_BASE_ADDR-8)>>2))
    
    extern T_U32 Image$$ER_RO$$Base;
    T_U32 bios_start_addr = &Image$$ER_RO$$Base;    
    T_U32 *des_addr = (T_U32 *)RAM_BASE_ADDR;
    T_U32 i;
    
    printf("bios run at 0x%x\n", bios_start_addr);
    
    if (bios_start_addr > RAM_BASE_ADDR)
    {
        for(i=0;i<8;i++)
        {
            *(des_addr+i) = LOC_CODE(bios_start_addr);
        }
    }
}

#define USB_FLUSH_INTERVAL   (1000)
void usb_disk()
{
    T_U8 c,ret;
    T_U32 mode = 0;
    T_U32 refTickCnt = 0,curTickCnt = 0;
    T_U8 curStatus = 0;
    mode = USB_MODE_20;

    if(!usbdisk_init(mode))
    {
        printf("usb disk init error\n");
        return;
    }

    if(!usbdisk_set_str_desc(STR_SERIAL_NUM,"USB DEVICE"))
    {
        printf("usb disk usbdisk_set_str_desc error\n");
    }

    //set access media type
    printf("###############################\n");
    printf("select access disk type\n");

    printf("0. Mount system reserve zone (Press any key)\n");
    printf("1. Mount user disk\n");
    printf("2. Mount all disk\n");

    if (normal_or_bios == BIOS_NORMAL)
    {
        c = getch();
    }
    else
    {
        c = '2';
    }
    Usb_MediumInfoInit();
    switch( c )
    {
        case '0':
#if (defined (NANDBOOT))             
            usb_MountLUN(NANDRESERVE_ZONE);
#else
            usb_MountLUN(SDCAED_ZONE);
#endif
            break;
        case '1':
#if (defined (NANDBOOT))                 
           usb_MountLUN(NANDFLASH_DISK);
#else
           usb_MountLUN(SDCARD_DISK);
#endif           
            break;
        case '2':
#if (defined (NANDBOOT))                 
            usb_MountLUN(NANDRESERVE_ZONE);
            usb_MountLUN(NANDFLASH_DISK);
#else
            usb_MountLUN(SDCAED_ZONE);
            usb_MountLUN(SDCARD_DISK);
#endif
            break;
        default:
#if (defined (NANDBOOT))            
            usb_MountLUN(NANDRESERVE_ZONE);
#else
            usb_MountLUN(SDCAED_ZONE);
#endif
            break;
    }
    
    Usb_DeInitFs();

    if(!usbdisk_start())
    {
        printf("usb disk start error\n");
        Usb_reInitFs();
        return;
    }

    
    vtimer_init();

    printf("config usbdisk...\npls insert usb cable and pls wait ...\n");

    refTickCnt = get_tick_count();
    while( 1 )
    {
        usbdisk_proc();
        
        curStatus = usb_slave_getstate();    
        if (USB_CONFIG == curStatus)
        {
            continue;
        }
        else if (USB_SUSPEND == curStatus)
        {
            printf("[UsbSlave]Status = %d\n",curStatus);
            break;
        }

        curTickCnt = get_tick_count();
        if ((curTickCnt > refTickCnt) \
             && ((curTickCnt -refTickCnt) > USB_FLUSH_INTERVAL))
        {
            Usb_FlushAllDisk(AK_FALSE);
            refTickCnt = curTickCnt;
        }
        
    }
    
    Usb_FlushAllDisk(AK_TRUE);
    Usb_reInitFs();
    usbdisk_stop();    
    vtimer_free();
    printf("\r\n#===================usb done!!!===========================================#\n");
    
}

T_VOID check_startup(T_VOID)
{
    T_U8 key_press;
    
    //store the switch key status on startup
    gpio_set_pin_dir(GPIO_SWITCH_KEY, GPIO_DIR_INPUT);
    key_press = gpio_get_pin_level(GPIO_SWITCH_KEY);

    printf("**BIOS check power on reason\n");

    gpio_set_pin_dir(GPIO_POWER_OFF, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_POWER_OFF, gpio_pin_get_ActiveLevel(GPIO_POWER_OFF));
        
#ifdef WM_SIMCOM         
    if (gpio_get_pin_level(GPIO_USB_DETECT) == gpio_pin_get_ActiveLevel(GPIO_USB_DETECT)
        && gpio_get_pin_level(GPIO_CHARGING) == gpio_pin_get_ActiveLevel(GPIO_CHARGING))
    {
        //USB cable plug in
    printf("**USB up\n");
        SYS_STATE_FLAG = SYS_STATE_USB;
    }
    if (gpio_get_pin_level( GPIO_SWITCH_KEY ) == gpio_pin_get_ActiveLevel(GPIO_SWITCH_KEY))
    {
    printf("**Keypad Startup\n");
        SYS_STATE_FLAG = SYS_STATE_POWER_KEY;
        gpio_set_pin_level(GPIO_MODULE_IGT, gpio_pin_get_ActiveLevel(GPIO_MODULE_IGT));
    }
    else
    {
    printf("**Alarm Startup\n");
        SYS_STATE_FLAG = SYS_STATE_ALARM;
    }    
#endif

#ifdef WM_INFINEON
    mini_delay(500); //wait a delay for module startup
        
    SYS_STATE_FLAG = get_poweron_reason();

    if (gpio_get_pin_level(GPIO_USB_DETECT) == gpio_pin_get_ActiveLevel(GPIO_USB_DETECT)
        && gpio_get_pin_level(GPIO_CHARGING) == gpio_pin_get_ActiveLevel(GPIO_CHARGING))
    {
        //USB cable plug in
    printf("**USB up\n");
        SYS_STATE_FLAG = SYS_STATE_USB;
    }
    else if (SYS_STATE_FLAG == SYS_STATE_POWER_KEY)
    {
        //if start up with power key, check it
        if (gpio_get_pin_level(GPIO_SWITCH_KEY) != gpio_pin_get_ActiveLevel(GPIO_SWITCH_KEY)
            && key_press != gpio_pin_get_ActiveLevel(GPIO_SWITCH_KEY))
        {
            //lock switch key
            gpio_set_pin_dir(GPIO_SWITCH_KEY, GPIO_DIR_OUTPUT);
            gpio_set_pin_level(GPIO_SWITCH_KEY, 1 - gpio_pin_get_ActiveLevel(GPIO_SWITCH_KEY));

    printf("press switch key too short,close mobile!!\n");

            shut_module();
            mini_delay(100);
            while(1)
            {
                gpio_set_pin_level(GPIO_POWER_OFF, 1 -gpio_pin_get_ActiveLevel(GPIO_POWER_OFF));
            }    
        }
        else 
        {
    printf("**Keypad Startup\n");
        }
    }
    else if (SYS_STATE_FLAG == -1)
    {    
        //lock switch key
        gpio_set_pin_dir(GPIO_SWITCH_KEY, GPIO_DIR_OUTPUT);
        gpio_set_pin_level(GPIO_SWITCH_KEY, 1 - gpio_pin_get_ActiveLevel(GPIO_SWITCH_KEY));

    printf("**Invalid Startup\n");

        shut_module();
        mini_delay(100);
        while(1)
        {
            gpio_set_pin_level(GPIO_POWER_OFF, 1 -gpio_pin_get_ActiveLevel(GPIO_POWER_OFF));
        }
    }    
#endif

#ifdef WM_INFINEON_ULC2
    SYS_STATE_FLAG = 0;
    if (gpio_get_pin_level(GPIO_SWITCH_KEY) == gpio_pin_get_ActiveLevel(GPIO_SWITCH_KEY))
    {
    printf("**Keypad Startup\n");
        SYS_STATE_FLAG = SYS_STATE_POWER_KEY;
    }    
    else if (gpio_get_pin_level(GPIO_CHARGING) == gpio_pin_get_ActiveLevel(GPIO_CHARGING))
    {
        //USB or charger cable plug in
    printf("**USB or Charger Startup\n");
        SYS_STATE_FLAG = SYS_STATE_USB;
    }
    else if(1)
    {
    printf("**Modem RTC  Startup\n");
        SYS_STATE_FLAG = SYS_STATE_ALARM;
    }
    else if (SYS_STATE_FLAG == -1)
    {
        //lock switch key
        gpio_set_pin_dir(GPIO_SWITCH_KEY, GPIO_DIR_OUTPUT);
        gpio_set_pin_level(GPIO_SWITCH_KEY, 1 - gpio_pin_get_ActiveLevel(GPIO_SWITCH_KEY));

    printf("**Invalid Startup\n");
        
        shut_module();
        mini_delay(100);
        while(1)
        {
            gpio_set_pin_level(GPIO_POWER_OFF, 1 -gpio_pin_get_ActiveLevel(GPIO_POWER_OFF));
        }
    }    
#endif    

    printf("Power reason: Addr:%d, SYS_STATE_FLAG:%d\r\n", &SYS_STATE_FLAG, SYS_STATE_FLAG);
}


T_VOID CMain(T_VOID)
{    
    T_S8 buf[128];
    T_S32 keypad_value;
    T_DRIVE_INITINFO drv_info;
    T_U32 Bat_Value;

    MMU_Init(_MMUTT_STARTADDRESS);

#ifdef CHIP_AK3771
    drv_info.chip = CHIP_3771;
#elif CHIP_AK3753
    drv_info.chip = CHIP_3753;
#elif CHIP_AK3750
    drv_info.chip = CHIP_3771;
#else
 #error "No define CHIP_37XX"
#endif

    drv_info.fRamAlloc = Fwl_MallocAndTrace;
    drv_info.fRamFree = Fwl_FreeAndTrace;
    
    drv_init(&drv_info);
    vtimer_init();
    sysctl_clock(0);

    gpio_set_pin_as_gpio(GPIO_POWER_OFF);
    gpio_set_pin_dir(GPIO_POWER_OFF,GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_POWER_OFF, GPIO_LEVEL_HIGH);
    rtc_set_wpinLevel(1);

    hardware_jump_table();

    //init console
#ifdef CHIP_AK3771    
    console_init(uiUART1, CONSOLE_UART, UART_BAUD_115200);
#else
    console_init(uiUART0, CONSOLE_UART, UART_BAUD_115200);
#endif
    printf("Bios start , ver=%s\r\n",BIOS_VERSION);
    
    Bat_Value = battery_get_voltage_value();
    printf("Bat_Value = %d\n",Bat_Value);
    if(Bat_Value <= 3420)  //3.42v
    {
        printf("Bat_Value = %d, battery value too low,stop boot...\n",
            Bat_Value);
        while(1)
        {            
            gpio_set_pin_level(GPIO_POWER_OFF, GPIO_LEVEL_LOW);
            rtc_set_wpinLevel(0);
        }
    }    
    

    set_pll_value(280);//DEFLAT_PLL_FREQ
    set_asic_freq(140*1000000L);
    printf("ASIC clock is %d\n", get_asic_freq());    

    Fwl_MallocInit();

    if (!Fwl_InitFs())
    {
        printf("disk init error, system halt!\n");
        while(1);
    }    

#if ((defined (CHIP_AK3771)) || (defined (CHIP_AK3750)))

    keypad_init(AK_NULL, (T_U32)keypad_get_platform_type(), (const T_VOID*)keypad_get_platform_parm());
    mini_delay(10);
    
    keypad_value = keypad_scan();

#elif  (defined (CHIP_AK3753))
#define KEY_AD_VALUE  211
#define KEY_AD_VALUE_RANG  20
    keypad_value = analog_getvalue_ad5();
    keypad_value += analog_getvalue_ad5();
    keypad_value >>= 1;
    
    if((keypad_value > KEY_AD_VALUE - KEY_AD_VALUE_RANG) && 
        (keypad_value < KEY_AD_VALUE + KEY_AD_VALUE_RANG))
    {
        keypad_value = kbUP;
    }
    else
    {
        keypad_value = -1 ;
    }
    
#undef KEY_AD_VALUE
#endif


    printf("keypad_value=%d\n", keypad_value);

    if ( keypad_value == kbUP)
    {
        normal_or_bios = BIOS_NORMAL;
    }
    else if (keypad_value == kbOK)
    {
        normal_or_bios = BIOS_AUTORUN;
    }
    else
    {
        vtimer_free();
        load_mmi();
        /* load mmi failed, turn to debug mode */
        normal_or_bios = BIOS_NORMAL;
    }
    
    //for debug mode, show tips
    //lcd_clean(LCD_0, 0, 0, MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT, COLOR_BLUE);
    //lcd_refresh(LCD_0, 0, 0, MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT);    

    Welcome();          
    
    while(1)
    {   
        if( normal_or_bios == BIOS_NORMAL )
        {
            printf("bios_no_usb>#");
            memset((T_U8 *)buf, 0, sizeof(buf));
            gets(buf, sizeof(buf) - 1);
            Parse_command((char*)buf);
        }
        else if (normal_or_bios == BIOS_AUTORUN)
        {
            printf("BIOS auto to mount disk\n");
            usb_disk();
        }
        else
        {
            printf("Error bios!\n");
            while(1);
        }       
    }
}

T_VOID Welcome(T_VOID)
{
    printf("\n #======================================================================#\n");
    printf(" #                                                                      #\n");
    printf(" #       OOOOOOOOOOOO        OOO                                        #\n");
    printf(" #        OO  OO  OO          OO                                        #\n");
    printf(" #        OO  OO  OO          OO                                        #\n");
    printf(" #        OO  OO  OO   OOOO   OO   OOO    OOO              OOOO         #\n");
    printf(" #         OO OOOOO   OO  OO  OO  OO OO  OO OO OOOOOOOOO  OO  OO        #\n");
    printf(" #         OOOOOOOO   OOOOOO  OO OO     OO   OO OO OO  OO OOOOOO        #\n");
    printf(" #         OOOOOOOO   OO      OO OO     OO   OO OO OO  OO OO            #\n");
    printf(" #         OOO OOO    OO      OO OO     OO   OO OO OO  OO OO            #\n");
    printf(" #          OO  OO    OO  OO  OO OO  OO  OO OO  OO OO  OO OO  OO        #\n");
    printf(" #          OO  OO     OOOO  OOOO OOOO    OOO  OOOOOOOOOOO OOOO         #\n");
    printf(" #                                                                      #\n");
    printf(" #====================Welcome to use AK3224M_bios=======================#\n");
    printf(" |            ANYKA (GUANGZHOU) SOFTWARE TECHNOLOGY CO., LTD.           |\n");
    printf(" |                                                                      |\n");
    printf(" |            AK322L BIOS Version: "BIOS_VERSION"\n");
    printf(" |                                                                      |\n");
    printf(" |            Board info: "HD_VERSION"\n");
    printf(" |----------------------------------------------------------------------|\n");
}

