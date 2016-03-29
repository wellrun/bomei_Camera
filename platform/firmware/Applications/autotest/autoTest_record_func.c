/**
 * @FILENAME: autotest_record.c
 * @BRIEF atuotest record script file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR lixingjian
 * @DATE 2012-02-28
 * @VERSION 1.0
 * @REF
 */ 
 


#include "autotest_record_func.h"
#include "Fwl_public.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "fwl_keyhandler.h"
#include "AutoTest_test_func.h"


#ifdef SUPPORT_AUTOTEST



#define     DATA_LEN                      10
#define     B_PATH_LEN                    20
#define     FILENAME_MAX_LEN              100
#define     RECOREDNUM_FILE               DRI_D"autotest\\recordnum.txt"
#define     B_PATH                 		  DRI_B
#define     AUTOTEST_DIR_APTH_SCRIPFILE   DRI_D"autotest\\Scriptfile"
#define     AUTOTEST_DIR_APTH_DIRECTFILE  DRI_D"autotest\\Directfile"
#define     RECOREDNUM_FILE               DRI_D"autotest\\recordnum.txt"



typedef struct tag_Scriptinfo
{
	//T_U8  Scriptname[FILENAME_MAX_LEN];   //脚本文件名
	T_U32 times;            //循环执行的次数
	T_U32 filenum;     //文件的个数
    T_pFILE file;      //文件句柄
} T_SCRIPT_INFO,  *T_pSCRIPT_INFO;


typedef struct tag_Keyinfo
{
	T_U8   keyID;           //按键名
	T_U8   presstype;       //按键类型
	T_U32  times;           //按键执行的次数
    T_U32  delaytime;       //按键delay的时间
    T_U32  lastdelaytime;   //上一次按键delay的时间
	T_U32  writelen;        //已写文件的长度
	T_U32  lastbuflen;      //前一次写的buf长度
	T_BOOL enterflag;       //按键每一次进来
} T_KEY_INFO, *T_pKEY_INFO;

typedef struct tag_Tscrinfo
{
	T_U32  times;           //按键执行的次数
    T_TOUCHSCR_ACTION act;
	T_POINT point;
	T_BOOL tscrenterflag;       //触模第一次进来
	T_BOOL tscrstartflag;       //写开始标志前不能写触摸屏的信息
} T_TSCR_INFO, *T_pTSCR_INFO;



T_SCRIPT_INFO scriptname;
T_KEY_INFO    key;
T_TSCR_INFO   tscr;

T_BOOL autotest_recordflag = AK_FALSE;
T_BOOL autotest_testflag = AK_FALSE;
T_BOOL autotest_screen_saver_falg = AK_FALSE;
T_BOOL endlongpress_no_writeflag  = AK_TRUE;

T_VOID autotest_record_closefile(T_BOOL safeflag);

extern T_U32 get_tick_count(T_VOID);

T_VOID creat_folderandfile(void)
{
	T_pFILE file = FS_INVALID_HANDLE;

	
	if (!Fwl_FileExistAsc(AUTOTEST_DIR_APTH))
	{	
		if (!Fwl_FsMkDirAsc(AUTOTEST_DIR_APTH))
		{
			AK_DEBUG_OUTPUT("Fwl_FsMkDirAsc :%s fail", AUTOTEST_DIR_APTH);
		}
		else
		{
			file = Fwl_FileOpenAsc(RECOREDNUM_FILE, _FMODE_CREATE, _FMODE_CREATE);
			if(FS_INVALID_HANDLE == file)
			{
				AK_DEBUG_OUTPUT(" %s creat fail", RECOREDNUM_FILE);
			}
			Fwl_FileClose(file);
			file = FS_INVALID_HANDLE;
		}
		AK_DEBUG_OUTPUT("Fwl_FsMkDirAsc :%s sucess", AUTOTEST_DIR_APTH);
	}
	else
	{ 
		AK_DEBUG_OUTPUT("%s is exist", AUTOTEST_DIR_APTH);
		if(!Fwl_FileExistAsc(RECOREDNUM_FILE))
		{
			file = Fwl_FileOpenAsc(RECOREDNUM_FILE, _FMODE_CREATE, _FMODE_CREATE);
			if(FS_INVALID_HANDLE == file)
			{
				AK_DEBUG_OUTPUT(" %s creat fail", RECOREDNUM_FILE);
			}
			Fwl_FileClose(file);
			file = FS_INVALID_HANDLE;		
		}
	}
	/*按要求加上需要分类的文件夹*/
	if (!Fwl_FileExistAsc(AUTOTEST_DIR_APTH_DIRECTFILE))
	{
		if (!Fwl_FsMkDirAsc(AUTOTEST_DIR_APTH_DIRECTFILE))
		{
			AK_DEBUG_OUTPUT("Fwl_FsMkDirAsc :%s fail", AUTOTEST_DIR_APTH_DIRECTFILE);
		}
	}
	
	if (!Fwl_FileExistAsc(AUTOTEST_DIR_APTH_SCRIPFILE))
	{
		if (!Fwl_FsMkDirAsc(AUTOTEST_DIR_APTH_SCRIPFILE))
		{
			AK_DEBUG_OUTPUT("Fwl_FsMkDirAsc :%s fail", AUTOTEST_DIR_APTH_SCRIPFILE);
		}
	}
	/***************************************/
}


