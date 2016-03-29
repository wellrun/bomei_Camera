#include "drv_api.h"
#include "guireg.h"

#define  MAX_WIDTH        1280
#define  MAX_HEIGHT       1024

#ifdef AKOS
#define  MAX_WAIT_COUNTER   500
#else
#define  MAX_WAIT_COUNTER   10000000
#endif

#define GUI_GET_REG(addr)               (*((volatile T_U32*)(addr)))
#define GUI_SET_REG(addr, value)        ((*((volatile T_U32*)(addr))) = (value))

#define CLK_RESET_ADDR              0x08000010            //soft clock reset address
#define CLK_ENABLE_ADDR             0x0800000c

T_U8  isYUVROTRunning = AK_FALSE;
T_U8  is2DGUIRunning  = AK_FALSE;

T_U8 GUI_Wait_For_Done(T_VOID);

void reset_2D(T_VOID)
{
    T_U32   reg_value, i;

    //reset
    reg_value = GUI_GET_REG(CLK_RESET_ADDR);
    reg_value |= (1<<12);
    GUI_SET_REG(CLK_RESET_ADDR, reg_value);
    //wait a piece time
    for(i=0; i<20; i++) ;

    //default instruction
    reg_value = GUI_GET_REG(CLK_RESET_ADDR);
    reg_value &= ~(1<<12);
    GUI_SET_REG(CLK_RESET_ADDR, reg_value);
    is2DGUIRunning = AK_FALSE;

    //set status register to zero
    GUI_SET_REG(GUI_INTSTATUS_ADDR, 0);
}

void Enable_2DModule(T_VOID)
{
    T_U32    reg_value = 0, i;

    reg_value = GUI_GET_REG(CLK_ENABLE_ADDR);
    reg_value |= (1<<12);
    GUI_SET_REG(CLK_ENABLE_ADDR, reg_value);

    //enable 2D graphic 0-->enable 1-->disable
    reg_value = GUI_GET_REG(CLK_ENABLE_ADDR);
    reg_value &= ~(1<<12);
    GUI_SET_REG(CLK_ENABLE_ADDR, reg_value);
#if 0
    reg_value = GUI_GET_REG(0x08000178);
    reg_value |= (1<<29);
    GUI_SET_REG(0x08000178, reg_value);
#endif
    reset_2D();
}

void Close_2DModule(T_VOID)
{
    T_U32 reg_value;

    GUI_Wait_For_Done();
    
    reg_value = GUI_GET_REG(CLK_ENABLE_ADDR);
    reg_value |= (1<<12);
    GUI_SET_REG(CLK_ENABLE_ADDR, reg_value);

    is2DGUIRunning = AK_FALSE;
}

void Reset_2DGraphic(T_VOID)
{
}

static void reset_YUV(T_VOID)
{
    T_U32   reg_value, i;

    //reset
    reg_value = GUI_GET_REG(CLK_RESET_ADDR);
    reg_value |= (1<<14);
    GUI_SET_REG(CLK_RESET_ADDR, reg_value);
    //wait a piece time
    for(i=0; i<20; i++) ;

    //default instruction
    reg_value = GUI_GET_REG(CLK_RESET_ADDR);
    reg_value &= ~(1<<14);
    GUI_SET_REG(CLK_RESET_ADDR, reg_value);
    isYUVROTRunning = AK_FALSE;

    //set operation status to zero
    GUI_SET_REG(YUVROTATE_CTRL_REG, 0);
}

void Enable_YUVROTModule(T_VOID)
{
    T_U32    reg_value = 0, i;
    T_U32    dma_cfg_value;

    reg_value = GUI_GET_REG(CLK_ENABLE_ADDR);
    reg_value |= (1<<14);
    GUI_SET_REG(CLK_ENABLE_ADDR, reg_value);

    //enable 2D graphic 0-->enable 1-->disable
    reg_value = GUI_GET_REG(CLK_ENABLE_ADDR);
    reg_value &= ~(1<<14);
    GUI_SET_REG(CLK_ENABLE_ADDR, reg_value);

    dma_cfg_value = *(volatile T_U32 *)0x2002d004;
    dma_cfg_value &= 0xbfffffff;
    dma_cfg_value |= 0x20000000;
    GUI_SET_REG(0x2002d004, dma_cfg_value);

    reset_YUV();
}

void Close_YUVROTModule(T_VOID)
{
    T_U32 reg_value;

    reg_value = GUI_GET_REG(CLK_ENABLE_ADDR);
    reg_value |= (1<<14);
    GUI_SET_REG(CLK_ENABLE_ADDR, reg_value);

    isYUVROTRunning = AK_FALSE;
}

void Reset_YUVROTGraphic(T_VOID)
{
}

T_U8 YUVROT_Wait_For_Done(T_VOID)
{
    T_U32 cnt, regVal;
    cnt = 0;
    do{

#ifdef AKOS
        AK_Sleep(1);
#endif

        regVal = GUI_GET_REG(YUVROTATE_CTRL_REG);
        cnt++;
    } while (!(regVal & (0x01<<YUVROTATE_INTSTAT_BIT)) && (cnt < MAX_WAIT_COUNTER));

    isYUVROTRunning = AK_FALSE;
    if (regVal & (1<<YUVROTATE_INTSTAT_BIT)){
        regVal &= ~(0x1<<YUVROTATE_INTSTAT_BIT);
        GUI_SET_REG(YUVROTATE_CTRL_REG, regVal);
        return GUI_ERROR_OK;
    }

    return GUI_ERROR_TIMEOUT;
}

T_U8 GUI_Wait_For_Done(T_VOID)
{
    T_U32 counter, ret;
    counter = 0;
    do {

#ifdef AKOS
        AK_Sleep(1);
#endif
        ret = GUI_GET_REG(GUI_INTSTATUS_ADDR);
        counter++;
    }while((counter<MAX_WAIT_COUNTER) && !(ret&0x1));

    is2DGUIRunning = AK_FALSE;
    if (ret & 0x1){
        ret &= ~(0x1);
        GUI_SET_REG(GUI_INTSTATUS_ADDR, ret);
        return GUI_ERROR_OK;
    }

    return GUI_ERROR_TIMEOUT;
}

