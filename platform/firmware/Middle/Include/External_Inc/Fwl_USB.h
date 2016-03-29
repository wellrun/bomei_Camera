#ifndef __ENG_USB_H__
#define __ENG_USB_H__

// #include "mount.h"

typedef enum {
    USB_NULL_MODE,
    USB_DISK_MODE,//slave disk
	USB_HOST_MODE, 
    USB_CAMERA_MODE,
} T_eUSB_MODE;

//access meida type
typedef enum {

    NO_MEDIA = 0 ,
    //SDRAM_DISK   ,
    NANDFLASH_DISK,
    SDCARD_DISK,
    NANDRESERVE_ZONE,
    UHOST_DISK,
    DUMMYDISK,
	MMC_DISK,
}T_ACCESS_MEDIUM;


typedef enum{
	USB_DEAL_BEGIN,
	USB_DEAL_ABORT,
	USB_DEAL_END
}T_eUSB_DEAl_PROC;

T_VOID USB_SendEvent(T_VOID);

/**
 * @brief Mount U-Disk 
 * @author 
 * @date 2012-03-29
 * @param[in] T_VOID
 * @return T_BOOL
 * @retval AK_TRUE if Mount Disk success
 * @retval AK_FALSE if Mount Disk  fail
 */
T_BOOL Fwl_UsbMountDisk(T_VOID);
/**
 * @brief To stop U-Disk Status.Stop U-Disk,Must Be Had Mounted success.
 * @author 
 * @date 2012-03-29
 * @param[in] T_VOID
 * @return T_VOID
 */
T_VOID Fwl_UsbDiskStop(T_VOID);


T_VOID Fwl_UsbInitDetectr(T_VOID);
T_VOID Fwl_UsbSetMode(T_eUSB_MODE usbMode);
T_eUSB_MODE Fwl_UsbGetMode(T_VOID);

T_VOID Fwl_UsbSetDealProc(T_eUSB_DEAl_PROC proc);
T_eUSB_DEAl_PROC Fwl_UsbGetDealProc(T_VOID);


#endif

