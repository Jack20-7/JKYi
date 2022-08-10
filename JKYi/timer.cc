#include"timer.h"
#include"util.h"
#include"log.h"

namespace JKYi{

static Logger::ptr g_logger=JKYI_LOG_NAME("system");

bool Timer::Comparator::operator()(const Timer::ptr &lhv,
                                        const Timer::ptr&rhv)const{
    if(!lhv&&!rhv){
		return false;
	}
	if(!lhv){
		return true;
	}
	if(!rhv){
		return false;
	}
	//
	if(lhv->m_next<rhv->m_next){
		return true;
	}
	if(lhv->m_next>rhv->m_next){
		return false;
	}
	//如果触发时间相当的话，就直接比较指针的大小
	return lhv.get()<rhv.get();
}
//
Timer::Timer(uint64_t ms,std::function<void ()>cb,bool recurring,TimerManager*manager)
  :m_recurring(recurring)
  ,m_ms(ms)
  ,m_cb(cb)
  ,m_manager(manager){
	  m_next=JKYi::GetCurrentMS()+m_ms;
}
//
Timer::Timer(uint64_t next)
  :m_next(next){

}
//
bool Timer::cancel(){
   TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
   //
   if(m_cb){
    m_cb=nullptr;
	auto it=m_manager->m_timers.find(shared_from_this());
	m_manager->m_timers.erase(it);
	return true;
   }
   return false;
}
//
bool Timer::refresh(){
  TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
  //
  if(!m_cb){
	  return false;
  }
  //
  auto it=m_manager->m_timers.find(shared_from_this());
  if(it==m_manager->m_timers.end()){
	  return false;
  }
  m_manager->m_timers.erase(it);
  m_next=JKYi::GetCurrentMS()+m_ms;
  m_manager->m_timers.insert(shared_from_this());
  return true;
}
//
bool Timer::reset(uint64_t ms,bool from_now){
	//这个from_now代表的是是否从当前开始算起
  if(ms==m_ms&&!from_now){
	  return true;
  }
  //
  TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
  if(!m_cb){
	  return false;
  }
  //
  auto it=m_manager->m_timers.find(shared_from_this());
  if(it==m_manager->m_timers.end()){
	  return false;
  }
  //
  m_manager->m_timers.erase(it);
  uint64_t start=0;
  if(from_now){
	  start=JKYi::GetCurrentMS();
  }else{
	  start=m_next-m_ms;
  }
  m_ms=ms;
  m_next=start+m_ms;
  m_manager->addTimer(shared_from_this(),lock);
  return true;
}
//
TimerManager::TimerManager(){
   m_previouseTime=JKYi::GetCurrentMS();
}
//析构函数
TimerManager::~TimerManager(){

}
//
Timer::ptr TimerManager::addTimer(uint64_t ms,std::function<void ()> cb,bool recurring){
   Timer::ptr timer(new Timer(ms,cb,recurring,this));
   RWMutexType::WriteLock lock(m_mutex);
   addTimer(timer,lock);
   return timer;
}
//该函数用在条件定时器的那个地方，用来判断是否符合条件定时器的触发条件，如果符合的话就进行触发
static void OnTimer(std::weak_ptr<void>weak_cond,std::function<void()>cb){
  std::shared_ptr<void>tmp=weak_cond.lock();
  if(tmp){
	  cb();
  }
}
//
Timer::ptr TimerManager::addConditionTimer(uint64_t ms,std::function<void()>cb,std::weak_ptr<void>weak_cond,bool recurring){
    return addTimer(ms,std::bind(&OnTimer,weak_cond,cb),recurring);
}
//
uint64_t TimerManager::getNextTimer(){
	RWMutexType::ReadLock lock(m_mutex);
	//每次调用该函数时就对这个成员的值进行重置
	m_tickled=false;

	if(m_timers.empty()){
		return ~0ull;
	}
	//
	const Timer::ptr &next=*m_timers.begin();
	uint64_t now_ms=JKYi::GetCurrentMS();
	if(now_ms>=next->m_next){
		return 0;
	}else{
		return next->m_next-now_ms;
	}
}
//将超时的计时器从set容器中移除并且返回他们的回调函数
void TimerManager::listExpiredCb(std::vector<std::function<void ()>>&cbs){
  uint64_t now_ms=JKYi::GetCurrentMS(); 
  std::vector<Timer::ptr>expired;
  {
      //JKYI_LOG_DEBUG(g_logger)<<"listExpiredCb里面加读锁";
	  RWMutexType::ReadLock lock(m_mutex);
	  if(m_timers.empty()){
		  return ;
	  }
      //JKYI_LOG_DEBUG(g_logger)<<"listExpiredCb里面解读锁";
  }
  //
  //JKYI_LOG_DEBUG(g_logger)<<"listExpiredCb里面加写锁";
  RWMutexType::WriteLock lock(m_mutex);
  if(m_timers.empty()){
	  return ;
  }
  //判断是否有时间被后调的问题出现
  bool rollover=deleteClockRollover(now_ms);
  if(!rollover&&((*m_timers.begin())->m_next>now_ms)){
     return ;
  }
  Timer::ptr now_timer(new Timer(now_ms));
  //
  auto it=rollover?m_timers.end():m_timers.lower_bound(now_timer);
  while(it!=m_timers.end()&&(*it)->m_next==now_ms){
	  it++;
  }
  //
  expired.insert(expired.begin(),m_timers.begin(),it);
  m_timers.erase(m_timers.begin(),it);
  cbs.reserve(expired.size());
  //
  for(auto&timer:expired){
	  cbs.push_back(timer->m_cb);
	  if(timer->m_recurring){
		  timer->m_next=now_ms+timer->m_ms;
		  m_timers.insert(timer);
	  }else{
		  timer->m_cb=nullptr;
	   }
  }
  //JKYI_LOG_DEBUG(g_logger)<<"listExpiredCb解写锁";
}
//
void TimerManager::addTimer(Timer::ptr val,RWMutexType::WriteLock& lock){
   auto it=m_timers.insert(val).first;
   bool at_front = (it == m_timers.begin()) && !m_tickled;
   if(at_front){
	   m_tickled = true;
   }
   lock.unlock();
   if(at_front){
	   onTimerInsertedAtFront();
   }
}
//
bool TimerManager::hasTimer(){
   RWMutexType::ReadLock lock(m_mutex);
   return !m_timers.empty();
}
//
bool TimerManager::deleteClockRollover(uint64_t now_ms){
  bool rollover=false; 
  //如果当前时间小于传入的时间并且小于一个小时以上
  if(now_ms<m_previouseTime&&now_ms<(m_previouseTime-60*60*1000)){
	  rollover=true;
  }
  //
  m_previouseTime=now_ms;
  return rollover;

}
}
