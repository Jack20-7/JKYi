#ifndef _JKYI_ATOMIC_H_
#define _JKYI_ATOMIC_H_

#include"JKYi/noncopyable.h"

#include<stdint.h>

namespace JKYi{

//封装一个原子操作类

template<class T>
class AtomicIntegerT : public Noncopyable{
public:
    AtomicIntegerT()
        :value_(0){
    }
    T get(){
        //该函数将第一个参数和第二个参数进行比较，如果相同的，就将第三个参数的值写入到第一个参数中去，并且返回第一个参数的值
        return __sync_val_compare_and_swap(&value_,0,0);
    }

    //将value+x，返回的是增加之前的值。等价于i++
    T getAndAdd(T x){
        return __sync_fetch_and_add(&value_,x);
    }
    //value+x,返回的值增加后的值，等价于++i
    T addAndGet(T x){
        return getAndAdd(x) + x;
    }
    //++i
    T incrementAndGet(){
        return addAndGet(1);
    }
    //--i
    T decrementAndGet(){
        return addAndGet(-1);
    }

    void add(T x){
        getAndAdd(x);
    }
    void increment(){
        incrementAndGet();
    }
    void decrement(){
        decrementAndGet();
    }

    T getAndSet(T newValue){
        return __sync_lock_test_and_set(&value_,newValue);
    }
private:
    volatile T value_;
};

typedef AtomicIntegerT<int32_t> AtomicInt32;
typedef AtomicIntegerT<int64_t> AtomicInt64;

}

#endif
