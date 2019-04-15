#include <iostream>
#include "audio.h"
#include "log.h"
#include "audio_pcm.h"
#include "serial.h"


unsigned char startCmd[] = { (unsigned char)0xAB, (unsigned char)0x01, (unsigned char)0x01 };


struct timeval t_prve_recv_time, t_curr_recv_time;
void serialRecv( const unsigned char * p_data, const size_t p_dataSize );

int main( int argc, char ** argv )
{

    info( "start" );

    if( !audio::init() )
    {
        err( "audio init final" );
        return 0;
    }

    if( !serial::init() )
    {
        err( "serial init final" );
        return 0;
    }

    if( !serial::setRecvFunc( serialRecv ) )
    {
        err( "set recvfunc final" );
        return 0;
    }

    gettimeofday( &t_prve_recv_time, NULL );

    serial::send( startCmd, sizeof( startCmd ) );

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

        info( "try play " << t_id );

        audio::play( t_id );

    }

    return 0;
}


void serialRecv( const unsigned char * p_data, const size_t p_dataSize )
{

    gettimeofday( &t_curr_recv_time, NULL );

    int t_time_diff = (t_curr_recv_time.tv_sec - t_prve_recv_time.tv_sec) * 1000000 + t_curr_recv_time.tv_usec - t_prve_recv_time.tv_usec;
    if( t_time_diff < 10000 )
    {
        return;
    }
    
    t_prve_recv_time = t_curr_recv_time;
    info( "recv time space: " << t_time_diff );

    audio::play( 1 );

    // std::cout << "cmd: ";
    // for( int n = 0; n < p_dataSize; ++n )
    // {
    //     std::cout << (unsigned short)p_data[n] << " ";
    // }
    // std::cout << std::endl;
}