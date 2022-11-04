#ifndef _JKYI_BOUNDED_BLOCKING_QUEUE_H_
#define _JKYI_BOUNDED_BLOCKING_QUEUE_H_

#include"JKYi/mutex.h"
#include"condition.h"
#include"JKYi/macro.h"

#include<boost/circular_buffer.hpp>

namespace JKYi{

template<class T>
class BoundedBlockingQueue : Noncopyable{
public:
    explicit BoundedBlockingQueue(int maxSize)
       :mutex_(),
        notEmpty_(mutex_),
        notFull_(mutex_),
        queue_(maxSize){
    }

    void push(const T& t){
        Mutex::Lock lock(mutex_);
        while(queue_.full()){
            notFull_.wait();
        }
        JKYI_ASSERT(!queue_.full());
        queue_.push_back(t);
        notEmpty_.notify();
    }
    void push(T&& t){
        Mutex::Lock lock(mutex_);
        while(queue_.full()){
            notFull_.wait();
        }
        JKYI_ASSERT(!queue_.full());
        queue_.push_back(std::move(t));
        notEmpty_.notify();
    }

    T take(){
        Mutex::Lock lock(mutex_);
        while(queue_.empty()){
            notFull_.wait();
        }
        JKYI_ASSERT(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        notFull_.notify();
        return front();
    }

    bool empty()const{
        Mutex::Lock lock(mutex_);
        return queue_.empty();
    }
    bool full()const{
        Mutex::Lock lock(mutex_);
        return queue_.full();
    }

    size_t size()const{
        Mutex::Lock lock(mutex_);
        return queue_.size();
    }
    size_t capacity()const{
        Mutex::Lock lock(mutex_);
        return queue_.capacity();
    }
private:
    mutable Mutex mutex_;
    Condition notEmpty_;
    Condition notFull_;
    //循环队列
    boost::circular_buffer<T> queue_;
};
}

#endif

