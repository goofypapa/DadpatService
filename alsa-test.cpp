#include <alsa/asoundlib.h>
#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <cstring>
#include <sys/time.h>

#include "alsa-test.h"


#define NULL NULL

using std::cout;
using std::endl;
using std::string;

#define PCM_HANDLE_POOL_SIZE 10

pcm_handle_t * s_pcm_handle_pool[PCM_HANDLE_POOL_SIZE];

wav_t * openwav( const char * p_filePath );
void closewav( wav_t ** p_wav );

void * randomPlay(void *);

void openserial( void );

int set_opt(int fd,int nSpeed,int nBits,char nEvent,int nStop);

struct timeval t_start, t_callPlay, t_openPcm, t_playStart;

wav_t * s_wav_b = openwav( "B.wav" );
wav_t * s_wav_t = openwav( "T.wav" );
wav_t * s_wav_s = openwav( "S.wav" );

int main(int argc, char ** argv)
{

    // pthread_attr_t attr;       // 线程属性
    // int rs;

    // /* 
    //  * 对线程属性初始化
    //  * 初始化完成以后，pthread_attr_t 结构所包含的结构体
    //  * 就是操作系统实现支持的所有线程属性的默认值
    //  */
    // rs = pthread_attr_init (&attr);
    // assert (rs == 0);     // 如果 rs 不等于 0，程序 abort() 退出

    // rs = pthread_attr_setschedpolicy ( &attr, SCHED_RR );
    // assert (rs == 0);


    char cmdBuffer[100];
    pthread_t pthread_id;
    string filePath;
    int playSpace;

    cout << "load B.wav T.wav S.wav ..." <<endl;

    // init pcm_handle_pool
    for( int i = 0; i < PCM_HANDLE_POOL_SIZE; ++i )
    {
        s_pcm_handle_pool[i] = ( pcm_handle_t * )malloc( sizeof( pcm_handle_t ) );

        open_pcm( s_pcm_handle_pool[i], s_wav_b );
    }

    openserial();

    // while( true )
    // {
    //     memset(cmdBuffer, 0, sizeof(cmdBuffer));
    //     cout << "input play file: " << endl;
    //     // std::cin >> cmdBuffer;

    //     std::cin.getline(cmdBuffer, sizeof(cmdBuffer));

    //     gettimeofday( &t_start, NULL );
    //     cout << "----> cmd: " << cmdBuffer << endl;

    //     if( !strcmp( cmdBuffer, "quit" ) )
    //     {
    //         exit(2);
    //     }


    //     if( !strcmp( cmdBuffer, "b" ) )
    //     {
    //         if( !pthread_create(&pthread_id, NULL, playWav, (void *)s_wav_b) )
    //         {
    //             cout << "playing B.wav ..." << endl;
    //         }
    //         continue;
    //     }

    //     if( !strcmp( cmdBuffer, "t" ) )
    //     {
    //         if( !pthread_create(&pthread_id, NULL, playWav, (void *)s_wav_t) )
    //         {
    //             cout << "playing T.wav ..." << endl;
    //         }
    //         continue;
    //     }

    //     // if( !strcmp( cmdBuffer, "s" ) )
    //     // {
    //         if( !pthread_create(&pthread_id, NULL, playWav, (void *)s_wav_s) )
    //         {
    //             cout << "playing S.wav ..." << endl;
    //         }
    //         continue;
    //     // }
        
    // }

    closewav( &s_wav_b );
    closewav( &s_wav_t );
    closewav( &s_wav_s );

    return 0;
}

int set_opt(int fd,int nSpeed,int nBits,char nEvent,int nStop)
{
    struct termios newtio,oldtio;
    if(tcgetattr(fd,&oldtio)!=0)
    {
        perror("error:SetupSerial 3\n");
        return -1;
    }
    bzero(&newtio,sizeof(newtio));
    //使能串口接收
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    newtio.c_lflag &=~ICANON;//原始模式

    //newtio.c_lflag |=ICANON; //标准模式

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
            newtio.c_iflag |= (INPCK | ISTRIP);
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
        perror("com set error\n");
        return -1;
    }
    return 0;
}

