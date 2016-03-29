# Project name

PROJ = Sword37 

TARGET= Sword37

# Pathes and Locations

ifndef BUILDPATH
BUILDPATH  = bd
endif
ifndef M3LIBPATH
M3LIBPATH  = c:/cygwin/tools
endif

# Flags

include system.cfg

BUILDPATH  := $(BUILDPATH)_$(CHIP)

APPPATH		= Applications
MIDPATH		= Middle
AKOSPATH	= AKOS
DRVPATH		= Drivers
		
MIDHPATH		= -I../../$(MIDPATH)/Include/External_Inc -I../../PubInclude
MIDINPATH		= -I../../$(MIDPATH)/Include/Internal_Inc
AKOSHPATH		= -I../../$(AKOSPATH)/Include/External_Inc -I../../PubInclude
AKOSINPATH		= -I../../$(AKOSPATH)/Include/Internal_Inc
DRVHPATH		= -I../../$(DRVPATH)/Include/External_Inc -I../../PubInclude
DRVINPATH		= -I../../$(DRVPATH)/Include/Internal_Inc
			  
ENDIANELF	= elf32-little
INCLUDE = -I../../$(DRVPATH)/Include/External_Inc

DEFINE		:= $(DEFINE) -DOS_ANYKA=1 -DDEPENDS=1

STATEPATH		:= ../../Applications/statelist


DEFINE		:= $(DEFINE) -D_UNICODE=1 



ifeq ($(SUPPORT_DEBUG_OUTPUT), Y)
DEFINE      := $(DEFINE) -DDEBUG_OUTPUT=1
endif

ifeq ($(BUILD_TYPE),TEST)
DEFINE		:= $(DEFINE) -DDEBUG_OUTPUT=1 -DASSERT_DEATH_LOOP=1 
ASFLAGS   = -keep -apcs /interwork -cpu 5TEJ
endif

ifeq ($(BUILD_TYPE),TRYOUT)
DEFINE		:= $(DEFINE) -DDEBUG_OUTPUT=1 -DJAVA_NOLIMIT=1 -DASSERT_REBOOT=1 
DEFINE		:= $(DEFINE) -DSUPPORT_PANNIC_REBOOT=1 
DEFINE		:= $(DEFINE) -DSUPPORT_NAND_TRACE=1
ASFLAGS   = -keep -apcs /interwork -cpu 5TEJ
endif

ifeq ($(BUILD_TYPE),PRODUCT)
DEFINE		:= $(DEFINE) -DJAVA_NOLIMIT=1  -DASSERT_REBOOT=1
ASFLAGS   = -keep -apcs /interwork -cpu 5TEJ -c
endif


CFLAGS		= -O2 -g -apcs /interwork -cpu ARM926EJ-S -Wb -Ec $(DEFINE)
LCCFLAGS	= -O2 -apcs /interwork -cpu ARM926EJ-S -Wb -Ec $(DEFINE) 

LDFLAGS     = -info totals,sizes -noremove -map -scatter scatter.scf -entry START

#define operation system
DEFINE		:= $(DEFINE) -DAKOS=1
ifdef NO_NORFLASH
DEFINE		:= $(DEFINE) -DNO_NORFLASH=1
endif

ifdef MEMLEAK
BUILDPATH	:= $(BUILDPATH)ml
TARGET		:= $(TARGET)_ml
DEFINE		:= $(DEFINE) -DDEBUG_TRACE_MEMORY_LEAK=1
else
endif

ifdef CHIP
DEFINE      := $(DEFINE) -DCHIP_$(CHIP)=1
endif

DEFINE		:= $(DEFINE) -D$(PLATFORM)_PLATFORM=1


ifdef SUPPORT_NAND_TRACE
DEFINE		:= $(DEFINE) -DSUPPORT_NAND_TRACE=1
endif

ifdef SUPPORT_PANNIC_REBOOT
DEFINE		:= $(DEFINE) -DSUPPORT_PANNIC_REBOOT=1
ifdef SUPPORT_PANNIC_REBOOT_TEST
DEFINE		:= $(DEFINE) -DSUPPORT_PANNIC_REBOOT_TEST=1
endif
endif

# Camera Flash Light
DEFINE      := $(DEFINE) -DCAMERA_FLASHLIGHT

# Tools
CC		=armcc
AS	     	= armasm
LD	     	= armlink
RM         	= rm -rf
MKDIR      	= mkdir
OBJDUMP      = objdump
OBJCOPY	     = fromelf
COMPRESS_BIN    = ./cpstool.exe

LIBS		:= $(LIBS) ../../AKOS/Library/fileSystem/fat32fs.a \
				../../AKOS/Library/fileSystem/mount.a \
				../../AKOS/Library/fileSystem/mtdlib.a \
				../../AKOS/Library/filesystem/fha.a \
				../../AKOS/Library/AKOS/akos.a \
				../../AKOS/Library/memory/memory.a \
				../../AKOS/Library/devManage/drv_ak37xx.a \
				../../Middle/Library/SMCore/smcore.a \
				../../Middle/Library/unicode/unilib.a \
				../../Middle/Library/media/media_lib_aspen.a \
				../../Middle/Library/media/video_lib_aspen.a



ifeq ($(SUPPORT_NETWORK), Y)
DEFINE      += -DSUPPORT_NETWORK
LIBS		:= $(LIBS) \
				../../Middle/Library/lwip/LwipLib.a 
endif

ifeq ($(SUPPORT_FLASH), Y)
DEFINE      += -DSUPPORT_FLASH
LIBS		:= $(LIBS) \
                ../../Middle/Library/flash/Flash_lib.a \
                ../../Middle/Library/freetype/libfreetype.a \
				../../Middle/Library/flash/libiconv.a
endif

ifeq ($(SUPPORT_VFONT), Y)
DEFINE      += -DSUPPORT_VFONT
ifneq ($(SUPPORT_FLASH), Y)
LIBS		:= $(LIBS) \
                ../../Middle/Library/freetype/libfreetype.a
endif
endif

ifeq ($(RAM_SIZE), 8)
	LIBS		:= $(LIBS) ../../Middle/Library/media/sdcodec.a
else
	LIBS		:= $(LIBS) ../../Middle/Library/media/sdcodec_midi.a
endif

ifdef BOOT_MODE
  DEFINE		:= $(DEFINE) -D$(BOOT_MODE)BOOT=1
				LIBS		:= $(LIBS) ../../Middle/Library/dynamicload/dynamicload.a

  ifeq ($(BOOT_MODE), SPI)
        LIBS		:= $(LIBS) ../../Middle/Library/dynamicfont/DynamicFont_v4_spi.a
  else
        LIBS		:= $(LIBS) ../../Middle/Library/dynamicfont/DynamicFont_v4.a
  endif
endif

#Locations
#ifndef NOSOUNDLIB

ifeq ($(PLATFORM), CI37XX)
LIBS		:= $(LIBS) ../../Middle/Library/media/sdfilter.a
LIBS		:= $(LIBS) ../../Middle/Library/image/image_37xx.a
LIBS		:= $(LIBS) ../../Middle/Library/media/motion_detector_lib.a
endif

#endif

ifeq ($(SUPPORT_EBK), Y)
    DEFINE      += -DSUPPORT_EBK
	LIBS		:= $(LIBS) ../../Middle/Library/ebook/ebookcore.a
endif


ifeq ($(SUPPORT_AUTOTEST), Y)
    DEFINE      += -DSUPPORT_AUTOTEST
endif
ifneq ($(RAM_SIZE), 8)
ifeq ($(SUPPORT_GESHADE), Y)
    DEFINE      += -DSUPPORT_GE_SHADE
    LIBS		:= $(LIBS) ../../Middle/Library/ge/libge.a
endif

ifeq ($(SUPPORT_AUDIOREC), Y)
		DEFINE      += -DSUPPORT_AUDIOREC_DENOICE
endif
endif


ifeq ($(SUPPORT_GAME), Y)
ifdef SUPPORT_GAME_NES
ifeq ($(SUPPORT_GAME_NES),Y)
	DEFINE      += -DINSTALL_GAME_NES
	LIBS		:= $(LIBS) ../../Middle/Library/game/fcsim_lib/fcsim.a 
endif
endif

ifdef SUPPORT_GAME_SFC
ifeq ($(SUPPORT_GAME_SFC),Y)
	DEFINE      += -DINSTALL_GAME_SFC
	LIBS		:= $(LIBS) ../../Middle/Library/game/snes_lib/snes.a 
endif
endif

ifdef SUPPORT_GAME_GBA
ifeq ($(SUPPORT_GAME_GBA),Y)
	DEFINE      += -DINSTALL_GAME_GBA
	LIBS		:= $(LIBS) ../../Middle/Library/game/gba_lib/gba.a
endif
endif