T_S32 Record_Utl_Atoi(T_pCSTR strMain)
{
    T_pCDATA    pMain = AK_NULL;
    T_S32       sum;
    T_BOOL      negv = AK_FALSE;
    T_S16       i = 0;

    pMain = (T_pCDATA)strMain;
    sum = 0;
    if ((*pMain) == '-')
    {
        negv = AK_TRUE;
        pMain++;
    }

    while (*pMain)
    {
        if ('0' <= (*pMain) && (*pMain) <= '9')
            sum = sum * 10 + (*pMain - '0');
        else
            sum = sum * 10;
        pMain++;
        i++;
    }

    if (negv)
        sum *= (-1);

    return sum;
}


T_U32 Fs_EXT_fgets(T_U8 *str, T_S32 nb, T_pFILE fp)
{
	T_U8 *p;
	T_U8 buf[10] = {0};
	T_U32 count = 0;

	p = str;
	while(Fwl_FileRead(fp,buf,1))
	{
		if('\n' == buf[0])
		{
			break;
		}
		if((buf[0] >= '0' && buf[0] <= '9' ) ||
		    (buf[0] >= 'a' && buf[0] <= 'z') ||
		    (buf[0] >= 'A' && buf[0] <= 'Z'))
		{
			*p = buf[0];
			p++;
			count++;
		}
		else
		{
			continue;
		}
	}
	return count;
}



T_U32 Fs_initConFigInfo(T_VOID)
{
	T_pFILE fTestCase = FS_INVALID_HANDLE;
	T_U8 *strLine = AK_NULL;
	T_U32 num = 0;

	AK_DEBUG_OUTPUT("Enter initConFigInfo() \n");
    
	strLine = (T_U8*)Fwl_Malloc(DATA_LEN);
	if(AK_NULL == strLine)
	{
		AK_DEBUG_OUTPUT("initConFigInfo(): Test_Malloc Fail \n");
		return 0;
	}

    
	fTestCase = Fwl_FileOpenAsc(RECOREDNUM_FILE, _FMODE_READ, _FMODE_READ);
	if (fTestCase == FS_INVALID_HANDLE)
	{
		AK_DEBUG_OUTPUT("open the test case error\n");
		Fwl_Free( strLine);
		return 0;
	}
    memset(strLine, 0, 10);
	Fs_EXT_fgets(strLine, DATA_LEN, fTestCase);
	AK_DEBUG_OUTPUT("strLine: %s\n", strLine);
	num = (T_U32)Record_Utl_Atoi(strLine);
	
	Fwl_FileClose(fTestCase);
	fTestCase = FS_INVALID_HANDLE;
	Fwl_Free( strLine);
	strLine = AK_NULL;
	return num;
 
}


