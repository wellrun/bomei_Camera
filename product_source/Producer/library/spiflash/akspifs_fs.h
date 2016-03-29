#ifndef __SPIFS_FS_H__
#define __SPIFS_FS_H__

#include "anyka_types.h"

#define FLASH_FS_VERSION   "V1.2.02"

#define ENABLE_DEBUG 0    //for open some print information!

extern struct AK_Config_Info setconfiginfo;


typedef unsigned long  vUINT32;
typedef signed   long  vINT32;
typedef unsigned short vUINT16;
typedef signed   short vINT16;
typedef void           vVOID;
typedef char           vCHAR;
/* data block size definition */
#define INODE_SECTOR_SIZE       8192 //8*1024
#define BLOCK_SIZE_4K			4096 /*4k*/
#define FORMAT_DELETE_SISE      65536//64*1024
#define BLK4KNUM_PER_64KBACK    16
#define SET_GROUP_FILE_BLKNUM		3
/* define the data block type */
#define DB_TYPE_4K      0
#define DB_TYPE_INODE   1
#define DB_TYPE_ERR     2

#define BITS_PER_LONG 32
/* file system limited parameter */
#define MAX_FILE_NAME_LEN	        32
#define DIR_FILE_ALLOC_BASE         256
//目前文件系统库中不支持创建目录的操作,也就是创建目录的接口是VME_FsbMkdir无用的。
//如果不支持的话，更好，这样的话，8K的superblock就可以放下super_block
#define MAX_DIR_NUM                 128
#define MAX_OPEN_FILE               16//8

/* define for magic */
#define AK_FS_MAGIC             0x1122      //super block magic
#define AK_INODE_MAGIC          0x3344      //inode magic
#define AK_DIRINFO_MAGIC        0x5566      //dir magic
#define AK_DATA_MAGIC           0x7788      //data block magic
#define SPIFLASH_ERASE_SIZE       4096
#define SPIFLASH_PAGE_SIZE        256 

/* definition for data block*/
#define DB_CONTROL_SIZE     8
#define CONTENT_LEN_4K      (BLOCK_SIZE_4K - DB_CONTROL_SIZE)


struct AK_Config_Info
{
	//本次使用的spi的容量，该变量用于格式化时AK_format时边界的确定
	T_U8   inode_sector_num;//inode需要的扇区个数,扇区以8K为单位。
	T_BOOL read_super_flag;
	T_U16  inode_bitmap_size;//保存inode的bitmap需要的bit数.inode_sector_num*128
	T_U32  setoffset;
	T_U32  spiflash_totalsize;
    T_U32  superblock_addr;//superblock地址，superblock_addr=FormatStartPage*256		    
    T_U32  back_up_8k_addr;	//备份的8k地址,back_up_8k_addr=superblock_addr+8K

	T_U32  inodetable_addr_start;//inode表的的起始地址,inodetable_addr_start=back_up_8k_addr=8K
	T_U32  inodetable_addr_end;//inode表的的结束地址.inodetable_addr_end=inodetable_addr_start+(inode_sector_num-1)*8K
	T_U32  mp_4k_bitmap_size;//保存数据块的bitmap需要的bit数.mp_4k_bitmap_size=(SPI_CAPACITY-inodetable_addr_end-8K)/4K*8
	T_U32  mempool_4k_start;//数据块区域的起始地址.mempool_4k_start=inodetable_addr_end+8K
	T_U32  mempool_4k_end;////数据块区域的结束地址.mempool_4k_end=mempool_4k_start+(mp_4k_bitmap_byte)*4K
	T_U32  mempool_4k_size;//数据块区域的总容量.mp_4k_bitmap_size*4K
	
};

struct vDATESTR {
	/*
	 the date in a DOS-compatible binary coded format:
	 - bits 0...4: days, valid are 1...31
	 - bits 5...8: months, valid are 1...12
	 - bits 9...15: years since 1980
	 */
	vUINT16 date;

	/*
	 the time in a DOS-compatible binary coded format:
	 - bits 0...4: seconds / 2, valid are 0...29 (i.e. the time can only be stored
	   with a resolution of 2 seconds)
	 - bits 5...10: minutes, valid are 0...59
	 - bits 11...15: hours, valid are 0...23
	 */
	vUINT16 hour;
};

