#include "playController.h"
#include "audioPcm.h"
#include "log.h"

#include <thread>


std::map<int, playController::play_controller_t *> playController::sm_playControllPoll;

bool playController::play( const int p_playId, const wav_t * p_wav )
{
    auto t_it = sm_playControllPoll.find( p_playId );

    int channels = p_wav->format.channels;
    int rate = p_wav->format.samples_per_sec;
    int bit = p_wav->format.bits_per_sample;
    int datablock = p_wav->format.block_align;

    pcm_handle_t * t_pcmHandle = nullptr;

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

    play_controller_t * t_playControll = ( play_controller_t * )malloc( sizeof( play_controller_t ) );
    t_playControll->pcm_handle = t_pcmHandle;
    t_playControll->play_state = Before;

    t_playControll->data = p_wav->data_buffer;
    t_playControll->datablock = datablock;
    t_playControll->dataSize = p_wav->data_size;
    t_playControll->playDataSize = 0;

    std::thread( [t_playControll](){
        t_playControll->play_state = PlayState::Playing;
        char * t_buffer = nullptr;
        int t_frames = 0;
        int ret = -1;
        pcm_handle_t * t_pcmHandle = t_playControll->pcm_handle;
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

        t_playControll->play_state = PlayState::End;

        play_controller_t * t_tmp = t_playControll;
        stop( &t_tmp );

    } ).detach();


    return true;
}


bool playController::stop( play_controller_t ** p_playController )
{
    play_controller_t * t_playController = *p_playController;

    if( !t_playController )
    {
        return false;
    }

    closePcm( &t_playController->pcm_handle );

    free( t_playController );
    t_playController = nullptr;

    return true;
}