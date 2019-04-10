#include "wav.h"
#include "log.h"
#include <cstring>

wav_t * openWav( const char * p_filePath )
{
    char buffer[256];
    int  read_len = 0;
    int  offset = 0;
    wav_t * t_result = (wav_t *)malloc( sizeof(wav_t) );
    FILE * t_fp = nullptr;

    bool t_parseWavRes = false;

    do
    {
        if(!t_result)
        {
            err( "Error malloc wav failedly" );
            break;
        }
        bzero(t_result, sizeof(wav_t));

        t_fp = fopen(p_filePath, "rb");
        if(!t_fp)
        {
            err( "open " << p_filePath << " failed");
            break;
        }

        read_len = fread(buffer, 1, 12, t_fp);
        if(read_len < 12)
        {
            err( "error wav file" );
            break;
        }

        if(strncasecmp("RIFF", buffer, 4))
        {
            err( "file style is not wav" );
            break;
        }

        memcpy(t_result->riff.id, buffer, 4); 
        t_result->riff.size = *(int *)(buffer + 4);
        if(strncasecmp("WAVE", buffer + 8, 4))
        {
            err( "Error wav file" );
            break;
        }

        memcpy(t_result->riff.type, buffer + 8, 4);
        t_result->file_size = t_result->riff.size + 8;
        offset += 12;

        while(true)
        {
            char id_buffer[5] = {0};
            int  tmp_size = 0;

            read_len = fread(buffer, 1, 8, t_fp);  
            if(read_len < 8)
            {
                err( "Error wav file" );
                break;
            }
            memcpy(id_buffer, buffer, 4);
            tmp_size = *(int *)(buffer + 4);

            if(!strncasecmp("FMT", id_buffer, 3))
            {
                memcpy(t_result->format.id, id_buffer, 3);
                t_result->format.size = tmp_size;
                read_len = fread(buffer, 1, tmp_size, t_fp);
                if(read_len < tmp_size)
                {
                    err( "Error wav file" );
                    break;
                }
                t_result->format.compression_code  = *(short *)buffer;
                t_result->format.channels          = *(short *)(buffer + 2);
                t_result->format.samples_per_sec   = *(int *)(buffer + 4);
                t_result->format.avg_bytes_per_sec = *(int *)(buffer + 8);
                t_result->format.block_align       = *(short *)(buffer + 12);
                t_result->format.bits_per_sample   = *(short *)(buffer + 14);
            }
            else if(!strncasecmp("DATA", id_buffer, 3))
            {
                memcpy(t_result->data.id, id_buffer, 4); 
                t_result->data.size = tmp_size;
                offset += 8;
                t_result->data_offset = offset;
                t_result->data_size = t_result->data.size; 
                t_parseWavRes = true;
                break;
            }else
            {
                fseek(t_fp, tmp_size, SEEK_CUR);
            }
            offset += 8 + tmp_size;
        }

        if( !t_parseWavRes )
        {
            break;
        }

        t_result->data_buffer = (char *)malloc( t_result->data.size );
        fseek( t_fp, t_result->data_offset, SEEK_SET);

        read_len = fread( t_result->data_buffer, 1, t_result->data.size,t_fp );
        if( read_len != t_result->data.size )
        {
            err( "read wav data error" );
        }
    }while(0);

    if( !t_parseWavRes )
    {
        closeWav( &t_result );
    }

    if( t_fp )
    {
        fclose(t_fp);
        t_fp = nullptr;
    }

    return t_result;

}

bool closeWav( wav_t ** p_wav )
{
    wav_t * t_wav = *p_wav;

    if( !t_wav )
    {
        return false;
    }

    if( t_wav->data_buffer )
    {
        free( t_wav->data_buffer );
        t_wav->data_buffer = nullptr;
    }

    free( t_wav );
    t_wav = nullptr;

    return true;
}