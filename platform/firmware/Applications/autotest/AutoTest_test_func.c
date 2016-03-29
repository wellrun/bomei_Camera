
#include "AutoTest_test_func.h"
#include "AutoTest_record_func.h"
#include "Fwl_public.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "fwl_keyhandler.h"
#include "fwl_tscrcom.h"
#include "AKError.h"
#include "Eng_String.h"



#ifdef SUPPORT_AUTOTEST


#define BUFFER_DATA_LEN          280
#define LINE_DATA_LEN            120
#define KEY_DATA_LEN             20


#define KEY_INFO_LEN             3
#define TSCR_INFO_LEN            4



#define AUTO_NORMAL_FILENAME        "Directfile\\正常功能测试.txt"
#define AUTO_CORSS_FILENAME         "Directfile\\交叉组合测试.txt"
#define AUTO_COMPATIBLE_FILENAME    "Directfile\\兼容性测试.txt"
#define AUTO_PRESS_FILENAME         "Directfile\\压力测试.txt"
#define AUTO_PERFORMANCE_FILENAME   "Directfile\\性能测试.txt"
#define AUTOTEST_DIR_APTH_SCRIPFILE DRI_D"autotest\\Scriptfile"

extern T_BOOL autotest_testflag;
extern T_BOOL autotest_screen_saver_falg;


typedef struct{
	T_U32			        filenameflag;
	F_GekeyinfoState	    SetState;
	T_PAUTO_CTRL    		thrdCtrl;
}T_AUTOTEST_TEST_PARM, *T_pAUTOTEST_TEST_PARM;

T_AUTOTEST_TEST_PARM    AutoTest_testfuncParm;


typedef struct{
	//T_U8			        scriptfilename[BUFFER_DATA_LEN];
	T_U8			        *scriptfilename;
	T_U32                   script_testtimes;
	T_U32		            direct_testtimes;
}T_AUTOTEST_SCRIPT_PARM, *T_pAUTOTEST_SCRIPT_PARM;


typedef struct{
	T_U8			        keyID;
	T_U8			        presstype;
	T_U32                   times;
	T_U32		            delaytime;
	
}T_AUTOTEST_KEY_PARM, *T_pAUTOTEST_KEY_PARM;


typedef struct tag_Tscrinfo
{
	T_U32  times;           //按键执行的次数
    T_TOUCHSCR_ACTION act;
	T_POINT point;
} T_AUTOTEST_TSCR_INFO, *T_pAUTOTEST_TSCR_INFO;


extern T_VOID mini_delay(T_U32 minisecond);


T_VOID Set_Filename_bynum(T_U32 num, T_U8 *filename)
{
	switch(num)
	{
		case 1:
			Utl_StrCpy(filename,  AUTOTEST_DIR_APTH);
			Utl_StrCat(filename,  "\\");
			Utl_StrCat(filename,  AUTO_NORMAL_FILENAME);	
			break;
			
		case 2:
			Utl_StrCpy(filename,  AUTOTEST_DIR_APTH);
			Utl_StrCat(filename,  "\\");
			Utl_StrCat(filename,  AUTO_CORSS_FILENAME);	
			break;
			
		case 3:
			Utl_StrCpy(filename,  AUTOTEST_DIR_APTH);
			Utl_StrCat(filename,  "\\");
			Utl_StrCat(filename,  AUTO_COMPATIBLE_FILENAME);	
			break;
			
		case 4:
			Utl_StrCpy(filename,  AUTOTEST_DIR_APTH);
			Utl_StrCat(filename,  "\\");
			Utl_StrCat(filename,  AUTO_PRESS_FILENAME);	
			break;
			
		case 5:
			Utl_StrCpy(filename,  AUTOTEST_DIR_APTH);
			Utl_StrCat(filename,  "\\");
			Utl_StrCat(filename,  AUTO_PERFORMANCE_FILENAME);	
			break;
			
		default:
			//默认
			Utl_StrCpy(filename,  AUTOTEST_DIR_APTH);
			Utl_StrCat(filename,  "\\");
			Utl_StrCat(filename,  AUTO_NORMAL_FILENAME);	
			break;
	}
}


T_S32 Fs_Utl_Atoi(T_pCSTR strMain)
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
    
    	if((*pMain == '\r') || (*pMain == '\n'))
    	{
    		break;
    	}
		
//    	AK_DEBUG_OUTPUT("sum = %d, *pMain=  %d\n",sum, *pMain);
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


