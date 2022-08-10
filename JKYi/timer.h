#ifndef _JKYI_TIMER_H_
#define _JKYI_TIMER_H_

#include<memory>
#include<vector>
#include<set>

#include"thread.h"

namespace JKYi{

class TimerManager;
//封装的一个定时器类
class Timer:public std::enable_shared_from_this<Timer>{
friend class TimerManager;
public:
   typedef std::shared_ptr<Timer> ptr;
   //取消定时器
   bool cancel();
   //刷新定时器的触发时间
   bool refresh();
   //重置定时器的触发时间
   bool reset(uint64_t ms,bool from_now);
private:
   //构造函数
   Timer(uint64_t ms,std::function<void ()>cb,bool recurring,TimerManager*manager);

   Timer(uint64_t next);
private:
   //是否要周期性执行
   bool m_recurring=false;
   //周期性执行的间隔
   uint64_t m_ms=0;
   //下一次出发的出发的时间
   uint64_t m_next=0;
   //出发时的回调函数
   std::function<void()>m_cb;
   //定时器管理器,也就是这个定时器属于哪一个manager来管理
   TimerManager* m_manager=nullptr;
private:
   //在timermanager中，会使用set容器来管timer进行管理
   struct Comparator{
     bool operator()(const Timer::ptr&lhv,const Timer::ptr &rhv)const;
   };

};
//封装的定时器管理类
class TimerManager{
friend class Timer;
public:
   typedef RWMutex RWMutexType;

   TimerManager();
   //
   virtual ~TimerManager(); 
   //添加定时器
   Timer::ptr addTimer(uint64_t ms,std::function<void ()>cb,bool recurring=false);
   //添加添加定时器，条件定时器和普通定时器之间的区别在于条件定时器在触发时还会判断条件是否有效，如果有效才会执行回调函数，如果无效是不会调用回调函数的
   //而这个条件是否有效是由weak_ptr的get函数来实现
   Timer::ptr addConditionTimer(uint64_t ms,std::function<void()>cb,std::weak_ptr<void>weak_cond,bool recurring=false);
   //返回当前set容器中下一个要触发的定时器距离当前的时间
   uint64_t getNextTimer();
   //返回当前已经超时的定时器的回调函数
   void listExpiredCb(std::vector<std::function<void ()>>&cbs);
   //是否还有定时器需要进行处理
   bool hasTimer();
protected:
   //当要插入新的定时器时，就需要调用该函数，用来判断插入的定时器是否是最小执行的定时器，如果是的话就需要重新设置iomanager的epoll_wait的超时时间
   virtual void onTimerInsertedAtFront()=0;
   //
   void addTimer(Timer::ptr val,RWMutexType::WriteLock &lock);
   //
private:
   //检测当前服务器的时间是否被调后
   bool deleteClockRollover(uint64_t now_ms);
private:
   //使用读写锁来实现线程安全
   RWMutexType m_mutex;
   //用来存储要管理的定时器
   std::set<Timer::ptr,Timer::Comparator>m_timers;
   //使用优化性能
   bool m_tickled=false;
   //记录上一次执行的时间，用来在主机时间被修改可的场景进行触发
   uint64_t m_previouseTime=0;

};
}
#endif
