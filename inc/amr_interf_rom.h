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

/*
 * interf_rom.h
 *
 *
 * Project:
 *    AMR Floating-Point Codec
 *
 * Contains:
 *    Tables:           Subjective importance
 *                      Homing frames
 *
 *
 */

#ifndef _interf_rom_h_
#define _interf_rom_h_

/*
 * include files
 */
#include"amr_typedef.h"

/*
 * definition of constants
 */

/* number of parameters */
#define PRMNO_MR475 17
#define PRMNO_MR515 19
#define PRMNO_MR59  19
#define PRMNO_MR67  19
#define PRMNO_MR74  19
#define PRMNO_MR795 23
#define PRMNO_MR102 39
#define PRMNO_MR122 57
#define PRMNO_MRDTX 5

/*
 * tables
 */
static const UWord8 block_size[16]={ 13, 14, 16, 18, 20, 21, 27, 32,
                                    6 , 0 , 0 , 0 , 0 , 0 , 0 , 1  };

static const UWord8 toc_byte[16]={0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x34, 0x3C,
								  0x44, 0x4C, 0x54, 0x5C, 0x64, 0x6C, 0x74, 0x7C};

static Word16 order_MR515[] =
{
   0, 0x1,
   0, 0x2,
   0, 0x4,
   0, 0x8,
   0, 0x10,
   0, 0x20,
   0, 0x40,
   0, 0x80,
   1, 0x1,
   1, 0x2,
   1, 0x4,
   1, 0x8,
   1, 0x10,
   1, 0x20,
   1, 0x40,
   1, 0x80,
   3, 0x80,
   3, 0x40,
   3, 0x20,
   3, 0x10,
   3, 0x8,
   7, 0x8,
   11, 0x8,
   15, 0x8,
   6, 0x1,
   6, 0x2,
   6, 0x4,
   10, 0x1,
   10, 0x2,
   10, 0x4,
   14, 0x1,
   14, 0x2,
   14, 0x4,
   18, 0x1,
   18, 0x2,
   18, 0x4,
   6, 0x8,
   10, 0x8,
   14, 0x8,
   18, 0x8,
   3, 0x4,
   7, 0x4,
   11, 0x4,
   15, 0x4,
   2, 0x10,
   6, 0x10,
   10, 0x10,
   14, 0x10,
   18, 0x10,
   3, 0x2,
   7, 0x2,
   11, 0x2,
   2, 0x20,
   2, 0x4,
   2, 0x1,
   6, 0x20,
   10, 0x20,
   14, 0x20,
   18, 0x20,
   2, 0x2,
   3, 0x1,
   7, 0x1,
   11, 0x1,
   15, 0x2,
   2, 0x8,
   2, 0x40,
   15, 0x1,
   5, 0x1,
   5, 0x2,
   9, 0x1,
   9, 0x2,
   13, 0x1,
   4, 0x4,
   8, 0x4,
   12, 0x4,
   16, 0x4,
   13, 0x2,
   17, 0x1,
   17, 0x2,
   4, 0x2,
   8, 0x2,
   12, 0x2,
   16, 0x2,
   4, 0x20,
   8, 0x20,
   4, 0x10,
   8, 0x10,
   12, 0x20,
   12, 0x10,
   16, 0x20,
   16, 0x10,
   4, 0x40,
   8, 0x40,
   12, 0x40,
   16, 0x40,
   4, 0x1,
   8, 0x1,
   12, 0x1,
   16, 0x1,
   4, 0x8,
   8, 0x8,
   12, 0x8,
   16, 0x8
};
static const Word16 dhf_MR515[PRMNO_MR515] =
{
   0x00F8,
   0x009D,
   0x001C,
   0x0066,
   0x0000,
   0x0003,
   0x0037,
   0x000F,
   0x0000,
   0x0003,
   0x0005,
   0x000F,
   0x0037,
   0x0003,
   0x0037,
   0x000F,
   0x0023,
   0x0003,
   0x001F
};

static const Word16 bitno_MR515[PRMNO_MR515] =
{
   8, 8, 7,    /* LSP VQ          */
   8, 7, 2, 6, /* first subframe  */
   4, 7, 2, 6, /* second subframe */
   4, 7, 2, 6, /* third subframe  */
   4, 7, 2, 6  /* fourth subframe */
};

#endif /*_interf_rom_h_*/


