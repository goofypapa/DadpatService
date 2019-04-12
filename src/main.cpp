#include <iostream>
#include "audio.h"
#include "log.h"
#include "audioPcm.h"

int main( int argc, char ** argv )
{


    info( "start" );

    assert( audioPcmInitPcmPoll() );

    int t_audiooId = audio::cache("audios/djembe/B.wav");

    info( "load audios/djembe/B.wav res id: " << t_audiooId );

    t_audiooId = audio::cache("audios/djembe/T.wav");
    
    info( "load audios/djembe/T.wav res id: " << t_audiooId );

    t_audiooId = audio::cache("audios/djembe/S.wav");

    info( "load audios/djembe/S.wav res id: " << t_audiooId );

    t_audiooId = audio::cache("audios/bgm.wav");

    info( "load audios/bgm.wav res id: " << t_audiooId );

    char buffer[ 125 ] = {0};
    while( true )
    {
        std::cin.getline( buffer, sizeof(buffer) );

        if( std::string( buffer ).compare( "exit" ) == 0 )
        {
            break;
        }

        int t_id = atoi( buffer );

        audio::play( t_id );

    }

    return 0;
}