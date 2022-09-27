#ifndef _JKYI_EVENT_LOOP_THREAD_POOL_H_
#define _JKYI_EVENT_LOOP_THREAD_POOL_H_

#include"JKYi/noncopyable.h"

#include<functional>
#include<memory>
#include<vector>

namespace JKYi{
namespace net{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : public Noncopyable{
public:
    typedef std::function<void (EventLoop*)> ThreadInitCallback;

    EventLoopThreadPool(EventLoop* baseloop,const std::string& nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads){ numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();     //round-robin
    EventLoop* getLoopForHash(size_t hashCode); //same hashcode -> same EventLoop

    std::vector<EventLoop*> getAllLoops();

    bool started()const{
        return started_;
    }
    const std::string& name()const{
        return name_;
    }
private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;          //用于实现round-robin

    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

} //namespace net
} //namespace JKYi


#endif