/*
 * @BRIEF                 image scale and format convert function, support format:yuv420, rgb565 -- no block version 
 * @PARAM ibuff           src buffer address arrays
 * @PARAM nibuff          num of src buffer address array.
 * @PARAM srcWidth        src image width
 * @PARAM srcRectX        src offset x
 * @PARAM srcRectY        src offset y
 * @PARAM srcRectW        src scale width
 * @PARAM srcRectH        src scale height
 * @PARAM format_in       src image format: FORMAT_YUV420, FORMAT_RGB565
 * @PARAM obuff           dst buffer address arrays
 * @PARAM nobuff          num of dst buffer address array
 * @PARAM dstWidth        dst image width
 * @PARAM dstRectX        dst offset x
 * @PARAM dstRectY        dst offset y
 * @PARAM dstRectW        dst scaled width
 * @PARAM dstRectH        dst scaled height
 * @PARAM format_out      dst image format:FORMAT_YUV420, FORMAT_RGB565
 * @PARAM luminance_enabled   if input image format is FORMAT_YUV420, this param can set AK_TRUE to transform luminance.
 * @PARAM luminance_table     a continuous memory buffer width length of 256 byte, used to transform yuv420 luminance.
 */
T_U8 Scale_ConvertNoBlock(T_U32 *ibuff, T_U32 nibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                     T_U16 srcRectH, E_ImageFormat format_in, 
                     T_U32* obuff, T_U8 nobuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY, T_U16 dstRectW, 
                     T_U16 dstRectH, E_ImageFormat format_out,
                     T_U8  luminance_enabled, T_U8* luminance_table)
{
    T_U32  cmd, ret, counter, value;
    T_BOOL  hbypass, vbypass;
    T_U16   invHeight;
    T_U32   stride=0;
	
    if (AK_TRUE == is2DGUIRunning)
    {
    	//akprintf(C3, M_DRVSYS, "@@@@ graphic: module is running @@@@\n");
        return GUI_ERROR_MODULE_RUN;
    }

    if (srcRectW > MAX_WIDTH ||
        srcRectH > MAX_HEIGHT||
        dstRectW > MAX_WIDTH ||
        dstRectH > MAX_HEIGHT)
    {
    	//akprintf(C3, M_DRVSYS, "@@@@ graphic:over the size @@@@\n");
        return GUI_ERROR_PARAMETER;
    }

    if (srcRectW < 18 ||
        srcRectH < 18 ||
        dstRectW < 18 ||
        dstRectH < 18)
    {
    	//akprintf(C3, M_DRVSYS, "@@@@ graphic:below the size @@@@\n");
        return GUI_ERROR_PARAMETER;
    }

    //in sundance3, the zoom facter is 4,
    //in sundance3e, the zoom factor is 10.
    if ((srcRectW*10) < dstRectW ||
        (srcRectH*10) < dstRectH)
    {
    	//akprintf(C3, M_DRVSYS, "@@@@ graphic:zoom factor is over 4 @@@@\n");
        return GUI_ERROR_PARAMETER;
    }

    if (FORMAT_YUV420 != format_in && AK_FALSE != luminance_enabled)
    {
        return GUI_ERROR_PARAMETER;
    }

	MMU_Clean_All_DCache();

//in sundance3e, luminance transform is cancealed
#if 0
    if (AK_FALSE != luminance_enabled)
    {
        cmd = ((T_U32)luminance_table)&0x1fffffff;
        cmd |= (1<<GUI_LUMINANCE_LOAD_START_SHIFT);

        GUI_SET_REG(GUI_LUMINANCE_ADDR, cmd);

        counter = 0;
        do{
            
#ifdef AKOS
            AK_Sleep(1);
#endif

            ret = GUI_GET_REG(GUI_LUMINANCE_ADDR);
            counter++;
        }while (!(ret>>GUI_LUMINANCE_LOAD_DONE_SHIFT) && (counter < MAX_WAIT_COUNTER));

        if (counter >= MAX_WAIT_COUNTER)
        {
            return GUI_ERROR_TIMEOUT;
        } 
        if (ret & (1<<GUI_LUMINANCE_LOAD_DONE_SHIFT)){
            ret &= ~(1<<GUI_LUMINANCE_LOAD_DONE_SHIFT);
            GUI_SET_REG(GUI_LUMINANCE_ADDR, ret);
        }
    }
#endif
    hbypass = 0;
    if (srcRectW == dstRectW)
    {
        hbypass = 1;
    }
    
    vbypass = 0;
    if (srcRectH == dstRectH)
    {
        vbypass = 1;
    }

    cmd = (GUI_SCALRATIO_FACTOR/(dstRectW-1))<<GUI_SCALRATIO_HSHIFT;
    cmd |= (GUI_SCALRATIO_FACTOR/(dstRectH-1))<<GUI_SCALRATIO_VSHIFT;
    GUI_SET_REG(GUI_SCALRATIO_ADDR, cmd);

    if (FORMAT_YUV420 == format_in && nibuff == 3)
    {
        cmd = (T_U32)ibuff[0];
        GUI_SET_REG(GUI_SCALSRC1BASE_ADDR, cmd);
        cmd = (T_U32)ibuff[1];
        GUI_SET_REG(GUI_SCALSRC2BASE_ADDR, cmd);
        cmd = (T_U32)ibuff[2];
        GUI_SET_REG(GUI_SCALSRC3BASE_ADDR, cmd);

        stride = srcRectH/dstRectH;
        switch(stride){
            case 0:
			case 1:
                stride = 0;
                break;
			case 2:
			case 3:
				stride = 1;
				break;
            default:
                stride = 2;
                break;
        }

        GUI_SET_REG(GUI_YUVSTRIDE_ADDR, (1<<16) | (stride));

        if (GUI_GET_REG(GUI_YUVSTRIDE_ADDR) != 0 && vbypass == 1){
			//in this situation, vertical scale should not be passed
            vbypass = 0;
        }
    }
    else if (FORMAT_RGB565 == format_in && nibuff == 1)
    {
        cmd = (T_U32)ibuff[0];
        GUI_SET_REG(GUI_SCALSRC1BASE_ADDR, cmd);
    }
    else
    {
        return GUI_ERROR_PARAMETER;
    }

    cmd = srcWidth;
    GUI_SET_REG(GUI_SCALSRCSTRD_ADDR, cmd);

    cmd = srcRectX << GUI_SCALSRCOFFSET_XSHIFT;
	if (FORMAT_YUV420 == format_in && GUI_GET_REG(GUI_YUVSTRIDE_ADDR) != 0)
	{
		cmd |= ((srcRectY/(stride+1)) & ~0x1) << GUI_SCALSRCOFFSET_YSHIFT;
	}
	else
	{
    	cmd |= srcRectY << GUI_SCALSRCOFFSET_YSHIFT;
	}
    GUI_SET_REG(GUI_SCALSRCOFFSET_ADDR, cmd);

    if (FORMAT_YUV420 == format_in && GUI_GET_REG(GUI_YUVSTRIDE_ADDR) != 0){
        cmd = (srcRectW << GUI_SCALSRCRECT_WSHIFT)|(((srcRectH/(stride+1)) & ~0x1) << GUI_SCALSRCRECT_HSHIFT);
    }else{
        cmd = (srcRectW << GUI_SCALSRCRECT_WSHIFT)|(srcRectH << GUI_SCALSRCRECT_HSHIFT);
    }
    GUI_SET_REG(GUI_SCALSRCRECT_ADDR, cmd);

    if (FORMAT_YUV420 == format_out && nobuff == 3)
    {
        cmd = (T_U32)obuff[0];
        GUI_SET_REG(GUI_SCALDST1BASE_ADDR, cmd);
        cmd = (T_U32)obuff[1];
        GUI_SET_REG(GUI_SCALDST2BASE_ADDR, cmd);
        cmd = (T_U32)obuff[2];
        GUI_SET_REG(GUI_SCALDST3BASE_ADDR, cmd);
    }
    else if (FORMAT_RGB565 == format_out && nobuff == 1)
    {
        cmd = (T_U32)obuff[0];
        GUI_SET_REG(GUI_SCALDST1BASE_ADDR, cmd);
    }
    else
    {
        return GUI_ERROR_PARAMETER;
    }

    cmd = dstWidth;
    GUI_SET_REG(GUI_SCALDSTSTRD_ADDR, cmd);

    cmd = (dstRectX << GUI_SCALDSTOFFSET_XSHIFT) |(dstRectY << GUI_SCALDSTOFFSET_YSHIFT);
    GUI_SET_REG(GUI_SCALDSTOFFSET_ADDR, cmd);

    cmd = (dstRectW << GUI_SCALDSTRECT_WSHIFT)|(dstRectH << GUI_SCALDSTRECT_HSHIFT);
    GUI_SET_REG(GUI_SCALDSTRECT_ADDR, cmd);

    //if (FORMAT_RGB565 == format_out)
    {
        value = 4;
        GUI_SET_REG(GUI_CMD_ADDR, value);
    }

    //set scale filter register
    cmd = 1<<GUI_SCAL_START_BIT;
    cmd |= hbypass << GUI_SCAL_HBYPASS_BIT;
    cmd |= vbypass << GUI_SCAL_VBYPASS_BIT;
    cmd |= (format_out == FORMAT_YUV420) << GUI_SCAL_FILTER_OFORMAT_BIT;
    cmd |= (format_in == FORMAT_YUV420) << GUI_SCAL_FILTER_IFORMAT_BIT;
    cmd |= (luminance_enabled?1:0) << GUI_LUMINANCE_ENABLE_BIT;
    if (FORMAT_RGB565 == format_in)
    {
        cmd |= 1<<GUI_SCAL_DST_NEEDED;
    }
    if (FORMAT_YUV420 == format_in && GUI_GET_REG(GUI_YUVSTRIDE_ADDR) != 0){
        invHeight = 65536/(srcRectH/(stride+1)-1);
    }else{
        invHeight = 65536/(srcRectH - 1);
    }
    cmd |= (invHeight&0xfff)<<GUI_SCAL_INV_HEIGHT;

    GUI_SET_REG(GUI_SCAL_FILTER_CTRL_ADDR, cmd);

    is2DGUIRunning = AK_TRUE;

    return GUI_ERROR_OK;
}

