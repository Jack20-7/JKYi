#ifndef _JKYI_TIMERQUEUE_H_
#define _JKYI_TIMERQUEUE_H_

#include"JKYi/mutex.h"
#include"JKYi/timestamp.h"
#include"JKYi/reactor/channel.h"
#include"JKYi/reactor/Callbacks.h"

#include<set>
#include<vector>

namespace JKYi{
namespace net{

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : public Noncopyable{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb,Timestamp when,double interval);
    void cancel(TimerId timerId);
private:
    typedef std::pair<Timestamp,Timer*> Entry;
    typedef std::set<Entry> TimerList;
    typedef std::pair<Timer* ,int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    void handleRead();
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired,Timestamp now);

    bool insert(Timer* timer);

    EventLoop* loop_;
    const int timerfd_;
    Channel* timerfdChannel_;

    TimerList timers_;              //加入的定时器列表

    ActiveTimerSet activeTimers_;   //当前有效的定时器列表
    bool callingExpiredTimers_;     //是否正在处理超时的定时器
    ActiveTimerSet cancelingTimers_; //在处理超时定时器过程中取消的定时器就暂存在这里
};
}
}


#endif
