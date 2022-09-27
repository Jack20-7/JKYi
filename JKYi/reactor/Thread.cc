#include"JKYi/reactor/Thread.h"
#include"JKYi/reactor/CurrentThread.h"
#include"JKYi/log.h"
#include"JKYi/macro.h"

#include<type_traits>
#include<errno.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/prctl.h>
#include<sys/types.h>
#include<linux/unistd.h>

namespace JKYi{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

namespace detail{

pid_t gettid(){
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

//程序在fork之后需要通过调用该函数来清理从父进程拷贝而来的消息
void afterFork(){
    JKYi::CurrentThread::t_cachedTid = 0;
    JKYi::CurrentThread::t_threadName = "main";
    CurrentThread::tid();
}

class ThreadNameInitializer{
public:
    ThreadNameInitializer(){
       JKYi::CurrentThread::t_threadName = "main";
       CurrentThread::tid();
       pthread_atfork(NULL,NULL,&afterFork);
    }
};

ThreadNameInitializer init;

struct ThreadData{
    typedef JKYi::net::Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t* tid_;
    CountDownLatch* latch_;

    ThreadData(ThreadFunc func,const std::string& name,pid_t* tid,
               CountDownLatch* latch)
        :func_(std::move(func)),
         name_(name),
         tid_(tid),
         latch_(latch){}

    //创建的线程底层真正跑的函数
    void runInThread(){
        *tid_ = JKYi::CurrentThread::tid();
        tid_ = nullptr;
        latch_->countDown();
        latch_ = nullptr;

        JKYi::CurrentThread::t_threadName = name_.empty() ? "JKYi-Thread" 
                                                          : name_.c_str();
        //应该是给线程设置名称
        ::prctl(PR_SET_NAME,JKYi::CurrentThread::t_threadName);
        try{
            //执行用户指定的函数
            func_();
            JKYi::CurrentThread::t_threadName = "finished";
        }catch(const std::exception& ex){
            JKYi::CurrentThread::t_threadName = "crashed";
            fprintf(stderr,"exception caught in Thread %s\n",name_.c_str());
            fprintf(stderr,"reason : %s\n",ex.what());
            abort();
        }catch(...){
            JKYi::CurrentThread::t_threadName = "crashed";
            fprintf(stderr,"unknow exception caught in Thread %s\n",name_.c_str());
            throw;
        }
    }
};

void * startThread(void * obj){
    ThreadData * data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return nullptr;
}
} // namespace detail

namespace CurrentThread{

void cacheTid(){
    if(t_cachedTid == 0){
        t_cachedTid = detail::gettid();
        t_tidStringLength = snprintf(t_tidString,sizeof(t_tidString),"%5d ",
                                     t_cachedTid);
    }
}
//多线程程序中进行的pid == 主线程的pid
bool isMainThread(){
    return tid() == ::getpid();
}

}

namespace net{
AtomicInt32 JKYi::net::Thread::numCreated_;

Thread::Thread(ThreadFunc func,const std::string& name)
    :started_(false),
     joined_(false),
     pthreadId_(0),
     tid_(0),
     func_(std::move(func)),
     name_(name),
     latch_(1){

     setDefaultName();
}

Thread::~Thread(){
    if(started_ && !joined_){
        pthread_detach(pthreadId_);
    }
}

void Thread::setDefaultName(){
    int num = numCreated_.incrementAndGet();
    if(name_.empty()){
        char buf[32];
        snprintf(buf,sizeof(buf),"Thread%d",num);
        name_ = buf;
    }
}
void Thread::start(){
    JKYI_ASSERT(!started_);
    started_ = true;
    JKYi::detail::ThreadData* data = 
                          new JKYi::detail::ThreadData(func_,name_,&tid_,&latch_);
    if(pthread_create(&pthreadId_,NULL,&JKYi::detail::startThread,data)){
        started_ = false;
        delete data;
        JKYI_LOG_ERROR(g_logger) << " Failed in pthread_create";
    }else{
        latch_.wait();  //等待子线程的tid被设置好
        JKYI_ASSERT(tid_ > 0);
    }
}

int Thread::join(){
    JKYI_ASSERT(started_);
    JKYI_ASSERT(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_,NULL);
}

}

}