/*
 * @BRIEF                 image scale and format convert function, support format:yuv420, rgb565 -- no block version
 * @BRIEF                 this function include alpha blending and color transparency function.
 * @BRIEF                 if alpha blending and color transparency used, output image format must be FORMAT_RGB565!
 * @PARAM ibuff           src buffer address arrays
 * @PARAM nibuff          num of src buffer address array.
 * @PARAM srcWidth        src image width
 * @PARAM srcRectX        src offset x
 * @PARAM srcRectY        src offset y
 * @PARAM srcRectW        src scale width
 * @PARAM srcRectH        src scale height
 * @PARAM format_in       src image format: FORMAT_YUV420, FORMAT_RGB565
 * @PARAM obuff           dst buffer address arrays
 * @PARAM nobuff          num of dst buffer address array
 * @PARAM dstWidth        dst image width
 * @PARAM dstRectX        dst offset x
 * @PARAM dstRectY        dst offset y
 * @PARAM dstRectW        dst scaled width
 * @PARAM dstRectH        dst scaled height
 * @PARAM format_out      dst image format:FORMAT_YUV420, FORMAT_RGB565
 * @PARAM alpha_enabled   if dst image is FORMAT_RGB565, then alpha enabled is effective.
 * @PARAM alpha           alpha value for alpha transparence (0 ~ 0xf).
 * @PARAM color_trans_enabled   if dst image is FORMAT_RGB565, then color transparency is effective.
 * @PARAM color           color value is 24bits, must value & 0xf8fcf8 because of input format FORMAT_RGB565.
 */
