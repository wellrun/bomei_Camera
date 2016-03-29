#ifndef    _GUIREG_H_
#define    _GUIREG_H_

#define    GUI_BASE_ADDR         0x20022000            // 2D & 3D graphic accelorator base address

#define    GUI_CMD_ADDR          (GUI_BASE_ADDR+0x0)  // draw command register,
#define    GUI_ALPHA_SHIFT       7                     // logic operation or alpha operation,
#define    GUI_DRAW_TYPE_SHIFT   8                     // draw type:rectangle 1, rect width scale and conversion 0
#define    GUI_FCOLOR_ENABLE_SHIFT   9                // source color is from forground color or source image
#define    GUI_DST_ROTATE_SHIFT      10               // roate 90 for RGB565, if rotate, set 1. 
#define    GUI_CLIP_ENABLE_SHIFT     11                // clip enable shift
#define    GUI_COLOR_TRANS_SHIFT     12                // color transparency enable shift
#define    GUI_COLOR_POLARITY_SHIFT  13                // color transparency polarity, update color if equal color or not
#define    GUI_COLOR_CMP_SOURCE_SHIFT 14               // color transparency cmpare with source image or dest image 
#define    GUI_CMD_START_SHIFT        15               // rectangle operarion start bit.

#define    GUI_INTMASK_ADDR      (GUI_BASE_ADDR+0x04)
#define    GUI_INTMASK_REGINT0_SHIFT  0
#define    GUI_INTMASK_REGINT1_SHIFT  1

#define    GUI_INTSTATUS_ADDR         (GUI_BASE_ADDR+0x08)
#define    GUI_INT_REGINT0_SHIFT      0               // gui operation over interrupt reg.
#define    GUI_INT_REGINT1_SHIFT      1               // luminance adjust download over interrupt reg

#define    GUI_SRCBASE_ADDR      (GUI_BASE_ADDR+0x0c)  // source image base address register, 29 bits
#define    GUI_SRCSTRD_ADDR      (GUI_BASE_ADDR+0x10)  // source stride register, 12 bits
#define    GUI_SRCXY_ADDR        (GUI_BASE_ADDR+0x14)  // source start x,y coordinates , x, y: 12bits each
#define    GUI_SRCXY_XSHIFT      0                     // start x shift
#define    GUI_SRCXY_YSHIFT      16                    // start y shift

#define    GUI_DSTBASE_ADDR      (GUI_BASE_ADDR+0x18)  // destination image base address register, 29 bits
#define    GUI_DSTSTRD_ADDR      (GUI_BASE_ADDR+0x1c)  // dest image line stride , 12 bits
#define    GUI_DSTXY_ADDR        (GUI_BASE_ADDR+0x20)  // dst start x, y coordinates, x, y: 12 bits
#define    GUI_DSTXY_XSHIFT      0
#define    GUI_DSTXY_YSHIFT      16

#define    GUI_SRCBITBLT_ADDR    (GUI_BASE_ADDR+0x24)  // source bit blit width and height register, 12 bits each
#define    GUI_SRCBITBLT_WSHIFT  0                     // width shift
#define    GUI_SRCBITBLT_HSHIFT  16                    // height shift

#define    GUI_FCOLOR_ADDR       (GUI_BASE_ADDR+0x28)  // forground color for rectangle filling, 24 bits

#define    GUI_CLIPLT_ADDR       (GUI_BASE_ADDR+0x2c)  // clip register left+top  left, top:12-bits
#define    GUI_CLIPRB_ADDR       (GUI_BASE_ADDR+0x30)  // clip register right+bottom right, bottom: 12 bits
#define    GUI_CLIPLT_LSHIFT     0                     // left shift
#define    GUI_CLIPLT_TSHIFT     16                    // top  shift
#define    GUI_CLIPRB_RSHIFT     0                     // right shift
#define    GUI_CLIPRB_BSHIFT     16                    // bottom shift

#define    GUI_TRANSMINCOL_ADDR   (GUI_BASE_ADDR+0x34)  // color compare register, 24 bits &0xf8fcf8 color for dest color transparency

#define    GUI_SCAL_FILTER_CTRL_ADDR      (GUI_BASE_ADDR+0x38) // scale filter control register
#define    GUI_SCAL_START_BIT  0                    // scale start doing bit
#define    GUI_SCAL_HBYPASS_BIT    1                    // horizonal scale is bypass or not, 0-not bypass, 1-bypass
#define    GUI_SCAL_VBYPASS_BIT    2                    // vertical  scale is bypass or not, 0-not bypass, 1-bypass
#define    GUI_SCAL_FILTER_OFORMAT_BIT    3                    // output image format: 1 yuv420, 0 rgb565.
#define    GUI_SCAL_FILTER_IFORMAT_BIT    4                    // input image format: 1 yuv420, 0 rgb565.
#define    GUI_SCAL_DST_NEEDED      5                    // if background picture is needed, used for blending, logic operation.
#define    GUI_SCAL_INV_HEIGHT      6                    // inverse height value, 12 bit
#define    GUI_FILTER_COLUMN          18                   // rect2_h/62, if rect2_h%62 > 1, rect2_h/62++, 
#define    GUI_FILTER_START_BIT 26
#define    GUI_LUMINANCE_ENABLE_BIT 27
#define    GUI_LUMINANCE_ONLY_BIT 28

