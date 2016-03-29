编译说明

1.编译批处理文件build_CI3771_8M_RAM.bat
此文件用来编译3771或3760芯片、8m内存，nand启动模式的bios。通过修改文件里的参数项，可以编译需要的bios。

2.文件内容如下
make CHIP=AK3771 SDRAM=8 BOOT_MODE=NAND 2>error.txt

3.参数项说明如下

CHIP=AK3771      芯片类型配置项，可选的配置项AK3771,AK3750,AK3753 ; 3760芯片使用AK3771的配置项
SDRAM=8          内存大小配置项，可选的配置项8,16,32
BOOT_MODE=NAND   启动模式配置项，可选的配置项NAND,SPI,SDIO,SDMMC