ifdef SUPPORT_GAME_MD
ifeq ($(SUPPORT_GAME_MD),Y)
	DEFINE      += -DINSTALL_GAME_MD
	LIBS		:= $(LIBS) ../../Middle/Library/game/mdsim_lib/mdsim_lib.a
endif
endif
endif


ifeq ($(SUPPORT_VISUAL_AUDIO), Y)
LIBS		:= $(LIBS) ../../Middle/Library/media/libva.a
endif

#build rule
.PHONY: clean makedirs maketarget debug release help msi compresscode

debug:
	@echo ---------------------[build debug version]---------------------
	@echo build start datetime:
	date
	$(MAKE) makedirs CFG=Debug
	$(MAKE) msi CFG=Debug
	$(MAKE) ctohdepends CFG=Debug >$(BUILDPATH)/depends.txt 2>&1
	$(MAKE) maketarget CFG=Debug
	$(MAKE) compresscode CFG=Debug
	@echo build end datetime:
	date
release:
	@echo ---------------------[build release version]---------------------
	$(MAKE) makedirs CFG=Release
	$(MAKE) msi CFG=Release
	$(MAKE) ctohdepends CFG=Release >$(BUILDPATH)/depends.txt 2>&1
	$(MAKE) maketarget CFG=Release
	$(MAKE) compresscode CFG=Release


maketarget: $(BUILDPATH)/$(TARGET).elf $(BUILDPATH)/$(TARGET).bin
compresscode:
ifeq ($(ENABLE_COMPRESS_BIN), Y)
	$(COMPRESS_BIN)  $(BUILDPATH)/../$(TARGET).bin
	@echo compress bin ok
else
	@echo bin neednot compress
endif

$(BUILDPATH):
	-@$(MKDIR) -p $@

BUILDDIRS =  $(BUILDPATH)/AKOS $(BUILDPATH)/AKOS/SourceCode\
 $(BUILDPATH)/AKOS/SourceCode/drv_init_cb $(BUILDPATH)/AKOS/SourceCode/init $(BUILDPATH)/AKOS/SourceCode/nand\
 $(BUILDPATH)/AKOS/SourceCode/ram $(BUILDPATH)/Applications $(BUILDPATH)/Applications/audioplayer\
 $(BUILDPATH)/Applications/audiorecorder $(BUILDPATH)/Applications/autotest $(BUILDPATH)/Applications/camera\
 $(BUILDPATH)/Applications/explorer $(BUILDPATH)/Applications/image $(BUILDPATH)/Applications/init\
 $(BUILDPATH)/Applications/network $(BUILDPATH)/Applications/pub_states $(BUILDPATH)/Applications/setup\
 $(BUILDPATH)/Applications/standby $(BUILDPATH)/Applications/statelist $(BUILDPATH)/Applications/timeset\
 $(BUILDPATH)/Applications/toolbox $(BUILDPATH)/Applications/videoplayer $(BUILDPATH)/Drivers\
 $(BUILDPATH)/Drivers/Simulator $(BUILDPATH)/Drivers/SourceCode $(BUILDPATH)/Drivers/SourceCode/callback\
 $(BUILDPATH)/Drivers/SourceCode/camera $(BUILDPATH)/Drivers/SourceCode/keypad $(BUILDPATH)/Drivers/SourceCode/lcd\
 $(BUILDPATH)/Drivers/SourceCode/platform_config $(BUILDPATH)/Drivers/SourceCode/platform_config/AK37XX\
 $(BUILDPATH)/Drivers/SourceCode/rtc $(BUILDPATH)/Middle $(BUILDPATH)/Middle/Library\
 $(BUILDPATH)/Middle/Library/SMCore $(BUILDPATH)/Middle/Library/freetype $(BUILDPATH)/Middle/Library/ge\
 $(BUILDPATH)/Middle/Library/image $(BUILDPATH)/Middle/Library/lwip $(BUILDPATH)/Middle/SourceCode\
 $(BUILDPATH)/Middle/SourceCode/appframe $(BUILDPATH)/Middle/SourceCode/camera $(BUILDPATH)/Middle/SourceCode/control\
 $(BUILDPATH)/Middle/SourceCode/control/display $(BUILDPATH)/Middle/SourceCode/control/logic\
 $(BUILDPATH)/Middle/SourceCode/debug $(BUILDPATH)/Middle/SourceCode/display\
 $(BUILDPATH)/Middle/SourceCode/filesystem $(BUILDPATH)/Middle/SourceCode/font $(BUILDPATH)/Middle/SourceCode/game\
 $(BUILDPATH)/Middle/SourceCode/graphic $(BUILDPATH)/Middle/SourceCode/image $(BUILDPATH)/Middle/SourceCode/init\
 $(BUILDPATH)/Middle/SourceCode/keypad $(BUILDPATH)/Middle/SourceCode/math $(BUILDPATH)/Middle/SourceCode/media\
 $(BUILDPATH)/Middle/SourceCode/memory $(BUILDPATH)/Middle/SourceCode/network $(BUILDPATH)/Middle/SourceCode/pcmmanage\
 $(BUILDPATH)/Middle/SourceCode/power $(BUILDPATH)/Middle/SourceCode/queue $(BUILDPATH)/Middle/SourceCode/resource\
 $(BUILDPATH)/Middle/SourceCode/string $(BUILDPATH)/Middle/SourceCode/time $(BUILDPATH)/Middle/SourceCode/tscr\
 $(BUILDPATH)/Middle/SourceCode/usb
$(BUILDDIRS):
	-@$(MKDIR) $@
makedirs: $(BUILDPATH) $(BUILDDIRS)

$(BUILDPATH)/$(TARGET).bin:  $(BUILDPATH)/$(TARGET).elf
	@echo ---------------------[build bin binary]----------------------------------
	@$(OBJCOPY) -bin $(BUILDPATH)/$(TARGET).elf  -output $(BUILDPATH)/../$(TARGET).bin
	#$(OBJCOPY)  -text -c -s $(BUILDPATH)/$(TARGET).elf -output $(BUILDPATH)/$(TARGET).txt

SOBJS =  $(BUILDPATH)/Drivers/SourceCode/platform_config/AK37XX/boot_ads.o

COBJS =  $(BUILDPATH)/Drivers/SourceCode/callback/drv_callback.o\
 $(BUILDPATH)/Drivers/SourceCode/camera/camera_SIM120B.o $(BUILDPATH)/Drivers/SourceCode/camera/camera_SIV100B.o\
 $(BUILDPATH)/Drivers/SourceCode/camera/camera_gc0308.o $(BUILDPATH)/Drivers/SourceCode/camera/camera_noon130pc11.o\
 $(BUILDPATH)/Drivers/SourceCode/camera/camera_ov2640.o $(BUILDPATH)/Drivers/SourceCode/camera/camera_ov2643.o\
 $(BUILDPATH)/Drivers/SourceCode/camera/camera_ov7663.o $(BUILDPATH)/Drivers/SourceCode/camera/camera_ov7670.o\
 $(BUILDPATH)/Drivers/SourceCode/camera/camera_ov7680.o $(BUILDPATH)/Drivers/SourceCode/camera/camera_ov7725.o\
 $(BUILDPATH)/Drivers/SourceCode/camera/camera_ov9650.o $(BUILDPATH)/Drivers/SourceCode/camera/camera_ov9655.o\
 $(BUILDPATH)/Drivers/SourceCode/camera/camera_ov9660.o $(BUILDPATH)/Drivers/SourceCode/camera/camera_ov9665.o\
 $(BUILDPATH)/Drivers/SourceCode/camera/camera_po1200.o $(BUILDPATH)/Drivers/SourceCode/keypad/keypad_analog.o\
 $(BUILDPATH)/Drivers/SourceCode/keypad/keypad_diode.o $(BUILDPATH)/Drivers/SourceCode/keypad/keypad_keyPerGpio.o\
 $(BUILDPATH)/Drivers/SourceCode/keypad/keypad_matrix.o $(BUILDPATH)/Drivers/SourceCode/keypad/keypad_matrix_tianxin.o\
 $(BUILDPATH)/Drivers/SourceCode/keypad/keypad_mixed.o $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_backlight.o\
 $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_mpu_7781.o $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_mpu_8907.o\
 $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_mpu_ILI9341V.o $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_mpu_S6D04H0.o\
 $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_mpu_r61580.o $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_mpu_spri8907.o\
 $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_rgb_OTA5180A.o $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_rgb_a050vm01.o\
 $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_rgb_hly070ml209.o $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_rgb_hsd0701dw1.o\
 $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_rgb_lq043t3dx02.o $(BUILDPATH)/Drivers/SourceCode/lcd/lcd_rgb_tm050rdz00.o\
 $(BUILDPATH)/Drivers/SourceCode/platform_config/AK37XX/detector_CI37XX.o $(BUILDPATH)/Drivers/SourceCode/platform_config/AK37XX/gpio_pin_CI37XX.o\
 $(BUILDPATH)/Drivers/SourceCode/platform_config/AK37XX/keypad_CI37XX_analog.o $(BUILDPATH)/Drivers/SourceCode/platform_config/AK37XX/keypad_CI37XX_matrix.o\
 $(BUILDPATH)/Drivers/SourceCode/rtc/external_rtc.o $(BUILDPATH)/Drivers/SourceCode/rtc/i2c_core.o\
 $(BUILDPATH)/Drivers/SourceCode/rtc/rtc_algo.o $(BUILDPATH)/Drivers/SourceCode/rtc/s35390a.o