//判断道号的个数，以方便区分是按键信息还是触摸屏信息

T_U32 autotest_isflag(char *input)
{
	T_U32 times = 0;
	T_U32 i = 0;

	for (i = 0; input[i] != 0; i++)
	{
		//AK_DEBUG_OUTPUT("input: %d,%d \n", i, input[i]);
		if(input[i] == ',' )
		{
			times++;
		}
	}
	return times;
	
}



void catch_data(char *ch, char *input, int times) /*提取input中第times个,和第（times + 1）个,之间的数据到ch中*/
{
	int i, j, k;
	int count1 = 0, count2 = 0;
	int num;
	
	if (times == 0)
	{
		for (i = 0; input[i] != ',' && input[i] != 0; i++)
		{
			ch[i] = input[i];
		}
		ch[i] = 0;
		return;
	}
	
	for (i = 0; input[i] != 0; i++)
	{
		if (input[i] == ',')
		{
			count1++;
			if (count1 == times)
			{
				break;	
			}	
		}	
	}
	
	for (j = 0; input[j] != 0; j++)
	{
		if (input[j] == ',')
		{
			count2++;
			if (count2 == times + 1)
			{
				break;	
			}
		}	
	}
	
	num = j - i;
	
	for (k = 0; k < num - 1; k++)
	{
		ch[k] = input[i + 1];
		i++;
	}
	ch[k] = 0;
}


T_U32 read_line_data(T_pFILE file, T_U8 *buf)
{
	T_U8 tempbuf[1] = {0};	
	T_U32 filecurptr = 0;
	
	while(Fwl_FileRead(file,  tempbuf,  1))
	{
		*buf = tempbuf[0];
		buf++;
		filecurptr++;
		if('\n' == tempbuf[0])
		{
			break;
		}
//		AK_DEBUG_OUTPUT("filecurptr: %d\n",filecurptr);
	}
	
	return filecurptr;
}



