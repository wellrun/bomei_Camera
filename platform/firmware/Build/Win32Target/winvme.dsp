# Microsoft Developer Studio Project File - Name="winVME" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=winVME - Win32 PDA Debug MemoryLeak
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "winvme.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "winvme.mak" CFG="winVME - Win32 PDA Debug MemoryLeak"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "winVME - Win32 PDA Debug MemoryLeak" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "winVME"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "winVME___Win32_PDA_Debug_MemoryLeak"
# PROP BASE Intermediate_Dir "winVME___Win32_PDA_Debug_MemoryLeak"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "winVME___Win32_PDA_Debug_MemoryLeak"
# PROP Intermediate_Dir "winVME___Win32_PDA_Debug_MemoryLeak"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\vmesdk\include" /I "..\vmesdk\driver\display_API" /I "..\source_code" /I "..\source_code\include" /I "..\source_code\AK_APL\m_statemachine" /I "..\source_code\M3PLATFORM\include" /I "..\source_code\ak_wnd\include" /I "..\source_code\ak_utl\include" /I "..\SOURCE_CODE\ps_lib" /I "..\SOURCE_CODE\mms_lib" /I "..\SOURCE_CODE\avi_lib" /I "..\SOURCE_CODE\wap_lib" /I "..\SOURCE_CODE\smcore_lib" /I "..\SOURCE_CODE\memory_lib" /I "..\SOURCE_CODE\ffs_lib" /I "..\SOURCE_CODE\image_lib" /I "..\SOURCE_CODE\IP_Lib" /I "..\SOURCE_CODE\ime_lib" /I "..\SOURCE_CODE\utl_lib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "OS_WIN32" /D "DEBUG_TRACE_MEMORY_LEAK" /D "DEBUG_TRACE_LINE" /FR /FD /GZ /Zm100 /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GX /ZI /Od /I "OS_WIN32" /I "DEBUG_TRACE_MEMORY_LEAK" /I "CI7802_PLATFORM" /I "..\..\AKOS/Include/External_Inc" /I "..\..\AKOS/Include/Internal_Inc" /I "..\..\drivers/Include/External_Inc" /I "..\..\drivers/Include/Internal_Inc" /I "..\../PubInclude" /I "..\..\middle/Include/External_Inc" /I "..\..\middle/Include/Internal_Inc" /I "..\..\drivers/simulator" /D "OS_WIN32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "MMI_PDA" /D "..\..\AKOS/Include" /D "..\..\drivers/Include" /D "..\../PubInclude" /D "NANDBOOT" /D "DEBUG_TRACE_LINE" /D "DEBUG_OUTPUT" /D FONTLIB_VERSION=4 /D SDRAM_MODE=16 /D INSTALL_GAME=1 /D "CAMERA_SUPPORT" /D "UI_USE_ICONMENU" /D BOARD_MODE=3 /D "BACKLIGHT_PWM" /D LCD_CONFIG_WIDTH=320 /D "MBLTV_STD_CMMB" /D "CHIP_AK3771" /D "CI37XX_PLATFORM" /D "SUPPORT_EBK" /D "SUPPORT_FM" /D "SUPPORT_VIDEOPLAYER" /D "SUPPORT_AUDIOPLAYER" /D "SUPPORT_AUDIOREC" /D "SUPPORT_IMG_BROWSE" /D "SUPPORT_EMAP" /D "SUPPORT_CALC" /D "INSTALL_GAME" /D "INSTALL_GAME_7COLOR" /D "INSTALL_GAME_RECT" /D "INSTALL_GAME_PIGBOAT" /D "SUPPORT_TOOLBOX" /D "SUPPORT_EXPLORER" /D "SUPPORT_SYS_SET" /D "SUPPORT_VFONT" /FR /FD /I /GZ /Zm100 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib memory_ml.lib image_ml.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\vmesdk\winvme" /libpath:"..\SOURCE_CODE\memory_lib" /libpath:"..\SOURCE_CODE\image_lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"LIBCD.lib" /nodefaultlib:"LIBCMTD.lib" /nodefaultlib:"MSVCRTD.lib" /pdbtype:sept /libpath:"..\..\library\memory" /libpath:"..\..\library\image /ignore:4048"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
# Begin Target

