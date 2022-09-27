#ifndef _JKYI_TIMER_H_
#define _JKYI_TIMER_H_

#include"JKYi/timestamp.h"
#include"JKYi/atomic.h"
#include"JKYi/reactor/Callbacks.h"

#include<functional>

namespace JKYi{
namespace net{

//定义的定时器类
class Timer : Noncopyable{
public:
    Timer(TimerCallback cb,Timestamp when,double interval)
        :callback_(std::move(cb)),
         expiration_(when),
         interval_(interval),
         repeat_(interval > 0),
         sequence_(s_numCreated_.incrementAndGet()){
     }

     void run()const{
         callback_();
     }

     Timestamp getExpiration()const { return expiration_; }
     bool isRepeat()const { return repeat_; }
     int64_t getSequence()const { return sequence_; }

     void restart(Timestamp now);

     static int64_t getNumCreated(){ return s_numCreated_.get(); }
private:
    const TimerCallback callback_;
    Timestamp expiration_;          //过期时间
    const double interval_;         //触发的时间间隔
    const bool repeat_;             //是否需要循环触发
    const int64_t sequence_;        //

    static AtomicInt64 s_numCreated_;  //记录一共创建的定时器的数目
};

}
}


#endif