#define    GUI_SCALRATIO_ADDR     (GUI_BASE_ADDR+0x3c) // scale ration set, 65536/(scale_width - 1), and 65536/(scale_heigt - 1)
#define    GUI_SCALRATIO_HSHIFT   0
#define    GUI_SCALRATIO_VSHIFT   16
#define    GUI_SCALRATIO_FACTOR   65536

#define    GUI_SCALSRC1BASE_ADDR     (GUI_BASE_ADDR+0x40) //1-st color base address register,if RGB888,this is image base address , if YUV420, this is Y-component base addr

#define    GUI_SCALSRC2BASE_ADDR     (GUI_BASE_ADDR+0x44) //2-nd color base address register, if RGB888, NO USE, if YUV420, this is U-component base addr

#define    GUI_SCALSRC3BASE_ADDR     (GUI_BASE_ADDR+0x48) //3-rd color base address register, if RGB888, NO USE, if YUV420, this is V-component base addr

#define    GUI_SCALSRCSTRD_ADDR   (GUI_BASE_ADDR+0x4c) //source image line stride!

#define    GUI_SCALSRCOFFSET_ADDR    (GUI_BASE_ADDR+0x50) //source image bliting to output image offset
#define    GUI_SCALSRCOFFSET_XSHIFT  0                    // x shift
#define    GUI_SCALSRCOFFSET_YSHIFT  16                   // y shift

#define    GUI_SCALSRCRECT_ADDR      (GUI_BASE_ADDR+0x54) //source image width and height register
#define    GUI_SCALSRCRECT_WSHIFT    0                //source image width shift
#define    GUI_SCALSRCRECT_HSHIFT    16               //source image height shift

#define    GUI_SCALDST1BASE_ADDR   (GUI_BASE_ADDR+0x58) //dst image base register 1

#define    GUI_SCALDST2BASE_ADDR   (GUI_BASE_ADDR+0x5c) //dst image base register 2

#define    GUI_SCALDST3BASE_ADDR   (GUI_BASE_ADDR+0x60) //dst image base register 3

#define    GUI_SCALDSTSTRD_ADDR     (GUI_BASE_ADDR+0x64) //dst image line stride!

#define    GUI_SCALDSTOFFSET_ADDR    (GUI_BASE_ADDR+0x68) //scaled image bliting to output image offset
#define    GUI_SCALDSTOFFSET_XSHIFT  0                    // x shift
#define    GUI_SCALDSTOFFSET_YSHIFT  16                   // y shift

#define    GUI_SCALDSTRECT_ADDR     (GUI_BASE_ADDR+0x6c) //dst image width and height register
#define    GUI_SCALDSTRECT_WSHIFT   0
#define    GUI_SCALDSTRECT_HSHIFT   16

#define    GUI_LUMINANCE_ADDR        (GUI_BASE_ADDR+0x70)  //luminance transform table register
#define    GUI_LUMINANCE_BASE_SHIFT   0                    // table addres shift, 29 bits
#define    GUI_LUMINANCE_LOAD_START_SHIFT  29              // load luminance table start bit
#define    GUI_LUMINANCE_LOAD_DONE_SHIFT   30              // load done signal bit.

#define    GUI_TRANSMAXCOL_ADDR      (GUI_BASE_ADDR+0x74)
#define    GUI_YUVSTRIDE_ADDR        (GUI_BASE_ADDR+0x78)
//////////////////////////////////////////////////////
// the register addr for YUV rotate
//////////////////////////////////////////////////////

#define YUVROTATE_BASE_ADDR                  (0x2002f000)         // gui registers base address

#define YUVROTATE_CTRL_REG                   (YUVROTATE_BASE_ADDR+0x0)
#define YUVROTATE_SIZE_REG                   (YUVROTATE_BASE_ADDR+0x4)
#define YUVROTATE_SADDR_REG                  (YUVROTATE_BASE_ADDR+0x8)
#define YUVROTATE_DADDR_REG                  (YUVROTATE_BASE_ADDR+0xc)

////////////////////////// register bit define ////////////////////////////////////
// YUV rotation control register bit
#define YUVROTATE_START_BIT            0
#define YUVROTATE_INTEN_BIT            1
#define YUVROTATE_INTSTAT_BIT          2
#define YUVROTATE_UVINTERL_BIT         3
#define YUVROTATE_ANGLE_BIT            4
#define YUVROTATE_OUT_ZERO_BIT         6

// YUV rotation size reigster bit
#define YUVROTATE_WIDTH_BIT            0
#define YUVROTATE_HEIGHT_BIT          16

#endif

