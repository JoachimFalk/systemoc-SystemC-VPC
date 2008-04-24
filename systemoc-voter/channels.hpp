//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_CHANNELS_HPP
#define _INCLUDED_CHANNELS_HPP

#ifndef XILINX_EDK_RUNTIME
# include <stdint.h>
# include <iostream>
#else
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef signed   char  int8_t;
typedef signed   short int16_t;
typedef signed   int   int32_t;
#endif

#include <vector>
#include <cassert>

#include "debug_config.h"

#ifndef KASCPAR_PARSING
typedef bool      bit_t;
typedef uint8_t   uint2_t;
typedef uint8_t   uint4_t;
typedef uint16_t  uint11_t;
typedef uint16_t  uint10_t;
typedef uint32_t  uint19_t;
typedef uint32_t  uint24_t;
typedef uint32_t  uint29_t;

typedef int16_t   int12_t;
typedef int32_t   int19_t;
#else
# define bit_t    bool
# define uint2_t  unsigned char
# define uint4_t  unsigned char
# define uint11_t unsigned short
# define uint10_t unsigned short
# define uint19_t unsigned int
# define uint24_t unsigned int
# define uint29_t unsigned int
# define int12_t  short
# define int19_t  int
#endif // KASCPAR_PARSING

#define JPEG_BLOCK_WIDTH  8U
#define JPEG_BLOCK_HEIGHT 8U
#define JPEG_BLOCK_SIZE   ((JPEG_BLOCK_HEIGHT) * (JPEG_BLOCK_WIDTH))
#define JPEG_MAX_COLOR_COMPONENTS 3

/// JPEG channel communication type
#define JPEGCHANNEL_BITS 29
#ifdef DBG_ENABLE
class JpegChannel_t {
public:
  typedef JpegChannel_t this_type;
protected:
  uint29_t val;
public:
  JpegChannel_t(uint29_t _val): val(_val) {}
  JpegChannel_t()                         {}

  this_type &operator = (uint29_t _val)
    { val = _val; return *this; }
  this_type &operator |= (uint29_t _val)
    { val |= _val; return *this; }

  operator uint29_t() const
    { return val; }
};
#else
# ifndef KASCPAR_PARSING
typedef uint29_t JpegChannel_t;
# else
#   define JpegChannel_t unsigned int
# endif // KASCPAR_PARSING
#endif

#define PIXEL_BITS 24
#ifndef KASCPAR_PARSING
typedef uint24_t Pixel_t;
#else
# define Pixel_t unsigned int
#endif // KASCPAR_PARSING

#define CODEWORD_BITS 8
#ifndef KASCPAR_PARSING
typedef uint8_t codeword_t;
#else
# define codeword_t unsigned char
#endif // KASCPAR_PARSING

// FIXME: Fix SysteMoC to also accept format below
//#define UDEMASK(x,off,width) (((x) >> (off)) & ((1 << (width)) - 1))

// Demask to unsigned value
#define UDEMASK(x,off,width)   (((x) / (1U << (off))) & ((1U << (width)) - 1))
// Demask to signed value
#define SDEMASK(x,off,width)   ((x) & (1U << ((off) + (width) - 1))	\
                                ? ((x) / (1U << (off))) |  ((~0U) << (width)) \
                                : ((x) / (1U << (off))) & ~((~0U) << (width)))

#define SET_MASK(x,off,width) (((x) & ((1U << (width)) -1 )) << (off))

/// Source -> Parser
#ifndef KASCPAR_PARSING
typedef codeword_t ct_src_parser_t;
#else
# define ct_src_parser_t unsigned char
#endif // KASCPAR_PARSING

/// Entry encoding the x dimension of the frame
#define FRAME_DIM_X_BITS 10
#ifndef KASCPAR_PARSING
typedef uint10_t FrameDimX_t;
#else
# define FrameDimX_t unsigned short
#endif // KASCPAR_PARSING

/// Entry encoding the y dimension of the frame
#define FRAME_DIM_Y_BITS 10
#ifndef KASCPAR_PARSING
typedef uint10_t FrameDimY_t;
#else
# define FrameDimY_t unsigned short
#endif // KASCPAR_PARSING

/// Entry coding QT table destination selection
#define QT_TBL_ID_BITS 4
#ifndef KASCPAR_PARSING
typedef uint4_t QtTblID_t;
#else
# define QtTblID_t unsigned char
#endif // KASCPAR_PARSING