$(BUILDPATH)/$(APPPATH).a:  $(BUILDPATH)/Applications/audioplayer/Eng_Lyric.o $(BUILDPATH)/Applications/audioplayer/s_audio_add_list_to_mylist.o $(BUILDPATH)/Applications/audioplayer/s_audio_add_to_mylist.o $(BUILDPATH)/Applications/audioplayer/s_audio_delete_all_cnfm.o $(BUILDPATH)/Applications/audioplayer/s_audio_delete_all_list.o $(BUILDPATH)/Applications/audioplayer/s_audio_delete_cnfm.o $(BUILDPATH)/Applications/audioplayer/s_audio_delete_list.o $(BUILDPATH)/Applications/audioplayer/s_audio_fetch_song.o $(BUILDPATH)/Applications/audioplayer/s_audio_list.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_curply.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_menu.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_mylist.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_pitch_mode.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_pre_time.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_repeat_mode.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_tone_mode.o $(BUILDPATH)/Applications/audioplayer/s_audio_menu.o $(BUILDPATH)/Applications/audioplayer/s_audio_player.o $(BUILDPATH)/Applications/audioplayer/s_audio_root.o $(BUILDPATH)/Applications/audioplayer/s_audio_set_default_path.o $(BUILDPATH)/Applications/audioplayer/s_audio_set_speed.o $(BUILDPATH)/Applications/audioplayer/s_audio_update.o $(BUILDPATH)/Applications/audiorecorder/s_audio_recorder.o $(BUILDPATH)/Applications/audiorecorder/s_audio_recorder_list.o $(BUILDPATH)/Applications/audiorecorder/s_audio_recorder_list_menu.o $(BUILDPATH)/Applications/audiorecorder/s_audio_recorder_menu.o $(BUILDPATH)/Applications/audiorecorder/s_audio_recorder_set_mode.o $(BUILDPATH)/Applications/audiorecorder/s_audio_recorder_set_rate.o $(BUILDPATH)/Applications/audiorecorder/s_audiorec_delete_all_cnfm.o $(BUILDPATH)/Applications/audiorecorder/s_audiorec_delete_cnfm.o $(BUILDPATH)/Applications/autotest/AutoTest_test_func.o $(BUILDPATH)/Applications/autotest/autoTest_record_func.o $(BUILDPATH)/Applications/autotest/s_autoTest_startrecord_menu.o $(BUILDPATH)/Applications/autotest/s_autoTest_starttest_compatible_menu.o $(BUILDPATH)/Applications/autotest/s_autoTest_starttest_crosstest_menu.o $(BUILDPATH)/Applications/autotest/s_autoTest_starttest_normaltest_menu.o $(BUILDPATH)/Applications/autotest/s_autoTest_starttest_performancetest_menu.o $(BUILDPATH)/Applications/autotest/s_autoTest_starttest_presstest_menu.o $(BUILDPATH)/Applications/autotest/s_autoTest_stoprecord_menu.o $(BUILDPATH)/Applications/autotest/s_autotest_menu.o $(BUILDPATH)/Applications/camera/s_camera_capture.o $(BUILDPATH)/Applications/camera/s_camera_multishot_show.o $(BUILDPATH)/Applications/camera/s_camera_preview.o $(BUILDPATH)/Applications/camera/s_camera_recorder.o $(BUILDPATH)/Applications/camera/s_camera_shot_show.o $(BUILDPATH)/Applications/explorer/s_explorer_delete_all_cnfm.o $(BUILDPATH)/Applications/explorer/s_explorer_delete_cnfm.o $(BUILDPATH)/Applications/explorer/s_explorer_paste_file.o $(BUILDPATH)/Applications/explorer/s_explorer_paste_folder.o $(BUILDPATH)/Applications/explorer/s_sd_format.o $(BUILDPATH)/Applications/explorer/s_set_default_path.o $(BUILDPATH)/Applications/explorer/s_set_disp_explorer.o $(BUILDPATH)/Applications/explorer/s_set_explorer_menu.o $(BUILDPATH)/Applications/explorer/s_set_save_path.o $(BUILDPATH)/Applications/image/s_img_browser.o $(BUILDPATH)/Applications/image/s_img_delete_all_cnfm.o $(BUILDPATH)/Applications/image/s_img_delete_cnfm.o $(BUILDPATH)/Applications/image/s_img_list.o $(BUILDPATH)/Applications/image/s_img_menu.o $(BUILDPATH)/Applications/image/s_img_set_slide.o $(BUILDPATH)/Applications/image/s_img_thumbnail_view.o $(BUILDPATH)/Applications/init/s_init_power_on.o $(BUILDPATH)/Applications/init/s_init_system.o $(BUILDPATH)/Applications/network/ctl_net_trans.o $(BUILDPATH)/Applications/network/s_ip_set.o $(BUILDPATH)/Applications/network/s_network_data_trans.o $(BUILDPATH)/Applications/network/s_network_menu.o $(BUILDPATH)/Applications/network/s_network_set_menu.o $(BUILDPATH)/Applications/network/s_ping_test.o $(BUILDPATH)/Applications/network/s_port_set.o $(BUILDPATH)/Applications/network/s_set_recv_path.o $(BUILDPATH)/Applications/network/s_set_send_file.o $(BUILDPATH)/Applications/network/s_tcp_client.o $(BUILDPATH)/Applications/network/s_tcp_server.o $(BUILDPATH)/Applications/network/s_udp_trans.o $(BUILDPATH)/Applications/pub_states/s_pub_alarm.o $(BUILDPATH)/Applications/pub_states/s_pub_bat_warn.o $(BUILDPATH)/Applications/pub_states/s_pub_del_file.o $(BUILDPATH)/Applications/pub_states/s_pub_empty.o $(BUILDPATH)/Applications/pub_states/s_pub_key_unlock.o $(BUILDPATH)/Applications/pub_states/s_pub_message_box.o $(BUILDPATH)/Applications/pub_states/s_pub_pre_message.o $(BUILDPATH)/Applications/pub_states/s_pub_reserved1.o $(BUILDPATH)/Applications/pub_states/s_pub_reserved2.o $(BUILDPATH)/Applications/pub_states/s_pub_reserved3.o $(BUILDPATH)/Applications/pub_states/s_pub_screen_saver.o $(BUILDPATH)/Applications/pub_states/s_pub_switch_off.o $(BUILDPATH)/Applications/pub_states/s_pub_usb.o $(BUILDPATH)/Applications/setup/s_display_switch.o $(BUILDPATH)/Applications/setup/s_set_adjust_speaker.o $(BUILDPATH)/Applications/setup/s_set_animated_speed.o $(BUILDPATH)/Applications/setup/s_set_disp_brightness.o $(BUILDPATH)/Applications/setup/s_set_disp_contrast.o $(BUILDPATH)/Applications/setup/s_set_disp_language.o $(BUILDPATH)/Applications/setup/s_set_disp_memory.o $(BUILDPATH)/Applications/setup/s_set_disp_ss_time.o $(BUILDPATH)/Applications/setup/s_set_disp_version.o $(BUILDPATH)/Applications/setup/s_set_factory_setting.o $(BUILDPATH)/Applications/setup/s_set_font.o $(BUILDPATH)/Applications/setup/s_set_lowbat_time.o $(BUILDPATH)/Applications/setup/s_set_menu.o $(BUILDPATH)/Applications/setup/s_set_personal_set_menu.o $(BUILDPATH)/Applications/setup/s_set_poff_time.o $(BUILDPATH)/Applications/setup/s_set_sys_update.o $(BUILDPATH)/Applications/setup/s_tscr_calibrate.o $(BUILDPATH)/Applications/setup/s_watchdog_test.o $(BUILDPATH)/Applications/standby/s_lib_version.o $(BUILDPATH)/Applications/standby/s_stdb_standby.o $(BUILDPATH)/Applications/statelist/Lib_state.o $(BUILDPATH)/Applications/timeset/Ctl_AlarmSet.o $(BUILDPATH)/Applications/timeset/Ctl_TimeSet.o $(BUILDPATH)/Applications/timeset/s_clk_world_map.o $(BUILDPATH)/Applications/timeset/s_func_calendar.o $(BUILDPATH)/Applications/timeset/s_set_alarm.o $(BUILDPATH)/Applications/timeset/s_set_alarm_sound.o $(BUILDPATH)/Applications/timeset/s_set_sysclock.o $(BUILDPATH)/Applications/timeset/s_timeset_menu.o $(BUILDPATH)/Applications/toolbox/s_toolbox_menu.o $(BUILDPATH)/Applications/toolbox/s_toolbox_usbhost.o $(BUILDPATH)/Applications/toolbox/s_usb_camera.o $(BUILDPATH)/Applications/videoplayer/s_video_add_movie.o $(BUILDPATH)/Applications/videoplayer/s_video_delete_all_cnfm.o $(BUILDPATH)/Applications/videoplayer/s_video_delete_cnfm.o $(BUILDPATH)/Applications/videoplayer/s_video_list.o $(BUILDPATH)/Applications/videoplayer/s_video_list_repeat_mode.o $(BUILDPATH)/Applications/videoplayer/s_video_menu.o $(BUILDPATH)/Applications/videoplayer/s_video_player.o $(BUILDPATH)/Applications/videoplayer/s_video_read_list.o $(BUILDPATH)/Applications/videoplayer/s_video_save_list.o Makefile
	@echo ---------------------[build out $(APPPATH).a:]----------------------------------
	-$(AR) -rsv $(BUILDPATH)/$(APPPATH).a  $(BUILDPATH)/Applications/audioplayer/Eng_Lyric.o $(BUILDPATH)/Applications/audioplayer/s_audio_add_list_to_mylist.o $(BUILDPATH)/Applications/audioplayer/s_audio_add_to_mylist.o $(BUILDPATH)/Applications/audioplayer/s_audio_delete_all_cnfm.o $(BUILDPATH)/Applications/audioplayer/s_audio_delete_all_list.o $(BUILDPATH)/Applications/audioplayer/s_audio_delete_cnfm.o $(BUILDPATH)/Applications/audioplayer/s_audio_delete_list.o $(BUILDPATH)/Applications/audioplayer/s_audio_fetch_song.o $(BUILDPATH)/Applications/audioplayer/s_audio_list.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_curply.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_menu.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_mylist.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_pitch_mode.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_pre_time.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_repeat_mode.o $(BUILDPATH)/Applications/audioplayer/s_audio_list_tone_mode.o $(BUILDPATH)/Applications/audioplayer/s_audio_menu.o $(BUILDPATH)/Applications/audioplayer/s_audio_player.o $(BUILDPATH)/Applications/audioplayer/s_audio_root.o $(BUILDPATH)/Applications/audioplayer/s_audio_set_default_path.o $(BUILDPATH)/Applications/audioplayer/s_audio_set_speed.o $(BUILDPATH)/Applications/audioplayer/s_audio_update.o $(BUILDPATH)/Applications/audiorecorder/s_audio_recorder.o $(BUILDPATH)/Applications/audiorecorder/s_audio_recorder_list.o $(BUILDPATH)/Applications/audiorecorder/s_audio_recorder_list_menu.o $(BUILDPATH)/Applications/audiorecorder/s_audio_recorder_menu.o $(BUILDPATH)/Applications/audiorecorder/s_audio_recorder_set_mode.o $(BUILDPATH)/Applications/audiorecorder/s_audio_recorder_set_rate.o $(BUILDPATH)/Applications/audiorecorder/s_audiorec_delete_all_cnfm.o $(BUILDPATH)/Applications/audiorecorder/s_audiorec_delete_cnfm.o $(BUILDPATH)/Applications/autotest/AutoTest_test_func.o $(BUILDPATH)/Applications/autotest/autoTest_record_func.o $(BUILDPATH)/Applications/autotest/s_autoTest_startrecord_menu.o $(BUILDPATH)/Applications/autotest/s_autoTest_starttest_compatible_menu.o $(BUILDPATH)/Applications/autotest/s_autoTest_starttest_crosstest_menu.o $(BUILDPATH)/Applications/autotest/s_autoTest_starttest_normaltest_menu.o $(BUILDPATH)/Applications/autotest/s_autoTest_starttest_performancetest_menu.o $(BUILDPATH)/Applications/autotest/s_autoTest_starttest_presstest_menu.o $(BUILDPATH)/Applications/autotest/s_autoTest_stoprecord_menu.o $(BUILDPATH)/Applications/autotest/s_autotest_menu.o $(BUILDPATH)/Applications/camera/s_camera_capture.o $(BUILDPATH)/Applications/camera/s_camera_multishot_show.o $(BUILDPATH)/Applications/camera/s_camera_preview.o $(BUILDPATH)/Applications/camera/s_camera_recorder.o $(BUILDPATH)/Applications/camera/s_camera_shot_show.o $(BUILDPATH)/Applications/explorer/s_explorer_delete_all_cnfm.o $(BUILDPATH)/Applications/explorer/s_explorer_delete_cnfm.o $(BUILDPATH)/Applications/explorer/s_explorer_paste_file.o $(BUILDPATH)/Applications/explorer/s_explorer_paste_folder.o $(BUILDPATH)/Applications/explorer/s_sd_format.o $(BUILDPATH)/Applications/explorer/s_set_default_path.o $(BUILDPATH)/Applications/explorer/s_set_disp_explorer.o $(BUILDPATH)/Applications/explorer/s_set_explorer_menu.o $(BUILDPATH)/Applications/explorer/s_set_save_path.o $(BUILDPATH)/Applications/image/s_img_browser.o $(BUILDPATH)/Applications/image/s_img_delete_all_cnfm.o $(BUILDPATH)/Applications/image/s_img_delete_cnfm.o $(BUILDPATH)/Applications/image/s_img_list.o $(BUILDPATH)/Applications/image/s_img_menu.o $(BUILDPATH)/Applications/image/s_img_set_slide.o $(BUILDPATH)/Applications/image/s_img_thumbnail_view.o $(BUILDPATH)/Applications/init/s_init_power_on.o $(BUILDPATH)/Applications/init/s_init_system.o $(BUILDPATH)/Applications/network/ctl_net_trans.o $(BUILDPATH)/Applications/network/s_ip_set.o $(BUILDPATH)/Applications/network/s_network_data_trans.o $(BUILDPATH)/Applications/network/s_network_menu.o $(BUILDPATH)/Applications/network/s_network_set_menu.o $(BUILDPATH)/Applications/network/s_ping_test.o $(BUILDPATH)/Applications/network/s_port_set.o $(BUILDPATH)/Applications/network/s_set_recv_path.o $(BUILDPATH)/Applications/network/s_set_send_file.o $(BUILDPATH)/Applications/network/s_tcp_client.o $(BUILDPATH)/Applications/network/s_tcp_server.o $(BUILDPATH)/Applications/network/s_udp_trans.o $(BUILDPATH)/Applications/pub_states/s_pub_alarm.o $(BUILDPATH)/Applications/pub_states/s_pub_bat_warn.o $(BUILDPATH)/Applications/pub_states/s_pub_del_file.o $(BUILDPATH)/Applications/pub_states/s_pub_empty.o $(BUILDPATH)/Applications/pub_states/s_pub_key_unlock.o $(BUILDPATH)/Applications/pub_states/s_pub_message_box.o $(BUILDPATH)/Applications/pub_states/s_pub_pre_message.o $(BUILDPATH)/Applications/pub_states/s_pub_reserved1.o $(BUILDPATH)/Applications/pub_states/s_pub_reserved2.o $(BUILDPATH)/Applications/pub_states/s_pub_reserved3.o $(BUILDPATH)/Applications/pub_states/s_pub_screen_saver.o $(BUILDPATH)/Applications/pub_states/s_pub_switch_off.o $(BUILDPATH)/Applications/pub_states/s_pub_usb.o $(BUILDPATH)/Applications/setup/s_display_switch.o $(BUILDPATH)/Applications/setup/s_set_adjust_speaker.o $(BUILDPATH)/Applications/setup/s_set_animated_speed.o $(BUILDPATH)/Applications/setup/s_set_disp_brightness.o $(BUILDPATH)/Applications/setup/s_set_disp_contrast.o $(BUILDPATH)/Applications/setup/s_set_disp_language.o $(BUILDPATH)/Applications/setup/s_set_disp_memory.o $(BUILDPATH)/Applications/setup/s_set_disp_ss_time.o $(BUILDPATH)/Applications/setup/s_set_disp_version.o $(BUILDPATH)/Applications/setup/s_set_factory_setting.o $(BUILDPATH)/Applications/setup/s_set_font.o $(BUILDPATH)/Applications/setup/s_set_lowbat_time.o $(BUILDPATH)/Applications/setup/s_set_menu.o $(BUILDPATH)/Applications/setup/s_set_personal_set_menu.o $(BUILDPATH)/Applications/setup/s_set_poff_time.o $(BUILDPATH)/Applications/setup/s_set_sys_update.o $(BUILDPATH)/Applications/setup/s_tscr_calibrate.o $(BUILDPATH)/Applications/setup/s_watchdog_test.o $(BUILDPATH)/Applications/standby/s_lib_version.o $(BUILDPATH)/Applications/standby/s_stdb_standby.o $(BUILDPATH)/Applications/statelist/Lib_state.o $(BUILDPATH)/Applications/timeset/Ctl_AlarmSet.o $(BUILDPATH)/Applications/timeset/Ctl_TimeSet.o $(BUILDPATH)/Applications/timeset/s_clk_world_map.o $(BUILDPATH)/Applications/timeset/s_func_calendar.o $(BUILDPATH)/Applications/timeset/s_set_alarm.o $(BUILDPATH)/Applications/timeset/s_set_alarm_sound.o $(BUILDPATH)/Applications/timeset/s_set_sysclock.o $(BUILDPATH)/Applications/timeset/s_timeset_menu.o $(BUILDPATH)/Applications/toolbox/s_toolbox_menu.o $(BUILDPATH)/Applications/toolbox/s_toolbox_usbhost.o $(BUILDPATH)/Applications/toolbox/s_usb_camera.o $(BUILDPATH)/Applications/videoplayer/s_video_add_movie.o $(BUILDPATH)/Applications/videoplayer/s_video_delete_all_cnfm.o $(BUILDPATH)/Applications/videoplayer/s_video_delete_cnfm.o $(BUILDPATH)/Applications/videoplayer/s_video_list.o $(BUILDPATH)/Applications/videoplayer/s_video_list_repeat_mode.o $(BUILDPATH)/Applications/videoplayer/s_video_menu.o $(BUILDPATH)/Applications/videoplayer/s_video_player.o $(BUILDPATH)/Applications/videoplayer/s_video_read_list.o $(BUILDPATH)/Applications/videoplayer/s_video_save_list.o