T_BOOL AutoTest_test_open_scriptfile(T_AUTOTEST_SCRIPT_PARM *AutoTest_scriptParm)
{
	T_pFILE file = FS_INVALID_HANDLE;
	
	T_U8 bufdat[LINE_DATA_LEN] = {0};
	char keyinfo[KEY_DATA_LEN] = {0};
	//T_BOOL endterflag = AK_TRUE;
	T_SYS_MAILBOX   mailbox;
	T_U8 *filename = AK_NULL;
	T_AUTOTEST_KEY_PARM  AutoTest_keyinfoParm;
	T_AUTOTEST_TSCR_INFO  AutoTest_TSCRinfoParm;
	T_MSGBOX   msgbox;
	T_U32 readlen = 0;
	T_U32 i = 0;
	T_U32 strnum = 0;
	T_U32 num = 0;
	T_U32 filename_len = 0;
	
	mailbox.event = SYS_EVT_USER_KEY;

	filename_len = strlen(AUTOTEST_DIR_APTH_SCRIPFILE) + strlen(AutoTest_scriptParm->scriptfilename);
	AK_DEBUG_OUTPUT("filename_len: %d\n", filename_len);

	filename = (T_U8 *)Fwl_Malloc(sizeof(T_U8)*(filename_len+5));
	if(AK_NULL == filename)
	{
		AK_DEBUG_OUTPUT("Fwl_Malloc filename AK_NULL\n");
		return AK_FALSE;	
	}
	memset(filename, 0, sizeof(T_U8)*(filename_len+5));
	
    sprintf(filename, "%s\\%s", AUTOTEST_DIR_APTH_SCRIPFILE, AutoTest_scriptParm->scriptfilename);
	AK_DEBUG_OUTPUT("filename: %s\n",filename);

	if (!Fwl_FileExistAsc(filename))
	{
		AK_DEBUG_OUTPUT("Err: %s is not exist ,pl check the scriptfile\n", filename);
		Fwl_Free(filename);
		filename = AK_NULL;
		return AK_FALSE;	
	}	
	
	
	file = Fwl_FileOpenAsc(filename, _FMODE_READ, _FMODE_READ);
	if (file == FS_INVALID_HANDLE)
	{
		AK_DEBUG_OUTPUT("open direct file error\n");
		Fwl_Free(filename);
		filename = AK_NULL;
		return AK_FALSE;	
	}

	while(1)
	{
		memset(bufdat, 0, LINE_DATA_LEN);
		readlen = read_line_data(file,  bufdat);
		AK_DEBUG_OUTPUT("readlen: %d\n",readlen);
		if(readlen >= LINE_DATA_LEN)
		{
			AK_DEBUG_OUTPUT("error: the len of line is more than 120 byte,exit the scriptfile test \n");
			mini_delay(2000);
			Fwl_Free(filename);
			filename = AK_NULL;
			Fwl_FileClose(file);
			file = FS_INVALID_HANDLE;
			return AK_FALSE;
		}

		if((bufdat[0] == 239) && (bufdat[1] == 187) && (bufdat[2] == 191))
		{
			AK_DEBUG_OUTPUT("error: the format of file is UIF-8, and is not ascii\n");
			AK_DEBUG_OUTPUT("PLS,chang the UIF-8 TO ASCII in the UE (file->chang->UIF-8 TO ASCII) \n");
			MsgBox_InitStr(&msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csSETUP_AUTOTEST_SCRIPTFILE_FORMATERROR), MSGBOX_INFORMATION);
            MsgBox_Show(&msgbox);
            Fwl_RefreshDisplay();
			
        	MsgBox_InitAfx(&msgbox, 20, ctHINT, csSETUP_AUTOTEST_SCRIPTFILE_FORMATERROR, MSGBOX_INFORMATION);
			MsgBox_SetDelay(&msgbox, MSGBOX_DELAY_1);
			
			//Fwl_Free(filename);
			//Fwl_FileClose(file);
			//autotest_testflag = AK_FALSE;
			//AutoTest_Test_KillThead(&AutoTest_testfuncParm.thrdCtrl);
    		//m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&msgbox);
			while(1);
		}
	
		if(((bufdat[0] == '/') && (bufdat[1] == '/')) || (bufdat[0] == '\n') || (bufdat[0] == '\r'))
		{
			continue;
		}
		else
		{
			
			//开头<startkey>
			if ((bufdat[0] == '<') && (bufdat[1] == 's') && (bufdat[2] == 't') 
				&& (bufdat[3] == 'a') && (bufdat[4] == 'r') && (bufdat[5] == 't') 
				&& (bufdat[6] == 'k') && (bufdat[7] == 'e') && (bufdat[8] == 'y') && (bufdat[9] == '>'))
			{ 
				continue;
			}
			
			//结尾<endkey>
			if ((bufdat[0] == '<') && (bufdat[1] == 'e') && (bufdat[2] == 'n') 
				&& (bufdat[3] == 'd') && (bufdat[4] == 'k') && (bufdat[5] == 'e') 
				&& (bufdat[6] == 'y') && (bufdat[7] == '>'))
			{ 
					AutoTest_scriptParm->script_testtimes--;
					if (AutoTest_scriptParm->script_testtimes == 0)
					{
						AK_DEBUG_OUTPUT("autotest: finish the autotest %s test**********\n", filename);
						//autotest_screen_saver_falg = AK_TRUE;
						//m_triggerEvent(M_EVT_EXIT, AK_NULL);//M_EVT_Z09COM_SYS_RESET
						mini_delay(2000);
						break;
					}
					AK_DEBUG_OUTPUT("autotest: finish the autotest %s times: %d**********\n", filename, AutoTest_scriptParm->script_testtimes);
					Fwl_FileSeek(file, 0, _FSEEK_SET);
					//autotest_screen_saver_falg = AK_TRUE;
					//m_triggerEvent(M_EVT_EXIT, AK_NULL);//M_EVT_Z09COM_SYS_RESET
					mini_delay(3000);
					continue;
			}
			

			//判断是按键信息还是触摸屏的信息
			//如果道号的个数是strnum = 3，那么是按键信息，如是是4那么是触摸屏信息，否则出错
			strnum = autotest_isflag(bufdat);
			if(strnum == KEY_INFO_LEN)//是按键信息
			{
				/* 提取按键名，按键执行次数，按键delay时间 ,按键类型*/
				memset(keyinfo, 0, KEY_DATA_LEN);
				catch_data(keyinfo, bufdat, 0);   
				AutoTest_keyinfoParm.keyID = Fs_Utl_Atoi(keyinfo);
				//AK_DEBUG_OUTPUT("AutoTest_keyinfoParm.keyID: %d\n",AutoTest_keyinfoParm.keyID);
				
				memset(keyinfo, 0, KEY_DATA_LEN);
				catch_data(keyinfo, bufdat, 1);
				AutoTest_keyinfoParm.times = Fs_Utl_Atoi(keyinfo);
				//AK_DEBUG_OUTPUT("AutoTest_keyinfoParm.times: %d\n",AutoTest_keyinfoParm.times);
				
				memset(keyinfo, 0, KEY_DATA_LEN);
				catch_data(keyinfo, bufdat, 2);
				AutoTest_keyinfoParm.delaytime = Fs_Utl_Atoi(keyinfo);
				//AK_DEBUG_OUTPUT("AutoTest_keyinfoParm.delaytime: %d\n",AutoTest_keyinfoParm.delaytime);
				
				memset(keyinfo, 0, KEY_DATA_LEN);
				catch_data(keyinfo, bufdat, 3);
				AutoTest_keyinfoParm.presstype = Fs_Utl_Atoi(keyinfo);
				//AK_DEBUG_OUTPUT("AutoTest_keyinfoParm.presstype: %d\n",AutoTest_keyinfoParm.presstype);

				AK_DEBUG_OUTPUT("(auto_key: %d, %d, %d, %d)\n",AutoTest_keyinfoParm.keyID, AutoTest_keyinfoParm.times, 
					AutoTest_keyinfoParm.delaytime, AutoTest_keyinfoParm.presstype);
				
				for(i = 0; i < AutoTest_keyinfoParm.times; i++)
				{
					mailbox.param.c.Param1 = (T_U8)AutoTest_keyinfoParm.keyID;
	       			mailbox.param.c.Param2 = (T_U8)AutoTest_keyinfoParm.presstype;
					AK_PostEventEx(&mailbox, AK_NULL,AK_TRUE, AK_FALSE,AK_TRUE);
					mini_delay(AutoTest_keyinfoParm.delaytime);
				}
			}
			else if(strnum == TSCR_INFO_LEN)//判断是触摸屏信息
			{
				//获取触摸屏的X和Y值，次数，delaytime,和act值
				memset(keyinfo, 0, KEY_DATA_LEN);
				catch_data(keyinfo, bufdat, 0);   
				AutoTest_TSCRinfoParm.point.x = Fs_Utl_Atoi(keyinfo);
				//AK_DEBUG_OUTPUT("AutoTest_TSCRinfoParm.point.x: %d\n",AutoTest_TSCRinfoParm.point.x);
	
				
				memset(keyinfo, 0, KEY_DATA_LEN);
				catch_data(keyinfo, bufdat, 1);
				AutoTest_TSCRinfoParm.point.y = Fs_Utl_Atoi(keyinfo);
				//AK_DEBUG_OUTPUT("AutoTest_TSCRinfoParm.point.y: %d\n",AutoTest_TSCRinfoParm.point.y);
	
				
				memset(keyinfo, 0, KEY_DATA_LEN);
				catch_data(keyinfo, bufdat, 2);
				AutoTest_TSCRinfoParm.times = Fs_Utl_Atoi(keyinfo);
				//AK_DEBUG_OUTPUT("AutoTest_TSCRinfoParm.times: %d\n",AutoTest_TSCRinfoParm.times);

				
				memset(keyinfo, 0, KEY_DATA_LEN);
				catch_data(keyinfo, bufdat, 3);
				AutoTest_keyinfoParm.delaytime = Fs_Utl_Atoi(keyinfo);
				//AK_DEBUG_OUTPUT("AutoTest_keyinfoParm.delaytime: %d\n",AutoTest_keyinfoParm.delaytime);

	
				memset(keyinfo, 0, KEY_DATA_LEN);
				catch_data(keyinfo, bufdat, 4);
				AutoTest_TSCRinfoParm.act = Fs_Utl_Atoi(keyinfo);
				//AK_DEBUG_OUTPUT("AutoTest_TSCRinfoParm.act: %d\n",AutoTest_TSCRinfoParm.act);

				AK_DEBUG_OUTPUT("(auto_ts: %d, %d, %d, %d, %d)\n",AutoTest_TSCRinfoParm.point.x, AutoTest_TSCRinfoParm.point.y, 
					AutoTest_TSCRinfoParm.times, AutoTest_keyinfoParm.delaytime, AutoTest_TSCRinfoParm.act);
				for(i = 0; i < AutoTest_TSCRinfoParm.times; i++)
				{
					Autotest_TSCR_SendEvent_Callback(AutoTest_TSCRinfoParm.act , AutoTest_TSCRinfoParm.point.x, AutoTest_TSCRinfoParm.point.y);
					mini_delay(AutoTest_keyinfoParm.delaytime);
				}

			}
			else
			{
				for(num = 0; num < readlen-1; num++)
				{
					//空格是32，此表示带有空格的空行
					if((bufdat[num] == 32  || bufdat[num] == '\r') && (bufdat[readlen-1] == '\n'))
					{
						continue;
					}
					else//如果不是空格的空行那格式出错
					{
						AK_DEBUG_OUTPUT("strnum: %d\n", strnum);
						AK_DEBUG_OUTPUT("error: the format of buf is error **********\n");
						mini_delay(2000);
						Fwl_Free(filename);
						filename = AK_NULL;
						Fwl_FileClose(file);
						file = FS_INVALID_HANDLE;
						return AK_FALSE;
					}
						
				}
				
			}

		}
	}

	Fwl_Free(filename);
	filename = AK_NULL;
	
	Fwl_FileClose(file);
	file = FS_INVALID_HANDLE;

	return AK_TRUE;
}



