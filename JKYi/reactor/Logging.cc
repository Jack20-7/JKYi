#include"JKYi/reactor/Logging.h"
#include"JKYi/reactor/CurrentThread.h"
#include"JKYi/timestamp.h"
#include"JKYi/reactor/AsyncLogging.h"

#include<errno.h>
#include<stdio.h>
#include<string.h>
#include<sstream>

namespace JKYi{
namespace net{

Logger::LogLevel g_logLevel = Logger::LogLevel::DEBUG;


const char* LogLevelName[Logger::NUM_LOG_LEVELS] = {
    "[TRACE] ",
    "[DEBUG] ",
    "[INFO]  ",
    "[WARN]  ",
    "[ERROR] ",
    "[FATAL] ",
};

//helper class for known string length at compile time
class T{
public:
    T(const char* str,unsigned len)
      :str_(str),
       len_(len){
       assert(strlen(str) == len);
    }
    const char* str_;
    const unsigned len_;
};

//重载<<
inline LogStream& operator<< (LogStream& s,T v){
    s.append(v.str_,v.len_);
    return s;
}

//默认是输出到屏幕
void defaultOutput (const char* msg,int len){
    size_t n = fwrite(msg,1,len,stdout);
    (void)n;
}
void defaultFlush(){
    fflush(stdout);
}

static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static AsyncLogging* asyncLogger;

void once_init(){
    asyncLogger = new AsyncLogging("AsyncLog",10 * 1024 *1024);
    asyncLogger->start();
}

void outputToFile(const char* msg,int len){
    pthread_once(&once_control,once_init);
    asyncLogger->append(msg,len);
}

Logger::outputFunc g_output = outputToFile;
Logger::FlushFunc g_flush = defaultFlush;

//输出日志格式
//time tidname level data   filename line
Logger::Impl::Impl(LogLevel level,const std::string& filename,int line)
    :stream_(),
     level_(level),
     line_(line),
     filename_(filename){

     formatTime();
     CurrentThread::tid();
     stream_ << T(CurrentThread::tidString(),CurrentThread::tidStringLength());
     stream_ << T(LogLevelName[level],8);
}

void Logger::Impl::formatTime(){
    struct timeval tv;
    time_t time;
    char str_t[26] = {0};
    gettimeofday(&tv,NULL);
    time = tv.tv_sec;
    struct tm* p_time = localtime(&time);
    strftime(str_t,26,"%Y-%m-%d %H:%M:%S\t",p_time);

    stream_ << str_t;
}

void Logger::Impl::finish(){
    stream_ << " - " << filename_ << ':' << line_ << '\n';
}

Logger::Logger(const std::string& filename,int line)
    :impl_(INFO,filename,line){}
Logger::Logger(const std::string& filename,int line,LogLevel level)
    :impl_(level,filename,line){}

Logger::~Logger(){
    impl_.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(),buf.length());
    if(impl_.level_ == FATAL){
        g_flush();
        abort();
    }
}

void Logger::setLogLevel(Logger::LogLevel level){
    g_logLevel = level;
}
void Logger::setOutput(outputFunc out){
    g_output = out;
}
void Logger::setFlush(FlushFunc flush){
    g_flush = flush;
}

}//namespace net
}//namespace JKYi