$(BUILDPATH)/$(MIDPATH).a:  $(BUILDPATH)/Middle/Library/SMCore/sm_port.o $(BUILDPATH)/Middle/Library/freetype/Lib_freetype.o $(BUILDPATH)/Middle/Library/ge/Lib_geshade.o $(BUILDPATH)/Middle/Library/image/Lib_ThumbsDB.o $(BUILDPATH)/Middle/Library/lwip/mac_if.o $(BUILDPATH)/Middle/Library/lwip/sys_arch.o $(BUILDPATH)/Middle/SourceCode/appframe/AKApp.o $(BUILDPATH)/Middle/SourceCode/appframe/AKAppMgr.o $(BUILDPATH)/Middle/SourceCode/appframe/AKFrameStream.o $(BUILDPATH)/Middle/SourceCode/appframe/AKList.o $(BUILDPATH)/Middle/SourceCode/appframe/AKMsgDispatch.o $(BUILDPATH)/Middle/SourceCode/appframe/AKPublicBGApp.o $(BUILDPATH)/Middle/SourceCode/appframe/AKSubThread.o $(BUILDPATH)/Middle/SourceCode/appframe/AKThread.o $(BUILDPATH)/Middle/SourceCode/appframe/AKVector.o $(BUILDPATH)/Middle/SourceCode/appframe/AKWnd.o $(BUILDPATH)/Middle/SourceCode/appframe/AKXxx.o $(BUILDPATH)/Middle/SourceCode/appframe/AKXxxApp.o $(BUILDPATH)/Middle/SourceCode/appframe/AKXxxBGApp.o $(BUILDPATH)/Middle/SourceCode/camera/Ctl_Capture.o $(BUILDPATH)/Middle/SourceCode/camera/Ctl_Preview.o $(BUILDPATH)/Middle/SourceCode/camera/Ctl_VideoRecord.o $(BUILDPATH)/Middle/SourceCode/camera/fwl_pfcamera.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_ClrSelect.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_ContextMenu.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_Dialog.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_DisplayList.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_FileList.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_Icon.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_IconExplorer.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_IconList.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_IconMenu.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_ListBox.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_ListItem.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_Msgbox.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_MultiSet.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_Progress.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_ScrollBar.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_SlipCalc.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_SlipItem.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_SlipItemMgr.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_SlipMgr.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_SlipMsg.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_SlipScrb.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_Text.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_Title.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_ToolBar.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_WaitBox.o $(BUILDPATH)/Middle/SourceCode/control/display/Eng_TopBar.o $(BUILDPATH)/Middle/SourceCode/control/logic/Eng_FileManage.o $(BUILDPATH)/Middle/SourceCode/control/logic/Eng_MsgQueue.o $(BUILDPATH)/Middle/SourceCode/control/logic/Eng_Pwr_OnOff_video.o $(BUILDPATH)/Middle/SourceCode/control/logic/Fwl_keyhandler.o $(BUILDPATH)/Middle/SourceCode/control/logic/preprochandler.o $(BUILDPATH)/Middle/SourceCode/debug/Eng_Debug.o $(BUILDPATH)/Middle/SourceCode/display/Dev_display.o $(BUILDPATH)/Middle/SourceCode/display/Fwl_DispOsd.o $(BUILDPATH)/Middle/SourceCode/display/Fwl_display.o $(BUILDPATH)/Middle/SourceCode/display/ImageLayer.o $(BUILDPATH)/Middle/SourceCode/display/fwl_font.o $(BUILDPATH)/Middle/SourceCode/display/fwl_graphic.o $(BUILDPATH)/Middle/SourceCode/display/fwl_pfdisplay.o $(BUILDPATH)/Middle/SourceCode/display/lcd_buffer.o $(BUILDPATH)/Middle/SourceCode/display/temp.o $(BUILDPATH)/Middle/SourceCode/filesystem/Fwl_usb_host.o $(BUILDPATH)/Middle/SourceCode/filesystem/fwl_osfs.o $(BUILDPATH)/Middle/SourceCode/filesystem/update.o $(BUILDPATH)/Middle/SourceCode/font/Eng_DynamicFont.o $(BUILDPATH)/Middle/SourceCode/font/Eng_Font.o $(BUILDPATH)/Middle/SourceCode/font/Eng_freetype.o $(BUILDPATH)/Middle/SourceCode/game/Fwl_gm.o $(BUILDPATH)/Middle/SourceCode/graphic/Eng_AkBmp.o $(BUILDPATH)/Middle/SourceCode/graphic/Eng_Graph.o $(BUILDPATH)/Middle/SourceCode/graphic/Eng_ImgConvert.o $(BUILDPATH)/Middle/SourceCode/graphic/Eng_ImgDec.o $(BUILDPATH)/Middle/SourceCode/graphic/Eng_Jpeg2Bmp.o $(BUILDPATH)/Middle/SourceCode/graphic/Eng_ScaleConvertSoft.o $(BUILDPATH)/Middle/SourceCode/graphic/Fwl_ImageLib.o $(BUILDPATH)/Middle/SourceCode/graphic/Fwl_gui.o $(BUILDPATH)/Middle/SourceCode/image/Ctl_ImgBrowse.o $(BUILDPATH)/Middle/SourceCode/init/AKMMIApp.o $(BUILDPATH)/Middle/SourceCode/init/Ak_Main.o $(BUILDPATH)/Middle/SourceCode/init/Eng_PowerOnThread.o $(BUILDPATH)/Middle/SourceCode/init/Fwl_Initialize.o $(BUILDPATH)/Middle/SourceCode/init/Fwl_Public.o $(BUILDPATH)/Middle/SourceCode/init/Fwl_multitask.o $(BUILDPATH)/Middle/SourceCode/init/eng_callback.o $(BUILDPATH)/Middle/SourceCode/init/system_serial.o $(BUILDPATH)/Middle/SourceCode/keypad/Eng_KeyTranslate.o $(BUILDPATH)/Middle/SourceCode/keypad/Eng_MctCode.o $(BUILDPATH)/Middle/SourceCode/keypad/fwl_pfKeypad.o $(BUILDPATH)/Middle/SourceCode/keypad/misc.o $(BUILDPATH)/Middle/SourceCode/math/Eng_Math.o $(BUILDPATH)/Middle/SourceCode/math/Eng_Math64.o $(BUILDPATH)/Middle/SourceCode/media/AKAudioBGApp.o $(BUILDPATH)/Middle/SourceCode/media/AKAudioListBGApp.o $(BUILDPATH)/Middle/SourceCode/media/AKMediaBGApp.o $(BUILDPATH)/Middle/SourceCode/media/AKVideoBGApp.o $(BUILDPATH)/Middle/SourceCode/media/AKVideoListBGApp.o $(BUILDPATH)/Middle/SourceCode/media/Ctl_APlayerList.o $(BUILDPATH)/Middle/SourceCode/media/Ctl_AVIPlayer.o $(BUILDPATH)/Middle/SourceCode/media/Ctl_AudioPlayer.o $(BUILDPATH)/Middle/SourceCode/media/Ctl_FlashPlayer.o $(BUILDPATH)/Middle/SourceCode/media/Ctl_MediaList.o $(BUILDPATH)/Middle/SourceCode/media/Ctl_RecAudio.o $(BUILDPATH)/Middle/SourceCode/media/Eng_Id3Inf.o $(BUILDPATH)/Middle/SourceCode/media/Fwl_pfAudio.o $(BUILDPATH)/Middle/SourceCode/media/Log_MediaAudio.o $(BUILDPATH)/Middle/SourceCode/media/Log_MediaDmx.o $(BUILDPATH)/Middle/SourceCode/media/Log_MediaPlayer.o $(BUILDPATH)/Middle/SourceCode/media/Log_MediaVideo.o $(BUILDPATH)/Middle/SourceCode/media/Log_MediaVisualAudio.o $(BUILDPATH)/Middle/SourceCode/media/Log_MotionDetec.o $(BUILDPATH)/Middle/SourceCode/media/Log_Mp3Player.o $(BUILDPATH)/Middle/SourceCode/media/Log_MpuRefresh.o $(BUILDPATH)/Middle/SourceCode/media/Log_RecAudio.o $(BUILDPATH)/Middle/SourceCode/media/Svc_MediaList.o $(BUILDPATH)/Middle/SourceCode/media/log_MediaEncode.o $(BUILDPATH)/Middle/SourceCode/media/log_media_recorder.o $(BUILDPATH)/Middle/SourceCode/media/log_videoDisp.o $(BUILDPATH)/Middle/SourceCode/media/log_videoZoom.o $(BUILDPATH)/Middle/SourceCode/media/sha204_device_personalizatio.o $(BUILDPATH)/Middle/SourceCode/memory/Fwl_osMalloc.o $(BUILDPATH)/Middle/SourceCode/network/Fwl_net.o $(BUILDPATH)/Middle/SourceCode/network/ping.o $(BUILDPATH)/Middle/SourceCode/pcmmanage/fwl_wavein.o $(BUILDPATH)/Middle/SourceCode/pcmmanage/fwl_waveout.o $(BUILDPATH)/Middle/SourceCode/power/Eng_AutoPowerOff.o $(BUILDPATH)/Middle/SourceCode/power/Eng_BatWarn.o $(BUILDPATH)/Middle/SourceCode/power/Eng_IdleThread.o $(BUILDPATH)/Middle/SourceCode/power/Eng_ScreenSave.o $(BUILDPATH)/Middle/SourceCode/power/fwl_power.o $(BUILDPATH)/Middle/SourceCode/queue/eng_queue.o $(BUILDPATH)/Middle/SourceCode/queue/fwl_evtmailbox.o $(BUILDPATH)/Middle/SourceCode/resource/Lib_res_port.o $(BUILDPATH)/Middle/SourceCode/string/Eng_DataConvert.o $(BUILDPATH)/Middle/SourceCode/string/Eng_GblString.o $(BUILDPATH)/Middle/SourceCode/string/Eng_String.o $(BUILDPATH)/Middle/SourceCode/string/Eng_String_UC.o $(BUILDPATH)/Middle/SourceCode/time/Eng_Alarm.o $(BUILDPATH)/Middle/SourceCode/time/Eng_Calendar.o $(BUILDPATH)/Middle/SourceCode/time/Eng_Time.o $(BUILDPATH)/Middle/SourceCode/time/Fwl_rtc.o $(BUILDPATH)/Middle/SourceCode/time/fwl_oscom.o $(BUILDPATH)/Middle/SourceCode/tscr/fwl_calibrate.o $(BUILDPATH)/Middle/SourceCode/tscr/fwl_tscrcom.o $(BUILDPATH)/Middle/SourceCode/tscr/ts_adc_CI37XX.o $(BUILDPATH)/Middle/SourceCode/tscr/tscr_input.o $(BUILDPATH)/Middle/SourceCode/usb/Fwl_USB.o $(BUILDPATH)/Middle/SourceCode/usb/fwl_sys_detect.o $(BUILDPATH)/Middle/SourceCode/usb/log_uvc_cam.o Makefile
	@echo ---------------------[build out $(MIDPATH).a:]----------------------------------
	-$(AR) -rsv $(BUILDPATH)/$(MIDPATH).a  $(BUILDPATH)/Middle/Library/SMCore/sm_port.o $(BUILDPATH)/Middle/Library/freetype/Lib_freetype.o $(BUILDPATH)/Middle/Library/ge/Lib_geshade.o $(BUILDPATH)/Middle/Library/image/Lib_ThumbsDB.o $(BUILDPATH)/Middle/Library/lwip/mac_if.o $(BUILDPATH)/Middle/Library/lwip/sys_arch.o $(BUILDPATH)/Middle/SourceCode/appframe/AKApp.o $(BUILDPATH)/Middle/SourceCode/appframe/AKAppMgr.o $(BUILDPATH)/Middle/SourceCode/appframe/AKFrameStream.o $(BUILDPATH)/Middle/SourceCode/appframe/AKList.o $(BUILDPATH)/Middle/SourceCode/appframe/AKMsgDispatch.o $(BUILDPATH)/Middle/SourceCode/appframe/AKPublicBGApp.o $(BUILDPATH)/Middle/SourceCode/appframe/AKSubThread.o $(BUILDPATH)/Middle/SourceCode/appframe/AKThread.o $(BUILDPATH)/Middle/SourceCode/appframe/AKVector.o $(BUILDPATH)/Middle/SourceCode/appframe/AKWnd.o $(BUILDPATH)/Middle/SourceCode/appframe/AKXxx.o $(BUILDPATH)/Middle/SourceCode/appframe/AKXxxApp.o $(BUILDPATH)/Middle/SourceCode/appframe/AKXxxBGApp.o $(BUILDPATH)/Middle/SourceCode/camera/Ctl_Capture.o $(BUILDPATH)/Middle/SourceCode/camera/Ctl_Preview.o $(BUILDPATH)/Middle/SourceCode/camera/Ctl_VideoRecord.o $(BUILDPATH)/Middle/SourceCode/camera/fwl_pfcamera.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_ClrSelect.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_ContextMenu.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_Dialog.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_DisplayList.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_FileList.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_Icon.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_IconExplorer.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_IconList.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_IconMenu.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_ListBox.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_ListItem.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_Msgbox.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_MultiSet.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_Progress.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_ScrollBar.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_SlipCalc.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_SlipItem.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_SlipItemMgr.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_SlipMgr.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_SlipMsg.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_SlipScrb.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_Text.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_Title.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_ToolBar.o $(BUILDPATH)/Middle/SourceCode/control/display/Ctl_WaitBox.o $(BUILDPATH)/Middle/SourceCode/control/display/Eng_TopBar.o $(BUILDPATH)/Middle/SourceCode/control/logic/Eng_FileManage.o $(BUILDPATH)/Middle/SourceCode/control/logic/Eng_MsgQueue.o $(BUILDPATH)/Middle/SourceCode/control/logic/Eng_Pwr_OnOff_video.o $(BUILDPATH)/Middle/SourceCode/control/logic/Fwl_keyhandler.o $(BUILDPATH)/Middle/SourceCode/control/logic/preprochandler.o $(BUILDPATH)/Middle/SourceCode/debug/Eng_Debug.o $(BUILDPATH)/Middle/SourceCode/display/Dev_display.o $(BUILDPATH)/Middle/SourceCode/display/Fwl_DispOsd.o $(BUILDPATH)/Middle/SourceCode/display/Fwl_display.o $(BUILDPATH)/Middle/SourceCode/display/ImageLayer.o $(BUILDPATH)/Middle/SourceCode/display/fwl_font.o $(BUILDPATH)/Middle/SourceCode/display/fwl_graphic.o $(BUILDPATH)/Middle/SourceCode/display/fwl_pfdisplay.o $(BUILDPATH)/Middle/SourceCode/display/lcd_buffer.o $(BUILDPATH)/Middle/SourceCode/display/temp.o $(BUILDPATH)/Middle/SourceCode/filesystem/Fwl_usb_host.o $(BUILDPATH)/Middle/SourceCode/filesystem/fwl_osfs.o $(BUILDPATH)/Middle/SourceCode/filesystem/update.o $(BUILDPATH)/Middle/SourceCode/font/Eng_DynamicFont.o $(BUILDPATH)/Middle/SourceCode/font/Eng_Font.o $(BUILDPATH)/Middle/SourceCode/font/Eng_freetype.o $(BUILDPATH)/Middle/SourceCode/game/Fwl_gm.o $(BUILDPATH)/Middle/SourceCode/graphic/Eng_AkBmp.o $(BUILDPATH)/Middle/SourceCode/graphic/Eng_Graph.o $(BUILDPATH)/Middle/SourceCode/graphic/Eng_ImgConvert.o $(BUILDPATH)/Middle/SourceCode/graphic/Eng_ImgDec.o $(BUILDPATH)/Middle/SourceCode/graphic/Eng_Jpeg2Bmp.o $(BUILDPATH)/Middle/SourceCode/graphic/Eng_ScaleConvertSoft.o $(BUILDPATH)/Middle/SourceCode/graphic/Fwl_ImageLib.o $(BUILDPATH)/Middle/SourceCode/graphic/Fwl_gui.o $(BUILDPATH)/Middle/SourceCode/image/Ctl_ImgBrowse.o $(BUILDPATH)/Middle/SourceCode/init/AKMMIApp.o $(BUILDPATH)/Middle/SourceCode/init/Ak_Main.o $(BUILDPATH)/Middle/SourceCode/init/Eng_PowerOnThread.o $(BUILDPATH)/Middle/SourceCode/init/Fwl_Initialize.o $(BUILDPATH)/Middle/SourceCode/init/Fwl_Public.o $(BUILDPATH)/Middle/SourceCode/init/Fwl_multitask.o $(BUILDPATH)/Middle/SourceCode/init/eng_callback.o $(BUILDPATH)/Middle/SourceCode/init/system_serial.o $(BUILDPATH)/Middle/SourceCode/keypad/Eng_KeyTranslate.o $(BUILDPATH)/Middle/SourceCode/keypad/Eng_MctCode.o $(BUILDPATH)/Middle/SourceCode/keypad/fwl_pfKeypad.o $(BUILDPATH)/Middle/SourceCode/keypad/misc.o $(BUILDPATH)/Middle/SourceCode/math/Eng_Math.o $(BUILDPATH)/Middle/SourceCode/math/Eng_Math64.o $(BUILDPATH)/Middle/SourceCode/media/AKAudioBGApp.o $(BUILDPATH)/Middle/SourceCode/media/AKAudioListBGApp.o $(BUILDPATH)/Middle/SourceCode/media/AKMediaBGApp.o $(BUILDPATH)/Middle/SourceCode/media/AKVideoBGApp.o $(BUILDPATH)/Middle/SourceCode/media/AKVideoListBGApp.o $(BUILDPATH)/Middle/SourceCode/media/Ctl_APlayerList.o $(BUILDPATH)/Middle/SourceCode/media/Ctl_AVIPlayer.o $(BUILDPATH)/Middle/SourceCode/media/Ctl_AudioPlayer.o $(BUILDPATH)/Middle/SourceCode/media/Ctl_FlashPlayer.o $(BUILDPATH)/Middle/SourceCode/media/Ctl_MediaList.o $(BUILDPATH)/Middle/SourceCode/media/Ctl_RecAudio.o $(BUILDPATH)/Middle/SourceCode/media/Eng_Id3Inf.o $(BUILDPATH)/Middle/SourceCode/media/Fwl_pfAudio.o $(BUILDPATH)/Middle/SourceCode/media/Log_MediaAudio.o $(BUILDPATH)/Middle/SourceCode/media/Log_MediaDmx.o $(BUILDPATH)/Middle/SourceCode/media/Log_MediaPlayer.o $(BUILDPATH)/Middle/SourceCode/media/Log_MediaVideo.o $(BUILDPATH)/Middle/SourceCode/media/Log_MediaVisualAudio.o $(BUILDPATH)/Middle/SourceCode/media/Log_MotionDetec.o $(BUILDPATH)/Middle/SourceCode/media/Log_Mp3Player.o $(BUILDPATH)/Middle/SourceCode/media/Log_MpuRefresh.o $(BUILDPATH)/Middle/SourceCode/media/Log_RecAudio.o $(BUILDPATH)/Middle/SourceCode/media/Svc_MediaList.o $(BUILDPATH)/Middle/SourceCode/media/log_MediaEncode.o $(BUILDPATH)/Middle/SourceCode/media/log_media_recorder.o $(BUILDPATH)/Middle/SourceCode/media/log_videoDisp.o $(BUILDPATH)/Middle/SourceCode/media/log_videoZoom.o $(BUILDPATH)/Middle/SourceCode/media/sha204_device_personalizatio.o $(BUILDPATH)/Middle/SourceCode/memory/Fwl_osMalloc.o $(BUILDPATH)/Middle/SourceCode/network/Fwl_net.o $(BUILDPATH)/Middle/SourceCode/network/ping.o $(BUILDPATH)/Middle/SourceCode/pcmmanage/fwl_wavein.o $(BUILDPATH)/Middle/SourceCode/pcmmanage/fwl_waveout.o $(BUILDPATH)/Middle/SourceCode/power/Eng_AutoPowerOff.o $(BUILDPATH)/Middle/SourceCode/power/Eng_BatWarn.o $(BUILDPATH)/Middle/SourceCode/power/Eng_IdleThread.o $(BUILDPATH)/Middle/SourceCode/power/Eng_ScreenSave.o $(BUILDPATH)/Middle/SourceCode/power/fwl_power.o $(BUILDPATH)/Middle/SourceCode/queue/eng_queue.o $(BUILDPATH)/Middle/SourceCode/queue/fwl_evtmailbox.o $(BUILDPATH)/Middle/SourceCode/resource/Lib_res_port.o $(BUILDPATH)/Middle/SourceCode/string/Eng_DataConvert.o $(BUILDPATH)/Middle/SourceCode/string/Eng_GblString.o $(BUILDPATH)/Middle/SourceCode/string/Eng_String.o $(BUILDPATH)/Middle/SourceCode/string/Eng_String_UC.o $(BUILDPATH)/Middle/SourceCode/time/Eng_Alarm.o $(BUILDPATH)/Middle/SourceCode/time/Eng_Calendar.o $(BUILDPATH)/Middle/SourceCode/time/Eng_Time.o $(BUILDPATH)/Middle/SourceCode/time/Fwl_rtc.o $(BUILDPATH)/Middle/SourceCode/time/fwl_oscom.o $(BUILDPATH)/Middle/SourceCode/tscr/fwl_calibrate.o $(BUILDPATH)/Middle/SourceCode/tscr/fwl_tscrcom.o $(BUILDPATH)/Middle/SourceCode/tscr/ts_adc_CI37XX.o $(BUILDPATH)/Middle/SourceCode/tscr/tscr_input.o $(BUILDPATH)/Middle/SourceCode/usb/Fwl_USB.o $(BUILDPATH)/Middle/SourceCode/usb/fwl_sys_detect.o $(BUILDPATH)/Middle/SourceCode/usb/log_uvc_cam.o