T_BOOL AutoTest_test_open_directfile(T_U8 *directfilename)
{
	T_AUTOTEST_SCRIPT_PARM   AutoTest_scriptParm;
	T_pFILE file = FS_INVALID_HANDLE;
	T_U8 script_times[20] = {0};
	T_U8 buf[10] = {0};
	T_U8 *bufdat = AK_NULL;
	T_U8 *tempbuf = AK_NULL;
    T_BOOL endreadflag = AK_FALSE;
	T_U32 count = 0;
	T_U32 filecurptr = 0;
	T_U32 readlen = 0;  
	T_U32 linelen = 0;  
	T_MSGBOX   msgbox;
	tempbuf = buf;

//	AK_DEBUG_OUTPUT("directfilename****************\n");
	AK_DEBUG_OUTPUT("directfilename: %s\n",directfilename);

	mini_delay(3000);
	if (!Fwl_FileExistAsc(directfilename))
	{
		AK_DEBUG_OUTPUT("Err: %s is not exist ,pl check the directfile\n", directfilename);
		autotest_testflag = AK_FALSE;
		AutoTest_Test_KillThead(&AutoTest_testfuncParm.thrdCtrl);
		return AK_FALSE;	
	}
	
	file = Fwl_FileOpenAsc(directfilename, _FMODE_READ, _FMODE_READ);
//	AK_DEBUG_OUTPUT("directfilename****************11111111\n");

	if (file == FS_INVALID_HANDLE)
	{
		AK_DEBUG_OUTPUT("open direct file error\n");
		autotest_testflag = AK_FALSE;
		AutoTest_Test_KillThead(&AutoTest_testfuncParm.thrdCtrl);
		return AK_FALSE;	
	}
//	AK_DEBUG_OUTPUT("directfilename****************\n");

	bufdat = (T_U8 *)Fwl_Malloc(BUFFER_DATA_LEN);
	if(AK_NULL == bufdat)
	{
		AK_DEBUG_OUTPUT("Fwl_Malloc bufdat AK_NULL\n");
		autotest_testflag = AK_FALSE;
		Fwl_FileClose(file);
		file = FS_INVALID_HANDLE;
		AutoTest_Test_KillThead(&AutoTest_testfuncParm.thrdCtrl);
		return AK_FALSE;
	}
	AutoTest_scriptParm.scriptfilename = (T_U8 *)Fwl_Malloc(BUFFER_DATA_LEN);
	if(AK_NULL == AutoTest_scriptParm.scriptfilename)
	{
		AK_DEBUG_OUTPUT("Fwl_Malloc bufdat AK_NULL\n");
		autotest_testflag = AK_FALSE;
		Fwl_Free(bufdat);
		Fwl_FileClose(file);
		file = FS_INVALID_HANDLE;
		AutoTest_Test_KillThead(&AutoTest_testfuncParm.thrdCtrl);
		return AK_FALSE;
	}
	memset(AutoTest_scriptParm.scriptfilename, 0, BUFFER_DATA_LEN);

	while(1)
	{
		memset(bufdat, 0, BUFFER_DATA_LEN);
		linelen = read_line_data(file,  bufdat);
		readlen += linelen;
		if(linelen >= BUFFER_DATA_LEN)
		{
			AK_DEBUG_OUTPUT("error: the len of line is more than 280 byte,exit the scriptfile test \n");
			Fwl_Free(bufdat);
			bufdat = AK_NULL;
			Fwl_Free(AutoTest_scriptParm.scriptfilename);
			AutoTest_scriptParm.scriptfilename = AK_NULL;
			Fwl_FileClose(file);
			file = FS_INVALID_HANDLE;

			autotest_testflag = AK_FALSE;
			AutoTest_Test_KillThead(&AutoTest_testfuncParm.thrdCtrl);
			return AK_FALSE;
		}
//		AK_DEBUG_OUTPUT("readlen0: %d\n",readlen);
//		AK_DEBUG_OUTPUT("%s", bufdat);

		if((bufdat[0] == 239) && (bufdat[1] == 187) && (bufdat[2] == 191))
		{
			AK_DEBUG_OUTPUT("error: the format of file is UIF-8, and is not ascii\n");
			AK_DEBUG_OUTPUT("PLS,chang the UIF-8 TO ASCII in the UE (file->chang->UIF-8 TO ASCII) \n");
			MsgBox_InitStr(&msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csSETUP_AUTOTEST_DIRECTFILE_FORMATERROR), MSGBOX_INFORMATION);
            MsgBox_Show(&msgbox);
            Fwl_RefreshDisplay();
			
        	MsgBox_InitAfx(&msgbox, 20, ctHINT, csSETUP_AUTOTEST_DIRECTFILE_FORMATERROR, MSGBOX_INFORMATION);
			MsgBox_SetDelay(&msgbox, MSGBOX_DELAY_1);
			
			Fwl_Free(bufdat);
			Fwl_Free(AutoTest_scriptParm.scriptfilename);
			autotest_testflag = AK_FALSE;
			Fwl_FileClose(file);
			file = FS_INVALID_HANDLE;
			//AutoTest_Test_KillThead(&AutoTest_testfuncParm.thrdCtrl);
    		//m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&msgbox);
			while(1);
		}

		
		
		if(((bufdat[0] == '/') && (bufdat[1] == '/')) || (bufdat[0] == '\n') || (bufdat[0] == 
		'\r'))
		{
			continue;
		}
		else
		{
			if ((bufdat[0] == '<') && (bufdat[1] == 's') && (bufdat[2] == 't')
				 && (bufdat[3] == 'a')&& (bufdat[4] == 'r') && (bufdat[5] == 't')
				    && (bufdat[6] == '>'))
			{
				count = 7;
				while(1)
				{
					if('\n' == bufdat[count])
					{
						endreadflag = AK_TRUE;
						break;
					}
					*tempbuf = bufdat[count];
					tempbuf++;
					count++;
//					AK_DEBUG_OUTPUT("readlen: %d\n",bufdat[count]);
//					AK_DEBUG_OUTPUT("readlen: %d\n",*tempbuf);
				}
			}
		}
		if(endreadflag == AK_TRUE)
		{
			break;
		}
	}

	

