#ifndef _JKYI_NET_THREAD_H_
#define _JKYI_NET_THREAD_H_
#include"JKYi/atomic.h"
#include"JKYi/CountDownLatch.h"

#include<functional>
#include<memory>
#include<pthread.h>

namespace JKYi{
namespace net{

//内部封装的线程
class Thread : public Noncopyable{
public:
    typedef std::function<void ()> ThreadFunc;

    explicit Thread(ThreadFunc ,const std::string& name = std::string());
    ~Thread();

    void start();
    int join();

    bool started()const { return started_; }
    pid_t tid()const { return tid_; }
    const std::string& name(){ return name_; }

    static int numCreated(){ return numCreated_.get(); }
private:
    void setDefaultName();

    bool started_;
    bool joined_;
    pthread_t pthreadId_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    CountDownLatch latch_;          //用来让主线程等待子线程得到自己的tid之后在往下执行

    static AtomicInt32 numCreated_; // 维护当前创建的线程数目
};
}
}
#endif