struct vSTAT
{
	vUINT32   st_mode;		/** the mode of the file */
	vUINT32   st_size;        		/**< file size in bytes */
	vINT32    st_blksize;     		/**< optimal block size for I/O (cluster size) */
	struct vDATESTR st_atime;      	 /**< date and time of last access (equal to st_mtime) */
	struct vDATESTR st_mtime;       	/**< date and time of last modification */
	struct vDATESTR st_ctime;      	 /**< date and time of last file status change (equal to st_mtime) */
	vUINT16  fattribute;     /**< File attributes, see section "\ref fattr" */
} ;

/* Values for the st_mode field */
#define vS_IFMT   0x0170000        /**< type of file mask */
#define vS_IFDIR  0x0040000        /**< directory */
#define vS_IFREG  0x0100000        /**< regular */
#define vS_IWRITE 0x0000400        /**< Write permitted  */
#define vS_IREAD  0x0000200        /**< Read permitted. (Always true anyway)*/
#define vEMAXPATH 100

struct vDSTAT {
	vUINT16  fattribute;              /**< file attributes, see section "\ref fattr"*/
	vUINT16  ftime;                   /**< time of last modification, coded as in vDATESTR*/
	vUINT16  fdate;                   /**< date of last modification, coded as in vDATESTR*/
	vUINT32  fsize;                   /**< file size in bytes*/
	vINT16   driveno;                 /**< drive number*/
	vCHAR    path[vEMAXPATH];         /**< */ 
	vCHAR    longFileName[vEMAXPATH];  /**< the filename itself*/
	vUINT16  usInternal;               /**< */ 
    vVOID    *pvInternal;              /**< */ 
    vINT32    dir_index;
    vINT32    inode_index;
    vCHAR    search_name[vEMAXPATH];
    vUINT16  search;
} ;

#define vPO_RDONLY       0x0000 /**< Open for read only. */
#define vPO_WRONLY       0x0001 /**< Open for write only. */
#define vPO_RDWR         0x0002 /**< Read/write access allowed. */
#define vPO_APPEND       0x0008 /**< Filepointer will be set to end of file on opening the file. */
#define vPO_CREAT        0x0100 /**< Create the file if it does not exist. */
#define vPO_TRUNC        0x0200 /**< Truncate the file if it already exists. */
#define vPO_EXCL         0x0400 /**<  Attempt to create will fail if  the given file already exists.  Used in conjunction with VPO_CREAT*/
#define vPO_NOSHAREANY   0x0004 /*Attempts to open will fail if file is already open.                                */
#define vPO_NOSHAREWRITE 0x0800 /**< Attempts to open will fail if file is opened for writing. */

#define vPSEEK_SET       0      /**< offset from begining of file */
#define vPSEEK_CUR       1      /**< offset from current file pointer */
#define vPSEEK_END       2      /**< offset from end of file */

#if 0
#define VME_STATUS_FSFLASH 0
/*    no error, everything ok    */
#define VME_NOERROR_FS                 (VME_STATUS_FSFLASH + 0)  

/*   invalid drive parameter passed      */
#define VME_ERROR_FS_INVALID_DRIVE     (VME_STATUS_FSFLASH + 1)  

/*   path of one of the given file or directory is not existing or no longer valid           */
#define VME_ERROR_FS_INVALID_PATH      (VME_STATUS_FSFLASH + 2)  

/*   one of the given file or directory name is not existing or no longer valid.          */
#define VME_ERROR_FS_INVALID_NAME      (VME_STATUS_FSFLASH + 3)  

/*   generic internal error, which can occur at every function        */
#define VME_ERROR_FS_INTERNAL          (VME_STATUS_FSFLASH + 4)  

/*   path name longer than vEMAXPATH      */
#define VME_ERROR_FS_PATH_TOO_LONG     (VME_STATUS_FSFLASH + 5)  

/*   formatting of a drive failed       */
#define VME_ERROR_FS_FORMAT_FAILED     (VME_STATUS_FSFLASH + 6)  

/*   file or dir does not exist (obsolete, used for backwards compatibility)       */
#define VME_ERROR_FS_FILE_NOT_FOUND    (VME_STATUS_FSFLASH + 7)  

/*   file already exists      */
#define VME_ERROR_FS_FILE_EXISTS       (VME_STATUS_FSFLASH + 8)  

/*   sharing violation       */
#define VME_ERROR_FS_SHARING_ERROR     (VME_STATUS_FSFLASH + 9)  

/*   no more file descriptors available       */
#define VME_ERROR_FS_NO_FILEDESC       (VME_STATUS_FSFLASH + 10) 

/*   invalid parameters passed to API function    */
#define VME_ERROR_FS_INVALID_PARAM     (VME_STATUS_FSFLASH + 11) 

