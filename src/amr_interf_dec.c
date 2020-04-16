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
 * interf_dec.c
 *
 *
 * Project:
 *     AMR Floating-Point Codec
 *
 * Contains:
 *    This module provides means to conversion from 3GPP or ETSI
 *    bitstream to AMR parameters
 */

/*
 * include files
 */
#include <stdlib.h>
#include <stdio.h>
//#include <memory.h>
#include "amr_typedef.h"
#include "amr_sp_dec.h"
#include "amr_interf_rom.h"
#include "amr_rom_dec.h"
#include "gm_stdlib.h"
#include "gm_memory.h"

/*
 * definition of constants
 */
#define EHF_MASK 0x0008 /* encoder homing frame pattern */
typedef

struct
{
   int reset_flag_old;   /* previous was homing frame */
   enum RXFrameType prev_ft;   /* previous frame type */
   enum Mode prev_mode;   /* previous mode */
   void *decoder_State;   /* Points decoder state */
}dec_interface_State;



/*
 * DecoderMMS
 *
 *
 * Parameters:
 *    param             O: AMR parameters
 *    stream            I: input bitstream
 *    frame_type        O: frame type
 *    speech_mode       O: speech mode in DTX
 *
 * Function:
 *    AMR file storage format frame to decoder parameters
 *
 * Returns:
 *    mode              used mode
 */
enum Mode DecoderMMS( Word16 *param, UWord8 *stream, enum RXFrameType
                      *frame_type, enum Mode *speech_mode, Word16 *q_bit )
{
	enum Mode mode;
	Word32 j;
	Word16 *mask;

	GM_memset( param, 0, sizeof(Word16)*PRMNO_MR122);
	*q_bit = 0x01 & (*stream >> 2);
	mode = (enum Mode)(0x0F & (*stream >> 3));
	stream++;
	if (mode == MR515)
	{
		mask = order_MR515;
		for (j = 1; j < 104; j++)
		{
			if (*stream & 0x80)
			{
				param[*mask] = (short)(param[*mask] + *(mask + 1));
			}
			mask += 2;

			if (j % 8)
			{
				*stream <<= 1;
			}
			else
			{
				stream++;
			}
		}
		*frame_type = RX_SPEECH_GOOD;
	}
	else
	{
		*frame_type = RX_SPEECH_BAD;
	}
      
   return mode;
}

/*
 * Decoder_Interface_reset
 *
 *
 * Parameters:
 *    st                O: state struct
 *
 * Function:
 *    Reset homing frame counter
 *
 * Returns:
 *    void
 */
void Decoder_Interface_reset( dec_interface_State *st )
{
   st->reset_flag_old = 1;
   st->prev_ft = RX_SPEECH_GOOD;
   st->prev_mode = MR475;   /* minimum bitrate */
}


/*
 * Decoder_Interface_init
 *
 *
 * Parameters:
 *    void
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    success           : pointer to structure
 *    failure           : NULL
 */
void * Decoder_Interface_init( void )
{
   dec_interface_State * s;

   if ((s = (dec_interface_State*)GM_MemoryAlloc(sizeof(dec_interface_State))) == NULL)
   {
      return NULL;
   }
   s->decoder_State = Speech_Decode_Frame_init();

   if ( s->decoder_State == NULL )
   {
      GM_MemoryFree(s);
      return NULL;
   }
   Decoder_Interface_reset(s);
   return s;
}


/*
 * Decoder_Interface_exit
 *
 *
 * Parameters:
 *    state                I: state structure
 *
 * Function:
 *    The memory used for state memory is freed
 *
 * Returns:
 *    Void
 */
void Decoder_Interface_exit( void *state )
{
   dec_interface_State *s;
   
   s = (dec_interface_State*)state;

   /* free memory */
   Speech_Decode_Frame_exit(s->decoder_State);
   GM_MemoryFree(s);
   s = NULL;
   state = NULL;
}


/*
 * Decoder_Interface_Decode
 *
 *
 * Parameters:
 *    st                B: state structure
 *    bits              I: bit stream
 *    synth             O: synthesized speech
 *    bfi               I: bad frame indicator
 *
 * Function:
 *    Decode bit stream to synthesized speech
 *
 * Returns:
 *    Void
 */
void Decoder_Interface_Decode(void *st, UWord8 *bits, Word16 *synth, int bfi)
{
	enum Mode mode;   /* AMR mode */
	enum Mode speech_mode = MR475;   /* speech mode */
	Word16 prm[PRMNO_MR122];   /* AMR parameters */
	enum RXFrameType frame_type;   /* frame type */
	dec_interface_State * s;   /* pointer to structure */
	const Word16 *homing;   /* pointer to homing frame */
	Word16 homingSize;   /* frame size for homing frame */
	Word32 i;   /* counter */
	Word32 resetFlag = 1;   /* homing frame */
	Word16 q_bit;

	s = (dec_interface_State *)st;

	/*
	* extract mode information and frametype,
	* octets to parameters
	*/
	mode = DecoderMMS(prm, bits, &frame_type, &speech_mode, &q_bit);
	if (!bfi)
	{
		bfi = 1 - q_bit;
	}

	if (bfi == 1) 
	{
		if ( mode <= MR122) 
		{
			frame_type = RX_SPEECH_BAD;
		}
		else if (frame_type != RX_NO_DATA) 
		{
			frame_type = RX_SID_BAD;
			mode = s->prev_mode;
		}
	} 
	else 
	{
		if ( frame_type == RX_SID_FIRST || frame_type == RX_SID_UPDATE)
		{
			mode = speech_mode;
		}
		else if (frame_type == RX_NO_DATA)
		{
			mode = s->prev_mode;
		}
		/*
		* if no mode information
		* guess one from the previous frame
		*/
		if (frame_type == RX_SPEECH_BAD) 
		{
			mode = s->prev_mode;
			if (s->prev_ft >= RX_SID_FIRST) 
			{
				frame_type = RX_SID_BAD;
			}
		}
	}

	/* test for homing frame */
	if ( s->reset_flag_old == 1 )
	{
		switch ( mode )
		{
			case MR515:
				homing = dhf_MR515;
				homingSize = 7;
				break;
			default:
				homing = NULL;
				homingSize = 0;
				break;
		}

		for ( i = 0; i < homingSize; i++ ) 
		{
			resetFlag = prm[i] ^ homing[i];
			if (resetFlag)
			{
				break;
			}
		}
	}

	if ((resetFlag == 0) && (s->reset_flag_old != 0))
	{
		for (i = 0; i < 160; i++) 
		{
			synth[i] = EHF_MASK;
		}
	}
	else
	{
		Speech_Decode_Frame( s->decoder_State, mode, prm, frame_type, synth );
	}

	if ( s->reset_flag_old == 0 ) 
	{
		/* check whole frame */
		switch ( mode ) 
		{
			case MR515:
				homing = dhf_MR515;
				homingSize = PRMNO_MR515;
				break;
			default:
				homing = NULL;
				homingSize = 0;
				break;
		}

		for (i = 0; i < homingSize; i++) 
		{
			resetFlag = prm[i] ^ homing[i];
			if ( resetFlag )
			{
				break;
			}
		}
	}

	/* reset decoder if current frame is a homing frame */
	if (resetFlag == 0) 
	{
		Speech_Decode_Frame_reset(s->decoder_State);
	}
	s->reset_flag_old = !resetFlag;
	s->prev_ft = frame_type;
	s->prev_mode = mode;
}


