#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <vector>
// #include <queue>
// #include <map>

class serial
{
public:

    typedef void( *recvFunc )( const unsigned char * p_data, const int p_dataSize );

    static bool init( void );

    static bool send( const unsigned char * p_data, const size_t p_dataSize );

private:

    static int sm_fd;

    static void * threadFunc( void * p_param );

    static int set_opt(int fd,int nSpeed,int nBits,char nEvent,int nStop);

    static std::vector< recvFunc > sm_recvFuncList;

    // static std::queue< std::pair< unsigned char *, int > > sm_sendDataPool;
};

#endif //__SERIAL_H__