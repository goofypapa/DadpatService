#ifndef __PLAY_CONTROLLER_H__
#define __PLAY_CONTROLLER_H__

#include "wav.h"
#include "audio_pcm.h"
#include <map>
#include <queue>
#include <pthread.h>

class playController
{
public:

    enum PlayState{
        Before = 0,
        Playing,
        Paused,
        Stop,
        End
    };

    struct play_controller_t{
        int                             play_id;
        pcm_handle_t *                  pcm_handle;
        playController::PlayState       play_state;

        char *  data;
        int datablock, dataSize, playDataSize;
    } ;

    typedef void(*PlayStateChancgedFunc)( const int, const playController::PlayState );

    static bool init( PlayStateChancgedFunc p_playStateChancged );

    static bool play( const int p_playId, const wav_t * p_wav );

    static bool stop( const int p_playId );

    static bool pause( const int p_playId );

    static bool resume( const int p_playId );

private:

    // static pthread_mutex_t sm_playStateChangeMutex;

    static void * _threadFunc( void * p_param );

    static bool playEnd( play_controller_t ** p_playController );

    static std::map<int, play_controller_t *> sm_playControllPoll;

    static PlayStateChancgedFunc sm_playStateChancged;
};



#endif //__PLAY_CONTROLLER_H__