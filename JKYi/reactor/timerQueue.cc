#include"JKYi/reactor/timerQueue.h"
#include"JKYi/log.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/timer.h"
#include"JKYi/reactor/timerId.h"
#include"JKYi/macro.h"


#include<sys/timerfd.h>
#include<unistd.h>
#include<string.h>
#include<vector>

namespace JKYi{
namespace net{
namespace detail{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

int createTimerfd(){
    //创建定时器，该定时器到期时会触发一个写事件
    //就可以和其他IO之间一起处理
    //统一事件源
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0){
        JKYI_LOG_ERROR(g_logger) << " Failed in timerfd_create";
    }
    return timerfd;
}
//now距离when的时间
struct timespec howMuchTimeFromNow(Timestamp when){
    int64_t microseconds = when.getMicroSecondsSinceEpoch() - 
                                    Timestamp::now().getMicroSecondsSinceEpoch();
    if(microseconds < 100){
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
            (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void readTimerfd(int timerfd,Timestamp now){
    uint64_t howmany;
    ssize_t n = ::read(timerfd,&howmany,sizeof(howmany));
    JKYI_LOG_DEBUG(g_logger) << "TimerQueue::handRead read " << howmany << " at " 
                             << now.toString();
    if(n != sizeof(howmany)){
        JKYI_LOG_ERROR(g_logger) << " TimerQueue::handleRead read " << n << " bytes ";
    }
    return ;
}
//重置定时器的超时时间
void resetTimerfd(int timerfd,Timestamp expiration){
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&oldValue,0,sizeof(oldValue));
    memset(&newValue,0,sizeof(newValue));
    //memZero(&newValue,sizeof(newValue));
    //memZero(&oldValue,sizeof(oldValue));

    newValue.it_value = howMuchTimeFromNow(expiration);
    int rt = ::timerfd_settime(timerfd,0,&newValue,&oldValue);
    if(rt){
        JKYI_LOG_ERROR(g_logger) << " timerfd_settime error";
    }
    return ;
}
}
}
}

using namespace JKYi;
using namespace net;
using namespace detail;

TimerQueue::TimerQueue(EventLoop* loop)
    :loop_(loop),
     timerfd_(createTimerfd()),
     timerfdChannel_(new Channel(loop,timerfd_)),
     timers_(),
     callingExpiredTimers_(false){
     timerfdChannel_->setReadCallback(std::bind(&TimerQueue::handleRead,this));
     timerfdChannel_->enableReading();
}
TimerQueue::~TimerQueue(){
    timerfdChannel_->disableAll();
    timerfdChannel_->remove();
    ::close(timerfd_);
    for(auto& i : timers_){
        delete i.second;
    }
}
TimerId TimerQueue::addTimer(TimerCallback cb,Timestamp when,double interval){
    Timer* timer = new Timer(std::move(cb),when,interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,this,timer));
    return TimerId(timer,timer->getSequence());
}

void TimerQueue::cancel(TimerId timerId){
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop,this,timerId));
}

void TimerQueue::addTimerInLoop(Timer* timer){
    loop_->assertInLoopThread();
    //返回插入的定时器是否是接下来第一个触发的定时器
    bool earliestChanged = insert(timer);
    if(earliestChanged){
        resetTimerfd(timerfd_,timer->getExpiration());
    }
    return ;
}
void TimerQueue::cancelInLoop(TimerId timerId){
   loop_->assertInLoopThread();
   JKYI_ASSERT(timers_.size() == activeTimers_.size());

   ActiveTimer timer(timerId.timer_,timerId.sequence_);
   ActiveTimerSet::iterator it = activeTimers_.find(timer);
   if(it != activeTimers_.end()){
       size_t n = timers_.erase(Entry(it->first->getExpiration(),it->first));
       JKYI_ASSERT(n == 1);(void)n;
       delete it->first;
       activeTimers_.erase(it);
   }else if(callingExpiredTimers_){
       //如果还在timers_中,但是已经不活跃了，并且当前正在对过期的定时器进行处理的话
       cancelingTimers_.insert(timer);
   }
   JKYI_ASSERT(timers_.size() == activeTimers_.size());
}
void TimerQueue::handleRead(){
   loop_->assertInLoopThread(); 
   Timestamp now(Timestamp::now());
   //将数据读出
   readTimerfd(timerfd_,now);

   std::vector<Entry> expired = getExpired(now);
   callingExpiredTimers_ = true;
   //先清空，因为处理的过程中如果又取消的定时器的话，会被当如
   cancelingTimers_.clear();

   for(const Entry& it : expired){
       it.second->run();
   }
   callingExpiredTimers_ = false;
   //重置定时器的超时时间
   reset(expired,now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now){
    JKYI_ASSERT(timers_.size() == activeTimers_.size());
    std::vector<Entry> expired;
    Entry sentry(now,reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator end = timers_.lower_bound(sentry);
    JKYI_ASSERT(end == timers_.end() || end->first > now);
    //将超时的定时器拷贝出来
    //back_inserter函数接收一个容器，返回该容器的插入迭代器
    std::copy(timers_.begin(),end,back_inserter(expired));
    //然后删除
    timers_.erase(timers_.begin(),end);

    //接下来再从activeTimers中进行删除
    for(const Entry& it : expired){
        ActiveTimer timer(it.second,it.second->getSequence());
        size_t n = activeTimers_.erase(timer);
        JKYI_ASSERT(n == 1);(void)n;
    }
    JKYI_ASSERT(timers_.size() == activeTimers_.size());
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired,Timestamp now){
    Timestamp nextExpire;
    for(const Entry& it : expired){
        ActiveTimer timer(it.second,it.second->getSequence());
        if(it.second->isRepeat() && 
                cancelingTimers_.find(timer) == cancelingTimers_.end()){
            it.second->restart(now);
            insert(it.second);
        }else{
            delete it.second;
        }
    }
    //接下来就应该重置定时器
    if(!timers_.empty()){
        nextExpire = timers_.begin()->second->getExpiration();
    }
    if(nextExpire.valid()){
        resetTimerfd(timerfd_,nextExpire);
    }
    return ;
}
bool TimerQueue::insert(Timer* timer){
    loop_->assertInLoopThread();
    JKYI_ASSERT(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;

    Timestamp when = timer->getExpiration();
    TimerList::iterator it = timers_.begin();

    if(it == timers_.end() || when < it->first){
        earliestChanged = true;
    }
    //接下来就是插入操作
    {
        //插入timers_中
        std::pair<TimerList::iterator,bool> result = 
            timers_.insert(Entry(when,timer));
        JKYI_ASSERT(result.second);(void)result;
    }
    {
        //插入activeTimers_中
        std::pair<ActiveTimerSet::iterator,bool> result = 
            activeTimers_.insert(ActiveTimer(timer,timer->getSequence()));
        JKYI_ASSERT(result.second);(void)result;
    }

    JKYI_ASSERT(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

