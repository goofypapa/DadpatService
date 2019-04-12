#include "audioPcm.h"
#include "log.h"

#define PCM_POLL_MAX_COUNT 6


#define PCM_DEFAULT_BIT 16
#define PCM_DEFAULT_DATABLOCK 4
#define PCM_DEFAULT_RATE 44100
#define PCM_DEFAULT_CHANNELS 2

std::queue< pcm_handle_t * > s_audioPcmPool;


bool audioPcmInitPcmPoll( void )
{

    pcm_handle_t * t_pcm_handle = NULL;

    while( s_audioPcmPool.size() < PCM_POLL_MAX_COUNT )
    {
        info( "-------------> " << s_audioPcmPool.size() );
        t_pcm_handle = _openPcm( PCM_DEFAULT_BIT, PCM_DEFAULT_DATABLOCK, PCM_DEFAULT_RATE, PCM_DEFAULT_CHANNELS );
        assert( t_pcm_handle );
        s_audioPcmPool.push( t_pcm_handle );
    }

    return true;
}

pcm_handle_t * openPcm( int p_bit, int p_datablock, int p_rate, int p_channels )
{
    pcm_handle_t * t_result = NULL;

    if( p_bit == PCM_DEFAULT_BIT && p_datablock == PCM_DEFAULT_DATABLOCK && p_rate == PCM_DEFAULT_RATE && p_channels == PCM_DEFAULT_CHANNELS )
    {
        if( s_audioPcmPool.size() )
        {
            t_result = s_audioPcmPool.front();
            s_audioPcmPool.pop();
        }
    }else{
        t_result = _openPcm( p_bit, p_datablock, p_rate, p_channels );
    }

    return t_result;
}

pcm_handle_t * _openPcm( int p_bit, int p_datablock, int p_rate, int p_channels )
{
    int t_rc, t_monotonic = 0;
    pcm_handle_t * t_result = ( pcm_handle_t * )malloc( sizeof( pcm_handle_t ) );
    snd_pcm_hw_params_t * params;//硬件信息和PCM流配置
    unsigned t_setRate = p_rate, t_bufferTime = 0, t_periodTime = 0;

    do
    {
        //SND_PCM_NONBLOCK 
        t_rc = snd_pcm_open(&t_result->handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
        if( t_rc < 0 )
        {
            err( "Error open PCM device failed" );
            break;
        }

        snd_pcm_hw_params_alloca(&params); //分配params结构体

        t_rc = snd_pcm_hw_params_any(t_result->handle, params);//初始化params
        if( t_rc < 0 )
        {
            err( "Error snd_pcm_hw_params_any" );
            break;
        }

        t_rc = snd_pcm_hw_params_set_access(t_result->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED); //初始化访问权限
        if( t_rc < 0 )
        {
            err( "Error snd_pcm_hw_params_set_access" );
            break;
        }

        switch( p_bit / 8 )
        {
            case 1:
                snd_pcm_hw_params_set_format(t_result->handle, params, SND_PCM_FORMAT_U8);
                break;
            case 2:
                snd_pcm_hw_params_set_format(t_result->handle, params, SND_PCM_FORMAT_S16_LE);
                break;
            case 3:
                //SND_PCM_FORMAT_S24_3BE
                snd_pcm_hw_params_set_format(t_result->handle, params, SND_PCM_FORMAT_S24_3LE);
                break;
            case 4:
                snd_pcm_hw_params_set_format(t_result->handle, params, SND_PCM_FORMAT_FLOAT);
                break;
        }

        t_rc = snd_pcm_hw_params_set_channels(t_result->handle, params, p_channels); //设置声道,1表示单声>道，2表示立体声
        if( t_rc < 0 )
        {
            err( "Error snd_pcm_hw_params_set_channels" );
            break;
        }

        t_rc = snd_pcm_hw_params_set_rate_near(t_result->handle, params, &t_setRate, 0);
        if( t_rc < 0 )
        {
            err( "Error snd_pcm_hw_params_set_rate_near" );
            break;
        }

        t_rc = snd_pcm_hw_params_get_buffer_time_max(params, &t_bufferTime, 0);
        assert(t_rc >= 0);

        if (t_bufferTime > 500000)
            t_bufferTime = 500000;

        t_periodTime = t_bufferTime / 4;

        t_rc = snd_pcm_hw_params_set_period_time_near(t_result->handle, params, &t_periodTime, 0);
        if( t_rc < 0 )
        {
            err( "Error snd_pcm_hw_params_set_period_time_near" );
            break;
        }
        t_rc = snd_pcm_hw_params_set_buffer_time_near(t_result->handle, params, &t_bufferTime, 0);
        if( t_rc < 0 )
        {
            err( "Error snd_pcm_hw_params_set_buffer_time_near" );
            break;
        }

        t_monotonic = snd_pcm_hw_params_is_monotonic(params);
        t_result->can_pause = snd_pcm_hw_params_can_pause(params);
        t_rc = snd_pcm_hw_params(t_result->handle, params);
        if( t_rc < 0 )
        {
            err( "Error snd_pcm_hw_params" );
            break;
        }

        t_rc = snd_pcm_hw_params_get_period_size(params, &t_result->frames, 0); /*获取周期长度*/
        if( t_rc < 0 )
        {
            err( "Error snd_pcm_hw_params_get_period_size" );
            break;
        }

        t_rc = snd_pcm_hw_params_get_buffer_size(params, &t_result->buffer_size);
        if( t_rc < 0 )
        {
            err( "Error snd_pcm_hw_params_get_buffer_size" );
            break;
        }

        t_result->buffer_size = t_result->frames * p_datablock; /*4 代表数据快长度*/

    } while ( 0 );


    if( t_rc <  0 )
    {
        closePcm( &t_result );
    }

    return t_result;
}


bool closePcm( pcm_handle_t ** p_pcm_handle )
{

    pcm_handle_t * t_pcm = *p_pcm_handle;
    
    if( t_pcm )
    {

        snd_pcm_drain( t_pcm->handle );
        snd_pcm_close( t_pcm->handle );

        free( t_pcm );
        t_pcm = NULL;
    }

    pcm_handle_t * t_pcm_handle = _openPcm( PCM_DEFAULT_BIT, PCM_DEFAULT_DATABLOCK, PCM_DEFAULT_RATE, PCM_DEFAULT_CHANNELS );
    assert( t_pcm_handle );
    s_audioPcmPool.push( t_pcm_handle );

    return true;
}