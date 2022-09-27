#include"JKYi/reactor/EventLoopThreadPool.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/EventLoopThread.h"
#include"JKYi/macro.h"

#include<stdio.h>

namespace JKYi{
namespace net{

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop,
                                         const std::string& nameArg)
    :baseLoop_(baseloop),
     name_(nameArg),
     started_(false),
     numThreads_(0),
     next_(0){
}

EventLoopThreadPool::~EventLoopThreadPool(){
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb){
    JKYI_ASSERT(!started_);
    baseLoop_->assertInLoopThread();
    started_ = true;
    //创建线程
    for(int i = 0;i < numThreads_;++i){
        char buf[name_.size() + 32];
        snprintf(buf,sizeof(buf),"%s%d",name_.c_str(),i);
        EventLoopThread* t = new EventLoopThread(cb,buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }
    if(numThreads_ == 0 && cb){
        cb(baseLoop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop(){
    baseLoop_->assertInLoopThread();
    JKYI_ASSERT(started_);

    EventLoop* loop = baseLoop_; // 默认返回主EventLoop
    if(!loops_.empty()){
        loop = loops_[next_];
        next_++;
        if(static_cast<size_t>(next_) >= loops_.size()){
            next_ = 0;
        }
    }
    return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode){
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if(!loops_.empty()){
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops(){
    baseLoop_->assertInLoopThread();
    JKYI_ASSERT(started_);
    if(loops_.empty()){
        return std::vector<EventLoop*>(1,baseLoop_);
    }else{
        return loops_;
    }
}

}
}