//	AK_DEBUG_OUTPUT("buf: %d,%d,%d,%d **** ",buf[0], buf[1], buf[2],buf[3]);
	AutoTest_scriptParm.direct_testtimes = Fs_Utl_Atoi(buf);
	AK_DEBUG_OUTPUT("AutoTest_scriptParm.direct_testtimes: %d\n",AutoTest_scriptParm.direct_testtimes);
	
	filecurptr = readlen;  //当前读完<start>time后的位置

	endreadflag = AK_FALSE;
	
	//开始读脚本名和循环次数
	while(1)
	{
		memset(bufdat, 0, BUFFER_DATA_LEN);
		
		linelen = read_line_data(file,  bufdat);
		readlen += linelen;
		if(linelen >= BUFFER_DATA_LEN)
		{
			AK_DEBUG_OUTPUT("error: the len of line is more than 280 byte,exit the scriptfile test \n");
			Fwl_Free(bufdat);
			bufdat = AK_NULL;
			Fwl_Free(AutoTest_scriptParm.scriptfilename);
			AutoTest_scriptParm.scriptfilename = AK_NULL;
			Fwl_FileClose(file);
			file = FS_INVALID_HANDLE;

			autotest_testflag = AK_FALSE;
			AutoTest_Test_KillThead(&AutoTest_testfuncParm.thrdCtrl);
			return AK_FALSE;
		}

//		AK_DEBUG_OUTPUT("readlen1 %d:\n",readlen);
		if(((bufdat[0] == '/') && (bufdat[1] == '/')) || (bufdat[0] == '\n') || (bufdat[0] == 
		'\r'))
		{
			continue;
		}
		else
		{

			//是否读到结束标志符
			if (bufdat[0] < 0x80) 
			{
				
				if ((bufdat[0] == '<') && ( bufdat[1] == 'e' ) 
					&& (bufdat[2] == 'n') && (bufdat[3]=='d')
					&& (bufdat[4] == '>'))
				{
					AutoTest_scriptParm.direct_testtimes = AutoTest_scriptParm.direct_testtimes - 1;
					endreadflag = AK_TRUE;
				}
			}
			if (endreadflag == AK_TRUE)
			{
				if(AutoTest_scriptParm.direct_testtimes == 0)
				{
					AK_DEBUG_OUTPUT("autotest:  finish the autotest %s test**********\n", directfilename);
					/*****防止重复打开执行测试*****/
					
					autotest_screen_saver_falg = AK_TRUE;
					mini_delay(1000);
					
					autotest_testflag = AK_FALSE;
					//m_triggerEvent(M_EVT_Z09COM_SYS_RESET, AK_NULL);
					MsgBox_InitStr(&msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csSETUP_AUTOTEST_FINISH_TEST_DIRECTFILE), MSGBOX_INFORMATION);
		            MsgBox_Show(&msgbox);
		            Fwl_RefreshDisplay();
					
		        	MsgBox_InitAfx(&msgbox, 20, ctHINT, csSETUP_AUTOTEST_FINISH_TEST_DIRECTFILE, MSGBOX_INFORMATION);
					MsgBox_SetDelay(&msgbox, MSGBOX_DELAY_1);
					
		    		m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&msgbox);
					AutoTest_Test_KillThead(&AutoTest_testfuncParm.thrdCtrl);
					break;
				}
				AK_DEBUG_OUTPUT("autotest:  finish the autotest %s times: %d**********\n", directfilename, AutoTest_scriptParm.direct_testtimes);
				endreadflag = AK_FALSE;
				Fwl_FileSeek( file, filecurptr, _FSEEK_SET);
				continue;
			}
			
			
			//获取到脚本名和循环次数
			memset(AutoTest_scriptParm.scriptfilename, 0, BUFFER_DATA_LEN);
			catch_data(AutoTest_scriptParm.scriptfilename, bufdat, 0);
			catch_data(script_times, bufdat, 1);
			AutoTest_scriptParm.script_testtimes = Fs_Utl_Atoi(script_times);
			AK_DEBUG_OUTPUT("AutoTest_scriptParm.scriptfilename: %s\n",AutoTest_scriptParm.scriptfilename);
			AK_DEBUG_OUTPUT("AutoTest_scriptParm.script_testtimes: %d\n",AutoTest_scriptParm.script_testtimes);
			mini_delay(1000);
			
			//开始执行读到的脚本用例
			if(!AutoTest_test_open_scriptfile(&AutoTest_scriptParm))
			{
				Fwl_Free(bufdat);
				bufdat = AK_NULL;
				Fwl_Free(AutoTest_scriptParm.scriptfilename);
				AutoTest_scriptParm.scriptfilename = AK_NULL;
				Fwl_FileClose(file);
				file = FS_INVALID_HANDLE;
	
				autotest_testflag = AK_FALSE;
				AutoTest_Test_KillThead(&AutoTest_testfuncParm.thrdCtrl);
				return AK_FALSE;
			}			
		}
	}

	Fwl_Free(bufdat);
	bufdat = AK_NULL;

	Fwl_Free(AutoTest_scriptParm.scriptfilename);
    AutoTest_scriptParm.scriptfilename = AK_NULL;

	Fwl_FileClose(file);
	file = FS_INVALID_HANDLE;
	
	return AK_TRUE;
}