void openserial( void )
{
    int fd, ret;
    unsigned char Rx_Data[100];
    unsigned char cmd_buffer[ 20 ];
    int cmd_buffer_cursor = 0;
    pthread_t pthread_id;

    struct timeval prve_play_time, tmp_time;

    fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY);
    if(fd == -1)
    {
        cout << "open /dev/ttyUSB0 fianl" << endl;
        exit(0);
    }

    ret = set_opt(fd,115200,8,'N',0);
    if(ret == -1)
    {
        cout << "set serial opt final" << endl;
        exit(0);
    }

    unsigned char data[] = { (unsigned char)0xAB, (unsigned char)0x01, (unsigned char)0x01 };

    cout << "---------------> write: ";

    cout.setf(std::ios::showbase); 
    cout.setf( std::ios_base::hex, std::ios_base::basefield); 

    for( int i = 0; i < sizeof(data); ++i )
    {
        cout << (unsigned short)data[i] << " ";
    }
    cout << endl;

    write( fd, data, sizeof( data ) );

    gettimeofday( &prve_play_time, NULL );

    while(1)
    {        
        ret = read( fd, Rx_Data, 100);
        if( ret >0 )
        {
            for( int i = 0; i < ret; ++i )
            {

                if( cmd_buffer_cursor == 0 && Rx_Data[i] != 0xBA )
                {
                    continue;
                }

                if( cmd_buffer_cursor == 1 && Rx_Data[i] != 0xA2 )
                {
                    cout << "not find comd " << (unsigned short)Rx_Data[i] << endl;
                    cmd_buffer_cursor = 0;
                    continue;
                }
                
                cmd_buffer[cmd_buffer_cursor++] = Rx_Data[i];

                if( cmd_buffer[1] == 0xA2 && cmd_buffer_cursor >= 10 )
                {
                    gettimeofday( &t_start, NULL );

                    tmp_time = t_start;

                    int t_time_diff = (tmp_time.tv_sec - prve_play_time.tv_sec) * 1000000 + tmp_time.tv_usec - prve_play_time.tv_usec;

                    if( t_time_diff >= 20000 )
                    {
                        cout.unsetf( std::ios::hex );
                        cout << "play time space: " << t_time_diff << endl;
                        cout.setf( std::ios_base::hex, std::ios_base::basefield);

                        if( !pthread_create(&pthread_id, NULL, playWav, (void *)s_wav_s) )
                        {
                            cout << "playing S.wav ..." << endl;
                        }
                        prve_play_time = tmp_time;
                    }else{
                        cout << "Invalid data" << endl;
                    }

                    cout << "cmd: ";
                    for( int n = 0; n < 10; ++n )
                    {
                        cout << (unsigned short)cmd_buffer[n] << " ";
                    }
                    cout << endl;
                    cmd_buffer_cursor = 0;
                }
            }
        }
        // else
        // {
        //     usleep(1000);
        // }

        // usleep( 100 );
    }
}

