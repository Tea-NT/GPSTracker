/*
 * ===================================================================
 *  TS 26.104
 *  REL-5 V5.4.0 2004-03
 *  REL-6 V6.1.0 2004-03
 *  REL-15 V15.1.0 2018-07
 *  3GPP AMR Floating-point Speech Codec
 * ===================================================================
 *
 */
/* This is valid for PC */

//#ifndef _GM_AMR_TYPEDEF_H
#define _GM_AMR_TYPEDEF_H

#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef __int8  int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
#else
#include <stdint.h>
#endif

typedef signed char Word8; 
typedef unsigned char UWord8; 
typedef signed short int Word16;
typedef signed int Word32;
typedef float Float32;
typedef double Float64;

//#define IF2

//#endif