static T_VOID AutoTest_TestThreadFun(T_U32 argc, T_VOID *argv)
{
	T_BOOL ret;
	T_AUTOTEST_TEST_PARM *AutoTest_funcParm = (T_AUTOTEST_TEST_PARM *)argv;
	T_U8 filename[30] = {0};
	
	AK_DEBUG_OUTPUT("AutoTest_TestThreadFun START:\n");
	
	if (AK_NULL == AutoTest_funcParm)
	{
		autotest_testflag = AK_FALSE;
		return;
	}

	if (AK_NULL != AutoTest_funcParm->SetState)
	{
		AutoTest_funcParm->SetState(eFS_AUTOTEST_ING);
	}
	
	AK_DEBUG_OUTPUT("AutoTest_test_open_directfile start %s:\n",AutoTest_funcParm->filenameflag);


	Set_Filename_bynum(AutoTest_funcParm->filenameflag, filename);
		
	ret = AutoTest_test_open_directfile(filename);

	AK_DEBUG_OUTPUT("AutoTest_test_open_directfile End :\n");
	
	if (ret)
	{
		if (AK_NULL != AutoTest_funcParm->SetState)
		{
			AutoTest_funcParm->SetState(eFS_AUTOTEST_Success);
		}
	}
	else
	{
		if (AK_NULL != AutoTest_funcParm->SetState)
		{
			AutoTest_funcParm->SetState(eFS_AUTOTEST_Fail);
		}
	}
	AK_DEBUG_OUTPUT("AutoTest_TestThreadFun End :\n");
}

