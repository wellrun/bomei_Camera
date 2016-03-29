@echo off
perl .\pub\readxls.pl .\pub\string.xls .\320x240\qvga_bin_37xx.xls
.\pub\ResMaker.exe NAND_SD_RES=Y SUPPORT_EBK=Y SUPPORT_VIDEOPLAYER=Y SUPPORT_AUDIOPLAYER=Y SUPPORT_IMG_BROWSE=Y CAMERA_SUPPORT=Y SUPPORT_TOOLBOX=Y SUPPORT_FM=Y SUPPORT_EMAP=Y SUPPORT_AUDIOREC=Y INSTALL_GAME=Y INSTALL_GAME_7COLOR=Y INSTALL_GAME_RECT=Y INSTALL_GAME_PIGBOAT=Y SUPPORT_CALC=Y SUPPORT_EXPLORER=Y SUPPORT_SYS_SET=Y SUPPORT_AUTOTEST=N
del /q/f string.txt
del /q/f binary.txt
del *.aktmp
cp AkResData.Bin W:\AkResData.bin
cp .\pub\LangCodepage.bin W:\LangCodepage.bin
cp .\pub\DynamicFont4_16.bin W:\DynamicFont4_16.bin
cp .\pub\DynamicFont4_12.bin W:\DynamicFont4_12.bin