/// Entry coding Huffman table destination selection (Page. 38)
#define HUFF_TBL_ID_BITS 4
#ifndef KASCPAR_PARSING
typedef uint4_t HuffTblID_t;
#else
# define HuffTblID_t unsigned char
#endif // KASCPAR_PARSING

enum HuffTblType_t {
  HUFFTBL_DC = 0,
  HUFFTBL_AC = 1
};

/// Coded Huffman Table Item
#ifndef KASCPAR_PARSING
typedef codeword_t CodedHuffTblItem_t;
#else
# define CodedHuffTblItem_t unsigned char
#endif // KASCPAR_PARSING

/// Expanded Huffman Tables
#ifndef KASCPAR_PARSING
typedef uint8_t DecodedSymbol_t;
typedef uint16_t HuffmanCode_t;
#else
# define DecodedSymbol_t unsigned char
# define HuffmanCode_t   unsigned short
#endif // KASCPAR_PARSING

/// Internal Component Identifier
/// (Component counter, not component label specified in the standard)
#define INTCOMPID_BITS 2
#ifndef KASCPAR_PARSING
typedef uint2_t IntCompID_t;
#else
# define IntCompID_t unsigned char
#endif // KASCPAR_PARSING

#define SCANPATTERN_LENGTH 6

#define RUNLENGTH_BITS 4
#ifndef KASCPAR_PARSING
typedef uint4_t RunLength_t;
#else
# define RunLength_t unsigned char
#endif // KASCPAR_PARSING

#define CATEGORY_BITS 4
#ifndef KASCPAR_PARSING
typedef uint4_t Category_t;
#else
# define Category_t unsigned char
#endif // KASCPAR_PARSING

/// The IDCT Category Amplitude
#define CATEGORYAMPLITUDE_BITS 11
#ifndef KASCPAR_PARSING
typedef uint11_t CategoryAmplitude_t;
#else
# define CategoryAmplitude_t unsigned short
#endif // KASCPAR_PARSING

/// The quantized IDCT coefficients
#define QUANTIDCTCOEFF_BITS 12
#ifndef KASCPAR_PARSING
typedef int12_t QuantIDCTCoeff_t;
#else
# define QuantIDCTCoeff_t short
#endif // KASCPAR_PARSING

/// The IDCT coefficients (maximum of 8 bit extension by dequantization)
/// But only 11 Bits needed for AC coefficients (DC coefficients need one
/// more bit because of difference coding, which is removed before
/// dequantization)
#define IDCTCOEFF_BITS (11+8)
#ifndef KASCPAR_PARSING
typedef int19_t IDCTCoeff_t;
#else
# define IDCTCoeff_t int
#endif // KASCPAR_PARSING

/// The value for one component of a pixel.
/// In case of grayscale images this is the pixel.
#define COMPONENTVAL_BITS 8
#ifndef KASCPAR_PARSING
typedef uint8_t ComponentVal_t;
#else
# define ComponentVal_t unsigned char
#endif // KASCPAR_PARSING

#define RESTART_INTERVAL_BITS (16+3)
#ifndef KASCPAR_PARSING
typedef uint19_t RestartInterval_t;
#else
# define RestartInterval_t unsigned int
#endif // KASCPAR_PARSING

#define CTRLCMD_BITS 4
enum CtrlCmd_t {
  /// Signals for each component, which AC and DC Huffman Table to use
  CTRLCMD_USEHUFF,
  /// Signals that a given AC or DC Huffman Table won't be used any more
  CTRLCMD_DISCARDHUFF,
  /// Signals a new scan and transmit internal component pattern 
  /// describing component interleaving, e.g,
  /// (I)   for a scan containing only internal component n transmit
  ///       n,n,n,n,n,n
  /// (II)  for a scan containing a interleaving of component n,m transmit
  ///       n,m,n,m,n,m
  /// (III) for a scan containing a interleaving of components l,n,m transmit
  ///       l,n,m,l,n,m
  CTRLCMD_NEWSCAN,
  /// Resets Huffman Coder and DC-Coefficient Difference
  /// Note: We do not transmit the "modulo 8 count m value" (see Standard, p. 32)
  CTRLCMD_SCANRESTART,
  /// Signals for each component, which QT table to use
  CTRLCMD_USEQT,
  /// Signals, that a given QT table won't be used any more
  CTRLCMD_DISCARDQT,
  /// Signals, that the following data belongs to a new component
  CTRLCMD_INTERNALCOMPSTART,
  /// Signals the restart periodicity
  /// ATTENTION: In contrast to standard, the value is transmitted 
  /// in number of blocks instead of MCUs
  CTRLCMD_DEF_RESTART_INTERVAL,
  /// Signals the beginning of a new frame
  CTRLCMD_NEWFRAME,
  /// Signals end of image
  CTRLCMD_ENDOFIMAGE
};