T_U8 Scale_Convert2NoBlock(T_U32 *ibuff, T_U32 nibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                     T_U16 srcRectH, E_ImageFormat format_in, 
                     T_U32* obuff, T_U8 nobuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY, T_U16 dstRectW, 
                     T_U16 dstRectH, E_ImageFormat format_out,
                     T_BOOL alpha_enabled, T_U8 alpha, T_BOOL color_trans_enabled, T_U32 color)
{
    T_U32  cmd, ret, value;
    T_BOOL  hbypass, vbypass;
    T_U16   invHeight;
    T_U32   stride;

    if (AK_TRUE == is2DGUIRunning)
    {
    	//akprintf(C3, M_DRVSYS, "@@@@ graphic: module is running @@@@\n");
        return GUI_ERROR_MODULE_RUN;
    }

    if (srcRectW > MAX_WIDTH ||
        srcRectH > MAX_HEIGHT||
        dstRectW > MAX_WIDTH ||
        dstRectH > MAX_HEIGHT)
    {
    	//akprintf(C3, M_DRVSYS, "@@@@ graphic:over the size @@@@\n");
        return GUI_ERROR_PARAMETER;
    }

    if (srcRectW < 18 ||
        srcRectH < 18 ||
        dstRectW < 18 ||
        dstRectH < 18)
    {
    	//akprintf(C3, M_DRVSYS, "@@@@ graphic:below the size @@@@\n");
        return GUI_ERROR_PARAMETER;
    }

    // sundance3, zoom factor is 4, sundance3e , zoom factor is 10
    if ((srcRectW*10) < dstRectW ||
        (srcRectH*10) < dstRectH)
    {
    	//akprintf(C3, M_DRVSYS, "@@@@ graphic:zoom factor is over 4 @@@@\n");
        return GUI_ERROR_PARAMETER;
    }

    hbypass = 0;
    if (srcRectW == dstRectW)
    {
        hbypass = 1;
    }
    
    vbypass = 0;
    if (srcRectH == dstRectH)
    {
        vbypass = 1;
    }

	MMU_Clean_All_DCache();
    //reset_2D();

    cmd = (GUI_SCALRATIO_FACTOR/(dstRectW-1))<<GUI_SCALRATIO_HSHIFT;
    cmd |= (GUI_SCALRATIO_FACTOR/(dstRectH-1))<<GUI_SCALRATIO_VSHIFT;
    GUI_SET_REG(GUI_SCALRATIO_ADDR, cmd);

    if (FORMAT_YUV420 == format_in && nibuff == 3)
    {
        cmd = (T_U32)ibuff[0];
        GUI_SET_REG(GUI_SCALSRC1BASE_ADDR, cmd);
        cmd = (T_U32)ibuff[1];
        GUI_SET_REG(GUI_SCALSRC2BASE_ADDR, cmd);
        cmd = (T_U32)ibuff[2];
        GUI_SET_REG(GUI_SCALSRC3BASE_ADDR, cmd);

        stride = srcRectH/dstRectH;
        switch(stride){
            case 0:
			case 1:
                stride = 0;
                break;
			case 2:
			case 3:
				stride = 1;
				break;
            default:
                stride = 2;
                break;
        }

       	GUI_SET_REG(GUI_YUVSTRIDE_ADDR, (1<<16) | (stride));

        if (GUI_GET_REG(GUI_YUVSTRIDE_ADDR) != 0 && vbypass == 1){
			//in this situation, vertical scale should not be passed
            vbypass = 0;
        }
    }
    else if (FORMAT_RGB565 == format_in && nibuff == 1)
    {
        cmd = (T_U32)ibuff[0];
        GUI_SET_REG(GUI_SCALSRC1BASE_ADDR, cmd);
    }
    else
    {
        return GUI_ERROR_PARAMETER;
    }

    cmd = srcWidth;
    GUI_SET_REG(GUI_SCALSRCSTRD_ADDR, cmd);

    cmd = srcRectX << GUI_SCALSRCOFFSET_XSHIFT;
	if (FORMAT_YUV420 == format_in && GUI_GET_REG(GUI_YUVSTRIDE_ADDR) != 0)
	{
		cmd |= ((srcRectY/(stride+1)) & ~0x1) << GUI_SCALSRCOFFSET_YSHIFT;
	}
	else
	{
    	cmd |= srcRectY << GUI_SCALSRCOFFSET_YSHIFT;
	}
    GUI_SET_REG(GUI_SCALSRCOFFSET_ADDR, cmd);

    if (FORMAT_YUV420 == format_in && GUI_GET_REG(GUI_YUVSTRIDE_ADDR) != 0){
        cmd = (srcRectW << GUI_SCALSRCRECT_WSHIFT)|(((srcRectH/(stride+1)) & ~0x1) << GUI_SCALSRCRECT_HSHIFT);
    }else{
        cmd = (srcRectW << GUI_SCALSRCRECT_WSHIFT)|(srcRectH << GUI_SCALSRCRECT_HSHIFT);
    }
    GUI_SET_REG(GUI_SCALSRCRECT_ADDR, cmd);

    if (FORMAT_YUV420 == format_out && nobuff == 3)
    {
        cmd = (T_U32)obuff[0];
        GUI_SET_REG(GUI_SCALDST1BASE_ADDR, cmd);
        cmd = (T_U32)obuff[1];
        GUI_SET_REG(GUI_SCALDST2BASE_ADDR, cmd);
        cmd = (T_U32)obuff[2];
        GUI_SET_REG(GUI_SCALDST3BASE_ADDR, cmd);
    }
    else if (FORMAT_RGB565 == format_out && nobuff == 1)
    {
        cmd = (T_U32)obuff[0];
        GUI_SET_REG(GUI_SCALDST1BASE_ADDR, cmd);
    }
    else
    {
        return GUI_ERROR_PARAMETER;
    }

    cmd = dstWidth;
    GUI_SET_REG(GUI_SCALDSTSTRD_ADDR, cmd);

    cmd = (dstRectX << GUI_SCALDSTOFFSET_XSHIFT) |(dstRectY << GUI_SCALDSTOFFSET_YSHIFT);
    GUI_SET_REG(GUI_SCALDSTOFFSET_ADDR, cmd);

    cmd = (dstRectW << GUI_SCALDSTRECT_WSHIFT)|(dstRectH << GUI_SCALDSTRECT_HSHIFT);
    GUI_SET_REG(GUI_SCALDSTRECT_ADDR, cmd);

    value = 0;
    if (color_trans_enabled)
    {
        cmd = color & 0xf8fcf8;
        GUI_SET_REG(GUI_TRANSMINCOL_ADDR, cmd);
        GUI_SET_REG(GUI_TRANSMAXCOL_ADDR, cmd);
        value |= 1 << GUI_COLOR_TRANS_SHIFT;
    }

    if (alpha_enabled)
    {
        value |= 1<<GUI_ALPHA_SHIFT;
        value |= (alpha & 0x0f);
    }
    else
    {
        value = 4; //LOGIC_COPY
    }
    GUI_SET_REG(GUI_CMD_ADDR, value);

    //set scale filter register
    cmd = 1<<GUI_SCAL_START_BIT;
    cmd |= hbypass << GUI_SCAL_HBYPASS_BIT;
    cmd |= vbypass << GUI_SCAL_VBYPASS_BIT;
    cmd |= (format_out == FORMAT_YUV420) << GUI_SCAL_FILTER_OFORMAT_BIT;
    cmd |= (format_in == FORMAT_YUV420) << GUI_SCAL_FILTER_IFORMAT_BIT;

    if (FORMAT_RGB565 == format_in)
    {
        cmd |= 1<<GUI_SCAL_DST_NEEDED;
    }
    if (FORMAT_YUV420 == format_in && GUI_GET_REG(GUI_YUVSTRIDE_ADDR) != 0){
        invHeight = 65536/(srcRectH/(stride+1)-1);
    }else{
        invHeight = 65536/(srcRectH - 1);
    }
    cmd |= (invHeight&0xfff)<<GUI_SCAL_INV_HEIGHT;

    GUI_SET_REG(GUI_SCAL_FILTER_CTRL_ADDR, cmd);

    is2DGUIRunning = AK_TRUE;

    return GUI_ERROR_OK;
}

