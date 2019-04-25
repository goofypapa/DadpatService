#include "audio.h"
#include <iostream>
#include <cstring>
#include "log.h"


#define MAX_PLAY_COUT 1

int audio::sm_audioId = 0, audio::sm_playId = 0;
std::map< int, wav_t * > audio::sm_audioCachePool;
std::map< std::string, int > audio::sm_audioPathCachePool;
std::vector<int> audio::sm_playingList;

pthread_mutex_t audio::sm_cacheUncacheMutex;

bool audio::init( void )
{
    pthread_mutex_init( &sm_cacheUncacheMutex, NULL );

    if( !audioPcmInitPcmPoll() )
    {
        return false;
    }

    if( !playController::init( playStateChangeed ) )
    {
        return false;
    }

    return true;
}

int audio::cache( const std::string & p_audioFile )
{
    int t_audioId = sm_audioId;

    pthread_mutex_lock( &sm_cacheUncacheMutex );

    std::map<int, wav_t *>::iterator t_it = sm_audioCachePool.find( t_audioId );
    if( t_it != sm_audioCachePool.end() )
    {
        closeWav( &t_it->second );
    }

    do
    {
        std::map<std::string, int>::iterator t_findAudioId = sm_audioPathCachePool.find( p_audioFile );
        if( t_findAudioId != sm_audioPathCachePool.end() )
        {
            t_audioId = t_findAudioId->second;
            break;
        }
    
        wav_t * t_wav = openWav( p_audioFile.c_str() );

        if( !t_wav )
        {
            t_audioId = -1;
            break;
        }

        sm_audioCachePool[t_audioId] = t_wav;
        sm_audioPathCachePool[p_audioFile] = t_audioId;

        sm_audioId ++;
    }while(0);

    pthread_mutex_unlock( &sm_cacheUncacheMutex );
    
    return t_audioId;
}


bool audio::uncache( const int p_audioId )
{
    bool t_result = false;

    pthread_mutex_lock( &sm_cacheUncacheMutex );

    do{
        std::map<int, wav_t *>::iterator t_it = sm_audioCachePool.find( p_audioId );
        if( t_it == sm_audioCachePool.end() )
        {
            break;
        }

        t_result = closeWav( &t_it->second );
        sm_audioCachePool.erase( t_it );

    }while(0);
    pthread_mutex_unlock( &sm_cacheUncacheMutex );

    return t_result;
}

bool audio::uncache( const std::string & p_audioFile )
{

    bool t_result = false;
    int t_audioId = -1;

    pthread_mutex_lock( &sm_cacheUncacheMutex );
    do{

        std::map<std::string, int>::iterator t_it = sm_audioPathCachePool.find( p_audioFile );
        if( t_it == sm_audioPathCachePool.end() )
        {
           break;
        }

        t_audioId = t_it->second;
        sm_audioPathCachePool.erase(t_it);
        
    }while(0);
    pthread_mutex_unlock( &sm_cacheUncacheMutex );

    if( t_audioId != -1 )
    {
        t_result = uncache( t_audioId );
    }

    return t_result;
}


int audio::play( int p_audioId, int p_playGroup )
{

    int t_playId = sm_playId++;

    std::map<int, wav_t *>::iterator t_it = sm_audioCachePool.find( p_audioId );

    if( t_it == sm_audioCachePool.end() )
    {
        return -1;
    }

    return playController::play( t_playId, t_it->second );
}


bool audio::stop( int p_playId )
{
    return playController::stop( p_playId );
}



void audio::playStateChangeed( const int p_playId, const playController::PlayState p_playState )
{
    
    if( p_playState == playController::Playing )
    {
        info( "-------------->>" << (int)sm_playingList.size() - MAX_PLAY_COUT );
        for( int i = 0; i < (int)sm_playingList.size() - MAX_PLAY_COUT; ++i )
        {
            playController::stop( sm_playingList[i] );
        }

        sm_playingList.push_back( p_playId );
    }
    else if( p_playState == playController::End )
    {
        
        for( std::vector< int >::iterator t_item = sm_playingList.begin(); t_item < sm_playingList.end(); ++ t_item )
        {
            if( *t_item == p_playId )
            {
                sm_playingList.erase( t_item );
            }
        }
    }
    info( "-------------> playId: " << p_playId << ", playState: " << p_playState );
}