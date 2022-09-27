#ifndef _JKYI_EVENT_LOOP_THREAD_H_
#define _JKYI_EVENT_LOOP_THREAD_H_

#include"JKYi/reactor/Thread.h"
#include"JKYi/mutex.h"
#include"JKYi/condition.h"

namespace JKYi{
namespace net{

class EventLoop;

class EventLoopThread : public Noncopyable{
public:
    typedef std::function<void (EventLoop*)> ThreadInitCallback;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();
private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    Mutex mutex_;
    Condition condition_;

    ThreadInitCallback callback_;
};

}//namespace net
}//namespace JKYi


#endif
