#include "bios.h"
#include "anyka_types.h"


#ifdef OS_ANYKA
    #define HAL_READ_UINT32( _register_, _value_ )      ((_value_) = *((volatile T_U32 *)(_register_)))
    #define HAL_WRITE_UINT32( _register_, _value_ )     (*((volatile T_U32 *)(_register_)) = (_value_))
#else
    #define HAL_READ_UINT32( _register_, _value_ )
    #define HAL_WRITE_UINT32( _register_, _value_ )
#endif

void setvalue()
{
	unsigned long addr,value;
    printf("please input adress:");
	addr = getul(0xffffffff);
    printf("please input value:");
	value = getul(0xffffffff);
	HAL_WRITE_UINT32(addr,value);
    printf("Adress 0x%x value is :0x%x\n",addr,value);
	return;
}

void download(void)
{
    #define DOWNLOAD_UART 0
    
    int i ,j , k;
    unsigned long start_address,tmp;
    unsigned long Len, num=0;
    unsigned short cs;
    char buf[4];
    char cstmp [2];

    cs = 0;
    memset(buf,0,4);
    printf("Please input the address to be downloaded (0x%x):", DEFAULT_RAM_ADDRESS);
    start_address = getul(DEFAULT_RAM_ADDRESS);
    printf("Now will traslater data from pc ! \n");
    printf("Use your transfer dialog,and NOT INPUT HERE MORE!\n");

    //get len
    while(1)
    {
        if (uart_read(DOWNLOAD_UART, buf, 4) == 4)
            break;
    }

    Len = buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
    Len -= 6;

    printf("File length is 0x%x \n",Len);
    //get data and write
    for(i = 0 ; i <Len; i+=4)
    {
        while(1)
        {
            num = uart_read(DOWNLOAD_UART, buf, 4);
            if (num != 0)
                break;
        }        
    
        /* check sum*/
    	for(j =0 ; j < num ; j++)
    	{
    		if( i+j == Len)
    			break;
    		cs += buf[j];
    	}


        /* Len ==4*n */
        if (i + num == Len)
        {
            /*
                    ...
                    ________
                    |d|d|d|d|
                    ________
                    ________
                    |c|c| | |
                    ________
                    */
            /*write data to memory*/
            tmp = buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
            *(volatile T_U32 *)(start_address) = tmp;
            start_address += 4;
            
            /*get checksum*/
            while(1)
            {
                j = uart_read(DOWNLOAD_UART, buf, 4);
                if (j != 0)
                    break;
            }

            if (j != 2)
            {
    printf("checksum byte number error!\n");
                break;
            }

            for (k=0; k<2; k++)
            {
                cstmp[k] = buf[k];
            }

            break;
        }
        else if ((i + num == Len + 2) && (num < 4))
        {
            /*            
                    ...
                    ________
                    |d|d|d|d|
                    ________
                    ________
                    |d|c|c| |
                    ________
                    */
            cstmp[0] = buf[1];
            cstmp[1] = buf[2];

            for (k=1; k<4; k++)
            {
                buf[k] = 0;
            }
            
            /*write data to memory*/
            tmp = buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
            *(volatile T_U32 *)(start_address) = tmp;
            start_address += 4;

            break;            
        }
        else if ((i + num == Len + 2) && (num == 4))
        {
            /*
                    ...
                    ________
                    |d|d|d|d|
                    ________
                    ________
                    |d|d|c|c|
                    ________
                    */
            cstmp[0] = buf[2];
            cstmp[1] = buf[3];

            for (k=2; k<4; k++)
            {
                buf[k] = 0;
            }
            
            /*write data to memory*/
            tmp = buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
            *(volatile T_U32 *)(start_address) = tmp;
            start_address += 4;

            break;            
        }
        else if (i + num == Len + 1)
        {
        
            /*
                    ...
                    ________
                    |d|d|d|c|
                    ________
                    ________
                    |c| | | |
                    ________
                    */
            cstmp[0] = buf[3];
            
            buf[3] = 0;
            
            /*write data to memory*/
            tmp = buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
            *(volatile T_U32 *)(start_address) = tmp;
            start_address += 4;
            
            /*get checksum*/
            while(1)
            {
                j = uart_read(DOWNLOAD_UART, buf, 4);
                if (j != 0)
                    break;
            }
            cstmp[1] = buf[0];  

            break;
        }
        
        /*write data to memory*/
        tmp = buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
        *(volatile T_U32 *)(start_address) = tmp;
        start_address += 4;
        
    }

    printf("Now check cs !\n");
    if(cs == (cstmp[1] << 8 | cstmp[0]))
    printf("Download OK!\n");
    else
    printf("Download Faild!\n");
}

//this functin looks good
void dump ()
{
	unsigned long start ,end;
	unsigned long tmp;
	unsigned long i,j;
	
    printf("Please input the start address(0x%x):", DEFAULT_RAM_ADDRESS);
	//printf("请输入起始地址:(十六进制):");
	start = getul(DEFAULT_RAM_ADDRESS);
    printf("Please input the end address(0x%x):", DEFAULT_RAM_ADDRESS);
	//printf("请输入结束地址:(十六进制):");
	end = getul(DEFAULT_RAM_ADDRESS);
    printf("   Adress \t    0   \t     4    \t     8    \t      c\n");
	for(i = start ,j=1; i <=end ;)
	{
		/*
		if (j%4 == 0)
		{
			printf("0x%x:",i);
		 	
                }
		*/
		HAL_READ_UINT32(i,tmp);
		if (j%4 == 1)
		{
    printf("0x%x:",i);
    printf("\t0x%x",tmp);
                }else
    printf("\t0x%x",tmp);
		
		if (j%4 == 0)
		{
		 	putch(0x0A);
                	//console_write(CR);
                }
                i+=4;
		j++;
                
	}
    printf("\n");	
}

typedef void (*F)(void);
	
void go(void)
{
	F f;
    printf("Please input the address you will run from(0x%x):", DEFAULT_RAM_ADDRESS);
	//printf("请输入要执行的地址:(十六进制):");
	f = (F)getul(DEFAULT_RAM_ADDRESS);
	if ((unsigned long)f == DEFAULT_RAM_ADDRESS)
	{
    printf("@@@@@@    Start From RAM      \r\n");
		HAL_WRITE_UINT32( 0x30c00000, 0x1 );
	}
	else
	{
    printf("@@@@@@    Normal Start      \r\n");
		HAL_WRITE_UINT32( 0x30c000000, 0x0 );
	}

	f();
	return;
}

void reboot(void)
{
	void (*F)(void);
        MMU_Clean_Invalidate_Dcache();
        MMU_InvalidateICache();
        MMU_DrainWriteBuffer();
        
        MMU_DisableMMU();
        MMU_DisableDCache();
        MMU_DisableICache();
        mini_delay(5);
	F = 0x0;
	F();
	return;
}