T_BOOL Fs_endConfig(T_U32 num)
{
	T_pFILE fTestCase = FS_INVALID_HANDLE;
	T_U8 *strLine=AK_NULL;
	T_U32 count=0;

	AK_DEBUG_OUTPUT("Enter endConfig() \n");
	strLine = (T_U8*)Fwl_Malloc(DATA_LEN);
	if(AK_NULL == strLine)
	{
		AK_DEBUG_OUTPUT("endConfig(): Test_Malloc Fail \n");
		return AK_FALSE;
	}
	fTestCase = Fwl_FileOpenAsc(RECOREDNUM_FILE, _FMODE_CREATE, _FMODE_CREATE);
	if (fTestCase == FS_INVALID_HANDLE)
	{
		Fwl_Free(strLine);
		strLine = AK_NULL;
		AK_DEBUG_OUTPUT("\r\n open the test case error\n");
		return AK_FALSE;
	}
	AK_DEBUG_OUTPUT("Enter endConfig()1 \n");
	
    memset(strLine, 0, DATA_LEN);
	count = sprintf(strLine, "%ld\n", num);
	AK_DEBUG_OUTPUT("sprintf:  %s, count = %d \n", strLine, count);
	Fwl_FileWrite(fTestCase, strLine,count);

	
	Fwl_FileClose(fTestCase);
	fTestCase = FS_INVALID_HANDLE;
	Fwl_Free(strLine);
	strLine = AK_NULL;
	AK_DEBUG_OUTPUT("End of endConfig() \n");

	return AK_TRUE;
    
    
}





T_VOID autotest_record_scriptinfo_init(T_VOID)
{
	scriptname.file = FS_INVALID_HANDLE;
}




T_VOID autotest_record_keyinfo_init(T_VOID)
{
	
	key.keyID = 0xFF;
	key.presstype= 0;
	key.delaytime = 0;
	key.enterflag = AK_TRUE;
	key.lastbuflen = 0;
	key.times = 0;
	key.writelen = 0;

	tscr.tscrenterflag = AK_TRUE;
	tscr.tscrstartflag = AK_FALSE;
	tscr.times = 0;
	tscr.act = 0;
	tscr.point.x = 0;
	tscr.point.y = 0;

	//是否写入最后一个长按的按键信息的标志
	endlongpress_no_writeflag  = AK_TRUE;
	
}



T_BOOL autotest_record_openfile(T_VOID)
{
 	//T_pFILE file = FS_INVALID_HANDLE;
	T_U8 *FileName = AK_NULL;
	T_U8 *buf = AK_NULL;
	T_U32 buf_len = 0;

	//Fwl_RamLeakMonitorPointBeg();
	FileName = (T_U8 *)Fwl_Malloc(FILENAME_MAX_LEN);
	if(AK_NULL == FileName)
	{
		AK_DEBUG_OUTPUT("autotest_record_openfile: Fwl_Malloc fail\n");
		return AK_FALSE;
	}


	//读文件,如果文件的长度为0就是脚本名需要从0开始,否则接着脚本的数字
	if (0 == Fwl_FileGetSizeAsc(RECOREDNUM_FILE))
	{
		scriptname.filenum = 0;
	}
	else
	{
		scriptname.filenum = Fs_initConFigInfo();
	}

	 
	 sprintf(FileName, "%s\\script%ld.txt", AUTOTEST_DIR_APTH_SCRIPFILE, scriptname.filenum);
	 AK_DEBUG_OUTPUT("record the %d scriptfile: %s \r\n", scriptname.filenum, FileName);

	scriptname.file = Fwl_FileOpenAsc(FileName, _FMODE_CREATE, _FMODE_CREATE);
	if(FS_INVALID_HANDLE == scriptname.file)
	{
		AK_DEBUG_OUTPUT("autotest_record_openfile: open script file %s fail\n", FileName);
		Fwl_Free(FileName);
		return AK_FALSE;
	}

	//记录当前录制脚本的个数
	if(!Fs_endConfig(scriptname.filenum))
	{
		AK_DEBUG_OUTPUT("autotest_record_closefile: Fs_endConfig fail\n");
		Fwl_Free(FileName);
		autotest_record_closefile(AK_FALSE);
		return AK_FALSE;
	}

	//初始化key的结构体成员
	autotest_record_keyinfo_init();

	buf = Fwl_Malloc(20);
	if(AK_NULL == buf)
	{
		AK_DEBUG_OUTPUT("autotest_record_writefile: Fwl_Malloc fail\n");
		Fwl_Free(FileName);
		autotest_record_closefile(AK_FALSE);
		return AK_FALSE;
	}

	sprintf(buf, "<startkey>\n");
	buf_len = strlen(buf);
	key.writelen += buf_len;
	key.lastbuflen = buf_len;
	AK_DEBUG_OUTPUT("key.writelen: %d \r\n", key.writelen);
	if (buf_len != Fwl_FileWrite(scriptname.file, buf, buf_len))
	{
		AK_DEBUG_OUTPUT("autotest_record_openfile: <startkey> fail\n");
		Fwl_Free(buf);
		Fwl_Free(FileName);
		autotest_record_closefile(AK_FALSE);
		return AK_FALSE;
	}
	tscr.tscrstartflag = AK_TRUE;
	
	//scriptname.file = file;
	//Utl_StrCpy(scriptname.Scriptname, FileName);
	
	Fwl_Free(buf);
	Fwl_Free(FileName);
	
	return AK_TRUE;
}