/*
 * @BRIEF               RGB565 format triangle fill and rotate function,  -- no block version
 * @PARAM ibuff         source image buffer address
 * @PARAM srcWidth      source image width
 * @PARAM srcRectX      source image rectangle start X
 * @PARAM srcRectY      source image rectangle start Y
 * @PARAM srcRectW      source image rectangle width
 * @PARAM srcRectH      source image rectangle height
 * @PARAM obuff         destination image buffer address
 * @PARAM dstWidth      destination image width
 * @PARAM dstRectX      destination image rectangle startX
 * @PARAM dstRectY      destination image rectangle startY
 * @PARAM rotate90      rotate rectangle clockwise 90 degree, value: 1, true; 0,false.
 */
T_U8 Rotate_FillRGBNoBlock(T_U8* ibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                    T_U16 srcRectH, T_U8* obuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY,
                    T_U8  rotate90)
{
    T_U32 cmd, value, counter, ret;
    
    if (AK_TRUE == is2DGUIRunning)
    {
    	//akprintf(C3, M_DRVSYS, "@@@@ graphic: module is running @@@@\n");
        return GUI_ERROR_MODULE_RUN;
    }

    if (srcRectW > MAX_WIDTH ||
        srcRectH > MAX_HEIGHT)
    {
    	//akprintf(C3, M_DRVSYS, "@@@@ graphic:over the size @@@@\n");
        return GUI_ERROR_PARAMETER;
    }

    if (srcRectW < 16 ||
        srcRectH < 16)
    {
    	//akprintf(C3, M_DRVSYS, "@@@@ graphic:below the size @@@@\n");
        return GUI_ERROR_PARAMETER;
    }

	MMU_Clean_All_DCache();
    //reset_2D();

    value = 0;
    cmd = (T_U32)ibuff;
    GUI_SET_REG(GUI_SRCBASE_ADDR, cmd);

    cmd = srcWidth;
    GUI_SET_REG(GUI_SRCSTRD_ADDR, cmd);

    cmd = (srcRectX<<GUI_SRCXY_XSHIFT)|(srcRectY<<GUI_SRCXY_YSHIFT);
    GUI_SET_REG(GUI_SRCXY_ADDR, cmd);

    cmd = (T_U32)obuff;
    GUI_SET_REG(GUI_DSTBASE_ADDR, cmd);

    cmd = dstWidth;
    GUI_SET_REG(GUI_DSTSTRD_ADDR, cmd);

    cmd = (dstRectX<<GUI_DSTXY_XSHIFT)|(dstRectY<<GUI_DSTXY_YSHIFT);
    GUI_SET_REG(GUI_DSTXY_ADDR, cmd);

    if (rotate90)
    {
        cmd = (srcRectH<<GUI_SRCBITBLT_WSHIFT)|(srcRectW<<GUI_SRCBITBLT_HSHIFT);
        GUI_SET_REG(GUI_SRCBITBLT_ADDR, cmd);
        value |= 1<<GUI_DST_ROTATE_SHIFT;
    }
    else
    {
        cmd = (srcRectW<<GUI_SRCBITBLT_WSHIFT)|(srcRectH<<GUI_SRCBITBLT_HSHIFT);
        GUI_SET_REG(GUI_SRCBITBLT_ADDR, cmd);
    }

    cmd = 1<<GUI_SCAL_DST_NEEDED;
    GUI_SET_REG(GUI_SCAL_FILTER_CTRL_ADDR, cmd);

    value |= 1<<GUI_DRAW_TYPE_SHIFT;
    value |= 1<<GUI_CMD_START_SHIFT;
/*  if (clip_enabled)
    {
        cmd = clip_top << GUI_CLIPLT_TSHIFT;
        cmd |= clip_left << GUI_CLIPLT_LSHIFT;
        GUI_SET_REG(GUI_CLIPLT_ADDR, cmd);

        cmd = clip_right << GUI_CLIPRB_RSHIFT;
        cmd |= clip_bottom << GUI_CLIPRB_BSHIFT;
        GUI_SET_REG(GUI_CLIPRB_ADDR, cmd);

        value |= 1<<GUI_CLIP_ENABLE_SHIFT;
    }

    if (trans_ctrl_enabled)
    {
        cmd = transparency & 0xf8fcf8;
        GUI_SET_REG(GUI_TRANSCOLOR_ADDR, cmd);

        value |= 1<<GUI_COLOR_TRANS_SHIFT;
        value |= (trans_polarity)<<GUI_COLOR_POLARITY_SHIFT;
        value |= (trans_compare_source)<<GUI_COLOR_CMP_SOURCE_SHIFT;
    }

    if (fill_color_enabled)
    {
        cmd = penColor&0xf8fcf8;
        GUI_SET_REG(GUI_FCOLOR_ADDR, cmd);

        value |= 1<<GUI_FCOLOR_ENABLE_SHIFT;
    }

    if (alpha_enabled)
    {
        value |= 1<<GUI_ALPHA_SHIFT;
        value |= (alpha & 0x0f);
    }
    else
*/  {
        value |= 4;//LOGIC_COPY.
    }

    GUI_SET_REG(GUI_CMD_ADDR, value);

    is2DGUIRunning = AK_TRUE;

    return GUI_ERROR_OK;
}


T_U8 hwRotateYUVNoBlock(T_U32 srcAddr, T_U32 dstAddr, T_U16 width, T_U16 height, E_RotateAngle rotate_angle,
                 T_U8 uv_interleaved)
{
    T_U32 regVal, cnt;

    if (AK_TRUE == isYUVROTRunning)
    {
        return GUI_ERROR_MODULE_RUN;
    }

    if (width > MAX_WIDTH ||
        height > MAX_HEIGHT)
    {
        return GUI_ERROR_PARAMETER;
    }

    if (width < 18 ||
        height < 18)
    {
        return GUI_ERROR_PARAMETER;
    }

	MMU_Clean_All_DCache();

    GUI_SET_REG(YUVROTATE_SADDR_REG, srcAddr);
    
    GUI_SET_REG(YUVROTATE_DADDR_REG, dstAddr);
    
    regVal = (width << YUVROTATE_WIDTH_BIT)
            + (height << YUVROTATE_HEIGHT_BIT);
    GUI_SET_REG(YUVROTATE_SIZE_REG, regVal);

    regVal = (rotate_angle << YUVROTATE_ANGLE_BIT)
        | ((uv_interleaved?1:0) << YUVROTATE_UVINTERL_BIT)
        | (1 << YUVROTATE_START_BIT);
    GUI_SET_REG(YUVROTATE_CTRL_REG, regVal);

    isYUVROTRunning = AK_TRUE;

    return GUI_ERROR_OK;
}

