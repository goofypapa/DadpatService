#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <vector>
#include <cstddef>
#include <queue>
#include <map>
#include <pthread.h>
#include <sys/time.h>

class serial
{
public:

    typedef void( *recvFunc )( const unsigned char * p_data, const size_t p_dataSize );

    static bool init( void );

    static bool send( const unsigned char * p_data, const size_t p_dataSize );

    static bool setRecvFunc( const recvFunc p_recvFunc );

private:

    static int sm_fd;
    static pthread_mutex_t sm_recvFuncMutex, sm_sendDataMutex;

    static void eventRecv( const unsigned char * p_data, const size_t p_dataSize );

    static void * threadFunc( void * p_param );

    static int set_opt(int fd,int nSpeed,int nBits,char nEvent,int nStop);

    static std::vector< recvFunc > sm_recvFuncList;

    static std::queue< std::pair< unsigned char *, int > > sm_sendDataPool;
};

#endif //__SERIAL_H__