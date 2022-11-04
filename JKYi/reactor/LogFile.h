#ifndef _JKYI_NET_LOGFILE_H_
#define _JKYI_NET_LOGFILE_H_

#include"JKYi/mutex.h"
#include"JKYi/Types.h"

#include<memory>

namespace JKYi{
namespace net{

class AppendFile;
class LogFile : public Noncopyable{
public:
    LogFile(const std::string& filename,
             off_t rollSize,
             bool threadSafe = true,
             int flushInterval = 3,
             int chechEveryN = 1024);
    ~LogFile();

    void append(const char* logline,int len);
    void flush();
    bool rollFile();
private:
    void append_unlocked(const char* logline,int len);
    static std::string getLogFileName(const std::string& name,time_t* now);

    const std::string basename_;
    const off_t rollSize_;          //日志文件的临界大小
    const int flushInterval_;
    const int checkEveryN_;       //对同一个日志文件最多能够写的次数

    int count_;                   //用来记录对一个日志文件进行写操作的次数

    std::unique_ptr<JKYi::Mutex> mutex_;
    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
    std::unique_ptr<AppendFile> file_;

    const static int kRollPerSeconds_ = 60 * 60 * 24; 
};

}
}

#endif