/*
 * @BRIEF            yuv420 rotate function, angle:0, 90, 180, 270.counter clockwise. -- no block version
 * @PARAM ibuff      source image buffer address arrays.
 * @PARAM nibuff     number of source image buffer
 * @PARAM srcWidth   source image width
 * @PARAM srcHeight  source image height
 * @PARAM obuff      destination image buffer address arrays.
 * @PARAM rotate     rotate anlge, value: ROTATE_0, ROTATE_90, ROTATE_180, ROTATE_270
 * @PARAM uv_interleaved  whether uv is interleaved, if true, nibuff and nobuff is 2, else nibuff and nobuff is 3.
 */
T_U8 Rotate_FillYUVNoBlock(T_U32* ibuff, T_U8 nibuff, T_U16 srcWidth, T_U16 srcHeight, T_U32* obuff, T_U8 nobuff,
                    E_RotateAngle rotate, T_U8 uv_interleaved)
{
    T_U32  ret;
    T_U32  srcY, srcU, srcV = 0;
    T_U32  dstY, dstU, dstV = 0;

    if (AK_TRUE == isYUVROTRunning)
    {
        return GUI_ERROR_MODULE_RUN;
    }

    if ((uv_interleaved != 0) && (nibuff != 2 || nobuff != 2))
    {
        return GUI_ERROR_PARAMETER;
    }

    if ((uv_interleaved == 0) && (nibuff != 3 || nobuff != 3))
    {
        return GUI_ERROR_PARAMETER;
    }

    srcY = ibuff[0];
    dstY = obuff[0];
    srcU = ibuff[1];
    dstU = obuff[1];

    if (!uv_interleaved)
    {
        srcV = ibuff[2];
        dstV = obuff[2];
    }

    //reset_YUV();

    hwRotateYUVNoBlock(srcY, dstY, srcWidth, srcHeight, rotate, 0);
    ret = YUVROT_Wait_For_Done();
    if (ret != GUI_ERROR_OK)
    {
        return ret;
    }
    if (uv_interleaved)
    {
        hwRotateYUVNoBlock(srcU, dstU, srcWidth, srcHeight/2, rotate, uv_interleaved);
        ret = YUVROT_Wait_For_Done();
        if (ret != GUI_ERROR_OK)
        {
            return ret;
        }
    }
    else
    {
        hwRotateYUVNoBlock(srcU, dstU, srcWidth/2, srcHeight/2, rotate, 0);
        ret = YUVROT_Wait_For_Done();
        if (ret != GUI_ERROR_OK)
        {
            return ret;
        }

        ret = hwRotateYUVNoBlock(srcV, dstV, srcWidth/2, srcHeight/2, rotate, 0);
        if(ret != GUI_ERROR_OK)
        {
            return ret;
        }
    }

    return GUI_ERROR_OK;
}

/*
 * @BRIEF            luminance transform function only for yuv420.
 * @PARAM ibuff      source image Y address
 * @PARAM srcWidth   source image width
 * @PARAM srcHeight  source image height
 * @PARAM format_in  source image format:FORMAT_YUV420
 * @PARAM obuff      destination image Y address
 */
T_U8 Luminance_TransformNoBlock(T_U8*ibuff, T_U16 srcWidth, T_U16 srcHeight, E_ImageFormat format_in, T_U8* obuff,
        T_U8* luminance_table)
{
    // set (load) luminance table
    T_U32 ret, cmd, counter;
    T_U32 regVal;

    if (AK_TRUE == is2DGUIRunning)
    {
        return GUI_ERROR_MODULE_RUN;
    }

    if (srcWidth > MAX_WIDTH ||
        srcHeight > MAX_HEIGHT)
    {
        return GUI_ERROR_PARAMETER;
    }

    if (srcWidth < 18 ||
        srcHeight < 18)
    {
        return GUI_ERROR_PARAMETER;
    }

    if (FORMAT_YUV420 != format_in)
    {
        return GUI_ERROR_PARAMETER;
    }

	MMU_Clean_All_DCache();

    cmd = ((T_U32)luminance_table)&0x1fffffff;
    cmd |= (1<<GUI_LUMINANCE_LOAD_START_SHIFT);

    GUI_SET_REG(GUI_LUMINANCE_ADDR, cmd);

    counter = 0;
    do{
        
#ifdef AKOS
        AK_Sleep(1);
#endif

        ret = GUI_GET_REG(GUI_LUMINANCE_ADDR);
        counter++;
    }while (!(ret>>GUI_LUMINANCE_LOAD_DONE_SHIFT) && (counter < MAX_WAIT_COUNTER));

    if (counter >= MAX_WAIT_COUNTER)
    {
        return GUI_ERROR_TIMEOUT;
    }

    ret &= ~(1<<GUI_LUMINANCE_LOAD_DONE_SHIFT);
    GUI_SET_REG(GUI_LUMINANCE_ADDR, ret);
        
    // src addr
    cmd = (T_U32)ibuff;
    GUI_SET_REG(GUI_SCALSRC1BASE_ADDR, cmd);

    //src rect h,w
    cmd = (srcWidth<<GUI_SCALSRCRECT_WSHIFT)|(srcHeight<<GUI_SCALSRCRECT_HSHIFT);
    GUI_SET_REG(GUI_SCALSRCRECT_ADDR, cmd);

    //dst address
    cmd = (T_U32)obuff;
    GUI_SET_REG(GUI_SCALDST1BASE_ADDR, cmd);

    // set ctrl reg
        regVal = (1 << GUI_SCAL_START_BIT) 
                | (1 << GUI_SCAL_FILTER_OFORMAT_BIT)
                | (1 << GUI_SCAL_FILTER_IFORMAT_BIT)
                | (1 << GUI_LUMINANCE_ENABLE_BIT)
                | (1 << GUI_LUMINANCE_ONLY_BIT);
    
    GUI_SET_REG(GUI_SCAL_FILTER_CTRL_ADDR, regVal);

    is2DGUIRunning = AK_TRUE;

    return GUI_ERROR_OK;
}

/*
 * @BRIEF           Filter source image function, support for YUV420 and RGB565.    -- no block version
 * @PARAM ibuff     source image address, if format_in is FORMAT_YUV420, ibuff is source image Y address
 * @PARAM srcWidth  source image width.
 * @PARAM srcHeight source image height.
 * @PARAM format_in source image format:FORMAT_YUV420 or FORMAT_RGB565.
 * @PARAM obuff     destination image address, if format_in is FORMAT_YUV420, obuff is destinantion image Y address.
 */
