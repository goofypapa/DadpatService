#ifndef __PLAY_CONTROLLER_H__
#define __PLAY_CONTROLLER_H__

#include "wav.h"
#include "audio_pcm.h"
#include <map>
#include <queue>

class playController
{
public:

    enum PlayState{
        Before = 0,
        Playing,
        Paused,
        End
    };

    struct play_controller_t{
        pcm_handle_t *                  pcm_handle;
        playController::PlayState       play_state;

        char *  data;
        int datablock, dataSize, playDataSize;
    } ;

    static bool play( const int p_playId, const wav_t * p_wav );

private:

    static void * _threadFunc( void * p_param );

    static bool playEnd( play_controller_t ** p_playController );

    static std::map<int, play_controller_t *> sm_playControllPoll;
};



#endif //__PLAY_CONTROLLER_H__