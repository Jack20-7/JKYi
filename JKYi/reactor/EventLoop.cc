#include"JKYi/reactor/EventLoop.h"
#include"JKYi/log.h"
#include"JKYi/mutex.h"
#include"JKYi/reactor/channel.h"
#include"JKYi/reactor/poller.h"
#include"JKYi/reactor/timerQueue.h"
#include"JKYi/reactor/poller/epollPoller.h"
#include"JKYi/macro.h"

#include<algorithm>
#include<signal.h>
#include<sys/eventfd.h>
#include<unistd.h>

namespace {

__thread JKYi::net::EventLoop* t_loopInThisThread = nullptr; //在每一个线程中存储当前线程的eventloop

static JKYi::Logger::ptr g_logger = JKYI_LOG_NAME("system");

const int kPollTimeMs = 10000;

int createEventfd(){
  int evtfd = ::eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
  if(evtfd < 0){
      JKYI_LOG_ERROR(g_logger) << " Failed in eventfd";
      abort();
  }
  return evtfd;
}

//编译的时候忽略 wold-style-cast
#pragma GCC diagnostic ignored "-Wold-style-cast"
//忽略sigpipe信号
class IgnoreSigPipe{
public:
    IgnoreSigPipe(){
        ::signal(SIGPIPE,SIG_IGN);
    }
};
#pragma GCC diagnostic error "-Wold-style-cast";

IgnoreSigPipe initObj;

}
namespace JKYi{
namespace net{

EventLoop* EventLoop::getEventLoopOfCurrentThread(){
    return t_loopInThisThread;
}

EventLoop::EventLoop()
    :looping_(false),
     quit_(false),
     eventHandling_(false),
     callingPendingFunctors_(false),
     iteration_(0),
     threadId_(CurrentThread::tid()),
     poller_(new EpollPoller(this)),
     timerQueue_(new TimerQueue(this)),
     wakeupFd_(createEventfd()),
     wakeupChannel_(new Channel(this,wakeupFd_)),
     currentActiveChannel_(nullptr){
     JKYI_LOG_DEBUG(g_logger) << " EventLoop created " << this 
                             << "in thread" << threadId_;

    if(t_loopInThisThread){
        JKYI_LOG_DEBUG(g_logger) << " Anthor EventLoop " << t_loopInThisThread
                                 << " exists in this thread " << threadId_;
    }else{
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop(){
    JKYI_LOG_DEBUG(g_logger) << " EventLoop " << this << " of thread " << threadId_
                             << " destructs in thread " << CurrentThread::tid();
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop(){
    JKYI_ASSERT(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    JKYI_LOG_DEBUG(g_logger) << " EventLoop " << this << "start looping";

    while(!quit_){
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs,&activeChannels_);
        ++iteration_;
        eventHandling_ = true;
        for(Channel* channel : activeChannels_){
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = nullptr;
        eventHandling_ = false;
        doPendingFunctors();
    }
    JKYI_LOG_DEBUG(g_logger) << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit(){
    quit_ = true;
    if(!isInLoopThread()){
        wakeup();
    }
    return ;
}

void EventLoop::runInLoop(Functor cb){
    if(isInLoopThread()){
        cb();
    }else{
        queueInLoop(std::move(cb));
    }
    return ;
}

void EventLoop::queueInLoop(Functor cb){
    {
        Mutex::Lock lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    if(!isInLoopThread() || callingPendingFunctors_){
        wakeup();
    }
}

size_t EventLoop::queueSize()const{
    Mutex::Lock lock(mutex_);
    return pendingFunctors_.size();
}

TimerId EventLoop::runAt(Timestamp time,TimerCallback cb){
    return timerQueue_->addTimer(std::move(cb),time,0.0);
}

TimerId EventLoop::runAfter(double delay,TimerCallback cb){
    Timestamp time(addTime(Timestamp::now(),delay));
    return timerQueue_->addTimer(std::move(cb),time,0.0);
}

TimerId EventLoop::runEvery(double interval,TimerCallback cb){
    Timestamp time(addTime(Timestamp::now(),interval));
    return timerQueue_->addTimer(std::move(cb),time,interval);
}

void EventLoop::cancel(TimerId timerId){
    return timerQueue_->cancel(timerId);
}

void EventLoop::wakeup(){
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_,&one,sizeof(one));
    if(n != sizeof(one)){
        JKYI_LOG_ERROR(g_logger) << " EventLoop::wakeup() write " << n 
                                 << " bytes instead of 8";
    }
    return ;
}

void EventLoop::updateChannel(Channel* channel){
    JKYI_ASSERT(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel* channel){
    JKYI_ASSERT(channel->ownerLoop() == this);
    assertInLoopThread();
    if(eventHandling_){
       JKYI_ASSERT(currentActiveChannel_ == channel || 
               std::find(activeChannels_.begin(),activeChannels_.end(),channel) 
               == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel){
    JKYI_ASSERT(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread(){
    JKYI_LOG_ERROR(g_logger) << "EventLoop::abortInNotLoopThread EventLoop = "
                             << this << " was created in threadId " << threadId_
                             << " ,current thread id = " << CurrentThread::tid();
}
void EventLoop::handleRead(){
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_,&one,sizeof(one));
    if(n != sizeof(one)){
        JKYI_LOG_ERROR(g_logger) << " EventLoop::handleRead read " << n 
                                 << " bytes instead of 8 ";
    }
}

void EventLoop::doPendingFunctors(){
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        Mutex::Lock lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(const Functor& functor : functors){
        functor();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels()const{
    for(const Channel* channel : activeChannels_){
        JKYI_LOG_DEBUG(g_logger) << " { " << channel->reventsToString() << " }";
    }
}







}
}

