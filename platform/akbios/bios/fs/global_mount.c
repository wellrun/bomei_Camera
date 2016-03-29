#include "bios.h"
#include "anyka_types.h"
#include "Fwl_osfs.h"
#include "mount.h"
#include "nand_list.h"
//#include "encrypt.h"


#define  ENCRYPTFILE "a:/encrypt.bin"


#define NAME_LEN    50    // define mmi_name and res_name's length.
extern int get_line_data(char *buf, int n);

void test(void)
{
printf("$$OK$$\r\n");
}


void getid(void)
{
	char buf1[36];
	char buf2[36];
	char rets;
	char i;
	char bufxx[4096];
	char buf3[36];
	
	char hexStr[] =  {'0', '1', '2', '3',
		                     '4', '5', '6', '7',
		                     '8', '9', 'A', 'B',
		                     'C', 'D','E', 'F'};
	

	FHA_asa_read_file("MID", bufxx, 4096);
	

	bufxx[22] = 0;

       for(i = 0; i < 10; i++)
	{
		if((bufxx[i] ^(i * i +5*i+ 7)) != bufxx[i + 10])
			break;
	}

       
	if(i == 10)
	{
		for(i = 0; i < 10; i++)
	       {
	       	bufxx[i] = bufxx[i] ^ (i * 5 + i *i  + 3);
			bufxx[i] = bufxx[i] + 23;
	       }
	   
		for(i = 0; i < 10; i++)
		{
			buf3[i*2] = hexStr[bufxx[i] >> 4];
			buf3[i*2+1] = hexStr[bufxx[i] &0x0f];
		}
		buf3[i*2] = 0;
printf("MobileID:%s\r\n", buf3);
	}
	else
	{
		for(i = 0; i < 20; i++)
		{
			buf3[i] = '0';
		}
		buf3[20] = 0;
printf("MobileID:%s\r\n", buf3);
	}

	return;
/*
	rets = sdsddfdw(buf1);
	
	if(0 == rets)
	{
		for(i = 0; i < 10; i++)
		{
			buf2[i*2] = hexStr[buf1[i+6] >> 4];
			buf2[i*2+1] = hexStr[buf1[i+6] &0x0f];
		}
		buf2[i*2] = 0;
		printf("MobileID:%s\r\n", buf2);
	       return;
	}
	else
	{
		printf("get id failue: %d\r\n",rets);
	}
*/
	
}

int get_line_data(char *buf, int n)
{
	int i;
	for(i = 0; i < n; i++)
	{
		buf[i] = (char)getch();
/*		if(buf[i] == CR)
		{
			buf[i + 1] = 0;
			return i;
		}*/
	}
	return n;
}

void storepassword(void)
{
	T_hFILE fs_h, fs_s;
	T_S8 buffer[1024*1024];
  T_S8 paths[] = ENCRYPTFILE;
	T_U32   rets;
	T_U32		datalen, readlen, writelen;
	T_U32		i;
	
	datalen = getul(0xffffffff);

printf("%d\r\n", datalen);
	

	paths[0] += Nand_GetZoneIdByType(SYSTEM_PARTITION);
	 
	fs_h = Fwl_FileOpen(paths ,_FMODE_CREATE ,_FMODE_CREATE);

	writelen = 0;
	while(writelen < datalen)
	{
		 readlen = datalen - writelen;
		 if(readlen > 1024)
		 {
		     readlen = 1024;
		 }
		 rets = get_line_data(buffer + writelen, readlen);
		 
		 if(rets != readlen)
		 {
printf("$FAILE$, rets = %d, readlen = %d\r\n", rets, readlen);
		 		return;
		 }
		 
		 
		 writelen += readlen;
	}

	 Fwl_FileWrite(fs_h, buffer, writelen);

	 Fwl_FileClose(fs_h);
printf("$OK$\r\n");
}

T_BOOL FS_Load_BiosPic(T_U8 *path,T_U8 *buffer,T_U32 size)
{
	T_hFILE fs_h;
	T_U32 file_len;
	T_S32 ret;
	
	path[0] += Nand_GetZoneIdByType(SYSTEM_PARTITION);

printf("path : %s\r\n",path);

    fs_h = Fwl_FileOpen(path ,_FMODE_READ,_FMODE_READ);
	if( fs_h != FS_INVALID_HANDLE )
    {
printf("fs_h != -1\r\n");
    	file_len = Fwl_GetFileLen(fs_h);
		if( file_len > size )
		{
printf("\nBiosPic file lens is 0x%x >0x%x",file_len,size);
		    Fwl_FileClose(fs_h);
		    return AK_FALSE;
		}
printf("\nBiosPic file lens is %d\n",file_len);
		ret = Fwl_FileRead(fs_h,buffer,file_len);
		if( ret != file_len )
		{
printf("\nBiosPic file read fail %d", ret);
		    Fwl_FileClose(fs_h);
		    return AK_FALSE;
		}
		Fwl_FileClose(fs_h);
printf("\nBiosPic file read SUCCESS!\r\n");
		return AK_TRUE;
	}
printf("\nBiosPic file IS NOT EXIT fs_h : %d\r\n",fs_h);
	return AK_FALSE;
		
}