# Name "winVME - Win32 PDA Debug MemoryLeak"
# Begin Group "AKOS"

# PROP Default_Filter ""
# Begin Group "AkosInclude"

# PROP Default_Filter ""
# Begin Group "Akos_Ex_Inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_analog.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_camera.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_dac.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_freq.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_gui.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_htimer.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_i2c.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_i2s.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_init.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_interrupt.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_lcd.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_mmc_sd.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_mmu.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_nand.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_pwm.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_rtc.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\arch_uart.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\asa.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\cache.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\driver.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\drv_api.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\drv_gpio.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\drv_init_callback.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\external_rtc.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\file.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\fs.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_camera.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_detector.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_except.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_fm.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_gpio.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_keypad.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_lcd.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_print.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_sound.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_sysdelay.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_timer.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_ts.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_usb_h_disk.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_usb_s_disk.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_usb_s_state.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\hal_usb_s_UVC.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\l2_cache.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\Mbltv_DRV.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\mem_api.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\mount.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\mount_pub_api.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\mtdlib.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\nandflash.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\paddle_init.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\raminit.h
# End Source File
# End Group
# Begin Group "Akos_In_Inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\AKOS\Include\Internal_Inc\arch_sdio.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\Internal_Inc\arch_spi.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\Internal_Inc\drv_sccb.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\Internal_Inc\drv_ts_cap.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\Internal_Inc\hal_spiflash.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\Internal_Inc\hal_usb_s_anyka.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\Internal_Inc\hal_usb_s_camera.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\Internal_Inc\hal_usb_s_debug.h
# End Source File
# End Group
# End Group
# Begin Group "AkosLibrary"

# PROP Default_Filter ""
# Begin Group "filesystem"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\AKOS\Library\filesystem\fat32fs_FS_TEST_FUNC_SINGLE.lib
# End Source File
# End Group
# Begin Group "memory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\AKOS\Library\memory\memory.lib
# End Source File
# End Group
# End Group
# Begin Group "SourceCode"

# PROP Default_Filter ""
# Begin Group "paddleInit"

# PROP Default_Filter ""
# End Group
# Begin Group "drv_init_cb"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\AKOS\SourceCode\drv_init_cb\drv_init_callback.c
# End Source File
# End Group
# Begin Group "ram"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\AKOS\SourceCode\ram\raminit.c
# End Source File
# End Group
# Begin Group "nand"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\AKOS\SourceCode\nand\fwl_nandflash.c
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\SourceCode\nand\fwl_nandflash.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\SourceCode\nand\mount.c
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\SourceCode\nand\mount_pub_api.c
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\SourceCode\nand\nand.c
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\SourceCode\nand\nand_list.h
# End Source File
# End Group
# End Group
# End Group
# Begin Group "Drivers"

# PROP Default_Filter ""
# Begin Group "DrvInclude"

# PROP Default_Filter ""
# Begin Group "Drv_Ex_Inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Drivers\Include\External_Inc\anyka_bsp.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Include\External_Inc\drv_callback.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Include\External_Inc\inno_api.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Include\External_Inc\SianoCmmbApi.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Include\External_Inc\SianoIsdbtApi.h
# End Source File
# End Group
# Begin Group "Drv_In_Inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Drivers\Include\Internal_Inc\arch_init.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Include\Internal_Inc\dev_camera.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Include\Internal_Inc\dev_fm.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Include\Internal_Inc\dev_keypad.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Include\Internal_Inc\dev_lcd.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Include\Internal_Inc\dev_paddle.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Include\Internal_Inc\drv_in_callback.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Include\Internal_Inc\platform_hd_config.h
# End Source File
# End Group
# End Group
# Begin Group "DrvLibrary"

