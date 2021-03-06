#include "serial.h"
#include "log.h"

#include <cstdlib>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>

#define SERIAL_DEVICE_NAME "/dev/ttyUSB0"
#define SERIAL_SPEED 115200
#define SERIAL_BITS 8
#define SERIAL_CHECK_TYPE 'O'
#define SERIAL_STOP_BITS 1

std::vector< serial::recvFunc > serial::sm_recvFuncList;
std::queue< std::pair< unsigned char *, int > > serial::sm_sendDataPool;
int serial::sm_fd;
pthread_mutex_t serial::sm_recvFuncMutex, serial::sm_sendDataMutex;


bool serial::init( void )
{

    int ret;
    pthread_t pthread_id;

    if( pthread_mutex_init(&sm_recvFuncMutex, NULL) )
    {
        err( " sm_recvFuncMutex init final " );
        return false;
    }

    if( pthread_mutex_init(&sm_sendDataMutex, NULL) )
    {
        err( " sm_sendDataMutex init final " );
        return false;
    }

    sm_fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY);
    if( sm_fd == -1 )
    {
        err( "open /dev/ttyUSB0 fianl" );
        return false;
    }

    ret = set_opt(sm_fd, SERIAL_SPEED, SERIAL_BITS, SERIAL_CHECK_TYPE, SERIAL_STOP_BITS);
    if( ret == -1 )
    {
        err( "set serial opt final" );
        return false;
    }

    ret = pthread_create( &pthread_id, NULL, threadFunc, NULL );

    if( ret )
    {
        err( "create thread final" );
        return false;
    }

    return true;
}

bool serial::send( const unsigned char * p_data, const size_t p_dataSize )
{

    if( !p_data )
    {
        err( "send data is NULL" );
        return false;
    }

    unsigned char * t_data = (unsigned char *)malloc( p_dataSize );
    memcpy( t_data, p_data, p_dataSize );


    pthread_mutex_lock( &sm_sendDataMutex );
    sm_sendDataPool.push( std::pair< unsigned char *, int >( t_data, p_dataSize ) );
    pthread_mutex_unlock( &sm_sendDataMutex );

    return true;
}


bool serial::setRecvFunc( const serial::recvFunc p_recvFunc )
{
    if( !p_recvFunc )
    {
        return false;
    }

    pthread_mutex_lock( &sm_recvFuncMutex );
    sm_recvFuncList.push_back( p_recvFunc );
    pthread_mutex_unlock( &sm_recvFuncMutex );

    return true;
}

void * serial::threadFunc( void * p_param )
{
    int ret;
    unsigned char Rx_Data[100];
    unsigned char cmd_buffer[ 20 ];
    int cmd_buffer_cursor = 0;

    unsigned char t_cmdType = 0;

    int t_cmdCount = -1;


    while( true )
    {
        ret = read( sm_fd, Rx_Data, 100 );
        if( ret > 0 )
        {
            for( int i = 0; i < ret; ++i )
            {
                if( cmd_buffer_cursor == 0 && Rx_Data[i] != 0xBA )
                {
                    continue;
                }

                if( cmd_buffer_cursor == 1 )
                {
                    t_cmdType = Rx_Data[i];

                    if( t_cmdType == 0xa2 )
                    {
                        t_cmdCount = 10;
                    }
                    else{
                        err( "can not recognition cmd type: " << Rx_Data[i] );
                        cmd_buffer_cursor = 0;
                        continue;
                    }
                }
                
                cmd_buffer[cmd_buffer_cursor++] = Rx_Data[i];


                if( cmd_buffer_cursor == t_cmdCount )
                {
                    eventRecv( cmd_buffer, cmd_buffer_cursor );

                    cmd_buffer_cursor = 0;
                }
            }

            // eventRecv( Rx_Data, ret );
        }else{
            usleep( 1000 );
        }

        pthread_mutex_lock( &sm_sendDataMutex );
        while( sm_sendDataPool.size() )
        {
            std::pair< unsigned char *, int > t_item = sm_sendDataPool.front();
            sm_sendDataPool.pop();

            write( sm_fd, t_item.first, t_item.second );
        }
        pthread_mutex_unlock( &sm_sendDataMutex );

        usleep( 100 );
    }

    return NULL;
}


void serial::eventRecv( const unsigned char * p_data, const size_t p_dataSize )
{
    pthread_mutex_lock( &sm_recvFuncMutex );
    for( int i = 0; i < sm_recvFuncList.size(); ++i )
    {
        sm_recvFuncList[i]( p_data, p_dataSize );
    }
    pthread_mutex_unlock( &sm_recvFuncMutex );
}

int serial::set_opt(int fd,int nSpeed,int nBits,char nEvent,int nStop)
{
    struct termios newtio,oldtio;
    if(tcgetattr(fd,&oldtio)!=0)
    {
        err("SetupSerial");
        return -1;
    }
    bzero(&newtio,sizeof(newtio));
    //使能串口接收
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    newtio.c_lflag &=~ICANON;//原始模式

    // newtio.c_lflag |=ICANON; //标准模式

    //设置串口数据位
    switch(nBits)
    {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |=CS8;
            break;
    }
    //设置奇偶校验位
    switch(nEvent)

    {
        case 'O':
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            // newtio.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'E':
            newtio.c_iflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'N':
            newtio.c_cflag &=~PARENB;
            break;
    }
    //设置串口波特率
    switch(nSpeed)
    {
        case 2400:
            cfsetispeed(&newtio,B2400);
            cfsetospeed(&newtio,B2400);
            break;
        case 4800:
            cfsetispeed(&newtio,B4800);
            cfsetospeed(&newtio,B4800);
            break;
        case 9600:
            cfsetispeed(&newtio,B9600);
            cfsetospeed(&newtio,B9600);
            break;
        case 115200:
            cfsetispeed(&newtio,B115200);
            cfsetospeed(&newtio,B115200);
            break;
        case 460800:
            cfsetispeed(&newtio,B460800);
            cfsetospeed(&newtio,B460800);
            break;
        default:
            cfsetispeed(&newtio,B9600);
            cfsetospeed(&newtio,B9600);
            break;
    }
    //设置停止位
    if(nStop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if(nStop == 2)
        newtio.c_cflag |= CSTOPB;
    newtio.c_cc[VTIME] = 1;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd,TCIFLUSH);

    if(tcsetattr(fd,TCSANOW,&newtio)!=0)
    {
        err("com set error");
        return -1;
    }
    return 0;
}