#ifndef __PCM_H__
#define __PCM_H__

#include <alsa/asoundlib.h>
#include <queue>

typedef struct _pcm_handle_t{
    snd_pcm_t   *handle;
    snd_pcm_uframes_t    buffer_size;
    snd_pcm_uframes_t    frames;
    int can_pause;
}pcm_handle_t;


bool audioPcmInitPcmPoll( void );

pcm_handle_t * openPcm( int p_bit, int p_datablock, int p_rate, int p_channels );
bool closePcm( pcm_handle_t ** p_pcm_handle );


pcm_handle_t * _openPcm( int p_bit, int p_datablock, int p_rate, int p_channels );

#endif //__PCM_H__