# PROP Default_Filter ""
# End Group
# Begin Group "Simulator"

# PROP Default_Filter ""
# Begin Group "res"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Drivers\Simulator\res\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\res\winvme.rc
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Drivers\Simulator\akos_win32.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\comport.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\comport.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\driver.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\error.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\isr.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\mailbox.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\mailbox.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\nand_win32.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\nand_win32.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\Queue.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\Queue.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\timer.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\vdisplay.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\vme.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\vme_engine.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\vme_engine.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\vme_event.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\vme_event.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\vme_interface.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\vme_util.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\vme_util.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\w_fm.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\w_freq.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\w_gpio.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\w_keypad.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\w_melody.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\w_melody.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\w_rtc.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\w_sysdelay.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\w_usb.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\w_vtimer.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\winvme.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\winvme.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\winvme_debug.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\zmjDump.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\zmjDump.h
# End Source File
# End Group
# Begin Group "DrvSourceCode"

# PROP Default_Filter ""
# Begin Group "DrvCamera"

# PROP Default_Filter ""
# End Group
# Begin Group "Keypad"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Drivers\SourceCode\keypad\keypad_diode.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\SourceCode\keypad\keypad_keyPerGpio.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\SourceCode\keypad\keypad_matrix_tianxin.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\SourceCode\keypad\keypad_mixed.c
# End Source File
# End Group
# Begin Group "LCD"

# PROP Default_Filter ""
# End Group
# Begin Group "Paddle"

# PROP Default_Filter ""
# End Group
# Begin Group "callback"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Drivers\SourceCode\callback\drv_callback.c
# End Source File
# End Group
# Begin Group "platform_config"

# PROP Default_Filter ""
# Begin Group "AK37XX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Drivers\SourceCode\platform_config\AK37XX\boot.h
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\SourceCode\platform_config\AK37XX\gpio_pin_CI37XX.c
# End Source File
# End Group
# End Group
# End Group
# End Group
# Begin Group "Middle"

# PROP Default_Filter ""
# Begin Group "MidInclude"