T_U8 Filter3x3NoBlock(T_U8* ibuff, T_U16 srcWidth, T_U16 srcHeight, E_ImageFormat format_in, T_U8* obuff)
{
    //  coding here 
    T_U32 cmd;
    T_U32 regVal = 0;
    T_U8 colnum = 0;
    T_U32 counter, ret;

    if (AK_TRUE == is2DGUIRunning)
    {
        return GUI_ERROR_MODULE_RUN;
    }

    if (srcWidth > MAX_WIDTH ||
        srcHeight > MAX_HEIGHT)
    {
        return GUI_ERROR_PARAMETER;
    }

    if (srcWidth < 18 ||
        srcHeight < 18)
    {
        return GUI_ERROR_PARAMETER;
    }

    if (FORMAT_RGB565 != format_in && FORMAT_YUV420 != format_in)
    {
        return GUI_ERROR_PARAMETER;
    }

	MMU_Clean_All_DCache();
    //reset_2D();

    cmd = (T_U32) ibuff;
    GUI_SET_REG(GUI_SCALSRC1BASE_ADDR, cmd);

    cmd = (T_U32)obuff;
    GUI_SET_REG(GUI_SCALDST1BASE_ADDR, cmd);

    cmd = (srcWidth<<GUI_SCALSRCRECT_WSHIFT)|(srcHeight<<GUI_SCALSRCRECT_HSHIFT);
    GUI_SET_REG(GUI_SCALSRCRECT_ADDR, cmd);

    colnum = (T_U8)(srcWidth / 62);
    if (srcWidth % 62 >1)
    {
        colnum++;
    }

     regVal = (1 << GUI_FILTER_START_BIT)
        | ((T_U8)(format_in)<<GUI_SCAL_FILTER_IFORMAT_BIT)
        | ((T_U8)(format_in) << GUI_SCAL_FILTER_OFORMAT_BIT)
        | (colnum << GUI_FILTER_COLUMN);
    
    GUI_SET_REG(GUI_SCAL_FILTER_CTRL_ADDR, regVal);

    is2DGUIRunning = AK_TRUE;

    return GUI_ERROR_OK;
}


/*
 * @BRIEF                 image scale and format convert function, support format:yuv420, rgb565 
 * @PARAM ibuff           src buffer address arrays
 * @PARAM nibuff          num of src buffer address array.
 * @PARAM srcWidth        src image width
 * @PARAM srcRectX        src offset x
 * @PARAM srcRectY        src offset y
 * @PARAM srcRectW        src scale width
 * @PARAM srcRectH        src scale height
 * @PARAM format_in       src image format: FORMAT_YUV420, FORMAT_RGB565
 * @PARAM obuff           dst buffer address arrays
 * @PARAM nobuff          num of dst buffer address array
 * @PARAM dstWidth        dst image width
 * @PARAM dstRectX        dst offset x
 * @PARAM dstRectY        dst offset y
 * @PARAM dstRectW        dst scaled width
 * @PARAM dstRectH        dst scaled height
 * @PARAM format_out      dst image format:FORMAT_YUV420, FORMAT_RGB565
 * @PARAM luminance_enabled   if input image format is FORMAT_YUV420, this param can set AK_TRUE to transform luminance.
 * @PARAM luminance_table     a continuous memory buffer width length of 256 byte, used to transform yuv420 luminance.
*/
T_U8 Scale_Convert(T_U32 *ibuff, T_U32 nibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                     T_U16 srcRectH, E_ImageFormat format_in, 
                     T_U32* obuff, T_U8 nobuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY, T_U16 dstRectW, 
                     T_U16 dstRectH, E_ImageFormat format_out,
                     T_U8 luminance_enabled, T_U8* luminance_table)
{
    T_U8  ret;

    if (AK_TRUE == is2DGUIRunning)
    {
        GUI_Wait_For_Done();
    }

    reset_2D();

    ret = Scale_ConvertNoBlock(ibuff, nibuff, srcWidth, srcRectX, srcRectY, srcRectW, srcRectH, format_in, 
            obuff, nobuff, dstWidth, dstRectX, dstRectY, dstRectW, dstRectH, format_out,
            luminance_enabled, luminance_table);

    if (GUI_ERROR_OK == ret)
    {
        ret = GUI_Wait_For_Done();
    }

    is2DGUIRunning = AK_FALSE;
    return ret;
}

/*
 * @BRIEF                 image scale and format convert function, support format:yuv420, rgb565 -- block version 
 * @BRIEF                 this function include alpha blending and color transparency function.
 * @BRIEF                 if alpha blending and color transparency used, output image format must be FORMAT_RGB565!
 * @PARAM ibuff           src buffer address arrays
 * @PARAM nibuff          num of src buffer address array.
 * @PARAM srcWidth        src image width
 * @PARAM srcRectX        src offset x
 * @PARAM srcRectY        src offset y
 * @PARAM srcRectW        src scale width
 * @PARAM srcRectH        src scale height
 * @PARAM format_in       src image format: FORMAT_YUV420, FORMAT_RGB565
 * @PARAM obuff           dst buffer address arrays
 * @PARAM nobuff          num of dst buffer address array
 * @PARAM dstWidth        dst image width
 * @PARAM dstRectX        dst offset x
 * @PARAM dstRectY        dst offset y
 * @PARAM dstRectW        dst scaled width
 * @PARAM dstRectH        dst scaled height
 * @PARAM format_out      dst image format:FORMAT_YUV420, FORMAT_RGB565
 * @PARAM alpha_enabled   if dst image is FORMAT_RGB565, then alpha enabled is effective.
 * @PARAM alpha           alpha value for alpha transparence (0 ~ 0xf).
 * @PARAM color_trans_enabled   if dst image is FORMAT_RGB565, then color transparency is effective.
 * @PARAM color           color value is 24bits, must value & 0xf8fcf8 because of input format FORMAT_RGB565.
 */
T_U8 Scale_Convert2(T_U32 *ibuff, T_U32 nibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                     T_U16 srcRectH, E_ImageFormat format_in, 
                     T_U32* obuff, T_U8 nobuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY, T_U16 dstRectW, 
                     T_U16 dstRectH, E_ImageFormat format_out,
                     T_BOOL alpha_enabled, T_U8 alpha, T_BOOL color_trans_enabled, T_U32 color)
{
    T_U8 ret;

    if (AK_TRUE == is2DGUIRunning)
    {
        GUI_Wait_For_Done();
    }

    reset_2D();

    ret = Scale_Convert2NoBlock(ibuff, nibuff, srcWidth, srcRectX, srcRectY, srcRectW,
                     srcRectH, format_in, obuff, nobuff, dstWidth, dstRectX, dstRectY, dstRectW, 
                     dstRectH, format_out, alpha_enabled, alpha, color_trans_enabled, color);

    if (GUI_ERROR_OK == ret)
    {
        ret = GUI_Wait_For_Done();
    }

    is2DGUIRunning = AK_FALSE;
    return ret;
}

