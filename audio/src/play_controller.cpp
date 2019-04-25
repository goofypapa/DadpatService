#include "play_controller.h"
#include "audio_pcm.h"
#include "log.h"

#include <pthread.h>
#include <sys/time.h>


std::map<int, playController::play_controller_t *> playController::sm_playControllPoll;

// pthread_mutex_t playController::sm_playStateChangeMutex;

playController::PlayStateChancgedFunc playController::sm_playStateChancged = NULL;

bool playController::init( playController::PlayStateChancgedFunc p_playStateChancged )
{
    // if( pthread_mutex_init( &sm_playStateChangeMutex, NULL ) )
    // {
    //     return false;
    // }

    sm_playStateChancged = p_playStateChancged;

    return true;
}

bool playController::play( const int p_playId, const wav_t * p_wav )
{
    std::map<int, playController::play_controller_t *>::iterator t_it = sm_playControllPoll.find( p_playId );

    int channels = p_wav->format.channels;
    int rate = p_wav->format.samples_per_sec;
    int bit = p_wav->format.bits_per_sample;
    int datablock = p_wav->format.block_align;

    struct timeval t_create_thread_before_time, t_create_thread_after_time;

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

    play_controller_t * t_playControll = ( play_controller_t * )malloc( sizeof( play_controller_t ) );
    t_playControll->play_id = p_playId;
    t_playControll->pcm_handle = t_pcmHandle;
    t_playControll->play_state = Before;

    t_playControll->data = p_wav->data_buffer;
    t_playControll->datablock = datablock;
    t_playControll->dataSize = p_wav->data_size;
    t_playControll->playDataSize = 0;

    pthread_t t_threadId;

    gettimeofday( &t_create_thread_before_time, NULL );

    int t_hreadId = pthread_create( &t_threadId, NULL, _threadFunc, (void *)t_playControll );

    gettimeofday( &t_create_thread_after_time, NULL );

    info( "--------------> create thread time: " << (t_create_thread_after_time.tv_sec - t_create_thread_before_time.tv_sec) * 1000000 
    + t_create_thread_after_time.tv_usec - t_create_thread_before_time.tv_usec );

    if( t_hreadId )
    {
        err( "pthread_create final" );

        playEnd( &t_playControll );
        return false;
    }
    


    sm_playControllPoll[p_playId] = t_playControll;

    return true;
}


bool playController::stop( const int p_playId )
{
    int err = 0;

    std::map<int, playController::play_controller_t *>::iterator t_it = sm_playControllPoll.find( p_playId );

    if( t_it == sm_playControllPoll.end() )
    {
        return false;
    }

    t_it->second->play_state = Stop;
    // if( t_it->second->pcm_handle->can_pause )
    // {
    //     if( err = snd_pcm_pause( t_it->second->pcm_handle->handle, true ) < 0 )
    //     {
    //         err( "snd_pcm_pause err: " << snd_strerror(err) );
    //     }
    // }else{
        if( err = snd_pcm_drop( t_it->second->pcm_handle->handle ) < 0 )
        {
            err( "snd_pcm_drop err: " << snd_strerror(err) );
        }
    // }
    
    return true;
}


void * playController::_threadFunc( void * p_param )
{
    play_controller_t * t_playControll = (play_controller_t *)p_param;

    t_playControll->play_state = Playing;
    sm_playStateChancged( t_playControll->play_id, t_playControll->play_state );

    char * t_buffer = NULL;
    int t_frames = 0;
    int ret = -1;
    pcm_handle_t * t_pcmHandle = t_playControll->pcm_handle;

    while( true )
    {

        if( t_playControll->play_state == Paused )
        {
            usleep( 1000 );
            continue;
        }
        else if( t_playControll->play_state == Stop )
        {
            break;
        }

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
                if( t_playControll->play_state == Stop )
                {
                    break;
                }
                err( "underrun occurred" );
                snd_pcm_prepare(t_pcmHandle->handle);
            }
            else if( ret < 0 )
            {
                if( t_playControll->play_state == Stop )
                {
                    break;
                }else if( t_playControll->play_state == Paused )
                {
                    usleep( 100 );
                }
                err( "error from writei: " << snd_strerror(ret) );
            }
        }

        t_playControll->playDataSize += ret * t_playControll->datablock;
    }

    t_playControll->play_state = End;
    sm_playStateChancged( t_playControll->play_id, t_playControll->play_state );

    sm_playControllPoll.erase( t_playControll->play_id );

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