static start_run(void)
{
    extern T_U32 Image$$ER_RO$$Base;
    T_U8 *src_start = &Image$$ER_RO$$Base;
    T_U8 *dst_start = DEFAULT_RAM_ADDRESS;
    void (*F)(void);

	(void*)F = (void*)DEFAULT_MMI_ADDRESS; 

    if (('N' == dst_start[32]) && ('A' == dst_start[33]) && ('N' == dst_start[34]) && ('D' == dst_start[35]))
    {
        memcpy(dst_start + 36, src_start + 36, sizeof(T_NAND_PHY_INFO) - 4);
    }

    printf("Run MMI...\n");
	F();      
}    
void loadmmi(T_U8* MMI_path, T_U8* MMI_back_path)
{
	T_hFILE fp_new=FS_INVALID_HANDLE, fp_bak=FS_INVALID_HANDLE;
	T_U32 file_len;
	T_U32 file_bak_len;
	T_S32 ret;
	T_U8 path_new[NAME_LEN];
    T_U8 path_bak[NAME_LEN];
    extern T_U32 Image$$ER_RO$$Base;
    T_U32 bios_start_addr = &Image$$ER_RO$$Base; 

    memcpy(path_new, MMI_path, NAME_LEN);
    memcpy(path_bak, MMI_back_path, NAME_LEN);

	printf("Loading MMI...\n");
    //load mmi file
	
    fp_new = Fwl_FileOpenAsc(path_new ,_FMODE_READ ,_FMODE_READ);
	
    if( fp_new != FS_INVALID_HANDLE )
    {
    	file_len = Fwl_GetFileLen(fp_new);
		
		//printf("file lens is %d,bak:%d\n",file_len,file_bak_len);
		
		if (0 == file_len) 
		{
			printf("file length error\n");
		    Fwl_FileClose(fp_new);
		    goto openBak;
		}
		if (DEFAULT_RAM_ADDRESS+file_len >= bios_start_addr)
		{
		    printf("mmi bin size %lu is too big, beyond bios run address 0x%x, check it\n", file_len, bios_start_addr);
		    Fwl_FileClose(fp_new);
		    return;
		}
		
		printf("load mmi to ram address 0x%x, bin size=%lu\n", DEFAULT_RAM_ADDRESS, file_len);
		ret = Fwl_FileRead(fp_new,(volatile T_pVOID)DEFAULT_RAM_ADDRESS,file_len);

		Fwl_FileClose(fp_new); 
        if (ret != file_len)
        {
			printf("\n file read error, readed size=%lu\n", ret);
            goto openBak;
        }
		else
		{
		    Fwl_FileClose(fp_bak); 
            goto end;
		}
    }
openBak:
	printf("MMI file open failed, try to open backup MMI\n");

	fp_bak = Fwl_FileOpenAsc(path_bak ,_FMODE_READ ,_FMODE_READ);
	//path_bak[0] += Nand_GetZoneIdByType(ZT_MMI_BK);
	//fp_bak = Fwl_FileOpenAsc(path_bak ,_FMODE_READ,_FMODE_READ);
	if( fp_bak == FS_INVALID_HANDLE )
	{
		printf("mmi file open backup failed\n");
	    return;
	}
	else
	{
		printf("open bakeup success, recover it now\n");
	    
	    file_bak_len = Fwl_GetFileLen(fp_bak);
	    ret = Fwl_FileRead(fp_bak, (volatile T_pVOID)DEFAULT_RAM_ADDRESS, file_bak_len);
		Fwl_FileClose(fp_bak);

		if( ret != file_bak_len )
	    {
			printf("\n file read fail %d", ret);
	        return;
	    }
	    
	    fp_new = Fwl_FileOpenAsc(path_new, _FMODE_CREATE, _FMODE_CREATE);
		if ( fp_new == FS_INVALID_HANDLE )
		{
			printf("\n file create fail\n");
	        return;
		}
		
		ret = Fwl_FileWrite(fp_new, (volatile T_pVOID)DEFAULT_RAM_ADDRESS, file_bak_len);
        Fwl_FileClose(fp_new);
        if( ret != file_bak_len )
	    {
            Fwl_FileDelete(path_new);
			printf("\n file write fail %d\n", ret);
			printf("recover MMI fail!\n");
	    }
        else
        { 
			printf("recover MMI success!\n");
        }
	}
        
end:
	printf("Ready to start...\n");
    Nand_Restore_Default_Scale(0);

	Fwl_DeInitFs();

    start_run();
}

