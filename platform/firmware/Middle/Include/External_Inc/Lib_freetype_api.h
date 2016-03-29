/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: lib_freetype_api.h
* Function: 
*
* Author: songmengxing
* Date: 2011-09-28
* Version: 1.0
*
* Revision: 
* Author: 
* Date: 
***************************************************************************/
#ifdef SUPPORT_VFONT
#ifndef _LIB_FREETYPE_H
#define _LIB_FREETYPE_H

#ifdef __cplusplus
extern "C" {
#endif



#define FREETYPE_LIB_VERSION "FreetypeLib V2.4.4"


#define FT_LOAD_DEFAULT                      0x0
#define FT_LOAD_NO_SCALE                     0x1
#define FT_LOAD_NO_HINTING                   0x2
#define FT_LOAD_RENDER                       0x4
#define FT_LOAD_NO_BITMAP                    0x8
#define FT_LOAD_VERTICAL_LAYOUT              0x10
#define FT_LOAD_FORCE_AUTOHINT               0x20
#define FT_LOAD_CROP_BITMAP                  0x40
#define FT_LOAD_PEDANTIC                     0x80
#define FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH  0x200
#define FT_LOAD_NO_RECURSE                   0x400
#define FT_LOAD_IGNORE_TRANSFORM             0x800
#define FT_LOAD_MONOCHROME                   0x1000
#define FT_LOAD_LINEAR_DESIGN                0x2000
#define FT_LOAD_NO_AUTOHINT                  0x8000U


#ifndef FT_IMAGE_TAG
#define FT_IMAGE_TAG( value, _x1, _x2, _x3, _x4 )  \
          value = ( ( (unsigned long)_x1 << 24 ) | \
                    ( (unsigned long)_x2 << 16 ) | \
                    ( (unsigned long)_x3 << 8  ) | \
                      (unsigned long)_x4         )
#endif /* FT_IMAGE_TAG */


typedef int				FT_Error;
typedef signed long		FT_Long;
typedef unsigned long	FT_ULong;
typedef signed int		FT_Int;
typedef unsigned int	FT_UInt;
typedef signed long		FT_Int32;
typedef char			FT_String;
typedef unsigned char	FT_Byte;
typedef signed short	FT_Short;                                                                     
typedef unsigned short	FT_UShort;
typedef signed long		FT_Fixed;
typedef void*			FT_Pointer;
typedef signed long		FT_Pos;
typedef unsigned char	FT_Bool;




typedef struct FT_LibraryRec_  *FT_Library;
typedef struct FT_FaceRec_*  FT_Face;

typedef struct FT_MemoryRec_*  FT_Memory;
typedef struct FT_ListNodeRec_*  FT_ListNode;
typedef struct FT_Slot_InternalRec_*  FT_Slot_Internal;
typedef struct FT_GlyphSlotRec_*  FT_GlyphSlot;
typedef struct FT_CharMapRec_*  FT_CharMap;
typedef struct FT_DriverRec_*  FT_Driver;
typedef struct FT_StreamRec_*  FT_Stream;
typedef struct FT_Face_InternalRec_*  FT_Face_Internal;
typedef struct FT_SubGlyphRec_*  FT_SubGlyph;
typedef struct FT_SizeRec_*  FT_Size;


typedef struct  FT_Bitmap_
{
    int             rows;
    int             width;
    int             pitch;
    unsigned char*  buffer;
    short           num_grays;
    char            pixel_mode;
    char            palette_mode;
    void*           palette;
} FT_Bitmap;

typedef struct  FT_Vector_
{
    FT_Pos  x;
    FT_Pos  y;
} FT_Vector;

typedef void  (*FT_Generic_Finalizer)(void*  object);


typedef struct  FT_Generic_
{
    void*                 data;
    FT_Generic_Finalizer  finalizer;
} FT_Generic;

typedef struct  FT_Glyph_Metrics_
{
    FT_Pos  width;
    FT_Pos  height;

    FT_Pos  horiBearingX;
    FT_Pos  horiBearingY;
    FT_Pos  horiAdvance;

    FT_Pos  vertBearingX;
    FT_Pos  vertBearingY;
    FT_Pos  vertAdvance;
} FT_Glyph_Metrics;



typedef struct  FT_Outline_
{
    short       n_contours;      /* number of contours in glyph        */
    short       n_points;        /* number of points in the glyph      */

    FT_Vector*  points;          /* the outline's points               */
    char*       tags;            /* the points flags                   */
    short*      contours;        /* the contour end points             */

    int         flags;           /* outline masks                      */
} FT_Outline;

typedef enum  FT_Glyph_Format_
{
    FT_IMAGE_TAG( FT_GLYPH_FORMAT_NONE, 0, 0, 0, 0 ),

    FT_IMAGE_TAG( FT_GLYPH_FORMAT_COMPOSITE, 'c', 'o', 'm', 'p' ),
    FT_IMAGE_TAG( FT_GLYPH_FORMAT_BITMAP,    'b', 'i', 't', 's' ),
    FT_IMAGE_TAG( FT_GLYPH_FORMAT_OUTLINE,   'o', 'u', 't', 'l' ),
    FT_IMAGE_TAG( FT_GLYPH_FORMAT_PLOTTER,   'p', 'l', 'o', 't' )
} FT_Glyph_Format;

typedef struct  FT_GlyphSlotRec_
{
    FT_Library        library;
    FT_Face           face;
    FT_GlyphSlot      next;
    FT_UInt           reserved;       /* retained for binary compatibility */
    FT_Generic        generic;

    FT_Glyph_Metrics  metrics;
    FT_Fixed          linearHoriAdvance;
    FT_Fixed          linearVertAdvance;
    FT_Vector         advance;

    FT_Glyph_Format   format;

    FT_Bitmap         bitmap;
    FT_Int            bitmap_left;
    FT_Int            bitmap_top;

    FT_Outline        outline;

    FT_UInt           num_subglyphs;
    FT_SubGlyph       subglyphs;

    void*             control_data;
    long              control_len;

    FT_Pos            lsb_delta;
    FT_Pos            rsb_delta;

    void*             other;

    FT_Slot_Internal  internal;
} FT_GlyphSlotRec;


typedef struct  FT_Bitmap_Size_
{
    FT_Short  height;
    FT_Short  width;

    FT_Pos    size;

    FT_Pos    x_ppem;
    FT_Pos    y_ppem;
} FT_Bitmap_Size;

typedef struct  FT_BBox_
{
    FT_Pos  xMin, yMin;
    FT_Pos  xMax, yMax;
} FT_BBox;


typedef struct  FT_ListNodeRec_
{
    FT_ListNode  prev;
    FT_ListNode  next;
    void*        data;
} FT_ListNodeRec;




typedef struct  FT_ListRec_
{
    FT_ListNode  head;
    FT_ListNode  tail;
} FT_ListRec;


typedef struct  FT_FaceRec_
{
    FT_Long           num_faces;
    FT_Long           face_index;

    FT_Long           face_flags;
    FT_Long           style_flags;

    FT_Long           num_glyphs;

    FT_String*        family_name;
    FT_String*        style_name;

    FT_Int            num_fixed_sizes;
    FT_Bitmap_Size*   available_sizes;

    FT_Int            num_charmaps;
    FT_CharMap*       charmaps;

    FT_Generic        generic;

    /*# The following member variables (down to `underline_thickness') */
    /*# are only relevant to scalable outlines; cf. @FT_Bitmap_Size    */
    /*# for bitmap fonts.                                              */
    FT_BBox           bbox;

    FT_UShort         units_per_EM;
    FT_Short          ascender;
    FT_Short          descender;
    FT_Short          height;

    FT_Short          max_advance_width;
    FT_Short          max_advance_height;

    FT_Short          underline_position;
    FT_Short          underline_thickness;

    FT_GlyphSlot      glyph;
    FT_Size           size;
    FT_CharMap        charmap;

    /*@private begin */

    FT_Driver         driver;
    FT_Memory         memory;
    FT_Stream         stream;

    FT_ListRec        sizes_list;

    FT_Generic        autohint;
    void*             extensions;

    FT_Face_Internal  internal;

    /*@private end */

} FT_FaceRec;
  typedef enum  FT_Render_Mode_
  {
    FT_RENDER_MODE_NORMAL = 0,
    FT_RENDER_MODE_LIGHT,
    FT_RENDER_MODE_MONO,
    FT_RENDER_MODE_LCD,
    FT_RENDER_MODE_LCD_V,

    FT_RENDER_MODE_MAX

  } FT_Render_Mode;

  typedef struct FT_GlyphRec_*  FT_Glyph;

  typedef struct  FT_Matrix_
  {
    FT_Fixed  xx, xy;
    FT_Fixed  yx, yy;

  } FT_Matrix;

  /* create a new glyph object */
  typedef FT_Error
  (*FT_Glyph_InitFunc)( FT_Glyph      glyph,
                        FT_GlyphSlot  slot );

  /* destroys a given glyph object */
  typedef void
  (*FT_Glyph_DoneFunc)( FT_Glyph  glyph );

  typedef void
  (*FT_Glyph_TransformFunc)( FT_Glyph          glyph,
                             const FT_Matrix*  matrix,
                             const FT_Vector*  delta );

  typedef void
  (*FT_Glyph_GetBBoxFunc)( FT_Glyph  glyph,
                           FT_BBox*  abbox );

  typedef FT_Error
  (*FT_Glyph_CopyFunc)( FT_Glyph   source,
                        FT_Glyph   target );

  typedef FT_Error
  (*FT_Glyph_PrepareFunc)( FT_Glyph      glyph,
                           FT_GlyphSlot  slot );

  struct  FT_Glyph_Class_
  {
    FT_Long                 glyph_size;
    FT_Glyph_Format         glyph_format;
    FT_Glyph_InitFunc       glyph_init;
    FT_Glyph_DoneFunc       glyph_done;
    FT_Glyph_CopyFunc       glyph_copy;
    FT_Glyph_TransformFunc  glyph_transform;
    FT_Glyph_GetBBoxFunc    glyph_bbox;
    FT_Glyph_PrepareFunc    glyph_prepare;
  };

  typedef struct FT_Glyph_Class_  FT_Glyph_Class;

  typedef struct  FT_GlyphRec_
  {
    FT_Library             library;
    const FT_Glyph_Class*  clazz;
    FT_Glyph_Format        format;
    FT_Vector              advance;

  } FT_GlyphRec;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_BitmapGlyph                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an object used to model a bitmap glyph image.  This is */
  /*    a sub-class of @FT_Glyph, and a pointer to @FT_BitmapGlyphRec.     */
  /*                                                                       */
  typedef struct FT_BitmapGlyphRec_*  FT_BitmapGlyph;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_BitmapGlyphRec                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used for bitmap glyph images.  This really is a        */
  /*    `sub-class' of @FT_GlyphRec.                                       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    root   :: The root @FT_Glyph fields.                               */
  /*                                                                       */
  /*    left   :: The left-side bearing, i.e., the horizontal distance     */
  /*              from the current pen position to the left border of the  */
  /*              glyph bitmap.                                            */
  /*                                                                       */
  /*    top    :: The top-side bearing, i.e., the vertical distance from   */
  /*              the current pen position to the top border of the glyph  */
  /*              bitmap.  This distance is positive for upwards~y!        */
  /*                                                                       */
  /*    bitmap :: A descriptor for the bitmap.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    You can typecast an @FT_Glyph to @FT_BitmapGlyph if you have       */
  /*    `glyph->format == FT_GLYPH_FORMAT_BITMAP'.  This lets you access   */
  /*    the bitmap's contents easily.                                      */
  /*                                                                       */
  /*    The corresponding pixel buffer is always owned by @FT_BitmapGlyph  */
  /*    and is thus created and destroyed with it.                         */
  /*                                                                       */
  typedef struct  FT_BitmapGlyphRec_
  {
    FT_GlyphRec  root;
    FT_Int       left;
    FT_Int       top;
    FT_Bitmap    bitmap;

  } FT_BitmapGlyphRec;



/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT_Init_FreeType                                                   */
/*                                                                       */
/* <Description>                                                         */
/*    Initialize a new FreeType library object.  The set of modules      */
/*    that are registered by this function is determined at build time.  */
/*                                                                       */
/* <Output>                                                              */
/*    alibrary :: A handle to a new library object.                      */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0~means success.                             */
/*                                                                       */
/* <Note>                                                                */
/*    In case you want to provide your own memory allocating routines,   */
/*    use @FT_New_Library instead, followed by a call to                 */
/*    @FT_Add_Default_Modules (or a series of calls to @FT_Add_Module).  */
/*                                                                       */
FT_Error FT_Init_FreeType(FT_Library *alibrary);



/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT_Done_FreeType                                                   */
/*                                                                       */
/* <Description>                                                         */
/*    Destroy a given FreeType library object and all of its children,   */
/*    including resources, drivers, faces, sizes, etc.                   */
/*                                                                       */
/* <Input>                                                               */
/*    library :: A handle to the target library object.                  */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0~means success.                             */
/*                                                                       */
FT_Error FT_Done_FreeType(FT_Library library);



/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT_New_Face                                                        */
/*                                                                       */
/* <Description>                                                         */
/*    This function calls @FT_Open_Face to open a font by its pathname.  */
/*                                                                       */
/* <InOut>                                                               */
/*    library    :: A handle to the library resource.                    */
/*                                                                       */
/* <Input>                                                               */
/*    pathname   :: A path to the font file.                             */
/*                                                                       */
/*    face_index :: The index of the face within the font.  The first    */
/*                  face has index~0.                                    */
/*                                                                       */
/* <Output>                                                              */
/*    aface      :: A handle to a new face object.  If `face_index' is   */
/*                  greater than or equal to zero, it must be non-NULL.  */
/*                  See @FT_Open_Face for more details.                  */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0~means success.                             */
/*                                                                       */
FT_Error FT_New_Face(FT_Library library, const char* filepathname, FT_Long face_index, FT_Face *aface );



/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT_Done_Face                                                       */
/*                                                                       */
/* <Description>                                                         */
/*    Discard a given face object, as well as all of its child slots and */
/*    sizes.                                                             */
/*                                                                       */
/* <Input>                                                               */
/*    face :: A handle to a target face object.                          */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0~means success.                             */
/*                                                                       */
/* <Note>                                                                */
/*    See the discussion of reference counters in the description of     */
/*    @FT_Reference_Face.                                                */
/*                                                                       */
FT_Error FT_Done_Face(FT_Face face);



/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT_Set_Pixel_Sizes                                                 */
/*                                                                       */
/* <Description>                                                         */
/*    This function calls @FT_Request_Size to request the nominal size   */
/*    (in pixels).                                                       */
/*                                                                       */
/* <InOut>                                                               */
/*    face         :: A handle to the target face object.                */
/*                                                                       */
/* <Input>                                                               */
/*    pixel_width  :: The nominal width, in pixels.                      */
/*                                                                       */
/*    pixel_height :: The nominal height, in pixels.                     */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0~means success.                             */
/*                                                                       */
FT_Error FT_Set_Pixel_Sizes(FT_Face face, FT_UInt pixel_width, FT_UInt pixel_height);




/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT_Load_Char                                                       */
/*                                                                       */
/* <Description>                                                         */
/*    A function used to load a single glyph into the glyph slot of a    */
/*    face object, according to its character code.                      */
/*                                                                       */
/* <InOut>                                                               */
/*    face        :: A handle to a target face object where the glyph    */
/*                   is loaded.                                          */
/*                                                                       */
/* <Input>                                                               */
/*    char_code   :: The glyph's character code, according to the        */
/*                   current charmap used in the face.                   */
/*                                                                       */
/*    load_flags  :: A flag indicating what to load for this glyph.  The */
/*                   @FT_LOAD_XXX constants can be used to control the   */
/*                   glyph loading process (e.g., whether the outline    */
/*                   should be scaled, whether to load bitmaps or not,   */
/*                   whether to hint the outline, etc).                  */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0~means success.                             */
/*                                                                       */
/* <Note>                                                                */
/*    This function simply calls @FT_Get_Char_Index and @FT_Load_Glyph.  */
/*                                                                       */
FT_Error FT_Load_Char(FT_Face face, FT_ULong char_code, FT_Int32 load_flags);



/*************************************************************************/
/*																		 */
/* <Function>															 */
/*	  FT_Glyph_To_Bitmap												 */
/*																		 */
/* <Description>														 */
/*	  Convert a given glyph object to a bitmap glyph object.			 */
/*																		 */
/* <InOut>																 */
/*	  the_glyph   :: A pointer to a handle to the target glyph. 		 */
/*																		 */
/* <Input>																 */
/*	  render_mode :: An enumeration that describes how the data is		 */
/*					 rendered.											 */
/*																		 */
/*	  origin	  :: A pointer to a vector used to translate the glyph	 */
/*					 image before rendering.  Can be~0 (if no			 */
/*					 translation).	The origin is expressed in			 */
/*					 26.6 pixels.										 */
/*																		 */
/*	  destroy	  :: A boolean that indicates that the original glyph	 */
/*					 image should be destroyed by this function.  It is  */
/*					 never destroyed in case of error.					 */
/*																		 */
/* <Return> 															 */
/*	  FreeType error code.	0~means success.							 */
/*																		 */
/* <Note>																 */
/*	  This function does nothing if the glyph format isn't scalable.	 */
/*																		 */
/*	  The glyph image is translated with the `origin' vector before 	 */
/*	  rendering.														 */
/*																		 */
/*	  The first parameter is a pointer to an @FT_Glyph handle, that will */
/*	  be _replaced_ by this function (with newly allocated data).		 */
/*	  Typically, you would use (omitting error handling):				 */
/*																		 */
/*																		 */
/*		{																 */
/*		  FT_Glyph		  glyph;										 */
/*		  FT_BitmapGlyph  glyph_bitmap; 								 */
/*																		 */
/*																		 */
/*		  // load glyph 												 */
/*		  error = FT_Load_Char( face, glyph_index, FT_LOAD_DEFAUT );	 */
/*																		 */
/*		  // extract glyph image										 */
/*		  error = FT_Get_Glyph( face->glyph, &glyph );					 */
/*																		 */
/*		  // convert to a bitmap (default render mode + destroying old)  */
/*		  if ( glyph->format != FT_GLYPH_FORMAT_BITMAP )				 */
/*		  { 															 */
/*			error = FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL,	 */
/*										0, 1 ); 						 */
/*			if ( error ) // `glyph' unchanged							 */
/*			  ...														 */
/*		  } 															 */
/*																		 */
/*		  // access bitmap content by typecasting						 */
/*		  glyph_bitmap = (FT_BitmapGlyph)glyph; 						 */
/*																		 */
/*		  // do funny stuff with it, like blitting/drawing				 */
/*		  ...															 */
/*																		 */
/*		  // discard glyph image (bitmap or not)						 */
/*		  FT_Done_Glyph( glyph );										 */
/*		}																 */
/*																		 */
/*																		 */
/*	  Here another example, again without error handling:				 */
/*																		 */
/*																		 */
/*		{																 */
/*		  FT_Glyph	glyphs[MAX_GLYPHS]									 */
/*																		 */
/*																		 */
/*		  ...															 */
/*																		 */
/*		  for ( idx = 0; i < MAX_GLYPHS; i++ )							 */
/*			error = FT_Load_Glyph( face, idx, FT_LOAD_DEFAULT ) ||		 */
/*					FT_Get_Glyph ( face->glyph, &glyph[idx] );			 */
/*																		 */
/*		  ...															 */
/*																		 */
/*		  for ( idx = 0; i < MAX_GLYPHS; i++ )							 */
/*		  { 															 */
/*			FT_Glyph  bitmap = glyphs[idx]; 							 */
/*																		 */
/*																		 */
/*			... 														 */
/*																		 */
/*			// after this call, `bitmap' no longer points into			 */
/*			// the `glyphs' array (and the old value isn't destroyed)	 */
/*			FT_Glyph_To_Bitmap( &bitmap, FT_RENDER_MODE_MONO, 0, 0 );	 */
/*																		 */
/*			... 														 */
/*																		 */
/*			FT_Done_Glyph( bitmap );									 */
/*		  } 															 */
/*																		 */
/*		  ...															 */
/*																		 */
/*		  for ( idx = 0; i < MAX_GLYPHS; i++ )							 */
/*			FT_Done_Glyph( glyphs[idx] );								 */
/*		}																 */
/*																		 */
FT_Error   FT_Glyph_To_Bitmap( FT_Glyph* the_glyph,
                      FT_Render_Mode  render_mode,
                      FT_Vector*      origin,
                      FT_Bool         destroy );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT_Get_Glyph                                                       */
/*                                                                       */
/* <Description>                                                         */
/*    A function used to extract a glyph image from a slot.  Note that   */
/*    the created @FT_Glyph object must be released with @FT_Done_Glyph. */
/*                                                                       */
/* <Input>                                                               */
/*    slot   :: A handle to the source glyph slot.                       */
/*                                                                       */
/* <Output>                                                              */
/*    aglyph :: A handle to the glyph object.                            */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0~means success.                             */
/*                                                                       */
FT_Error FT_Get_Glyph(FT_GlyphSlot slot, FT_Glyph* aglyph);


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT_Glyph_Copy                                                      */
/*                                                                       */
/* <Description>                                                         */
/*    A function used to copy a glyph image.  Note that the created      */
/*    @FT_Glyph object must be released with @FT_Done_Glyph.             */
/*                                                                       */
/* <Input>                                                               */
/*    source :: A handle to the source glyph object.                     */
/*                                                                       */
/* <Output>                                                              */
/*    target :: A handle to the target glyph object.  0~in case of       */
/*              error.                                                   */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0~means success.                             */
/*                                                                       */
FT_Error FT_Glyph_Copy(FT_Glyph source, FT_Glyph* target);


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT_Done_Glyph                                                      */
/*                                                                       */
/* <Description>                                                         */
/*    Destroy a given glyph.                                             */
/*                                                                       */
/* <Input>                                                               */
/*    glyph :: A handle to the target glyph object.                      */
/*                                                                       */
void FT_Done_Glyph(FT_Glyph aglyph);



/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT_Glyph_Transform                                                 */
/*                                                                       */
/* <Description>                                                         */
/*    Transform a glyph image if its format is scalable.                 */
/*                                                                       */
/* <InOut>                                                               */
/*    glyph  :: A handle to the target glyph object.                     */
/*                                                                       */
/* <Input>                                                               */
/*    matrix :: A pointer to a 2x2 matrix to apply.                      */
/*                                                                       */
/*    delta  :: A pointer to a 2d vector to apply.  Coordinates are      */
/*              expressed in 1/64th of a pixel.                          */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code (if not 0, the glyph format is not scalable).  */
/*                                                                       */
/* <Note>                                                                */
/*    The 2x2 transformation matrix is also applied to the glyph's       */
/*    advance vector.                                                    */
/*                                                                       */
FT_Error FT_Glyph_Transform(FT_Glyph glyph, FT_Matrix* matrix, FT_Vector* delta);




#ifdef __cplusplus
}
#endif

#endif
#endif
