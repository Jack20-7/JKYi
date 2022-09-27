#ifndef _JKYI_EVENTLOOP_H_
#define _JKYI_EVENTLOOP_H_

#include"JKYi/mutex.h"
#include"JKYi/timestamp.h"
#include"JKYi/reactor/timerId.h"
#include"JKYi/reactor/Callbacks.h"
#include"boost/any.hpp"
#include"JKYi/reactor/CurrentThread.h"

#include<atomic>
#include<functional>
#include<vector>
#include<memory>

namespace JKYi{
namespace net{

class Channel;
class Poller;
class TimerQueue;

class EventLoop : public Noncopyable{
public:
    typedef std::function<void()> Functor;

    EventLoop();
    ~EventLoop();

    void loop();

    void quit();

    Timestamp pollReturnTime()const { return pollReturnTime_; }
    int64_t iteration()const { return iteration_; }

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    size_t queueSize()const;

    TimerId runAt(Timestamp time,TimerCallback cb);
    TimerId runAfter(double delay,TimerCallback cb);
    TimerId runEvery(double interval,TimerCallback cb);
    void cancel(TimerId timerId);

    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    void assertInLoopThread(){
        if(!isInLoopThread()){
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread()const { return threadId_ == CurrentThread::tid(); }
    bool eventHanding()const { return eventHandling_; }

    void setContext(const boost::any& context){
        context_ = context;
    }
    const boost::any& getContext()const{
        return context_;
    }
    
    boost::any* getMutableContext(){
        return &context_;
    }

    static EventLoop* getEventLoopOfCurrentThread();
private:
    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();

    void printActiveChannels()const;

    typedef std::vector<Channel*> ChannelList;

    bool looping_;
    std::atomic<bool> quit_;
    bool eventHandling_;
    bool callingPendingFunctors_;
    int64_t iteration_;                  //poll的次数
    const pid_t threadId_;
    Timestamp pollReturnTime_;

    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    boost::any context_;                   //用来携带一些用户自己的东西
    
    ChannelList activeChannels_;          
    Channel* currentActiveChannel_;

    mutable Mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};
}
}

#endif