// struct JpegScan {
  // bool ctrl : is data word or control
#define JS_ISCTRL(x) UDEMASK(x,0,1)
#define JS_SET_CTRL(is_ctrl)  SET_MASK(is_ctrl,0,1)

  /* *************************************************************** */
  /*             in case of ctrl (ctrl == true)                      */
  /* *************************************************************** */  
# define JS_GETCTRLCMD(x) \
  UDEMASK(x,1,CTRLCMD_BITS)
# define JS_SETCTRLCMD(cmd) \
  (JS_SET_CTRL(1) | \
   SET_MASK(cmd,1,CTRLCMD_BITS) \
  )

    /* *************************************************************** */
    /*                 in case of CTRLCMD_USEHUFF                      */
    /* *************************************************************** */
    // Get considered colour component
#   define JS_CTRL_USEHUFF_GETCOMP(x)  \
    UDEMASK(x,1+CTRLCMD_BITS,INTCOMPID_BITS)
#   define JS_CTRL_USEHUFF_SETCOMP(x)  \
    (SET_MASK(x,1+CTRLCMD_BITS,INTCOMPID_BITS))
    // Get DC Table ID
#   define JS_CTRL_USEHUFF_GETDCTBL(x)  \
    UDEMASK(x,1+CTRLCMD_BITS+INTCOMPID_BITS,HUFF_TBL_ID_BITS)
#   define JS_CTRL_USEHUFF_SETDCTBL(tbl_id) \
    SET_MASK(tbl_id,1+CTRLCMD_BITS+INTCOMPID_BITS,HUFF_TBL_ID_BITS)
    // Get AC Table ID
#   define JS_CTRL_USEHUFF_GETACTBL(x)  \
    UDEMASK(x,1+CTRLCMD_BITS+INTCOMPID_BITS+HUFF_TBL_ID_BITS,HUFF_TBL_ID_BITS)
#   define JS_CTRL_USEHUFF_SETACTBL(tbl_id) \
    SET_MASK(tbl_id,1+CTRLCMD_BITS+INTCOMPID_BITS+HUFF_TBL_ID_BITS,HUFF_TBL_ID_BITS)
#   if JPEGCHANNEL_BITS < (1+CTRLCMD_BITS+INTCOMPID_BITS+HUFF_TBL_ID_BITS+HUFF_TBL_ID_BITS)
#     error "Too many bits"
#   endif

    // Set complete channel word
    // Sets the Channel word for the USEHUFF CMD
    // Ci_DC: DC-Table ID for colour component Ci
    // Ci_AC: AC-Table ID for colour component Ci
#   define JS_CTRL_USEHUFF_SET_CHWORD(COMP_ID,DC_ID,AC_ID) \
  (JS_SETCTRLCMD(CTRLCMD_USEHUFF) |  \
   JS_CTRL_USEHUFF_SETCOMP(COMP_ID) | \
   JS_CTRL_USEHUFF_SETDCTBL(DC_ID) | \
   JS_CTRL_USEHUFF_SETACTBL(AC_ID) \
   )

    /* *************************************************************** */
    /*                in case of CTRLCMD_DISCARDHUFF                   */
    /* *************************************************************** */    
#   define JS_CTRL_DISCARDHUFFTBL_GETHUFFID(x) \
    UDEMASK(x,1+CTRLCMD_BITS,HUFF_TBL_ID_BITS)
#   define JS_CTRL_DISCARDHUFFTBL_SETHUFFID(id) \
    (SET_MASK(id,1+CTRLCMD_BITS,HUFF_TBL_ID_BITS))
#   define JS_CTRL_DISCARDHUFFTBL_GETTYPE(x) \
    UDEMASK(x,1+CTRLCMD_BITS+HUFF_TBL_ID_BITS,1)
#   define JS_CTRL_DISCARDHUFFTBL_SETTYPE(type) \
    (SET_MASK(type,1+CTRLCMD_BITS+HUFF_TBL_ID_BITS,1))
#   if JPEGCHANNEL_BITS < (1+CTRLCMD_BITS+HUFF_TBL_ID_BITS+1)
#     error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_DISCARDHUFFTBL_SET_CHWORD(id,type) \
    (JS_SETCTRLCMD(CTRLCMD_DISCARDHUFF) |               \
     JS_CTRL_DISCARDHUFFTBL_SETHUFFID(id) | \
     JS_CTRL_DISCARDHUFFTBL_SETTYPE(type) \
    )


    /* *************************************************************** */
    /*               in case of CTRLCMD_NEWSCAN                        */
    /* *************************************************************** */
    // (0 <= n < SCANPATTERN_LENGTH)    
