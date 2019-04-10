#ifndef __WS_LOG_H__
#define __WS_LOG_H__

#include <iostream>
#include <pthread.h>

namespace tools
{

    #define debug( LOG ) tools::limit_lavel <= tools::DEBUG && ws_log( tools::DEBUG, __FILE__, __FUNCTION__, __LINE__ ) << LOG << std::endl
    #define info( LOG ) tools::limit_lavel <= tools::INFO && ws_log( tools::INFO, __FILE__, __FUNCTION__, __LINE__ ) << LOG << std::endl
    #define notice( LOG ) tools::limit_lavel <= tools::NOTICE && ws_log( tools::NOTICE, __FILE__, __FUNCTION__, __LINE__ ) << LOG << std::endl
    #define warn( LOG ) tools::limit_lavel <= tools::WARN && ws_log( tools::WARN, __FILE__, __FUNCTION__, __LINE__ ) << LOG << std::endl
    #define err( LOG ) tools::limit_lavel <= tools::ERR && ws_log( tools::ERR, __FILE__, __FUNCTION__, __LINE__ ) << LOG << std::endl
    #define crit( LOG ) tools::limit_lavel <= tools::CRIT && ws_log( tools::CRIT, __FILE__, __FUNCTION__, __LINE__ ) << LOG << std::endl
    #define alert( LOG ) tools::limit_lavel <= tools::ALERT && ws_log( tools::ALERT, __FILE__, __FUNCTION__, __LINE__ ) << LOG << std::endl
    #define emerg( LOG ) tools::limit_lavel <= tools::EMERG && ws_log( tools::EMERG, __FILE__, __FUNCTION__, __LINE__ ) << LOG << std::endl

    enum LOG_LEVEL{ DEBUG = 0, INFO, NOTICE, WARN, ERR, CRIT, ALERT, EMERG };
    struct _WS_LOG
    {
    private:
        friend std::ostream & ws_log( LOG_LEVEL p_log_level, const char *, const char *, const int );
        static pthread_mutex_t m_mutex;
        static std::ostream * m_stream;
    };

    extern LOG_LEVEL limit_lavel;

    std::ostream & ws_log( LOG_LEVEL p_log_level, const char * p_file, const char * p_function, const int p_line );

}
#endif //__WS_LOG_H__