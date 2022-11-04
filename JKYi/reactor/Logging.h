#ifndef _JKYI_NET_LOGGING_H_
#define _JKYI_NET_LOGGING_H_

#include"JKYi/reactor/LogStream.h"
#include"JKYi/timestamp.h"

namespace JKYi{
namespace net{

class Logger{
public:
    enum LogLevel{
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    Logger(const std::string& filename,int line);
    Logger(const std::string& filename,int line,LogLevel level);
    ~Logger();

    LogStream& stream() { return impl_.stream_; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    typedef void (*outputFunc) (const char* msg,int len);
    typedef void (*FlushFunc) ();

    static void setOutput(outputFunc func);
    static void setFlush(FlushFunc func);
private:
    //pimp技法
    class Impl{
    public:
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level,const std::string& filename,int line);
        void formatTime();
        void finish();

        LogStream stream_;
        LogLevel level_;
        int line_;

        const std::string& filename_;
    };
    Impl impl_;
};

//声明一个外部链接
extern Logger::LogLevel g_logLevel;
inline Logger::LogLevel Logger::logLevel(){
    return g_logLevel;
}

//定义一些便于日志使用的宏
#define LOG_TRACE if(JKYi::net::Logger::logLevel() <= JKYi::net::Logger::TRACE) \
  JKYi::net::Logger(__FILE__,__LINE__,JKYi::net::Logger::TRACE).stream()
#define LOG_DEBUG if(JKYi::net::Logger::logLevel() <= JKYi::net::Logger::DEBUG) \
  JKYi::net::Logger(__FILE__,__LINE__,JKYi::net::Logger::DEBUG).stream()
#define LOG_INFO if(JKYi::net::Logger::logLevel() <= JKYi::net::Logger::INFO) \
  JKYi::net::Logger(__FILE__,__LINE__).stream()
#define LOG_WARN JKYi::net::Logger(__FILE__,__LINE__,JKYi::net::Logger::WARN).stream()
#define LOG_ERROR JKYi::net::Logger(__FILE__,__LINE__,JKYi::net::Logger::ERROR).stream()
#define LOG_FATAL JKYi::net::Logger(__FILE__,__LINE__,JKYi::net::Logger::FATAL).stream()

}//namespace net
}//namespace JKYi

#endif