#   define JS_CTRL_NEWSCAN_GETCOMP(x,n) \
    UDEMASK(x,1+CTRLCMD_BITS+(n)*INTCOMPID_BITS,INTCOMPID_BITS)
#   define JS_CTRL_NEWSCAN_SETCOMP(comp,n) \
    (SET_MASK(comp,1+CTRLCMD_BITS+(n)*INTCOMPID_BITS,INTCOMPID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+(5)*INTCOMPID_BITS+INTCOMPID_BITS
#     error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_NEWSCAN_SET_CHWORD(C0,C1,C2,C3,C4,C5)  \
    (JS_SETCTRLCMD(CTRLCMD_NEWSCAN) |                       \
     JS_CTRL_NEWSCAN_SETCOMP(C0,0) | \
     JS_CTRL_NEWSCAN_SETCOMP(C1,1) | \
     JS_CTRL_NEWSCAN_SETCOMP(C2,2) | \
     JS_CTRL_NEWSCAN_SETCOMP(C3,3) | \
     JS_CTRL_NEWSCAN_SETCOMP(C4,4) | \
     JS_CTRL_NEWSCAN_SETCOMP(C5,5) \
    )


    /* *************************************************************** */
    /*              in case of CTRLCMD_SCANRESTART                     */
    /* *************************************************************** */
    //  Does not have any parameters!
#   define JS_CTRL_SCANRESTART_SET_CHWORD  \
    JS_SETCTRLCMD(CTRLCMD_SCANRESTART)

    /* *************************************************************** */
    /*                   in case of CTRLCMD_USEQT                      */
    /* *************************************************************** */    
#   define JS_CTRL_USEQT_GETQTID(x) \
    UDEMASK(x,1+CTRLCMD_BITS,QT_TBL_ID_BITS)
#   define JS_CTRL_USEQT_SETQTID(qt_id) \
    (SET_MASK(qt_id,1+CTRLCMD_BITS,QT_TBL_ID_BITS))
#   define JS_CTRL_USEQT_GETCOMPID(x) \
    UDEMASK(x,1+CTRLCMD_BITS+QT_TBL_ID_BITS,INTCOMPID_BITS)
#   define JS_CTRL_USEQT_SETCOMPID(x) \
    (SET_MASK(x,1+CTRLCMD_BITS+QT_TBL_ID_BITS,INTCOMPID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+QT_TBL_ID_BITS+INTCOMPID_BITS
#     error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_USEQT_SET_CHWORD(QT_ID,COMP_ID)  \
    (JS_SETCTRLCMD(CTRLCMD_USEQT) |                   \
     JS_CTRL_USEQT_SETQTID(QT_ID) | \
     JS_CTRL_USEQT_SETCOMPID(COMP_ID) \
     )
    

    /* *************************************************************** */
    /*                  in case of CTRLCMD_DISCARDQT                   */
    /* *************************************************************** */    
#   define JS_CTRL_DISCARDQT_GETQTID(x) \
    UDEMASK(x,1+CTRLCMD_BITS,QT_TBL_ID_BITS)
#   define JS_CTRL_DISCARDQT_SETQTID(qt_id) \
    (SET_MASK(qt_id,1+CTRLCMD_BITS,QT_TBL_ID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+QT_TBL_ID_BITS
#     error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_DISCARDQT_SET_CHWORD(QT_ID)  \
    ( JS_SETCTRLCMD(CTRLCMD_DISCARDQT) | \
      JS_CTRL_DISCARDQT_SETQTID(QT_ID) \
    )

    /* *************************************************************** */
    /*            in case of CTRLCMD_INTERNALCOMPSTART                 */
    /* *************************************************************** */    
    
#   define JS_CTRL_INTERNALCOMPSTART_GETCOMPID(x) \
    UDEMASK(x,1+CTRLCMD_BITS,INTCOMPID_BITS)
#   define JS_CTRL_INTERNALCOMPSTART_SETCOMPID(comp) \
    (SET_MASK(comp,1+CTRLCMD_BITS,INTCOMPID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+INTCOMPID_BITS
#     error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_INTERNALCOMPSTART_SET_CHWORD(comp)  \
    (JS_SETCTRLCMD(CTRLCMD_INTERNALCOMPSTART) | \
     JS_CTRL_INTERNALCOMPSTART_SETCOMPID(comp) \
     )

    /* *************************************************************** */
    /*            in case of CTRLCMD_DEF_RESTART_INTERVAL              */
    /* *************************************************************** */    
#   define JS_CTRL_RESTART_GET_INTERVAL(x) \
    UDEMASK(x,1+CTRLCMD_BITS,RESTART_INTERVAL_BITS)
#   define JS_CTRL_RESTART_SET_INTERVAL(interval) \
      (SET_MASK(interval,1+CTRLCMD_BITS, RESTART_INTERVAL_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+ RESTART_INTERVAL_BITS
#     error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_DEF_RESTART_INTERVAL_SET_CHWORD(interval) \
    (JS_SETCTRLCMD(CTRLCMD_DEF_RESTART_INTERVAL) | \
     JS_CTRL_RESTART_SET_INTERVAL(interval) \
    )

    /* *************************************************************** */
    /*            in case of CTRLCMD_NEWFRAME                          */
    /* *************************************************************** */    
#   define JS_CTRL_NEWFRAME_GET_DIMX(x) \
    UDEMASK(x,1+CTRLCMD_BITS,FRAME_DIM_X_BITS)
#   define JS_CTRL_NEWFRAME_SET_DIMX(dimX) \
      (SET_MASK(dimX,1+CTRLCMD_BITS,FRAME_DIM_X_BITS))
#   define JS_CTRL_NEWFRAME_GET_DIMY(x) \
    UDEMASK(x,1+CTRLCMD_BITS+FRAME_DIM_X_BITS,FRAME_DIM_Y_BITS)
#   define JS_CTRL_NEWFRAME_SET_DIMY(dimY) \
      (SET_MASK(dimY,1+CTRLCMD_BITS+FRAME_DIM_X_BITS,FRAME_DIM_Y_BITS))
#   define JS_CTRL_NEWFRAME_GET_COMPCOUNT(x) \
    UDEMASK(x,1+CTRLCMD_BITS+FRAME_DIM_X_BITS+FRAME_DIM_Y_BITS,INTCOMPID_BITS)
#   define JS_CTRL_NEWFRAME_SET_COMPCOUNT(count) \
      (SET_MASK(count,1+CTRLCMD_BITS+FRAME_DIM_X_BITS+FRAME_DIM_Y_BITS,INTCOMPID_BITS))
#   if JPEGCHANNEL_BITS < 1+CTRLCMD_BITS+FRAME_DIM_X_BITS+FRAME_DIM_Y_BITS+INTCOMPID_BITS
#     error "Too many bits"
#   endif

    // Set complete channel word
#   define JS_CTRL_NEWFRAME_SET_CHWORD(dimX,dimY,count) \
    (JS_SETCTRLCMD(CTRLCMD_NEWFRAME) | \
     JS_CTRL_NEWFRAME_SET_DIMX(dimX) | \
     JS_CTRL_NEWFRAME_SET_DIMY(dimY) | \
     JS_CTRL_NEWFRAME_SET_COMPCOUNT(count) \
    )

    /* *************************************************************** */
    /*                in case of data (ctrl == false)                  */
    /* *************************************************************** */
    //   codeword_t data : the raw something
# define JS_DATA_GET(x) \
    UDEMASK(x,1,CODEWORD_BITS)
# define JS_DATA_SET(x) \
    (JS_SET_CTRL(0) | \
     SET_MASK(x,1,CODEWORD_BITS) \
    )
# if JPEGCHANNEL_BITS < 1+CODEWORD_BITS
#   error "Too many bits"
# endif


    /* *************************************************************** */
    /*            in case of data (ctrl == false)                      */
    /*              and tuppled data transmission                      */
    /* *************************************************************** */
# define JS_TUP_GETIDCTAMPLCOEFF(x) \
    UDEMASK(x,1,CATEGORYAMPLITUDE_BITS)
# define JS_TUP_SETIDCTAMPLCOEFF(x) \
    (SET_MASK(x,1,CATEGORYAMPLITUDE_BITS))
# define JS_TUP_GETRUNLENGTH(x) \
    UDEMASK(x,1+CATEGORYAMPLITUDE_BITS,RUNLENGTH_BITS)
# define JS_TUP_SETRUNLENGTH(x) \
    (SET_MASK(x,1+CATEGORYAMPLITUDE_BITS,RUNLENGTH_BITS))
# define JS_TUP_GETCATEGORY(x) \
    UDEMASK(x,1+CATEGORYAMPLITUDE_BITS+RUNLENGTH_BITS,CATEGORY_BITS)
# define JS_TUP_SETCATEGORY(x) \
    (SET_MASK(x,1+CATEGORYAMPLITUDE_BITS+RUNLENGTH_BITS,CATEGORY_BITS))
# if JPEGCHANNEL_BITS < 1+CATEGORYAMPLITUDE_BITS+RUNLENGTH_BITS+CATEGORY_BITS
#   error "Too many bits"
# endif

    // Set complete channel word
#   define JS_DATA_TUPPLED_SET_CHWORD(coeff,rle,cat)    \
    (JS_SET_CTRL(0) | \
     JS_TUP_SETIDCTAMPLCOEFF(coeff) | \
     JS_TUP_SETRUNLENGTH(rle) | \
     JS_TUP_SETCATEGORY(cat) \
    )

    /* *************************************************************** */
    /*                 in case of data (ctrl == false)                 */
    /*      and quantized IDCT coeff transmission                      */
    /* *************************************************************** */
# define JS_QCOEFF_GETIDCTCOEFF(x) \
    SDEMASK(x,1,QUANTIDCTCOEFF_BITS)
# define JS_QCOEFF_SETIDCTCOEFF(x) \
    (SET_MASK(x,1,QUANTIDCTCOEFF_BITS))
# if JPEGCHANNEL_BITS < 1+QUANTIDCTCOEFF_BITS
#   error "Too many bits"
# endif

    // Set complete channel word
#   define JS_DATA_QCOEFF_SET_CHWORD(coeff)    \
    (JS_SET_CTRL(0) | \
    JS_QCOEFF_SETIDCTCOEFF(coeff) \
     )

    /* *************************************************************** */
    /* in case of data (ctrl == false)                                 */
    /*   and de-quantized IDCT coeff transmission                      */
    /* *************************************************************** */
# define JS_COEFF_GETIDCTCOEFF(x) \
    SDEMASK(x,1,IDCTCOEFF_BITS)
# define JS_COEFF_SETIDCTCOEFF(x) \
    (SET_MASK(x,1,IDCTCOEFF_BITS))
# if JPEGCHANNEL_BITS < 1+IDCTCOEFF_BITS
#   error "Too many bits"
# endif

   // Write complete channel word    
#   define JS_DATA_COEFF_SET_CHWORD(coeff)    \
    (JS_SET_CTRL(0) | \
    JS_COEFF_SETIDCTCOEFF(coeff) \
     )

    /* *************************************************************** */
    /* in case of data (ctrl == false)                                 */
    /*   and component value transmission                              */
    /* *************************************************************** */
# define JS_COMPONENT_GETVAL(x) \
    UDEMASK(x,1,COMPONENTVAL_BITS)
# define JS_COMPONENT_SETVAL(x) \
    (SET_MASK(x,1,COMPONENTVAL_BITS))
# if JPEGCHANNEL_BITS < 1+COMPONENTVAL_BITS
#   error "Too many bits"
# endif





/* *************************************************************** */
/*                Macros for accessing raw pixels                  */
/* *************************************************************** */
//c: component, 0 <= c <= 2
#define JS_RAWPIXEL_GETCOMP(x,c)			\
  UDEMASK(x,(c)*COMPONENTVAL_BITS,COMPONENTVAL_BITS)
#if JPEGCHANNEL_BITS < 3*COMPONENTVAL_BITS
# error "Too many bits"
#endif

//Set single component of a raw pixel
#define JS_RAWPIXEL_SETCOMP(x,c)			\
  (SET_MASK(x,(c)*COMPONENTVAL_BITS,COMPONENTVAL_BITS))

//Set all components of a raw pixel
#define JS_RAWPIXEL_SETVAL(c0,c1,c2) \
  (JS_RAWPIXEL_SETCOMP(c0,0) |	     \
   JS_RAWPIXEL_SETCOMP(c1,1) |	     \
   JS_RAWPIXEL_SETCOMP(c2,2))

/* *************************************************************** */
/*     Data types for QT table                                     */
/* *************************************************************** */
#ifndef KASCPAR_PARSING
typedef uint8_t qt_table_t;
#else
# define qt_table_t unsigned char
#endif // KASCPAR_PARSING

/// Size of a SINGLE QT Table
/// consisting of one quantization coefficient for each code block pixel
/// and a header. (See standard, page 40)
#define JS_QT_TABLE_SIZE (JPEG_BLOCK_SIZE+1)

 

//};
//

/// Parser -> InvByteStuffing
#ifndef KASCPAR_PARSING
typedef codeword_t ct_src_parser_t;
#else
# define ct_src_parser_t unsigned char
#endif // KASCPAR_PARSING

/*
struct ImageParam {
  uint16_t width;     ///< Width of Image
  uint16_t height;    ///< Height if Image
  uint2_t  compCount; ///< Component count in Image
};

std::ostream &operator << (std::ostream &out, const ImageParam &val)
  { out << "[ImageParam width: " << val.width << ", height: " << val.height << ", component count: " << val.compCount;
return out; }
*/

#ifndef XILINX_EDK_RUNTIME
inline
std::ostream &operator << (std::ostream &out, const codeword_t val) {
  out << (unsigned int) val;
  return out;
}
#endif


// we can't use ExpHuffTbl as channel type now (more than 32bit), so use this
#ifndef KASCPAR_PARSING
typedef uint16_t HuffTableChannel_t;
#else
# define HuffTableChannel_t unsigned short
#endif

/// Struct for Huffman Decoder
// what does 'Exp' mean? -Expanded :-)
struct ExpHuffTbl {
  uint8_t          valPtr[16];   // value pointers to first symbol of codelength
                                 //  'index'
  uint16_t         minCode[16];  // minimal code value for codewords of length
                                 //  'index'
  uint16_t         maxCode[16];  // max ...
  DecodedSymbol_t  huffVal[256]; // symbol-length assignment parameters (B.2.4.2)

  bool operator==(const ExpHuffTbl &rhs) const {
    for(int i = 0; i < 16; ++i) {
      if(valPtr[i] != rhs.valPtr[i])
        return false;
    }
    for(int i = 0; i < 16; ++i) {
      if(minCode[i] != rhs.minCode[i])
        return false;
    }
    for(int i = 0; i < 16; ++i) {
      if(maxCode[i] != rhs.maxCode[i])
        return false;
    }
    for(int i = 0; i < 256; ++i) {
      if(huffVal[i] != rhs.huffVal[i])
        return false;
    }
    return true;
  }
};

#ifndef XILINX_EDK_RUNTIME
inline
std::ostream &operator<<(std::ostream &out, const ExpHuffTbl &eht) {
#ifndef DBG_ENABLE
  out << "ExpHuffTbl()";
#else
  out << std::hex << "valPtr:";
  for(int i = 0; i<16; ++i){
    out << " | " << eht.valPtr[i];
  }
 
  out << " |\nminCode:";
  for(int i = 0; i<16; ++i){
    out << " | " << eht.minCode[i];
  }
 
  out << " |\nmaxCode:";
  for(int i = 0; i<16; ++i){
    out << " | " << eht.maxCode[i];
  }

  out << " |\nhuffVal:";
  for(int i = 0; i<256; ++i){
    out << " | " << eht.huffVal[i];
  }
  out << " |";
  out << std::dec;
#endif
  return out;
}

#if defined(SYSTEMC_VERSION) && (SYSTEMC_VERSION <= 20020405)
inline
void sc_trace(sc_trace_file *, const ExpHuffTbl, sc_string) {}
#endif
#endif // XILINX_EDK_RUNTIME

/// Struct from FrameBufferWriter
struct Pos {
  FrameDimX_t x;
  FrameDimY_t y;
};

/// Typedef from FrameBufferWriter
#ifndef KASCPAR_PARSING
  typedef std::vector<ComponentVal_t> FrameBuffer;
#else
# define FrameBuffer std::vector<ComponentVal_t>
#endif // KASCPAR_PARSING


/// Defines for YCrCb2RGB
#define CALC_INT_FIXPOINT(value,frac_digits) \
  ((int)((value) * (1 << (frac_digits))))

#define SHIFT_AND_ROUND(value,shift) { \
	value >>= shift - 1; \
	value += 1; \
	value >>= 1; \
}

#ifdef DBG_ENABLE
inline
std::ostream &operator << (std::ostream &out, const JpegChannel_t &x) {
  out << "[JpegChannel_t:";
  if (!JS_ISCTRL(x)) {
    out << "raw(" << JS_DATA_GET(x) << "),"
        << "tup(" << JS_TUP_GETIDCTAMPLCOEFF(x) << ","
                  << JS_TUP_GETRUNLENGTH(x) << ","
                  << JS_TUP_GETCATEGORY(x) << "),"
        << "qco(" << JS_QCOEFF_GETIDCTCOEFF(x) << "),"
        << "ico(" << JS_COEFF_GETIDCTCOEFF(x) << "),"
        << "com(" << JS_COMPONENT_GETVAL(x) << ")";
  } else {
    switch (static_cast<CtrlCmd_t>(JS_GETCTRLCMD(x))) {
      case CTRLCMD_USEHUFF:
        out << "CTRLCMD_USEHUFF("
            << JS_CTRL_USEHUFF_GETCOMP(x) << ","
            << JS_CTRL_USEHUFF_GETDCTBL(x) << ","
            << JS_CTRL_USEHUFF_GETACTBL(x) << ")";
        break;
      case CTRLCMD_DISCARDHUFF:
        out << "CTRLCMD_DISCARDHUFF("
            << JS_CTRL_DISCARDHUFFTBL_GETHUFFID(x) << ","
            << JS_CTRL_DISCARDHUFFTBL_GETTYPE(x) << ")";
        break;
      case CTRLCMD_NEWSCAN:
        out << "CTRLCMD_NEWSCAN("
            << JS_CTRL_NEWSCAN_GETCOMP(x,0)
            << JS_CTRL_NEWSCAN_GETCOMP(x,1)
            << JS_CTRL_NEWSCAN_GETCOMP(x,2)
            << JS_CTRL_NEWSCAN_GETCOMP(x,3)
            << JS_CTRL_NEWSCAN_GETCOMP(x,4)
            << JS_CTRL_NEWSCAN_GETCOMP(x,5) << ")";
        break;
      case CTRLCMD_SCANRESTART:
        out << "CTRLCMD_SCANRESTART";
        break;
      case CTRLCMD_USEQT:
        out << "CTRLCMD_USEQT("
            << JS_CTRL_USEQT_GETQTID(x) << ","
            << JS_CTRL_USEQT_GETCOMPID(x) << ")";
        break;
      case CTRLCMD_DISCARDQT:
        out << "CTRLCMD_DISCARDQT("
            << JS_CTRL_DISCARDQT_GETQTID(x) << ")";
        break;
      case CTRLCMD_INTERNALCOMPSTART:
        out << "CTRLCMD_INTERNALCOMPSTART("
            << JS_CTRL_INTERNALCOMPSTART_GETCOMPID(x) << ")";
        break;
      case CTRLCMD_DEF_RESTART_INTERVAL:
        out << "CTRLCMD_DEF_RESTART_INTERVAL("
            << JS_CTRL_RESTART_GET_INTERVAL(x) << ")";
        break;
      case CTRLCMD_NEWFRAME:
        out << "CTRLCMD_NEWFRAME("
            << JS_CTRL_NEWFRAME_GET_DIMX(x) << ","
            << JS_CTRL_NEWFRAME_GET_DIMY(x) << ","
            << JS_CTRL_NEWFRAME_GET_COMPCOUNT(x) << ")";
        break;
      case CTRLCMD_ENDOFIMAGE:
        out << "CTRLCMD_ENDOFIMAGE";
        break;
      default:
        //assert(!"Unhandle control command in JpegChannel_t");
        out << "Unknown control command. Fix operator<<()"
    }
  }
  out << "]";
  return out;
}
#endif // DBG_ENABLE


/*
 * includes from HuffDecoder.hpp -START-
 */

// FIXME: Please move me! This belongs to HuffDecoder.hpp! But synthesis
//   doesn't include it. So I must declare it here.

enum HuffTableType {
  AC0,
  AC1,
  DC0,
  DC1
};

/*
  NOTE: we pack two 8 bit values (ValPtr and HuffVal) into one 16 bit word to
  save space.
*/

// size of expanded huffman table in 16 bit words
#define HUFF_TABLE_SIZE16 (16/2 + 16 + 16 + 256/2)
// offset of values when using 16 bit word array
#define HUFF_VALPTR_OFFSET16  (0)
#define HUFF_MINCODE_OFFSET16 (8)
#define HUFF_MAXCODE_OFFSET16 (8 + 16)
#define HUFF_HUFFVAL_OFFSET16 (8 + 16 + 16)

// by using offsets as values, index calculation is more easy
enum HuffTableChannelType {
  ValPtr  = HUFF_VALPTR_OFFSET16,
  MinCode = HUFF_MINCODE_OFFSET16,
  MaxCode = HUFF_MAXCODE_OFFSET16,
  HuffVal = HUFF_HUFFVAL_OFFSET16
};

/*
 * includes from HuffDecoder.hpp -END-
 */


#endif // _INCLUDED_CHANNELS_HPP