/*   invalid file descriptor passed          */
#define VME_ERROR_FS_INVALID_FILEDESC  (VME_STATUS_FSFLASH + 12) 

/*   filesystem full or no more heap available    */
#define VME_ERROR_FS_NO_SPACE          (VME_STATUS_FSFLASH + 13) 

/*   invalid access  */
#define VME_ERROR_FS_INVALID_ACCESS    (VME_STATUS_FSFLASH + 14) 

/*   no File System transaction IDs free (to many transactions at a time or no parallel transactions for the called function */
#define VME_ERROR_NO_FREE_FS_FST_ID   (VME_STATUS_FSFLASH + 15) 

/*   too many data for writing or reading  */
#define VME_ERROR_FS_TOO_MANY_DATA    (VME_STATUS_FSFLASH + 16)

/*   seek to negative file pointer position  */
#define VME_ERROR_FS_SEEK_TO_NEGATIVE (VME_STATUS_FSFLASH + 17)

#define VME_ERROR_FS_REPEAT_OPEN      (VME_STATUS_FSFLASH + 18)
#endif
struct AK_4k_data
{
    T_U16       file_id;			    /* id of the file that this data block belongs to*/
    T_U16	    block_num;				/* serial number of this data block in the file*/
	T_U8        buffer[ CONTENT_LEN_4K ]; /* data buffer */
	T_U32       next_block;
};


struct AK_4k_data_head
{
    T_U16       file_id;			    /* id of the file that this data block belongs to*/
    T_U16	    block_num;				/* serial number of this data block in the file*/
};

struct AK_4k_data_struct
{
    struct AK_4k_data_head head;
    T_U32       next_block;
};
struct DATABLK_INFO
{
	T_U16 start_blkidx;		//起始数据块的索引号
	T_U16 blk_cnt;		    //连续数据块的数目
};

#define  AK_4K_DATA_BIAS	  sizeof(struct AK_4k_data_struct)
#define  AK_4K_NEXTBLK_BIAS   sizeof(struct AK_4k_data_head)
#define  AK_DATEBLK_INFO_LEN  sizeof(struct DATABLK_INFO )
/* this struct is the ram and flash representation of the node */
struct AK_inode
{
    T_U16       magic;			                /* to comfirm the write operation */
    T_U16	    old;				            /* is the inode old if not FF means old */
    T_U32 	    first_db_addr;  	            /* the first data block address on the flash.  */
    T_U32       size;   		                /* The total size of the file's data.  */
    T_U16       dir_index;		                /* which directory this file belong to */
    T_U16       mode;   		                /* file_type, mode  */ 
    T_S8	    file_name[ MAX_FILE_NAME_LEN ];	/* file name */
    struct vDATESTR time;	                    /* Creation time.  */
	T_U8		pad[12];						/*为了字节对齐*/
};

#define INODE_SIZE  sizeof( struct AK_inode )
#define SPI_8M_CAP			(8*1024*1024)
#define SIZE_BIAS           8
#define DATA_OFFSET_BIAS    4
#define INODE_NUM_IN_SECTOR  (INODE_SECTOR_SIZE/sizeof( struct AK_inode ))

/* This data struct is use to describe the directory information */
struct AK_dirinfo
{
    T_U16   magic;                      /* comfirm write */
    T_U16	old;					    /* is this entry valid */
	T_S8 	dir_name[ MAX_FILE_NAME_LEN ];	/* the dir name */
	T_U16	mode;					    /* dir mode */
	T_U16	file_num;				    /* file number under this dir */
	T_U32   *inode;                     /* inode in this directory */
	T_U32   pad;
};
struct AK_SaveSuperInfo
{
	T_U16	magic;			/* magic of anyka file system */
    T_U16	dirty;  			/* is the super block dirty */
	T_U16   copy_inode_index;   /* in garbage collect  the most dirty inode secotr index*/
	/* whole file system information */
    T_U16	file_number;		/* total file number on the file system */
    /* free space of the file system*/
    T_U32	free_size_4k;		    /* 4k free space in the file system*/
	/* dir inforation */
    struct  AK_dirinfo  dirinfo[ MAX_DIR_NUM ];
};
/* AK_superblock is a struct for the overall file system control */
struct AK_superblock
{
    struct AK_SaveSuperInfo saveinfo;
    /* used bitmap */
    T_U8    *used_inode_bitmap;
	T_U8    *alloc_inode_bitmap;
    T_U8    *used_data_bitmap_4k;
	T_U8    *fs_buffer;
	struct AK_4k_data_struct *m_data4k;
	struct DATABLK_INFO *datablk_segTbl;
};
#define SB_DIRTY             0x1234
#define COPY_INODE_BIAS      4

