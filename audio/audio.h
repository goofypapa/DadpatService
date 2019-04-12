#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "wav.h"
#include <string>
#include <map>
#include <pthread.h>

class audio
{
public:

    static bool init( void );

    static int cache( const std::string & p_audioFile );

    static bool uncache( const int p_audioId );
    static bool uncache( const std::string & p_audioFile );

    static int play( int p_audioId, int p_playGroup = 0 );
    static int play( const std::string & p_audioFile, int p_playGroup = 0 );

    static bool pause( int p_playId );
    static bool resume( int p_playId );
    static int pauseAll( int p_playGroup = 0 );
    static int resumeAll( int p_playGroup = 0 );

    static bool stop( int p_playId );
    static bool stopAll( int p_playGroup = 0 );

private:

    static int sm_audioId;
    static pthread_mutex_t sm_cacheUncacheMutex;
    
    static std::map< int, wav_t * > sm_audioCachePool;
    static std::map< std::string, int > sm_audioPathCachePool;
};

#endif //__AUDIO_H__