static T_PAUTO_CTRL AutoTest_TestThread(ThreadAutoTestFunPTR Fun, T_pVOID pData, T_U32 priority)
{
	T_PAUTO_CTRL thrdCtrl = AK_NULL;
		
	thrdCtrl = (T_PAUTO_CTRL)Fwl_Malloc(sizeof(T_AUTO_CTRL));
	AK_DEBUG_OUTPUT("AutoTest_TestThread malloc:0x%x\n",thrdCtrl);
	
	if (AK_NULL == thrdCtrl)
	{
		AK_DEBUG_OUTPUT("AutoTest_TestThread Malloc Error\n");
		autotest_testflag = AK_FALSE;
		return 0;
	}

	thrdCtrl->thread = AK_Create_Task((T_VOID*)Fun, "AutoTest",
					1, pData, 
				   thrdCtrl->Stack, AUTOTEST_THRD_STACK_SIZE,
				   (T_OPTION)priority, AUTOTEST_THREAD_TIMESLICE,
				   (T_OPTION)AK_PREEMPT,(T_OPTION)AK_START);

	AK_DEBUG_OUTPUT("AutoTest_TestThread Create_Task END: %d\n", thrdCtrl->thread);
    if (AK_IS_INVALIDHANDLE(thrdCtrl->thread))
    {
		AK_DEBUG_OUTPUT("AutoTest_TestThread Create_Task Error\n");
		Fwl_Free(thrdCtrl);
		thrdCtrl = AK_NULL;
		autotest_testflag = AK_FALSE;
		return 0;
	}
	AK_DEBUG_OUTPUT("AutoTest_TestThread Create_Task END\n");
    return thrdCtrl;
}