# PROP Default_Filter ""
# Begin Group "Mid_Ex_Inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\AKInterface.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\AKVector.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_APlayerList.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_AudioPlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_AVIPlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_Capture.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_ClrSelect.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_Dialog.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_DisplayList.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_Ebook.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_FileList.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_Fm.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_global.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_Icon.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_IconExplorer.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_IconMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_ImgBrowse.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_ListBox.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_ListItem.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_MediaList.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_MsgBox.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_MultiSet.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_Preview.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_Progress.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_RecAudio.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_ScrollBar.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_SlipMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_Title.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_ToolBar.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_VideoRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Ctl_WaitBox.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_AkBmp.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_Alarm.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_AutoPowerOff.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_BatWarn.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_Calendar.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\eng_callback.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_convert_unicode.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_DataConvert.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_Debug.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_DynamicFont.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_FileManage.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_Font.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_freetype.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_GblString.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_Graph.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_Id3Inf.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_IdleThread.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_ImgConvert.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_ImgDec.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_Jpeg2Bmp.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_KeyMapping.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_Math.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_MsgQueue.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_PowerOnThread.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_Pwr_OnOff_video.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_ScaleConvertSoft.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_ScreenSave.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_SimGame.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_String.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_String_UC.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_Time.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Eng_TopBar.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_calibrate.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_display.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_font.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_FreqManager.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_gm.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_graph.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_graphic.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_gui.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_Image.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_ImageLib.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_Initialize.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_keyhandler.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_oscom.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_osfs.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_osMalloc.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_pfAudio.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_pfcamera.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_pfdisplay.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_pfFm.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_pfKeypad.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_power.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_public.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_rtc.h
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\Include\External_Inc\Fwl_sd.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_sys_detect.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_sysevent.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_touchscr.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_ts_adc_list.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_tscrcom.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_USB.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_usb_h_disk.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Fwl_usb_host.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_vme.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_wave.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\fwl_waveout.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_cmmb.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_demux.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_event.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_event_api.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_expat.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_expat_external.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_FCSIM.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_freetype_api.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_GBA.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_GBA_callback.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_geapi.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_geshade.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_image_api.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_ISDB_T.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_MDSIM.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_media_global.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_media_struct.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_res_api.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_res_port.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_rgstockapi.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_sdcodec.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_sdfilter.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_SNES.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_state.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_state_api.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Lib_ThumbsDB.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Log_ImgRender.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\log_mbltv_api.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\log_Mbltv_LOGMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\log_mbltv_XMLCfgManage.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\log_media_recorder.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Log_MediaPlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Log_MotionDetec.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Log_RecAudio.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\log_uvc_cam.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\motion_detector_lib.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\External_Inc\Svc_MediaList.h
# End Source File
# End Group
# Begin Group "Mid_In_Inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\AKApp.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\AKApp_Def.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\AKAppMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\AKComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\AKList.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\AKMediaBGApp.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\AKMMIApp.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\AKMsgDispatch.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\AKPublicBGApp.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\AKSubThread.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\AKThread.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\AKWnd.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\Ctl_ContextMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\Eng_Math64.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\eng_queue.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\fwl_evtmailbox.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\fwl_wavein.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\ImageLayer.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\media_demuxer_lib.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\media_player_lib.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\media_recorder_lib.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\misc.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\mobiletv_recorder_lib.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\mount.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\sys_ctl.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\tscr_command.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\tscr_input.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Include\Internal_Inc\video_stream_lib.h
# End Source File
# End Group
# End Group
# Begin Group "MidLibrary"

# PROP Default_Filter ""
# Begin Group "Demux"

# PROP Default_Filter ""
# Begin Group "demuxCmmb"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\demux\cmmb\cmmb.lib
# End Source File
# End Group
# Begin Group "demuxIsdbt"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\demux\isdbt\ISDB_T.lib
# End Source File
# End Group
# End Group
# Begin Group "Dynamicfont"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\dynamicfont\DYNAMICFONT_v4.lib
# End Source File
# End Group
# Begin Group "Dynamicload"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\dynamicload\dynamicload.lib
# End Source File
# End Group
# Begin Group "MidEbookLib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\ebook\ebookcore.lib
# End Source File
# End Group
# Begin Group "Expat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\expat\Expat.lib
# End Source File
# End Group
# Begin Group "MidGameLib"

# PROP Default_Filter ""
# Begin Group "fcsimLib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\game\fcsim_lib\FCSIM_lib.lib
# End Source File
# End Group
# Begin Group "gbaLib"

# PROP Default_Filter ""
# End Group
# Begin Group "mdsimLib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\game\mdsim_lib\MDSIM_lib.lib
# End Source File
# End Group
# Begin Group "snesLib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\game\snes_lib\SNES_lib.lib
# End Source File
# End Group
# End Group
# Begin Group "GE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\ge\getypes.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Library\ge\Lib_geshade.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Library\ge\libge.lib
# End Source File
# End Group
# Begin Group "MidImageLib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\image\Lib_ThumbsDB.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Library\image\image.lib
# End Source File
# End Group
# Begin Group "Media"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\media\sdcodeclib.lib
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Library\media\video_lib_aspen.lib
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Library\media\media_lib_aspen.lib
# End Source File
# End Group
# Begin Group "Rg"

# PROP Default_Filter ""
# End Group
# Begin Group "SMCore"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\SMCore\sm_port.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Library\SMCore\sm_port.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Library\SMCore\smport_cfg.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Library\SMCore\smcore.lib
# End Source File
# End Group
# Begin Group "Unicode"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\unicode\unilib.lib
# End Source File
# End Group
# Begin Group "freetype"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\Library\freetype\Lib_freetype.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\Library\freetype\freetype244ST_D.lib
# End Source File
# End Group
# End Group
# Begin Group "MidSourceCode"

