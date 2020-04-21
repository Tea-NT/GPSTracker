
#include <stdlib.h>
#include <stdio.h>
#include "amr_interf_dec.h"
#include "amr_sp_dec.h"
#include "amr_typedef.h"
#include "log_service.h"
#include <math.h>
#include "gm_fs.h"
#include "config_service.h"
#include <string.h>
#include "gm_stdlib.h"
#include "gm_type.h"
#define AMR_MAGIC_NUMBER "#!AMR\n"

float calculation_frame_db(short *data , unsigned short len)
{
	char i = 0;
	float total_db = 0;

	for(i = 0; i < len ; i++)
	{
		total_db += abs((short)data[i]);
	}

	total_db = total_db / (float)len;

	if(total_db)
	{
		return (int)(20.0*(log10(total_db)));
	}

	return 0;
}
 
int GM_GetRecoderDB(const u16  * FileName)
{
	int readcnt ,file_speech = -1;
	int cur_db,totaldb = 0;
	short synth[160]={0};
    int frames = 0;
    int * destate;
    int read_size;
	unsigned int filesize = 0;
	u16 i = 0;
    unsigned char analysis[32];
    enum Mode dec_mode;
    char magic[8] = {0};
    short block_size[16]={ 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };

	if(FileName)
	{
		file_speech = GM_FS_Open(FileName,GM_FS_READ_ONLY | GM_FS_ATTR_ARCHIVE);
		LOG(INFO, "record file open %d",file_speech);
		if(file_speech <= 0) return -1;
	}

   	destate = Decoder_Interface_init();
	GM_FS_Read(file_speech , magic, GM_strlen(AMR_MAGIC_NUMBER),(UINT *)&readcnt);
	LOG(INFO, "record file magic %s",magic);
    if (strncmp( magic, AMR_MAGIC_NUMBER, GM_strlen( AMR_MAGIC_NUMBER ) ) ) 
	{
 	   GM_FS_Close( file_speech );
 	   return -2;
    }
    GM_FS_GetFileSize(file_speech, &filesize);
    while(GM_FS_Read(file_speech,analysis, 1, (UINT *)&readcnt ) >= 0)
    {
		i++;
        dec_mode = (enum Mode)((analysis[0] >> 3) & 0x000F);
    	read_size = block_size[dec_mode];
        GM_FS_Read(file_speech,&analysis[1],read_size, (UINT *)&readcnt);

		if(i%2 == 0)
		{
	      	u8 sound_energy;
	      	Decoder_Interface_Decode(destate, analysis, synth, 0);
			cur_db = calculation_frame_db(synth,160);
			config_service_get(CFG_VOICE_ENERGY, TYPE_BYTE, &sound_energy, sizeof(sound_energy));
			if (config_service_is_test_mode())
			{
				sound_energy = 20;
			}
			if((cur_db >= sound_energy)&&(cur_db <= 100))
			{
				frames ++;
				totaldb += cur_db;
			}
		}
	  	
		if(i >= (filesize/read_size - 1))
		{
			LOG(INFO, "frame %d , totaldb %d , total frame %d",frames,totaldb,filesize/read_size);
			if(frames < (i/2)/8)
			{
				totaldb = 1;
			}
			else
			{
				totaldb = frames*60;
			}
			
			break;
		}
    }
	
	Decoder_Interface_exit(destate);
	GM_FS_Close(file_speech);
	return totaldb/frames;
}
   
