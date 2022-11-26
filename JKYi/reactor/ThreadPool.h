#ifndef _JKYI_NET_THREADPOOL_H_
#define _JKYI_NET_THREADPOOL_H_

#include"JKYi/condition.h"
#include"JKYi/mutex.h"
#include"JKYi/reactor/Thread.h"
#include"JKYi/Types.h"

#include<vector>
#include<deque>
#include<string>
#include<memory>

namespace JKYi{
namespace net{

class ThreadPool : public Noncopyable{
public:
    typedef std::function<void ()> Task;

    explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
    ~ThreadPool();

    void setMaxQueueSize(int size){
        maxQueueSize_ = size;
    }
    void setThreadInitCallback(const Task& cb){
        threadInitCallback_ = cb;
    }

    void start(int numThreads);
    void stop();

    const std::string& name()const{
        return name_;
    }
    size_t queueSize()const;

    void run(Task f);
private:
    bool isFull()const;
    void runInThread();
    Task take();

    mutable Mutex mutex_;
    Condition notEmpty_;
    Condition notFull_;
    std::string name_;

    Task threadInitCallback_;
    std::vector<std::unique_ptr<Thread>> threads_;
    std::deque<Task> queue_;                      //任务队列
    size_t maxQueueSize_;                         //任务队列的最大长度
    bool running_;
};

}
}

#endif
