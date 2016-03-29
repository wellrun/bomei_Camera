/**
 * @file Eng_Id3Inf.c
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */


#include "fwl_public.h"
#include "eng_string_uc.h"
#include "eng_string.h"
#include "Media_Demuxer_lib.h"
#include "Eng_Id3Inf.h"


/**
* @brief Read MetaInfo to SongInfo
*
* @author Songmengxing
* @date 2011-12-26
* @param out T_pSONG_INFO songInfo : info
* @param in T_MEDIALIB_META_INFO *pMetaInfo : MetaInfo
* @return T_VOID
* @retval
*/
static T_VOID ID3_ReadMetaInfo2SongInfo(T_pSONG_INFO songInfo, T_MEDIALIB_META_INFO *pMetaInfo)
{
	T_U32	len;
    T_U8 	meta[MEDIA_CLASSNAME_SIZE/2+1];
    T_U16 	unimeta[MEDIA_CLASSNAME_SIZE/2+1];
    T_U16	strtmp[6] = {0};
    
	AK_ASSERT_VAL_VOID(songInfo, "ID3_ReadMetaInfo2SongInfo(): songInfo");
	AK_ASSERT_VAL_VOID(pMetaInfo, "ID3_ReadMetaInfo2SongInfo(): pMetaInfo");

    Eng_StrMbcs2Ucs("Other", strtmp);
	

    /*get artist*/
    if (pMetaInfo->m_MetaContent.pArtist != AK_NULL)
    {  
        if (pMetaInfo->m_MetaTypeInfo.ArtistType == 0)
        {
			//unicode
        	len = (pMetaInfo->m_MetaSizeInfo.uArtistLen/2 > MEDIA_CLASSNAME_SIZE/2) ?
            	MEDIA_CLASSNAME_SIZE/2 : pMetaInfo->m_MetaSizeInfo.uArtistLen/2;
        
            Utl_UStrCpyN(songInfo->artist, pMetaInfo->m_MetaContent.pArtist, len);
        }
        else
        {
			//non-unicode
            T_U32 ret;

			len = (pMetaInfo->m_MetaSizeInfo.uArtistLen > MEDIA_CLASSNAME_SIZE/2) ?
            	MEDIA_CLASSNAME_SIZE/2 : pMetaInfo->m_MetaSizeInfo.uArtistLen;
			
            Utl_StrCpyN(meta, pMetaInfo->m_MetaContent.pArtist, len);
            ret = Eng_StrMbcs2Ucs(meta, unimeta);
            Utl_UStrCpyN(songInfo->artist, unimeta, ret);   
        }

        if(0 == (T_U16)Utl_UStrLen(songInfo->artist))
			Utl_UStrCpy(songInfo->artist, strtmp);
    }
    else
    {
        Utl_UStrCpy(songInfo->artist, strtmp);
    }
    
    /*get album*/
    if (pMetaInfo->m_MetaContent.pAlbum != AK_NULL)
    {            
        if (pMetaInfo->m_MetaTypeInfo.AlbumType == 0)
        {
			//unicode
        	len = (pMetaInfo->m_MetaSizeInfo.uAlbumLen/2 > MEDIA_CLASSNAME_SIZE/2) ?
            	MEDIA_CLASSNAME_SIZE/2 : pMetaInfo->m_MetaSizeInfo.uAlbumLen/2;
			
            Utl_UStrCpyN(songInfo->album, pMetaInfo->m_MetaContent.pAlbum, len);
        }
        else
        {
			//non-unicode
            T_U32 ret;
			
			len = (pMetaInfo->m_MetaSizeInfo.uAlbumLen > MEDIA_CLASSNAME_SIZE/2) ?
            	MEDIA_CLASSNAME_SIZE/2 : pMetaInfo->m_MetaSizeInfo.uAlbumLen;
			
            Utl_StrCpyN(meta, pMetaInfo->m_MetaContent.pAlbum, len);
            ret = Eng_StrMbcs2Ucs(meta, unimeta);
            Utl_UStrCpyN(songInfo->album, unimeta, ret);   
        }			

		if(0 == (T_U16)Utl_UStrLen(songInfo->album))
			Utl_UStrCpy(songInfo->album, strtmp);
    }
    else
    {
        Utl_UStrCpy(songInfo->album, strtmp);
    }
    
    
    /*get genre*/
    if (pMetaInfo->m_MetaContent.pGenre != AK_NULL)
    {            
        if (pMetaInfo->m_MetaTypeInfo.GenreType == 0)
        {	
			//unicode
			len = (pMetaInfo->m_MetaSizeInfo.uGenreLen/2 > MEDIA_CLASSNAME_SIZE/2) ?
            	MEDIA_CLASSNAME_SIZE/2 : pMetaInfo->m_MetaSizeInfo.uGenreLen/2;
			
            Utl_UStrCpyN(songInfo->genre, pMetaInfo->m_MetaContent.pGenre, len);
        }
        else
        {	
			//non-unicode
            T_U32 ret;
			
			len = (pMetaInfo->m_MetaSizeInfo.uGenreLen > MEDIA_CLASSNAME_SIZE/2) ?
            	MEDIA_CLASSNAME_SIZE/2 : pMetaInfo->m_MetaSizeInfo.uGenreLen;

            Utl_StrCpyN(meta, pMetaInfo->m_MetaContent.pGenre, len);
            ret = Eng_StrMbcs2Ucs(meta, unimeta);
            Utl_UStrCpyN(songInfo->genre, unimeta, ret);   
        }
		
		if(0 == (T_U16)Utl_UStrLen(songInfo->genre))
			Utl_UStrCpy(songInfo->genre, strtmp);
    }
    else
    {
        Utl_UStrCpy(songInfo->genre, strtmp);
    }

    /*get composer*/
    if (pMetaInfo->m_MetaContent.pComposer != AK_NULL)
    {
    	// AK_DEBUG_OUTPUT("pComposer++%s\n", pMetaInfo->m_MetaContent.pComposer);
        
        if (pMetaInfo->m_MetaTypeInfo.ComposerType == 0)
        {
			//unicode
        	len = (pMetaInfo->m_MetaSizeInfo.uComposerLen/2 > MEDIA_CLASSNAME_SIZE/2) ?
            	MEDIA_CLASSNAME_SIZE/2 : pMetaInfo->m_MetaSizeInfo.uComposerLen/2;
			
            Utl_UStrCpyN(songInfo->composer, pMetaInfo->m_MetaContent.pComposer, len);
        }
        else
        {
			//non-unicode
            T_U32 ret;

			len = (pMetaInfo->m_MetaSizeInfo.uComposerLen > MEDIA_CLASSNAME_SIZE/2) ?
            	MEDIA_CLASSNAME_SIZE/2 : pMetaInfo->m_MetaSizeInfo.uComposerLen;
			
            Utl_StrCpyN(meta, pMetaInfo->m_MetaContent.pComposer, len);
            ret = Eng_StrMbcs2Ucs(meta, unimeta);
            Utl_UStrCpyN(songInfo->composer, unimeta, ret);   
        }
		
		if(0 == (T_U16)Utl_UStrLen(songInfo->composer))
			Utl_UStrCpy(songInfo->composer, strtmp);
    }
    else
    {
        Utl_UStrCpy(songInfo->composer, strtmp);
    }        
	
}


