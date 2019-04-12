#include "play_controller.h"
#include "audio_pcm.h"
#include "log.h"

#include <pthread.h>


std::map<int, playController::play_controller_t *> playController::sm_playControllPoll;

bool playController::play( const int p_playId, const wav_t * p_wav )
{
    std::map<int, playController::play_controller_t *>::iterator t_it = sm_playControllPoll.find( p_playId );

    int channels = p_wav->format.channels;
    int rate = p_wav->format.samples_per_sec;
    int bit = p_wav->format.bits_per_sample;
    int datablock = p_wav->format.block_align;

    pcm_handle_t * t_pcmHandle = NULL;

    if( t_it != sm_playControllPoll.end() )
    {
        err( "replay" );
        return false;
    }

    t_pcmHandle = openPcm( bit, datablock, rate, channels );

    if( !t_pcmHandle )
    {
        return false;
    }

    info( "get pcmHandle" );

    play_controller_t * t_playControll = ( play_controller_t * )malloc( sizeof( play_controller_t ) );
    t_playControll->pcm_handle = t_pcmHandle;
    t_playControll->play_state = Before;

    t_playControll->data = p_wav->data_buffer;
    t_playControll->datablock = datablock;
    t_playControll->dataSize = p_wav->data_size;
    t_playControll->playDataSize = 0;


    pthread_t t_threadId;
    int t_hreadId = pthread_create( &t_threadId, NULL, _threadFunc, (void *)t_playControll );

    if( t_hreadId )
    {
        err( "pthread_create final" );

        playEnd( &t_playControll );
        return false;
    }

    return true;
}


void * playController::_threadFunc( void * p_param )
{
    play_controller_t * t_playControll = (play_controller_t *)p_param;

    t_playControll->play_state = Playing;
    char * t_buffer = NULL;
    int t_frames = 0;
    int ret = -1;
    pcm_handle_t * t_pcmHandle = t_playControll->pcm_handle;

    info( "in play thread" );

    while( true )
    {
        if( t_playControll->dataSize <= t_playControll->playDataSize )
        {
            break;
        }

        t_buffer = t_playControll->data + t_playControll->playDataSize;
        t_frames = t_playControll->dataSize - t_playControll->playDataSize >= t_pcmHandle->buffer_size ? t_pcmHandle->frames : ( t_playControll->dataSize - t_playControll->playDataSize ) / t_playControll->datablock;
        while( ( ret = snd_pcm_writei( t_pcmHandle->handle, t_buffer, t_frames ) ) < 0 )
        {
            if( ret == -EPIPE )
            {
                err( "underrun occurred" );
                snd_pcm_prepare(t_pcmHandle->handle);
            }
            else if( ret < 0 )
            {
                err( "error from writei: " << snd_strerror(ret) );
            }
        }

        t_playControll->playDataSize += ret * t_playControll->datablock;
    }

    t_playControll->play_state = End;

    play_controller_t * t_tmp = t_playControll;
    playEnd( &t_tmp );
    return NULL;
}


bool playController::playEnd( play_controller_t ** p_playController )
{
    play_controller_t * t_playController = *p_playController;

    if( !t_playController )
    {
        return false;
    }

    closePcm( &t_playController->pcm_handle );

    free( t_playController );
    t_playController = NULL;

    return true;
}