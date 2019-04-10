#ifndef __ALSA_TEST_H__
#define __ALSA_TEST_H__

typedef struct _wav_riff_t{
    char id[5];               //ID:"RIFF"
    int  size;                //file_len - 8
    char type[5];             //type:"WAVE"
}wav_riff_t;


typedef struct _wav_format_t{
    char  id[5];              //ID:"fmt"
    int   size;
    short compression_code;
    short channels;
    int   samples_per_sec;
    int   avg_bytes_per_sec;
    short block_align;
    short bits_per_sample;
}wav_format_t;


typedef struct _wav_fact_t{
    char id[5];
    int  size;
}wav_fact_t;


typedef struct _wav_data_t{
    char id[5];
    int  size;
}wav_data_t;


typedef struct _wav_t{
    FILE         *fp;
    wav_riff_t   riff;
    wav_format_t format;
    wav_fact_t   fact;
    wav_data_t   data;
    int          file_size;
    int          data_offset;
    int          data_size;
    char *       data_buffer;
}wav_t;

typedef struct _pcm_handle_t{
    snd_pcm_t   *handle;
    snd_pcm_uframes_t    buffer_size;
    snd_pcm_uframes_t    frames;
}pcm_handle_t;

void open_pcm( pcm_handle_t * p_pcm_handle, wav_t * p_wav );
void close_pcm( pcm_handle_t * p_pcm_handle );
void * playWav( void * p_filePath );

#endif //__ALSA_TEST_H__