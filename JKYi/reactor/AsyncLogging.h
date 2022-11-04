#ifndef _JKYI_NET_ASYNCLOGGING_H_
#define _JKYI_NET_ASYNCLOGGING_H_

#include"JKYi/BlockingQueue.h"
#include"JKYi/BoundedBlockingQueue.h"
#include"JKYi/CountDownLatch.h"
#include"JKYi/mutex.h"
#include"JKYi/reactor/Thread.h"
#include"JKYi/reactor/LogStream.h"
#include"JKYi/noncopyable.h"

#include<atomic>
#include<vector>

namespace JKYi{
namespace net{

//日志功能的核心类，用来完成异步日志
class AsyncLogging :public Noncopyable{
public:
    AsyncLogging(const std::string& basename,
                       off_t rollSize,
                       int flushInterval = 2);
    ~AsyncLogging(){
        if(running_){
            stop();
        }
    }

    void append(const char* logline,int len);
    void start(){
        running_ = true;
        thread_.start();
        latch_.wait();
    }
    void stop(){
        running_ = false;
        cond_.notify();
        thread_.join();
    }
private:
    void threadFunc();

    typedef JKYi::net::detail::FixedBuffer<JKYi::net::detail::kLargeBuffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    const int flushInterval_;           //刷盘的间隔
    std::atomic<bool> running_;
    const std::string basename_;       //日志文件的名称
    const off_t rollSize_;             //更换目标文件的大小
    JKYi::net::Thread thread_;
    JKYi::CountDownLatch latch_;
    JKYi::Mutex mutex_;
    JKYi::Condition cond_;

    //前台的buffer
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
};

}
}

#endif
