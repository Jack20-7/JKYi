#ifndef _JKYI_TIMERID_H_
#define _JKYI_TIMERID_H_

#include"JKYi/reactor/timer.h"
#include"JKYi/reactor/timerQueue.h"

namespace JKYi{
namespace net{

class Timer;

//该类的作用就在取消timer时对timer进行唯一的标识
class TimerId{
friend class TimerQueue;
public:
    TimerId()
        :timer_(nullptr),
         sequence_(0){
     }
     TimerId(Timer* timer,int64_t seq)
         :timer_(timer),
          sequence_(seq){
      }
private:
    Timer* timer_;
    int64_t sequence_;
};

}
}

#endif
