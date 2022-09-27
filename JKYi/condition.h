#ifndef _JKYI_CONDITION_H_
#define _JKYI_CONDITION_H_

#include"JKYi/mutex.h"
namespace JKYi{

class Condition : public Noncopyable{
public:
    explicit Condition(Mutex& mutex)
        :mutex_(mutex){
        pthread_cond_init(&pcond_,NULL);
    }
    ~Condition(){
        pthread_cond_destroy(&pcond_);
    }

    void wait(){
        pthread_cond_wait(&pcond_,mutex_.getPthreadMutex()); 
    }
    void waitForSeconds(double seconds);

    void notify(){
        pthread_cond_signal(&pcond_);
    }
    void notifyAll(){
        pthread_cond_broadcast(&pcond_);
    }
private:
    Mutex& mutex_;
    pthread_cond_t pcond_;
};
}

#endif
