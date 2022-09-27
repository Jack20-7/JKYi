#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/EventLoopThread.h"
#include"JKYi/macro.h"
#include"JKYi/condition.h"

namespace JKYi{
namespace net{

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const std::string& name)
    :loop_(nullptr),
     exiting_(false),
     thread_(std::bind(&EventLoopThread::threadFunc,this),name),
     mutex_(),
     condition_(mutex_),
     callback_(cb){
}

EventLoopThread::~EventLoopThread(){
    exiting_ = true;
    if(loop_ != nullptr){      //这里由于没有加锁，所以存在竞态问题
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop(){
    JKYI_ASSERT(!thread_.started());
    thread_.start();

    EventLoop* loop = nullptr;
    {
        Mutex::Lock lock(mutex_);
        while(loop_ == nullptr){
            condition_.wait();
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc(){
    EventLoop loop;

    if(callback_){
        callback_(&loop);
    }

    {
        Mutex::Lock lock(mutex_);
        loop_  = &loop;
        condition_.notify();
    }

    loop_->loop();
    Mutex::Lock lock(mutex_);
    loop_ = nullptr;
}

}
}