# PROP Default_Filter ""
# Begin Group "AppFrame"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKApp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKAppMgr.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKList.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKMsgDispatch.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKPublicBGApp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKSubThread.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKThread.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKVector.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKWnd.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKXxx.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKXxx.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKXxxApp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKXxxApp.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKXxxBGApp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKXxxBGApp.h
# End Source File
# End Group
# Begin Group "MidCamera"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\camera\Ctl_Capture.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\camera\Ctl_Preview.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\camera\Ctl_VideoRecord.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\camera\fwl_pfcamera.c
# End Source File
# End Group
# Begin Group "Control"

# PROP Default_Filter ""
# Begin Group "display"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_ClrSelect.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_ContextMenu.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_Dialog.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_DisplayList.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_FileList.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_Icon.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_Icon.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_IconExplorer.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_IconList.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_IconList.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_IconMenu.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_ListBox.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_ListItem.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_Msgbox.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_MultiSet.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_Progress.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_ScrollBar.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_SlipCalc.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_SlipCalc.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_SlipItem.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_SlipItem.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_SlipItemMgr.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_SlipItemMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_SlipMgr.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_SlipMsg.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_SlipMsg.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_SlipScrb.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_SlipScrb.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_Text.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_Text.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_Title.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_ToolBar.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Ctl_WaitBox.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\display\Eng_TopBar.c
# End Source File
# End Group
# Begin Group "logic"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\logic\Eng_FileManage.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\logic\Eng_MsgQueue.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\logic\Eng_Pwr_OnOff_video.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\logic\Fwl_keyhandler.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\control\logic\preprochandler.c
# End Source File
# End Group
# End Group
# Begin Group "Debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\debug\Eng_Debug.c
# End Source File
# End Group
# Begin Group "MidDisplay"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\display\Dev_display.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\display\Dev_display.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\display\Display_func_common.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\display\Fwl_display.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\display\fwl_font.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\display\fwl_graphic.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\display\fwl_pfdisplay.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\display\ImageLayer.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\display\temp.c
# End Source File
# End Group
# Begin Group "MidEbook"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\ebook\Ctl_Ebook.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\ebook\ebookcore.h
# End Source File
# End Group
# Begin Group "MidFm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\fm\Ctl_Fm.c
# End Source File
# End Group
# Begin Group "Font"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\font\Eng_DynamicFont.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\font\Eng_Font.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\font\Eng_freetype.c
# End Source File
# End Group
# Begin Group "Freq"

