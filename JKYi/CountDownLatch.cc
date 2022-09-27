#include"JKYi/CountDownLatch.h"

namespace JKYi{
        
CountDownLatch::CountDownLatch(int count)
    :count_(count),
     mutex_(),
     condition_(mutex_){
}

void CountDownLatch::wait(){
    Mutex::Lock lock(mutex_); 
    while(count_ > 0){
        condition_.wait();
    }
    return ;
}

void CountDownLatch::countDown(){
    Mutex::Lock lock(mutex_);
    --count_;
    if(count_ == 0){
        condition_.notifyAll();
    }
}

int CountDownLatch::getCount()const{
    Mutex::Lock lock(mutex_);
    return count_;
}

}
