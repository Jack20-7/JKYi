#ifndef _BLOCKINGQUEUE_H_
#define _BLOCKINGQUEUE_H_

#include"JKYi/condition.h"
#include"JKYi/mutex.h"
#include"JKYi/macro.h"

#include<deque>

namespace JKYi{

//阻塞队列,无边界的阻塞队列
template<class T>
class BlockingQueue : Noncopyable{
public:
    BlockingQueue()
       :mutex_(),
        condition_(mutex_),
        queue_(){
    }

    void push(const T& t){
        Mutex::Lock lock(mutex_);
        queue_.push_back(t);
        condition_.notify();
    }
    void push(T&& t){
        Mutex::Lock lock(mutex_);
        queue_.push_back(std::move(t));
        condition_.notify();
    }

    T take(){
        Mutex::Lock lock(mutex_);
        while(queue_.empty()){
            condition_.wait();
        }
        JKYI_ASSERT(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        return front();
    }

    std::deque<T> drain(){
        std::deque<T> queue;
        {
            Mutex::Lock lock(mutex_);
            queue = std::move(queue_);
            JKYI_ASSERT(queue_.empty());
        }
        return queue;
    }

    size_t size()const{
        Mutex::Lock lock(mutex_);
        return queue_.size();
    }
private:
    mutable Mutex mutex_;
    Condition condition_;
    std::deque<T> queue_;
};

}

#endif