$(BUILDPATH)/$(AKOSPATH).a:  $(BUILDPATH)/AKOS/SourceCode/drv_init_cb/drv_init_callback.o $(BUILDPATH)/AKOS/SourceCode/init/akoslay_init.o $(BUILDPATH)/AKOS/SourceCode/nand/Fwl_sd.o $(BUILDPATH)/AKOS/SourceCode/nand/fwl_nandflash.o $(BUILDPATH)/AKOS/SourceCode/nand/fwl_nandflash_L.o $(BUILDPATH)/AKOS/SourceCode/nand/fwl_spiflash.o $(BUILDPATH)/AKOS/SourceCode/nand/mount.o $(BUILDPATH)/AKOS/SourceCode/nand/mount_pub_api.o $(BUILDPATH)/AKOS/SourceCode/nand/nand.o $(BUILDPATH)/AKOS/SourceCode/ram/raminit.o Makefile
	@echo ---------------------[build out $(AKOSPATH).a:]----------------------------------
	-$(AR) -rsv $(BUILDPATH)/$(AKOSPATH).a  $(BUILDPATH)/AKOS/SourceCode/drv_init_cb/drv_init_callback.o $(BUILDPATH)/AKOS/SourceCode/init/akoslay_init.o $(BUILDPATH)/AKOS/SourceCode/nand/Fwl_sd.o $(BUILDPATH)/AKOS/SourceCode/nand/fwl_nandflash.o $(BUILDPATH)/AKOS/SourceCode/nand/fwl_nandflash_L.o $(BUILDPATH)/AKOS/SourceCode/nand/fwl_spiflash.o $(BUILDPATH)/AKOS/SourceCode/nand/mount.o $(BUILDPATH)/AKOS/SourceCode/nand/mount_pub_api.o $(BUILDPATH)/AKOS/SourceCode/nand/nand.o $(BUILDPATH)/AKOS/SourceCode/ram/raminit.o

