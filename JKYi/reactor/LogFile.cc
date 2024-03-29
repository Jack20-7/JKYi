#include"JKYi/reactor/LogFile.h"
#include"JKYi/reactor/FileUtil.h"

#include<assert.h>
#include<stdio.h>
#include<time.h>

namespace JKYi{
namespace net{

LogFile::LogFile(const std::string& filename,
                 off_t rollSize,
                 bool threadSafe,
                 int flushInterval,
                 int checkEveryN)
    :basename_(filename),
     rollSize_(rollSize),
     flushInterval_(flushInterval),
     checkEveryN_(checkEveryN),
     count_(0),
     mutex_(threadSafe ? new JKYi::Mutex : NULL),
     startOfPeriod_(0),
     lastRoll_(0),
     lastFlush_(0){

     assert(filename.find('/') == std::string::npos);
     rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline,int len){
    if(mutex_){
        JKYi::Mutex::Lock lock(*mutex_);
        append_unlocked(logline,len);
    }else{
        append_unlocked(logline,len);
    }
    return ;
}
void LogFile::flush(){
    if(mutex_){
        JKYi::Mutex::Lock lock(*mutex_);
        file_->flush();
    }else{
        file_->flush();
    }
    return ;
}

void LogFile::append_unlocked(const char* logline,int len){
    file_->append(logline,len);
    if(file_->writtenBytes() > rollSize_){
        rollFile();
    }else{
        ++count_;
        if(count_ >= checkEveryN_){
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod = now / kRollPerSeconds_ * kRollPerSeconds_;
            if(thisPeriod != startOfPeriod_){
                rollFile();
            }else if(now - lastFlush_ > flushInterval_){
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
    return ;
}

bool LogFile::rollFile(){
    time_t now = 0;
    std::string filename = getLogFileName(basename_,&now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;
    if(now > lastRoll_){
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new AppendFile(filename));
        return true;
    }
    return false;
}

std::string LogFile::getLogFileName(const std::string& basename,time_t* now){
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now,&tm);
    strftime(timebuf,sizeof timebuf,".%Y%m%d-%H%M%S",&tm);
    filename += timebuf;
    filename += ".log";

    return filename;
}


}
}