# PROP Default_Filter ""
# End Group
# Begin Group "MidGame"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\game\Fwl_gm.c
# End Source File
# End Group
# Begin Group "Graphic"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\graphic\Eng_AkBmp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\graphic\Eng_Graph.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\graphic\Eng_ImgConvert.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\graphic\Eng_ImgDec.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\graphic\Eng_Jpeg2Bmp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\graphic\Eng_ScaleConvertSoft.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\graphic\Fwl_gui.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\graphic\Fwl_ImageLib.c
# End Source File
# End Group
# Begin Group "MidImage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\image\Ctl_ImgBrowse.c
# End Source File
# End Group
# Begin Group "MidInit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\init\Ak_Main.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\init\AKMMIApp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\init\eng_callback.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\init\Eng_PowerOnThread.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\init\Fwl_Initialize.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\init\Fwl_multitask.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\init\Fwl_Public.c
# End Source File
# End Group
# Begin Group "MidKeypad"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\keypad\Eng_KeyTranslate.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\keypad\Eng_MctCode.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\keypad\fwl_pfKeypad.c
# End Source File
# End Group
# Begin Group "MidMath"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\math\Eng_Math.c
# End Source File
# End Group
# Begin Group "MidMedia"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\AKAudioBGApp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\AKAudioBGApp.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\AKAudioListBGApp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\AKAudioListBGApp.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\AKMediaBGApp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\AKVideoBGApp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\AKVideoBGApp.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\AKVideoListBGApp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\AKVideoListBGApp.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Ctl_APlayerList.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Ctl_AudioPlayer.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Ctl_AVIPlayer.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Ctl_MediaList.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Ctl_RecAudio.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Eng_Id3Inf.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Fwl_pfAudio.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\log_media_recorder.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_MediaAudio.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_MediaAudio.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_MediaDmx.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_MediaDmx.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_MediaPlayer.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_MediaStruct.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_MediaVideo.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_MediaVideo.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_MotionDetec.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_Mp3Player.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_Mp3Player.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_MpuRefresh.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Log_RecAudio.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\Svc_MediaList.c
# End Source File
# End Group
# Begin Group "MidMemory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\memory\Fwl_osMalloc.c
# End Source File
# End Group
# Begin Group "PcmManage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\pcmmanage\fwl_wavein.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\pcmmanage\fwl_waveout.c
# End Source File
# End Group
# Begin Group "Power"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\power\Eng_AutoPowerOff.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\power\Eng_BatWarn.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\power\Eng_IdleThread.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\power\Eng_ScreenSave.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\power\fwl_power.c
# End Source File
# End Group
# Begin Group "Queue"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\queue\eng_queue.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\queue\fwl_evtmailbox.c
# End Source File
# End Group
# Begin Group "Resource"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\resource\Lib_res_port.c
# End Source File
# End Group
# Begin Group "String"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\string\Eng_DataConvert.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\string\Eng_GblString.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\string\Eng_String.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\string\Eng_String_UC.c
# End Source File
# End Group
# Begin Group "Time"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\time\Eng_Alarm.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\time\Eng_Calendar.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\time\Eng_Time.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\time\fwl_oscom.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\time\Fwl_rtc.c
# End Source File
# End Group
# Begin Group "Tscr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\tscr\fwl_tscrcom.c
# End Source File
# End Group
# Begin Group "USB"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\usb\fwl_sys_detect.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\usb\Fwl_USB.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\usb\log_uvc_cam.c
# End Source File
# End Group
# Begin Group "MidFilesystem"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Middle\SourceCode\filesystem\Fwl_osfs.c
# End Source File
# Begin Source File

SOURCE=..\..\AKOS\SourceCode\nand\Fwl_sd.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\filesystem\Fwl_usb_host.c
# End Source File
# End Group
# End Group
# Begin Group "MidSimulator"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Drivers\Simulator\power.c
# End Source File
# Begin Source File

SOURCE=..\..\Drivers\Simulator\tscr_input.c
# End Source File
# End Group
# End Group
# Begin Group "Applications"

