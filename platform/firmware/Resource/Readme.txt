1.Resource目录中的图片资源说明：
	
    目前只使用两种图片类型：BMP和PNG，不再使用JPG图片，资源中的JPG图片将和声音资源一样作为普通二进制数据看待。 

    综合显示性能和内存占用的衡量，推荐的使用图片原则：
        1. BMP图片必须是16位565格式的; 
        2. 对于PNG图片，只有使用了渐变效果的才使用带alpha通道的PNG，否则建议使用565BMP或不带alpha通道的PNG作为图片资源。


补充：AkResData.Bin中不带alpha通道的PNG经DynamicLoad库加载到RAM后的图像数据格式，是16位565格式BMP图像数据。
                         而带alpha通道的PNG，加载到RAM后则是32位BMP图像数据。（可参看Res_PngToBmp函数）

2.编译批处理说明
build_37xx_480x272.cmd    用来产生适合480x272分辨率屏幕的资源
build_37xx_qvga.cmd       用来产生适合320x240分辨率屏幕的资源
build_37xx_qvga_spi.cmd   用来产生适合320x240分辨率屏幕spi启动版本的资源
build_37xx_qvga_win32.cmd 用来产生适合模拟器用的资源，屏幕分辨率320x240