T_BOOL autotest_record_TSCR_writefile(T_TOUCHSCR_ACTION act ,T_S16 pointx, T_S16 pointy, T_U32 delaytime)
{
	T_U32 buf_len = 0;
	T_U8 *buf = AK_NULL;
	T_S32 offset = 0;
	//T_U32 len = 0;
	
	if((tscr.act == act) && (key.lastdelaytime  == delaytime) && (tscr.point.x == pointx) && (tscr.point.y == pointy))
	{
		tscr.times++;
		offset = key.writelen - key.lastbuflen;
		Fwl_FileSeek(scriptname.file, offset, _FSEEK_SET);
	}
	else
	{
		tscr.times = 1;
	}
	
	buf = Fwl_Malloc(25);
	if(AK_NULL == buf)
	{
		AK_DEBUG_OUTPUT("autotest_record_writefile: Fwl_Malloc fail\n");
		return AK_FALSE;
	}
	

	
	sprintf(buf, "%d,%d,%ld,%ld,%d\n", tscr.point.x, tscr.point.y, tscr.times, delaytime, tscr.act);
	AK_DEBUG_OUTPUT("autotest_record_TSCR_writefile: tscr: %d,%d,%d,%d,%d\n", tscr.point.x, tscr.point.y, tscr.times, delaytime, tscr.act);
	buf_len = strlen(buf);
	key.writelen += buf_len;
	key.lastbuflen = buf_len;
	AK_DEBUG_OUTPUT("key.writelen: %d \r\n", key.writelen);
	if (buf_len != Fwl_FileWrite(scriptname.file, buf, buf_len))
	{
		AK_DEBUG_OUTPUT("autotest_record_writefile: keyname: %d,%d,%d,%d,%d,fail\n", tscr.point.x, tscr.point.y, tscr.times, delaytime, tscr.act);
		Fwl_Free(buf);
		return AK_FALSE;
	}
	
	Fwl_Free(buf);
 	return AK_TRUE;
}




T_BOOL autotest_record_writefile(T_U8 keyID, T_U8 presstype, T_U32 delaytime)
{
	T_U32 buf_len = 0;
	T_U8 *buf = AK_NULL;
	T_S32 offset = 0;
	//T_U32 len = 0;
	
	if((key.keyID == keyID) && (key.lastdelaytime  == delaytime) && (key.presstype == presstype))
	{
		key.times++;
		offset = key.writelen - key.lastbuflen;
		Fwl_FileSeek(scriptname.file, offset, _FSEEK_SET);
	}
	else
	{
		key.times = 1;
	}
	
	//key.times = 1;
	//1+1+4+1+4+1+1+1 = 14
	
	buf = Fwl_Malloc(14);
	if(AK_NULL == buf)
	{
		AK_DEBUG_OUTPUT("autotest_record_writefile: Fwl_Malloc fail\n");
		return AK_FALSE;
	}
	

	
	sprintf(buf, "%d,%ld,%ld,%d\n", key.keyID, key.times, delaytime, key.presstype);
	AK_DEBUG_OUTPUT("autotest_record_writefile: keyname: %d,%d,%d,%d\n", key.keyID, key.times, delaytime, key.presstype);
	buf_len = strlen(buf);
	key.writelen += buf_len;
	key.lastbuflen = buf_len;
	AK_DEBUG_OUTPUT("key.writelen: %d \r\n", key.writelen);
	if (buf_len != Fwl_FileWrite(scriptname.file, buf, buf_len))
	{
		AK_DEBUG_OUTPUT("autotest_record_writefile: keyname: %d,%d,%d,%d,fail\n", key.keyID, key.times, delaytime, key.presstype);
		Fwl_Free(buf);
		return AK_FALSE;
	}
	
	Fwl_Free(buf);
 	return AK_TRUE;
}



