#ifndef _JKYI_COUNT_DOWN_LATCH_H_
#define _JKYI_COUNT_DOWN_LATCH_H_

#include"JKYi/mutex.h"
#include"JKYi/condition.h"

namespace JKYi{

class CountDownLatch : Noncopyable{
public:
    explicit CountDownLatch(int count);

    void wait();

    void countDown();

    int getCount()const;
private:
    int count_;
    mutable Mutex mutex_;
    Condition condition_;
};

}

#endif