# PROP Default_Filter ""
# Begin Group "AudioPlayer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\audioplayer\Eng_Lyric.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\Eng_Lyric.h
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_add_list_to_mylist.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_add_to_mylist.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_delete_all_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_delete_all_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_delete_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_delete_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_fetch_song.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_list_curply.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_list_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_list_mylist.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_list_pitch_mode.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_list_pre_time.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_list_repeat_mode.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_list_tone_mode.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_player.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_root.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_set_default_path.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_set_speed.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audioplayer\s_audio_update.c
# End Source File
# End Group
# Begin Group "AudioRecorder"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\audiorecorder\s_audio_recorder.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audiorecorder\s_audio_recorder_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audiorecorder\s_audio_recorder_list_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audiorecorder\s_audio_recorder_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audiorecorder\s_audio_recorder_set_mode.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audiorecorder\s_audio_recorder_set_rate.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audiorecorder\s_audiorec_delete_all_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\audiorecorder\s_audiorec_delete_cnfm.c
# End Source File
# End Group
# Begin Group "Calculator"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\calculator\s_calculator.c
# End Source File
# End Group
# Begin Group "Camera"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\camera\s_camera_capture.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\camera\s_camera_multishot_show.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\camera\s_camera_preview.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\camera\s_camera_recorder.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\camera\s_camera_shot_show.c
# End Source File
# End Group
# Begin Group "EBook"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\ebook\s_ebk_delete_all_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\ebook\s_ebk_delete_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\ebook\s_ebk_fcs_bkclr.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\ebook\s_ebk_fcs_frclr.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\ebook\s_ebk_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\ebook\s_ebk_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\ebook\s_ebk_scroll_set.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\ebook\s_ebk_scroll_time.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\ebook\s_ebk_scroll_value.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\ebook\s_ebk_set_bkclr.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\ebook\s_ebk_set_frclr.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\ebook\s_ebk_view.c
# End Source File
# End Group
# Begin Group "EMap"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\emap\s_emap_browser.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\emap\s_emap_list.c
# End Source File
# End Group
# Begin Group "Explorer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\explorer\s_explorer_delete_all_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\explorer\s_explorer_delete_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\explorer\s_explorer_paste_file.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\explorer\s_explorer_paste_folder.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\explorer\s_sd_format.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\explorer\s_set_default_path.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\explorer\s_set_disp_explorer.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\explorer\s_set_explorer_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\explorer\s_set_save_path.c
# End Source File
# End Group
# Begin Group "FM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\fm\s_fm_change_area.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\fm\s_fm_del_station.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\fm\s_fm_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\fm\s_fm_list_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\fm\s_fm_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\fm\s_fm_player.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\fm\s_fm_read_station.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\fm\s_fm_recorder.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\fm\s_fm_recorder_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\fm\s_fm_write_station.c
# End Source File
# End Group
# Begin Group "Game"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\game\FCSIM_ext_api.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\FCSIM_ext_api.h
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\gm7coloreng.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\gm7coloreng.h
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\gm7colorengd.h
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\gmbteng.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\gmbteng.h
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\gmbtengd.h
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\gmrecteng.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\gmrecteng.h
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\gmrectengd.h
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_7color.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_7color_play.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_boat.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_boat_play.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_gba_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_gba_play.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_keymap.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_load.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_load_sel.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_md_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_md_play.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_nes_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_nes_play.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_rect.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_rect_play.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_save.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_set_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_snes_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\s_gam_snes_play.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\SNES_ext_api.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\game\SNES_ext_api.h
# End Source File
# End Group
# Begin Group "Image"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\image\s_img_browser.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\image\s_img_delete_all_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\image\s_img_delete_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\image\s_img_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\image\s_img_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\image\s_img_set_slide.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\image\s_img_thumbnail_view.c
# End Source File
# End Group
# Begin Group "Init"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\init\s_init_power_on.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\init\s_init_system.c
# End Source File
# End Group
# Begin Group "PubStates"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_alarm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_bat_warn.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_del_file.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_empty.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_key_unlock.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_message_box.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_pre_message.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_reserved1.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_reserved2.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_reserved3.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_screen_saver.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_switch_off.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\pub_states\s_pub_usb.c
# End Source File
# End Group
# Begin Group "Setup"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\setup\s_display_switch.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_adjust_speaker.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_animated_speed.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_disp_brightness.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_disp_contrast.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_disp_language.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_disp_memory.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_disp_ss_time.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_disp_version.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_factory_setting.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_font.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_lowbat_time.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_personal_set_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_poff_time.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_set_sys_update.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\setup\s_tscr_calibrate.c
# End Source File
# End Group
# Begin Group "Standby"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\standby\s_lib_version.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\standby\s_stdb_standby.c
# End Source File
# End Group
# Begin Group "StateList"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\statelist\Lib_state.c
# End Source File
# End Group
# Begin Group "TimeSet"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\timeset\Ctl_AlarmSet.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\timeset\Ctl_AlarmSet.h
# End Source File
# Begin Source File