T_VOID autotest_record_closefile(T_BOOL safeflag)
{
	T_U8 *buf = AK_NULL;
	T_U32 buf_len = 0;
	

    //当成功保存文件时，就执行下一个
 	if(AK_TRUE == safeflag)
	{
 		scriptname.filenum++;
	}
	else
	{
	    //由于取消保存，所以删除此文件
		 Fwl_FileDestroy(scriptname.file);
	}

	//记录当前录制脚本的个数
	if(!Fs_endConfig(scriptname.filenum))
	{
		AK_DEBUG_OUTPUT("autotest_record_closefile: Fs_endConfig fail\n");
		Fwl_FileClose(scriptname.file);
		return ;
	}

	buf = (T_U8 *)Fwl_Malloc(20);
	if(AK_NULL == buf)
	{
		AK_DEBUG_OUTPUT("autotest_record_closefile: Fwl_Malloc fail\n");
		Fwl_FileClose(scriptname.file);
		return ;
	}
	sprintf(buf, "<endkey>\n");
	AK_DEBUG_OUTPUT("buf :%s\n",buf);
	buf_len = strlen(buf);
	key.writelen += buf_len;
	key.lastbuflen = buf_len;

	Fwl_FileWrite(scriptname.file, buf, buf_len);
		
	key.writelen = 0;
	key.lastbuflen = 0;
	Fwl_Free(buf);
	Fwl_FileClose(scriptname.file);
	scriptname.file = FS_INVALID_HANDLE;
	
}





//记录指令
T_BOOL autotest_record_statement(T_U8 keyID, T_U8 presstype)
{
	T_U32 delaytime1 = 0 ;
	T_U32 delaytime = 0 ;
	//T_MSGBOX   msgbox;
	//T_pFILE file = FS_INVALID_HANDLE;

	if (autotest_recordflag == AK_FALSE)
	{
		autotest_record_scriptinfo_init();
	}
	else
	{
		if(scriptname.file != FS_INVALID_HANDLE)
		{

			//录制指令的过程
			if(key.enterflag == AK_TRUE)
			{
				key.keyID = keyID;
				key.presstype = presstype;
				key.enterflag = AK_FALSE;
				
				AK_DEBUG_OUTPUT("tscr.tscrenterflag: %d\n", tscr.tscrenterflag);

				//记录上一次触摸屏的信息
				if(tscr.tscrenterflag == AK_FALSE)
				{
					delaytime1 = get_tick_count();
					if(delaytime1 > key.delaytime)
					{
						delaytime = delaytime1 - key.delaytime;
					}
					else
					{
						delaytime = T_U32_MAX - 1 - key.delaytime + delaytime1;
					}
					
					
					AK_DEBUG_OUTPUT("tscr.act:%d, tscr.point.x:%d, tscr.point.y:%d, tscr.lastdelaytime:%d\n", 
									tscr.act,  tscr.point.x, tscr.point.y, key.lastdelaytime);
					
					if(!autotest_record_TSCR_writefile(tscr.act, tscr.point.x, tscr.point.y, delaytime))
					{
						AK_DEBUG_OUTPUT("autotest_record_TSCR_writefile: reture ak_false\n");
						autotest_record_closefile(AK_FALSE);
				    	return AK_FALSE;	
					}
				}
				
				key.delaytime = get_tick_count();
				if(delaytime == 0)
				{
					key.lastdelaytime = key.delaytime;
				}
				else
				{
					key.lastdelaytime = delaytime;
				}
				
				tscr.tscrenterflag = AK_TRUE;
			}
			else
			{
				delaytime1 = get_tick_count();
				if(delaytime1 > key.delaytime)
				{
					delaytime = delaytime1 - key.delaytime;
				}
				else
				{
					delaytime = T_U32_MAX - 1 - key.delaytime + delaytime1;
				}

				 
				 AK_DEBUG_OUTPUT("delaytime1: %d\n", delaytime);

				 if(AK_TRUE == endlongpress_no_writeflag)
				 {
					 if (!autotest_record_writefile(keyID, presstype, delaytime))
					 {
					 	AK_DEBUG_OUTPUT("autotest_record_statement: reture ak_false\n");
						autotest_record_closefile(AK_FALSE);
					    return AK_FALSE;
					 }
				 }
				 key.keyID = keyID;
				 key.presstype = presstype;
				 key.lastdelaytime = delaytime;
				 key.delaytime = get_tick_count();
				 AK_DEBUG_OUTPUT("key.delaytime2: %d\n", key.delaytime);
				 
				 //长按菜单键时，停此录制
				if((presstype == PRESS_LONG) &&(keyID == kbCLEAR))
				{
					//进入是否保存脚本文档
					endlongpress_no_writeflag  = AK_FALSE;
					tscr.tscrstartflag = AK_FALSE;
					VME_ReTriggerUniqueEvent(M_EVT_Z06_STOPRECORD, (vUINT32)AK_NULL);
				}
				
				AK_DEBUG_OUTPUT("presstype:%d,  keyID: %d\n",presstype, keyID);
			}
		}
	}
		
	return AK_TRUE;
}