#define RW_SUPERINFO_SIZE sizeof(struct AK_SaveSuperInfo)

/* this structure is the RAM representation of an open file  */
struct AK_file
{
	T_U32 inode_addr;   	    /* the address of inode entry */
	T_U32 db_addr;        	    /* the address of file data */

    struct vDATESTR time; 		        /* Creation time.  */
	T_U32 mode;   		        /* file_type, mode  */
	T_U32 size;   		        /* The total size of the file's data.  */
	T_U32 pos; 		            /* position of the file*/
	T_U16 dir_index;		    /* index of the directory */
	T_U16 inode_index;		    /* index of inode in the directory */
	T_U16 open;		            /* is the file struct used */
};

#define FILE_STRUCT_SIZE   sizeof( struct AK_file )
#define TRUNCATE_OR_NEW    0x00000000       //for file->data_addr

/* function declare */
T_U16 AK_alloc_dirinfo( T_VOID );
T_S16 AK_alloc_fd( T_VOID );
T_BOOL AK_create_dir( T_U16 dir_index, const T_S8 * dir_name );
T_BOOL AK_create_file( struct AK_file *fp, T_U16 dir_index, const T_S8 *file_name, T_U16 mode );
T_BOOL AK_delete_dir( T_U16 dir_index );
T_BOOL AK_delete_file( T_U16 dir_index, T_U16 inode_index );
T_BOOL AK_dir_empty( T_U16 dir_index );
T_BOOL AK_file_stat( struct vSTAT *pstat, T_U16 dir_index, T_U16 inode_index );
T_VOID AK_free_file( struct AK_file *fp );
T_BOOL AK_get_match( struct vDSTAT *statobj );
T_S32 AK_get_free_space( T_VOID );
T_BOOL AK_isfile_opened( T_U16 dir_index, T_U16 inode_index );
T_U16 AK_lookup_dirinfo( T_S8 *dir_name );
T_U16 AK_lookup_inode( T_S8 *file_name, T_U16 dir_index );
T_BOOL AK_movefile( T_U16 old_dir_index, T_U16 new_dir_index, T_U16 old_inode_index, T_S8 *file_name );
T_BOOL AK_open_file( struct AK_file *fp, T_U16 dir_index, T_U16 inode_index );
T_BOOL AK_parse_name( const T_S8 * path, T_S8 *dir_name, T_S8 *file_name );
T_S32 AK_read_4k_data( struct AK_file *file, T_U8 *buf, T_U32 count );

T_S32 AK_write_4k_data( struct AK_file *file, const T_U8 *buf, T_U32 count );


T_BOOL AK_truncate_file( struct AK_file *fp, T_U16 dir_index, T_U16 inode_index );

//internal function declare
T_U32 alloc_data_block( T_U32 type );

T_U16 alloc_dirinfo_inode_index( T_U16 dir_index );
T_U32 alloc_inode( T_U32 type );
struct vDATESTR get_fs_current_time( T_VOID );

T_BOOL delete_inode( T_U32 inode_addr, T_U32 first_db_addr );
T_BOOL free_data_block_list( T_U32 data_addr, T_U32 end_addr, T_U32 filelen);

T_VOID free_inode( T_U32 inode_addr );
T_U8 get_db_type( T_U32 data_addr );
T_U32 get_dirinfo_inode_address( T_U16 dir_index, T_U16 inode_index );
T_VOID set_dirinfo_inode_address( T_U16 dir_index, T_U16 inode_index, T_U32 addr );
T_BOOL write_inode( struct AK_inode *inode, T_U32 inode_addr );
T_BOOL write_4k_db( struct AK_4k_data *db, T_U32 data_addr, T_U32 pre_data_addr, T_U32 next_data_addr, T_U16 file_id, T_U16 block_num);

T_U8 get_final_used_index(T_U16 data[], T_U8 array_len);
T_BOOL write_64Kbackup_origial( T_U16 datatbl_finalidx, T_U16 *datablk_64k);
T_U16 get_group_file_blknum(T_S32 cur_segid, T_S32 total_segcnt, T_U16 curgroup_sidx, struct DATABLK_INFO *pFileSegData);
T_BOOL optimized_delete_datablk(T_S32 seg_cnt, struct DATABLK_INFO *pSegBlkTbl,T_U8 *guseddbm);
#endif