$(BUILDPATH)/$(TARGET).elf: $(SOBJS) $(COBJS) $(LIBS)   $(BUILDPATH)/$(APPPATH).a $(BUILDPATH)/$(MIDPATH).a $(BUILDPATH)/$(AKOSPATH).a
	@echo ---------------------[build out $(TARGET).elf:]----------------------------------
	$(LD) $(LDFLAGS) -o $(BUILDPATH)/$(TARGET).elf  $(BUILDPATH)/$(APPPATH).a $(BUILDPATH)/$(MIDPATH).a $(BUILDPATH)/$(AKOSPATH).a $(SOBJS) $(COBJS) $(LIBS)
	@$(OBJCOPY)  -text -c -s $(BUILDPATH)/$(TARGET).elf -output $(BUILDPATH)/$(TARGET).txt

msi: $(STATEPATH)/Lib_event.h  $(STATEPATH)/Lib_state.h  $(STATEPATH)/Lib_state.c

$(STATEPATH)/Lib_event.h  $(STATEPATH)/Lib_state.h  $(STATEPATH)/Lib_state.c: $(STATEPATH)/statelist.xls
	perl $(STATEPATH)/excelchg.pl $(STATEPATH)/statelist.xls $(STATEPATH)/states.cfg
	cd $(STATEPATH);  perl statecc.pl -f states.cfg

