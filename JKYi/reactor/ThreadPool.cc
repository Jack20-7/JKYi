#include"JKYi/reactor/ThreadPool.h"

#include<assert.h>
#include<stdio.h>

using namespace JKYi;
using namespace JKYi::net;

ThreadPool::ThreadPool(const std::string& name)
    :notEmpty_(mutex_),
     notFull_(mutex_),
     name_(name),
     maxQueueSize_(0),
     running_(false){
}

ThreadPool::~ThreadPool(){
    if(running_){
        stop();
    }
}

void ThreadPool::start(int numThreads){
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);

    for(int i = 0;i < numThreads;++i){
        char id[32];
        snprintf(id,sizeof id,"%d",i + 1);
        threads_.emplace_back(new Thread(
                    std::bind(&ThreadPool::runInThread,this),name_ + id));
        threads_[i]->start();
    }
    if(numThreads == 0 && threadInitCallback_){
        threadInitCallback_();
    }
}

void ThreadPool::stop(){
  {
     Mutex::Lock lock(mutex_);
     running_ = false;
     notEmpty_.notifyAll();
     notFull_.notifyAll();
  }

  for(auto& thr : threads_){
      thr->join();
  }
}

size_t ThreadPool::queueSize()const{
    Mutex::Lock lock(mutex_);
    return queue_.size();
}

void ThreadPool::run(Task f){
    //如果当前线程池中没有子线程的话，那么就主线程自己去执行任务
    if(threads_.empty()){
        f();
    }else{
        Mutex::Lock lock(mutex_);
        while(isFull() && running_){
            notFull_.wait();
        }
        if(!running_){
            return ;
        }
        assert(!isFull());
        queue_.push_back(std::move(f));
        notEmpty_.notify();
    }
}

bool ThreadPool::isFull()const{
    Mutex::Lock lock(mutex_);
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

ThreadPool::Task ThreadPool::take(){
    Mutex::Lock lock(mutex_);
    while(queue_.empty() && running_){
        notEmpty_.wait();
    }
    Task f;
    if(!queue_.empty()){
        f = queue_.front();
        queue_.pop_front();
        if(maxQueueSize_ > 0){
            notFull_.notify();
        }
    }
    return f;
}

void ThreadPool::runInThread(){
    try{
        if(threadInitCallback_){
            threadInitCallback_();
        }
        while(running_){
            Task f = take();
            if(f){
                f();
            }
        }
    }catch (...){
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw;
    }
    return ;
}