/**
* @brief Get metalInfo by path
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pVOID fp : file handle
* @param out T_pSONG_INFO songInfo : info
* @return T_BOOL
* @retval
*/
T_BOOL ID3_GetMetaInfo(T_pVOID fp, T_pSONG_INFO songInfo)
{
	T_pVOID 				hMedia = AK_NULL;
    T_MEDIALIB_META_INFO 	metaInfo; 
	T_BOOL					ret = AK_FALSE;
	T_U16					strtmp[6] = {0};

	AK_ASSERT_PTR(fp, "ID3_GetMetaInfo(): fp", ret);
	AK_ASSERT_PTR(songInfo, "ID3_GetMetaInfo(): songInfo", ret);


	Eng_StrMbcs2Ucs("Other", strtmp);

    memset(&metaInfo, 0, sizeof(T_MEDIALIB_META_INFO));

	hMedia = Media_GetMetaInfo(&metaInfo, fp, T_SRC_TYPE_FP);	
	
	if((T_pVOID)-1 == hMedia)
	{
		return ret;
	}
	else if (AK_NULL == hMedia)
	{
		Utl_UStrCpy(songInfo->artist, strtmp);
		Utl_UStrCpy(songInfo->album, strtmp);
		Utl_UStrCpy(songInfo->genre, strtmp);
		Utl_UStrCpy(songInfo->composer, strtmp);

		ret = AK_TRUE;
	}
	else
	{
		ID3_ReadMetaInfo2SongInfo(songInfo, &metaInfo);
		
		Media_ReleaseMetaInfo(hMedia);
		ret = AK_TRUE;
  	}
    
    return ret;	
}