//记录触摸屏信息的指令
T_BOOL autotest_record_Tscr(T_TOUCHSCR_ACTION act, T_S16 pointx, T_S16 pointy)
{
	T_U32 delaytime1 = 0 ;
	T_U32 delaytime = 0 ;
	//T_MSGBOX   msgbox;
	//T_pFILE file = FS_INVALID_HANDLE;

	if (autotest_recordflag == AK_FALSE)
	{
		autotest_record_scriptinfo_init();
	}
	else
	{

		if(scriptname.file != FS_INVALID_HANDLE && tscr.tscrstartflag == AK_TRUE)
		{

			//录制触摸屏的指令的过程
			if(tscr.tscrenterflag == AK_TRUE)
			{
				tscr.act = act;
				tscr.point.x = pointx;
				tscr.point.y = pointy;
				tscr.tscrenterflag = AK_FALSE;
				
				AK_DEBUG_OUTPUT("key.delaytime1: %d\n", key.delaytime);
				if(key.enterflag == AK_FALSE)
				{
					delaytime1 = get_tick_count();
					if(delaytime1 > key.delaytime)
					{
						delaytime = delaytime1 - key.delaytime;
					}
					else
					{
						delaytime = T_U32_MAX - 1 - key.delaytime + delaytime1;
					}
					
					AK_DEBUG_OUTPUT("key.keyID:%d, key.presstype:%d, key.lastdelaytime:%d\n", key.keyID,  key.presstype, key.lastdelaytime);
					if(AK_TRUE == endlongpress_no_writeflag)
				    {
						 if (!autotest_record_writefile(key.keyID, key.presstype, delaytime))
						 {
						 	AK_DEBUG_OUTPUT("autotest_record_statement: reture ak_false\n");
							autotest_record_closefile(AK_FALSE);
						    return AK_FALSE;
						 }
					}
				}
				
				key.delaytime = get_tick_count();
				if(delaytime == 0)
				{
					key.lastdelaytime = key.delaytime;
				}
				else
				{
					key.lastdelaytime = delaytime;
				}
				
				key.enterflag = AK_TRUE;
			}
			else
			{
				delaytime1 = get_tick_count();
				if(delaytime1 > key.delaytime)
				{
					delaytime = delaytime1 - key.delaytime;
				}
				else
				{
					delaytime = T_U32_MAX - 1 - key.delaytime + delaytime1;
				}

				 
				 AK_DEBUG_OUTPUT("delaytime: %d\n", delaytime);
				 if (!autotest_record_TSCR_writefile(act, pointx,  pointy, delaytime))
				 {
				 	AK_DEBUG_OUTPUT("autotest_record_statement: reture ak_false\n");
					autotest_record_closefile(AK_FALSE);
				    return AK_FALSE;
				 }
				 tscr.act = act;
				 tscr.point.x = pointx;
				 tscr.point.y = pointy;
				 key.lastdelaytime = delaytime;
				 key.delaytime = get_tick_count();
				 AK_DEBUG_OUTPUT("tscr.tscrdelaytime: %d\n", key.delaytime);
				 
			}
		}
	}
	return AK_TRUE;
}


#endif