clean :
	-@$(RM) $(BUILDPATH)* *.bin
	-@$(RM) $(STATEPATH)/Lib_event.h  $(STATEPATH)/Lib_state.h  $(STATEPATH)/Lib_state.c $(STATEPATH)/states.cfg
	-@$(RM) ../../allincfiles

help:
	@echo "Usage:   make [TARGET] [VARIABLE=XXX]"
	@echo "TARGET:"
	@echo "	debug:   Builds a debug version, default target"
	@echo "		release: Builds a release version"
	@echo "		clean:   Remove all created objects "
	@echo "VARIABLES:"
	@echo "The VARIABLES values are case sensitive."
	@echo "	BUILDPATH: Directory of all build objects,"
	@echo "	e.g. BUILDPATH = c:/build, default: BUILDPATH=build"
	@echo "	Don't use a backslash!"
	@echo "	MMI_PDA:   build PDA version MMI, build in BUILDPATH/pda directory"
	@echo "		MMI_PDA=1"
	@echo "	MEMLEAK:   build memory leak version, build in BUILDPATHml directory"
	@echo "		MEMLEAK=1"
	@echo "	NOSOUNDLIB:  no link sound library?"
	@echo "		NOSOUNDLIB=1"
	@echo "	SUPPORT_EMAIL:  support email function?"
	@echo "		SUPPORT_EMAIL=1"
	@echo "	SUPPORT_MUX:  support multiplexer protocol?"
	@echo "		SUPPORT_MUX=1"
	@echo " INSTALL_GAME_NES: install nes game simulator?"
	@echo "		INSTALL_GAME_NES=1"
	@echo " INSTALL_GAME_SFC: install sfc game simulator?"
	@echo "		INSTALL_GAME_SFC=1"
	@echo " INSTALL_GAME_GBA: install gba game simulator?"
	@echo "		INSTALL_GAME_GBA=1"	
	@echo "	SUPPORT_BCR:  support BCR?"
	@echo "		SUPPORT_BCR=1"
	@echo "	SUPPORT_OFFICEVIEWER:  support OFFICEVIEWER?"
	@echo "		SUPPORT_OFFICEVIEWER=1"
	@echo "	SUPPORT_CARELAND_EMAP:  support careland emap?"
	@echo "		SUPPORT_CARELAND_EMAP=1"
	@echo "	SUPPORT_GPS:  use gps?"
	@echo "		SUPPORT_GPS=1"
	@echo "	SUPPORT_GLONAV_GPS:  use GLONAV gps?"
	@echo "		SUPPORT_GLONAV_GPS=1"
	@echo "	GLONAV_GPS_DEBUG:  use GLONAV gps debug mode?"
	@echo "		GLONAV_GPS_DEBUG=1"
	@echo "	SUPPORT_JAVA:  support JAVA?"
	@echo "		SUPPORT_JAVA=1"
	@echo "	SDRAM_8M:  support 8M SDRAM?"
	@echo "		SDRAM_8M=1"
	@echo "	SDRAM_32M:  support 32M SDRAM?"
	@echo "		SDRAM_32M=1"
	@echo "	NANDBOOT:  build nandboot version, build in BUILDPATH/nandboot directory"
	@echo "		NANDBOOT=1"
	@echo "	NO_NORFLASH:  build no nor-flash version"
	@echo "		NO_NORFLASH=1, use NANDBOOT=1 and NO_NORFLASH=1 to demo nandboot"
	@echo "	DEPENDS:  build and generate dependence information(.d) file"
	@echo "		DEPENDS=1 to open, if this flag opened, when .h file changes, related .c file would be recompiled automatically"
	@echo "	IGNOREERR:  ignore compile error when building"
	@echo "		IGNOREERR=1 to open, if this flag opened, ignore compile error when building"
	@echo ""
	@echo "Build folder example:"
	@echo ""
	@echo "	build/debug build debug nor-boot version of board mobile"
	@echo "	build/release build release nor-boot version of board mobile"
	@echo "	build/nandboot/debug build debug nand-boot version of board mobile"
	@echo "	build/nandboot/release build release nand-boot version of board mobile"
	@echo ""
	@echo "	build/pda/debug build debug nor-boot version of PDA mobile"
	@echo "	build/pda/release build release nor-boot version of PDA mobile"
	@echo "	build/pda/nandboot/debug build debug nand-boot version of PDA mobile"
	@echo "	build/pda/nandboot/release build release nand-boot version of PDA mobile"
	