wav_t * openwav( const char * p_filePath )
{
    wav_t * wav = NULL; 
    
    char buffer[256];
    int  read_len = 0;
    int  offset = 0;

    wav = (wav_t *)malloc(sizeof(wav_t));
    if(!wav)
    {
        cout << "Error malloc wav failedly" << endl;
        return NULL;
    }
    bzero(wav, sizeof(wav_t));

    wav->fp = fopen(p_filePath, "rb");
    if(!wav->fp)
    {
        cout << "open " << p_filePath << "failed" << endl;
        return NULL;
    }

    read_len = fread(buffer, 1, 12, wav->fp);
    if(read_len < 12)
    {
        cout << "error wav file" << endl;
        closewav( &wav );
        return NULL;
    }

    if(strncasecmp("RIFF", buffer, 4))
    {
        cout << "file style is not wav" << endl;
        closewav( &wav );
        return NULL;
    }

    memcpy(wav->riff.id, buffer, 4); 
    wav->riff.size = *(int *)(buffer + 4);
    if(strncasecmp("WAVE", buffer + 8, 4))
    {
        cout << "Error wav file" << endl;
        closewav( &wav );
        return NULL;
    }

    memcpy(wav->riff.type, buffer + 8, 4);
    wav->file_size = wav->riff.size + 8;
    offset += 12;

    while(true)
    {
        char id_buffer[5] = {0};
        int  tmp_size = 0;

        read_len = fread(buffer, 1, 8, wav->fp);  
        if(read_len < 8)
        {
            cout << "Error wav file" << endl;
            closewav( &wav );
            return NULL;
        }
        memcpy(id_buffer, buffer, 4);
        tmp_size = *(int *)(buffer + 4);

        if(!strncasecmp("FMT", id_buffer, 3))
        {
            memcpy(wav->format.id, id_buffer, 3);
            wav->format.size = tmp_size;
            read_len = fread(buffer, 1, tmp_size, wav->fp);
            if(read_len < tmp_size)
            {
                cout << "Error wav file" << endl;
                closewav( &wav );
                return NULL;
            }
            wav->format.compression_code  = *(short *)buffer;
            wav->format.channels          = *(short *)(buffer + 2);
            wav->format.samples_per_sec   = *(int *)(buffer + 4);
            wav->format.avg_bytes_per_sec = *(int *)(buffer + 8);
            wav->format.block_align       = *(short *)(buffer + 12);
            wav->format.bits_per_sample   = *(short *)(buffer + 14);
        }
        else if(!strncasecmp("DATA", id_buffer, 3))
        {
            memcpy(wav->data.id, id_buffer, 4); 
            wav->data.size = tmp_size;
            offset += 8;
            wav->data_offset = offset;
            wav->data_size = wav->data.size; 
            break;
        }else
        {
            cout << "unhandled chunk: " << id_buffer << ", size: " << tmp_size << endl;
            fseek(wav->fp, tmp_size, SEEK_CUR);
        }
        offset += 8 + tmp_size;
    }

    wav->data_buffer = (char *)malloc( wav->data.size );
    fseek(wav->fp, wav->data_offset, SEEK_SET);

    read_len = fread( wav->data_buffer, 1, wav->data.size, wav->fp );
    cout << "read size: " << read_len << endl;
    if( read_len != wav->data.size )
    {
        cout << "read wav data error" << endl;
    }

    fclose( wav->fp );
    wav->fp = NULL;

    return wav;
}

void closewav( wav_t ** p_wav )
{
    cout << "closewav start" << endl;
    wav_t * wav = *p_wav;
    if( !wav )
    {
        return;
    }

    if( wav->data_buffer )
    {
        free( wav->data_buffer );
        wav->data_buffer = NULL;
    }

    if(wav->fp)
    {
        fclose(wav->fp);
        wav->fp = NULL;
    }
    free( wav );
    wav = NULL;

    cout << "closewav end" << endl;
}