//创建一个线程
T_BOOL  AutoTest_Test_CreatThread(T_U32 filenameflag, F_GekeyinfoState	 SetState)
{
	T_BOOL ret = AK_FALSE;
	T_PAUTO_CTRL **thrdCtrl;

	if (AK_NULL != AutoTest_testfuncParm.SetState)
	{
		AutoTest_testfuncParm.SetState(eFS_AUTOTEST_ING);
	}
	
	AutoTest_testfuncParm.SetState = SetState;
	AutoTest_testfuncParm.filenameflag = filenameflag;

//	AK_DEBUG_OUTPUT("AutoTest_testfuncParm.filenameflag: %d \n", AutoTest_testfuncParm.filenameflag);
//	AK_DEBUG_OUTPUT("AutoTest_Test_CreatThread  1\n");
	*thrdCtrl = AutoTest_TestThread((ThreadAutoTestFunPTR)AutoTest_TestThreadFun, (T_pVOID)&AutoTest_testfuncParm, AUTOTEST_THREAD_PRI);
	AutoTest_testfuncParm.thrdCtrl = *thrdCtrl;
	AK_DEBUG_OUTPUT("AutoTest_Test_CreatThread  end: %d\n", *thrdCtrl);
	if (0 != *thrdCtrl)
	{
		ret = AK_TRUE;
	}
	
	return ret;
}



T_VOID  AutoTest_Test_KillThead(T_PAUTO_CTRL **ThreadHandle)
{   
	T_PAUTO_CTRL thrdCtrl = AK_NULL;

	if (AK_NULL == ThreadHandle)
	{
		return;
	}
	
	thrdCtrl = *ThreadHandle;

	if (AK_NULL != thrdCtrl)
	{
		AK_Terminate_Task(thrdCtrl->thread);
		AK_Delete_Task(thrdCtrl->thread);
		AK_DEBUG_OUTPUT("FsApi_KillCopyThead free:0x%x\n",thrdCtrl);
		Fwl_Free(thrdCtrl);
		*ThreadHandle = AK_NULL;
	}
}


#endif

