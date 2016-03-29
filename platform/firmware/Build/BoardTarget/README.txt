/*******************************************************************************
编译说明文件：
    介绍本目录下的perl文件、bat文件、command_XXX.txt和所有编译参数的含义和用法。
*******************************************************************************/

1. 目录文件类型
    (1). *.pl文件： 用于生成makefile文件；
    (2). *.bat文件：编译批处理文件。
    (3). command_XXX.txt文件：编译参数文件。

2. 使用步骤
    (1). 安装完perl和cygwin软件，正确设置环境变量（详见编译、烧录文档）；
    (2). 选择需要的perl文件，双击和在命令行执行，生成makefile文件；   
    (3). 选择需要的编译参数文件command_XXX.txt，拷贝一份并重命名为command.txt；
    (4). 选择Build.bat文件，双击和在命令行执行；
    (5). 编译过程开始，大概需要5分钟即可生成可执行文件Sword37.bin，通过烧录工具，把资源文件和Sword37.bin一起写入系统，即可运行。

3. 各command_XXX.txt文件差别
    (1). command_CI3760_svt.txt：AK3760C svt硬件开发板 ，320*240尺寸lcd，8M SDRAM，SPIBOOT的编译参数文件 
    (2). command_CI3750.txt    ：AK3750C 硬件开发板, 320*240尺寸lcd，8M SDRAM，SPIBOOT的编译参数文件；
注：command_CI3760_svt.txt 仅供内部使用，此配置对应的开发板不外发。    
    

4. 编译参数说明
    以上command_XXX.txt文件中的编译参数，用户可以根据需要自行设置,选Y或N那类参数，等号右边选Y，表示支持，选N表示不支持。
   
注意：1、每个编译参数占一行，不要多写无关的文字符号。
      2、若某行编译参数临时不需要，可以在行首用一个英文的分号; 将其注释掉，这样它就不会参与编译。

各参数含义是：

    CHIP=AK3750 ：    	处理器型号选择，使用AK3750C处理器
    PLATFORM=CI37XX :   选择CI37XX 硬件平台；
		
    KEYPAD_TYPE：       键盘选择, GPIO按键: 0, AD按键: 1,
    
    PLATFORM_DEBUG_VER=N :  Y: 调试版本(打开部分调试功能)，N: 发布版本

    BOOT_MODE=SPI :    SPI:SPI启动模式 , 其他模式暂不支持

    LCD=MPU_R61580 : LCD TYPE
    LCDWIDTH=480 LCDHEIGHT=272：lcd480×272的；
    LCDWIDTH=320 LCDHEIGHT=240：lcd320×240的；

    RAM_SIZE=8 : RAM的大小是8M
    RAM_SIZE=16 : RAM的大小是16M

    ENABLE_COMPRESS_BIN=Y: 支持压缩
    
    SUPPORT_TSCR=Y ：       是否支持触摸屏，可选项，目前只能选N；

    TVOUT_TYPE=0: 不使用TV OUT功能
    TVOUT_TYPE=1: 使用内置TV OUT输出功能；
    TVOUT_TYPE=2: 使用TV OUT外挂芯片进行tv输出；
    TVOUT_TYPE=3: 使用TV OUT外挂芯片进行tv输出,而且7026的TV输出为720×480；
    
    SUPPORT_GESHADE=Y ：    支持GE动效，可选项,16M以上版本才支持

    SUPPORT_EXTERN_RTC=Y ： 支持外挂RTC，可选项；
    SUPPORT_DEBUG_OUTPUT=Y :支持串口调试, 可选项;   
    SUPPORT_DEBUG_OUTPUT_USB=Y: 支持USB模式调试信息, 此选项需SUPPORT_DEBUG_OUTPUT=Y才有效 
    SUPPORT_LCDPWM=Y

    SUPPORT_AUDIOPLAYER=Y ：支持音频播放，可选项；
    SUPPORT_CAMERA=Y ：     支持照相机，可选项；
    SUPPORT_AUDIOREC=Y ：   支持录音，可选项；
    SUPPORT_VIDEOPLAYER=Y ：支持视频播放，可选项；
    SUPPORT_IMG_BROWSE=Y ： 支持图片浏览，可选项；
    
    SUPPORT_UVC=Y ：	    支持pc camera，此选项需SUPPORT_CAMERA=Y才有效
    SUPPORT_HOST=Y ：       支持host主机模式，可选项；
    SUPPORT_EXPLORER=Y :    支持资源管理器，可选项；
    SUPPORT_SYS_SET=Y :     支持系统设置，可选项；

    SUPPORT_NETWORK=Y  :    支持网络应用， 可选项；
    SUPPORT_AUTOTEST=N：支持自动化测试可选项；
    
5. SD BOOT的使用方法：
   暂不支持SD BOOT

6. SPI BOOT的使用方法：
   1). SPI版本支持压缩,要把编译宏置为"Y":ENABLE_COMPRESS_BIN=Y;
   2). 编译资源请执行“build_37xx_qvga_spi.cmd”;
   3). 烧录工具请使用配置文件"config.txt”;
   4). 不需要编译bios

7、声明
   对于上述没有介绍到文件、编译参数是未开发完成版本，供anyka内部研发使用，客户擅自选择可能会带来不可预知后果，责任自负。