void open_pcm( pcm_handle_t * p_pcm_handle, wav_t * p_wav )
{


    int rc, monotonic = 0, can_pause = 0;
    snd_pcm_hw_params_t * params;//硬件信息和PCM流配置
    unsigned val, buffer_time = 0, period_time = 0;
    

    int channels = p_wav->format.channels;
    int frequency = p_wav->format.samples_per_sec;
    int bit = p_wav->format.bits_per_sample;
    int datablock = p_wav->format.block_align;

    /*
    cout << "file size = " << p_wav->file_size << endl;

    cout << "RIFF WAVE Chunk" <<endl;
    cout << "id: " << p_wav->riff.id << endl;
    cout << "size: " << p_wav->riff.size << endl;
    cout << "type: " << p_wav->riff.type << endl;
    cout << endl;

    cout << "FORMAT Chunk" << endl;
    cout << "id: " << p_wav->format.id << endl;
    cout << "size: " << p_wav->format.size << endl;
    cout << "Channels = " << p_wav->format.channels << endl;
    cout << "SamplesPersec = " << p_wav->format.samples_per_sec << endl;
    cout << "avg_bytes_per_sec = " << p_wav->format.avg_bytes_per_sec << endl;
    cout << "block_align = " << p_wav->format.block_align << endl;
    cout << "BitsPerSample = " << p_wav->format.bits_per_sample << endl;
    cout << endl;

    cout << "DATA Chunk" << endl;
    cout << "id: " << p_wav->data.id << endl;
    cout << "data size: " << p_wav->data.size << endl;
    cout << "data offset: " << p_wav->data_offset << endl;
    */


    rc = snd_pcm_open(&p_pcm_handle->handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if( rc < 0 )
    {
        cout << "Error open PCM device failed" << endl;
        return;
    }

    snd_pcm_hw_params_alloca(&params); //分配params结构体

    rc = snd_pcm_hw_params_any(p_pcm_handle->handle, params);//初始化params
    if( rc < 0 )
    {
        cout << "Error snd_pcm_hw_params_any" << endl;
        return;
    }

    rc = snd_pcm_hw_params_set_access(p_pcm_handle->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED); //初始化访问权限
    if( rc < 0 )
    {
        cout << "Error snd_pcm_hw_params_set_access" << endl;
        return;
    }

    switch( bit / 8 )
    {
        case 1:
            snd_pcm_hw_params_set_format(p_pcm_handle->handle, params, SND_PCM_FORMAT_U8);
            break;
        case 2:
            snd_pcm_hw_params_set_format(p_pcm_handle->handle, params, SND_PCM_FORMAT_S16_LE);
            break;
        case 3:
            //SND_PCM_FORMAT_S24_3BE
            snd_pcm_hw_params_set_format(p_pcm_handle->handle, params, SND_PCM_FORMAT_S24_3LE);
            break;
        case 4:
            snd_pcm_hw_params_set_format(p_pcm_handle->handle, params, SND_PCM_FORMAT_FLOAT);
            break;
    }

    rc = snd_pcm_hw_params_set_channels(p_pcm_handle->handle, params, channels); //设置声道,1表示单声>道，2表示立体声
    if( rc < 0 )
    {
        cout << "Error snd_pcm_hw_params_set_channels" << endl;
        return;
    }

    val = frequency;
    rc = snd_pcm_hw_params_set_rate_near(p_pcm_handle->handle, params, &val, 0);
    if( rc < 0 )
    {
        cout << "Error snd_pcm_hw_params_set_rate_near" << endl;
        return;
    }

    rc = snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, 0);
    assert(rc >= 0);

    if (buffer_time > 500000)
        buffer_time = 500000;

    period_time = buffer_time / 4;

    rc = snd_pcm_hw_params_set_period_time_near(p_pcm_handle->handle, params, &period_time, 0);
    if( rc < 0 )
    {
        cout << "Error snd_pcm_hw_params_set_period_time_near" << endl;
        return;
    }
    rc = snd_pcm_hw_params_set_buffer_time_near(p_pcm_handle->handle, params, &buffer_time, 0);
    if( rc < 0 )
    {
        cout << "Error snd_pcm_hw_params_set_buffer_time_near" << endl;
        return;
    }

    monotonic = snd_pcm_hw_params_is_monotonic(params);
    can_pause = snd_pcm_hw_params_can_pause(params);
    rc = snd_pcm_hw_params(p_pcm_handle->handle, params);
    if( rc < 0 )
    {
        cout << "Error snd_pcm_hw_params" << endl;
        return;
    }

    rc = snd_pcm_hw_params_get_period_size(params, &p_pcm_handle->frames, 0); /*获取周期长度*/
    if( rc < 0 )
    {
        cout << "Error snd_pcm_hw_params_get_period_size" << endl;
        return;
    }

    rc = snd_pcm_hw_params_get_buffer_size(params, &p_pcm_handle->buffer_size);
    if( rc < 0 )
    {
        cout << "Error snd_pcm_hw_params_get_buffer_size" << endl;
        return;
    }

    p_pcm_handle->buffer_size = p_pcm_handle->frames * datablock; /*4 代表数据快长度*/


    cout << "buffer_size:" << p_pcm_handle->buffer_size << ", frames: " << p_pcm_handle->frames << endl;
}

void close_pcm( pcm_handle_t * p_pcm_handle )
{
    snd_pcm_drain( p_pcm_handle->handle );
    snd_pcm_close( p_pcm_handle->handle );

    free( p_pcm_handle );
}