/*
 * @BRIEF               RGB565 format triangle fill and rotate function.       
 * @PARAM ibuff         source image buffer address
 * @PARAM srcWidth      source image width
 * @PARAM srcRectX      source image rectangle start X
 * @PARAM srcRectY      source image rectangle start Y
 * @PARAM srcRectW      source image rectangle width
 * @PARAM srcRectH      source image rectangle height
 * @PARAM obuff         destination image buffer address
 * @PARAM dstWidth      destination image width
 * @PARAM dstRectX      destination image rectangle startX
 * @PARAM dstRectY      destination image rectangle startY
 * @PARAM rotate90      rotate rectangle clockwise 90 degree, value: 1, true; 0,false.
 */
T_U8 Rotate_FillRGB(T_U8* ibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                    T_U16 srcRectH, T_U8* obuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY,
                    T_U8  rotate90)
{
    T_U8 ret;

    if (AK_TRUE == is2DGUIRunning)
    {
        GUI_Wait_For_Done();
    }

    ret = Rotate_FillRGBNoBlock(ibuff, srcWidth, srcRectX, srcRectY, srcRectW, srcRectH, 
                    obuff, dstWidth, dstRectX, dstRectY, rotate90);

    if (GUI_ERROR_OK == ret)
    {
        ret = GUI_Wait_For_Done();
    }

    is2DGUIRunning = AK_FALSE;

    return ret;
}

/*
 * @BRIEF            yuv420 rotate function, angle:0, 90, 180, 270.counter clockwise.
 * @PARAM ibuff      source image buffer address arrays.
 * @PARAM nibuff     number of source image buffer
 * @PARAM srcWidth   source image width
 * @PARAM srcHeight  source image height
 * @PARAM obuff      destination image buffer address arrays.
 * @PARAM rotate     rotate anlge, value: ROTATE_0, ROTATE_90, ROTATE_180, ROTATE_270
 * @PARAM uv_interleaved  whether uv is interleaved, if true, nibuff and nobuff is 2, else nibuff and nobuff is 3.
 */
T_U8 Rotate_FillYUV(T_U32* ibuff, T_U8 nibuff, T_U16 srcWidth, T_U16 srcHeight, T_U32* obuff, T_U8 nobuff,
                    E_RotateAngle rotate, T_U8 uv_interleaved)
{
    T_U8 ret;

    if (AK_TRUE == isYUVROTRunning)
    {
        YUVROT_Wait_For_Done();
    }

    ret = Rotate_FillYUVNoBlock(ibuff, nibuff, srcWidth, srcHeight, obuff, nobuff, rotate, uv_interleaved);

    if (GUI_ERROR_OK == ret)
    {
        ret = YUVROT_Wait_For_Done();
    }

    return ret;
}

/*
 * @BRIEF            luminance transform function only for yuv420.
 * @PARAM ibuff      source image Y address
 * @PARAM srcWidth   source image width
 * @PARAM srcHeight  source image height
 * @PARAM format_in  source image format:FORMAT_YUV420
 * @PARAM obuff      destination image Y address
 */
T_U8 Luminance_Transform(T_U8*ibuff, T_U16 srcWidth, T_U16 srcHeight, E_ImageFormat format_in, T_U8* obuff,
        T_U8* luminance_table)
{
    T_U8 ret;

    if (AK_TRUE == is2DGUIRunning)
    {
        GUI_Wait_For_Done();
    }

    ret = Luminance_TransformNoBlock(ibuff, srcWidth, srcHeight, format_in, obuff, luminance_table);

    if (GUI_ERROR_OK == ret)
    {
        ret = GUI_Wait_For_Done();
    }

    is2DGUIRunning = AK_FALSE;

    return ret;
}

/*
 * @BRIEF           Filter source image function, support for YUV420 and RGB565.
 * @PARAM ibuff     source image address, if format_in is FORMAT_YUV420, ibuff is source image Y address
 * @PARAM srcWidth  source image width.
 * @PARAM srcHeight source image height.
 * @PARAM format_in source image format:FORMAT_YUV420 or FORMAT_RGB565.
 * @PARAM obuff     destination image address, if format_in is FORMAT_YUV420, obuff is destinantion image Y address.
 */
T_U8 Filter3x3(T_U8* ibuff, T_U16 srcWidth, T_U16 srcHeight, E_ImageFormat format_in, T_U8* obuff)
{
    T_U8 ret;

    if (AK_TRUE == is2DGUIRunning)
    {
        GUI_Wait_For_Done();
    }

    ret = Filter3x3NoBlock(ibuff, srcWidth, srcHeight, format_in, obuff);

    if (GUI_ERROR_OK == ret)
    {
        ret = GUI_Wait_For_Done();
    }

    is2DGUIRunning = AK_FALSE;

    return ret;
}
/*
 * @BRIEF this function check if gui module is busy. if return AK_TRUE, the gui module is free,
 *        return AK_FALSE, gui module is busy now.
 */
T_BOOL  is2DGUIFinish(T_VOID)
{
    T_U32 ret;

    if (AK_FALSE == is2DGUIRunning)
    {
        return AK_TRUE;
    }
    ret = GUI_GET_REG(GUI_INTSTATUS_ADDR);
    if (ret & 0x1)
    {
        ret &= ~(0x1);
        GUI_SET_REG(GUI_INTSTATUS_ADDR, ret);
        is2DGUIRunning = AK_FALSE;
        return AK_TRUE;
    }

    return AK_FALSE;
}

/*
 * @BRIEF this function check if yuv rotate module is busy. if return AK_TRUE, the yuv rotate module is free,
 *        else return AK_FALSE, yuv rotate module is busy now.
 */
T_BOOL isYUVROTFinish(T_VOID)
{
    T_U32 ret;

    if (AK_FALSE == isYUVROTRunning)
    {
        return AK_TRUE;
    }

    ret = GUI_GET_REG(YUVROTATE_CTRL_REG);
    if (ret & (1<<YUVROTATE_INTSTAT_BIT))
    {
        ret &= ~(1<<YUVROTATE_INTSTAT_BIT);
        GUI_SET_REG(YUVROTATE_CTRL_REG, ret);
        isYUVROTRunning = AK_FALSE;
        return AK_TRUE;
    }

    return AK_FALSE;
}