# Rules


# ----------------------------- .s -> .o
ifdef IGNOREERR
$(BUILDPATH)/%.o:../../%.s
	@echo ---------------------build out .s obj file[$@]----------------------------------
	-$(AS) $(ASFLAGS) -o $@ $<
else
$(BUILDPATH)/%.o:../../%.s
	@echo ---------------------build out .s obj file[$@]----------------------------------
	$(CC) $(CFLAGS) $(INCLUDE) -E -o $(BUILDPATH)/preproc.s $<	
	$(AS) $(ASFLAGS) $(INCLUDE) -o $@ $(BUILDPATH)/preproc.s
	$(RM) $(BUILDPATH)/preproc.s

endif

# ----------------------------- .c -> .o
$(BUILDPATH)/$(APPPATH)/%.o:../../$(APPPATH)/%.c
	@echo ---------------------build out .c obj file[$@]----------------------------------
	$(CC) -c $(CFLAGS) $(MIDHPATH) -o $@ $<

$(BUILDPATH)/$(MIDPATH)/%.o:../../$(MIDPATH)/%.c
	@echo ---------------------build out .c obj file[$@]----------------------------------
	$(CC) -c $(CFLAGS) $(AKOSHPATH) $(MIDHPATH) $(MIDINPATH) $(DRVHPATH) -o $@ $<
	
$(BUILDPATH)/$(AKOSPATH)/%.o:../../$(AKOSPATH)/%.c
	@echo ---------------------build out .c obj file[$@]----------------------------------
	$(CC) -c $(CFLAGS) $(DRVHPATH) $(AKOSHPATH) $(AKOSINPATH) -o $@ $<
	
$(BUILDPATH)/$(DRVPATH)/%.o:../../$(DRVPATH)/%.c
	@echo ---------------------build out .c obj file[$@]----------------------------------
	$(CC) -c $(CFLAGS) $(DRVHPATH) $(DRVINPATH)  $(AKOSHPATH)  -o $@ $<

ifdef DEPENDS
# ----------------------------- .c,.s -> d
ifdef IGNOREERR
$(BUILDPATH)/%.d: ../../%.c
	@echo ---------------------build out depend file[$@]----------------------------------
	-$(CC) -MD -E $(CFLAGS) $(INCLUDE) -o $@ $< \
	| sed 's/\($(notdir $*)\)\.o[ :]*/$(subst /,\/,$(join $(BUILDPATH)/,$(dir $*)))\1.o $(subst /,\/,$@) : /g' > $@;
else
$(BUILDPATH)/%.d: ../../%.c
	@echo ---------------------build out depend file[$@]----------------------------------
	$(CC) -MD -E $(CFLAGS) $(INCLUDE) -o $@ $< \
	| sed 's/\($(notdir $*)\)\.o[ :]*/$(subst /,\/,$(join $(BUILDPATH)/,$(dir $*)))\1.o $(subst /,\/,$@) : /g' > $@;
endif

endif

CTOHDEPS    := $(patsubst %.o, %.d, $(COBJS))
# $(SOBJS))	$(CC) -MD xxx.s can't generate the .d file

ifdef DEPENDS
ctohdepends: $(CTOHDEPS)
	@echo ---------------------[build depend files]---------------------
else
ctohdepends:

endif


ifeq ($(MAKECMDGOALS),maketarget)
ifdef DEPENDS
include $(CTOHDEPS)
endif
endif