void * playWav( void * p_wav )
{

    // pthread_attr_t attr;       // 线程属性
    // int rs;

    // /* 
    //  * 对线程属性初始化
    //  * 初始化完成以后，pthread_attr_t 结构所包含的结构体
    //  * 就是操作系统实现支持的所有线程属性的默认值
    //  */
    // rs = pthread_attr_init (&attr);
    // assert (rs == 0);     // 如果 rs 不等于 0，程序 abort() 退出

    // rs = pthread_attr_setschedpolicy ( &attr, SCHED_RR );
    // assert (rs == 0);


    wav_t * wav = (wav_t *)p_wav;

    if( !wav )
    {
        return NULL;
    }

    int ret;
    char *data = wav->data_buffer;
    char *buffer = data;
    int datablock = wav->format.block_align;
    int dataSize = wav->data.size;
    int playDataSize = 0;

    gettimeofday( &t_callPlay, NULL );
    cout << "----> call play " << t_callPlay.tv_sec - t_start.tv_sec + (t_callPlay.tv_usec - t_start.tv_usec)/1000000.0f << endl;

    pcm_handle_t * pcm_handle = NULL;

    for( int i = 0; i < PCM_HANDLE_POOL_SIZE; ++i )
    {
        if( s_pcm_handle_pool[i] != NULL )
        {
            pcm_handle = s_pcm_handle_pool[i];
            s_pcm_handle_pool[i] = NULL;
            break;
        }
    }

    if( !pcm_handle )
    {
        cout << "----> not find pcm_handle" << endl;
        return NULL;
    }

    gettimeofday( &t_playStart, NULL );
    cout << "----> play start " << t_playStart.tv_sec - t_openPcm.tv_sec + (t_playStart.tv_usec - t_openPcm.tv_usec)/1000000.0f << endl;
    cout << "----> sum time " << t_playStart.tv_sec - t_start.tv_sec + (t_playStart.tv_usec - t_start.tv_usec)/1000000.0f << endl;

    int t_frames, t_timeDiff, t_theoryTime;
    while(true)
    {

        if( dataSize <= playDataSize )
        {
            cout << "play done" << endl;
            break;
        }

        buffer = data + playDataSize;
        t_frames = dataSize - playDataSize > pcm_handle->buffer_size ? pcm_handle->frames : ( dataSize - playDataSize ) / datablock;

        while( (ret = snd_pcm_writei(pcm_handle->handle, buffer, t_frames)) < 0 )
        {
            if( ret == -EPIPE )
            {
                cout << "underrun occurred" << endl;
                //完成硬件参数设置，使设备准备好
                snd_pcm_prepare(pcm_handle->handle);
                // xrun( pcm_handle->handle, monotonic );
            }
            else if( ret < 0 )
            {
                cout << "error from writei: " << snd_strerror(ret) <<endl;
            }
        }

        // cout << "t_frames: " << t_frames << ", ret: " << ret << endl;

        playDataSize += ret * datablock;
    }

    cout << "play Data size: " << playDataSize << endl;

    close_pcm( pcm_handle );    

    for( int i = 0; i < PCM_HANDLE_POOL_SIZE; ++i )
    {
        if( s_pcm_handle_pool[i] == NULL )
        {
            pcm_handle_t * t_handle = (pcm_handle_t*)malloc( sizeof( pcm_handle_t ) );
            open_pcm( t_handle, wav );
            s_pcm_handle_pool[i] = t_handle;
        }
    }

    return NULL;
}

void * randomPlay(void * s)
{
    int i = 0;
    int index = 1;
    string wav_path;
    srand(time(NULL)); 
    char buffer[10];
    pthread_t pthread_id;
    int _sleep = *(int *)s;

    while( i++ < 100 )
    {
        index = rand() % 6 + 1;
        memset( buffer, 0, sizeof(buffer) );
        sprintf(buffer,"%d",index);
        wav_path = string("/usr/bin/audio/") + string(buffer) + ".wav";
        if( !pthread_create(&pthread_id, NULL, playWav, (void *)wav_path.c_str()) )
        {
            cout << "playing " << wav_path << "..." << endl;
        }
        usleep( _sleep );
    }

    return NULL;
}

