@echo off
perl .\pub\readxls.pl .\pub\string.xls .\320x240_spi\qvga_bin_37xx_spi.xls
.\pub\ResMaker.exe SPIBOOT=Y SUPPORT_VIDEOPLAYER=Y SUPPORT_AUDIOPLAYER=Y SUPPORT_IMG_BROWSE=Y CAMERA_SUPPORT=Y SUPPORT_TOOLBOX=Y USB_HOST=Y SUPPORT_AUDIOREC=Y SUPPORT_UVC=Y SUPPORT_EXPLORER=Y SUPPORT_SYS_SET=Y SUPPORT_AUTOTEST=N
del /q/f string.txt
del /q/f binary.txt
del *.aktmp