SOURCE=..\..\Applications\timeset\Ctl_TimeSet.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\timeset\Ctl_TimeSet.h
# End Source File
# Begin Source File

SOURCE=..\..\Applications\timeset\s_clk_world_map.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\timeset\s_func_calendar.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\timeset\s_set_alarm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\timeset\s_set_alarm_sound.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\timeset\s_set_sysclock.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\timeset\s_timeset_menu.c
# End Source File
# End Group
# Begin Group "ToolBox"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\toolbox\s_toolbox_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\toolbox\s_toolbox_usbhost.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\toolbox\s_usb_camera.c
# End Source File
# End Group
# Begin Group "VideoPlayer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\videoplayer\s_video_add_movie.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\videoplayer\s_video_delete_all_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\videoplayer\s_video_delete_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\videoplayer\s_video_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\videoplayer\s_video_list_repeat_mode.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\videoplayer\s_video_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\videoplayer\s_video_player.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\videoplayer\s_video_read_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\videoplayer\s_video_save_list.c
# End Source File
# End Group
# Begin Group "FlashPlayer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\flash\S_flash_delete_all_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\flash\S_flash_delete_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\flash\s_flash_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\flash\s_flash_set_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\flash\s_flash_set_quality.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\flash\s_flashplayer.c
# End Source File
# End Group
# Begin Group "autotest"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\autotest\autoTest_record_func.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\autotest\autoTest_record_func.h
# End Source File
# Begin Source File

SOURCE=..\..\Applications\autotest\AutoTest_test_func.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\autotest\AutoTest_test_func.h
# End Source File
# Begin Source File

SOURCE=..\..\Applications\autotest\s_autotest_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\autotest\s_autoTest_startrecord_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\autotest\s_autoTest_starttest_compatible_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\autotest\s_autoTest_starttest_crosstest_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\autotest\s_autoTest_starttest_normaltest_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\autotest\s_autoTest_starttest_performancetest_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\autotest\s_autoTest_starttest_presstest_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\autotest\s_autoTest_stoprecord_menu.c
# End Source File
# End Group
# Begin Group "VFont"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Applications\vfont\s_vfont_delete_all_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\vfont\s_vfont_delete_cnfm.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\vfont\s_vfont_demo.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\vfont\s_vfont_list.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\vfont\s_vfont_option.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\vfont\s_vfont_set_bkclr.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\vfont\s_vfont_set_frclr.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\vfont\s_vfont_set_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\Applications\vfont\s_vfont_size.c
# End Source File
# End Group
# End Group
# Begin Group "PubInclude"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\PubInclude\akdefine.h
# End Source File
# Begin Source File

SOURCE=..\..\PubInclude\AKError.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\appframe\AKFrameStream.c
# End Source File
# Begin Source File

SOURCE=..\..\PubInclude\akos_api.h
# End Source File
# Begin Source File

SOURCE=..\..\PubInclude\anyka_types.h
# End Source File
# Begin Source File

SOURCE=..\..\PubInclude\config.h
# End Source File
# Begin Source File

SOURCE=..\..\PubInclude\Gbl_Global.h
# End Source File
# Begin Source File

SOURCE=..\..\PubInclude\gbl_macrodef.h
# End Source File
# Begin Source File

SOURCE=..\..\PubInclude\Gbl_Resource.h
# End Source File
# Begin Source File

SOURCE=..\..\PubInclude\gpio_config.h
# End Source File
# Begin Source File

SOURCE=..\..\PubInclude\gpio_config_CI3771.h
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\log_MediaEncode.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\log_videoDisp.c
# End Source File
# Begin Source File

SOURCE=..\..\Middle\SourceCode\media\log_videoZoom.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Drivers\Simulator\res\voice.bmp
# End Source File
# End Target
# End Project
