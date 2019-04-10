#ifndef __PCM_H__
#define __PCM_H__

#include <alsa/asoundlib.h>

typedef struct _pcm_handle_t{
    snd_pcm_t   *handle;
    snd_pcm_uframes_t    buffer_size;
    snd_pcm_uframes_t    frames;
    int can_pause;
}pcm_handle_t;

pcm_handle_t * openPcm( int p_bit, int p_datablock, int p_rate, int p_channels );
bool closePcm( pcm_handle_t ** p_pcm_handle );




#endif //__PCM_H__