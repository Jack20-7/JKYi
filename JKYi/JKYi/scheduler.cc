#include"scheduler.h"
#include"log.h"
#include"macro.h"
#include"hook.h"

namespace JKYi{
//系统的日志全是用system来打
static Logger::ptr g_logger=JKYI_LOG_NAME("system"); 

//当前线程正在使用的调度器
static thread_local Scheduler* t_scheduler = nullptr;

//当前线程中真正进行调度工作的协程,也代表当前线程使用的调度器中的主调度协程
static thread_local Fiber* t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads,bool use_caller,const std::string&name)
 :m_name(name){
	JKYI_ASSERT(threads>0);

      //判断是否要使用当前创建调度器的这个线程
	if(use_caller){
	  //如果要使用的话，首先对这个线程的主协程进行初始化
	  JKYi::Fiber::GetThis();
	  --threads;
      //一山不容二虎
	  JKYI_ASSERT(GetThis()==nullptr);
	  t_scheduler = this;
	  //
	  //在当前线程中创建正在进行调度工作的协程
	  m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run,this),0,true));
	  JKYi::Thread::setName(m_name);
	  //
	  t_scheduler_fiber = m_rootFiber.get();
	  m_rootThread = JKYi::GetThreadId();
	  m_threadIds.push_back(m_rootThread);
	}else{
      m_rootThread = -1;
	}
	m_threadCount = threads;
}

Scheduler::~Scheduler(){
	JKYI_ASSERT(m_stopping);
	//如果当前线程使用的正是该调度器的话，那么就将它置为空
	if(GetThis() == this){
		t_scheduler = nullptr;
	}
}
//
Scheduler* Scheduler::GetThis(){
	return t_scheduler;
}
//返回当前线程所使用的调度器的主调度协程
Fiber* Scheduler::GetMainFiber(){
	return t_scheduler_fiber;
}
//
void Scheduler::start(){
	MutexType::Lock lock(m_mutex);
	if(!m_stopping){
		return ;
	}
	m_stopping = false;
	JKYI_ASSERT(m_threads.empty());
	//
	m_threads.resize(m_threadCount);
	for(size_t i=0;i<m_threadCount;++i){
      m_threads[i].reset(new Thread(std::bind(&Scheduler::run,this),m_name+"-"+std::to_string(i)));
	  m_threadIds.push_back(m_threads[i]->getId());
	}
	//这里为了避免产生死锁，所以先解锁
   lock.unlock();
   //
   //if(m_rootFiber){
  //	   m_rootFiber->call();
   //   JKYI_LOG_INFO(g_logger)<<"call out "<<m_rootFiber->getState();
  // }
}
//
void Scheduler::stop(){
	//JKYI_LOG_DEBUG(g_logger)<<"stop";
	m_autoStop = true;
	if(m_rootFiber
             && m_threadCount == 0
             && (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::INIT)){
		//JKYI_LOG_INFO(g_logger)<<"Scheduer "<< m_name <<" stop";
		m_stopping = true;
		if(stopping()){
			return ;
		}
	}
	//这里的意思就是如果是use_caller的话，m_rootThread就是创建调度的的线程id
	
	if(m_rootThread != -1){
		JKYI_ASSERT(GetThis() == this);
	}else{
		JKYI_ASSERT(GetThis() != this);
	}
	//
	m_stopping = true;
	//将所有的线程唤醒
	//
	for(size_t i = 0;i < m_threadCount;++i){
        //JKYI_LOG_INFO(g_logger) << "thread tickle";
		tickle();
	}
	if(m_rootFiber){
		tickle();
	}
	//
	if(m_rootFiber){
		if(!stopping()){
			//如果stop的时候还有任务没有做完的话，就继续去执行主调度协程
			m_rootFiber->call();
		}
	}
	//
	std::vector<Thread::ptr> thrs;
	{
		MutexType::Lock lock(m_mutex);
		thrs.swap(m_threads);
	}
	for(auto&i :thrs){
        //JKYI_LOG_INFO(g_logger) << "join";
		i->join();
	}
}

std::ostream& Scheduler::dump(std::ostream& os){
    os << "[Scheduler name = " <<m_name
       << " size = " << m_threadCount
       << " active_count = " << m_activeThreadCount
       << " idle_count = " << m_idleThreadCount
       << " stoppng = " << m_stopping
       << " ]" << std::endl << "     ";
    for(size_t i =0 ;i < m_threadIds.size();++i){
        if(i){
            os << ",";
        }
        os << m_threadIds[i];
    }
    return os;
}
//
void Scheduler::setThis(){
	t_scheduler = this;
}
//
void Scheduler::run(){
	//自己创建的线程都需要hook
	set_hook_enable(true);

	setThis();
	//如果当前线程不是use_caller线程，那么他的主调度协程就是它的主协程
	if(JKYi::GetThreadId() != m_rootThread){
		t_scheduler_fiber = Fiber::GetThis().get();
	}
	//
    //创建一用来执行空闲任务的协程	
	Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle,this)));
	//创建一个用来执行函数任务的协程
    Fiber::ptr cb_fiber;	
   
    FiberAndThread ft;
    while(true) {
        ft.reset();
		bool tickle_me=false;
		bool is_active=false;
		{
			MutexType::Lock lock(m_mutex);
            auto it=m_fibers.begin();
			while(it != m_fibers.end()){
				if(it->thread!=-1 && it->thread!=JKYi::GetThreadId()){
					++it;
					tickle_me=true;
					continue;

				}
			 JKYI_ASSERT(it->fiber||it->cb);
			 if(it->fiber&&it->fiber->getState()==Fiber::EXEC){
				 ++it;
				 continue;
			   }
              ft=*it;
			  m_fibers.erase(it++);
			  ++m_activeThreadCount;
			  is_active=true;
			  break;
			}
			tickle_me |= it!=m_fibers.end();
		}
		//
		if(tickle_me){
			tickle();
		}
		//下面就是对取下的任务进行执行
		if(ft.fiber && (ft.fiber->getState()!=Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT)){
			ft.fiber->swapIn();
			--m_activeThreadCount;
			if(ft.fiber->getState()==Fiber::READY){
				schedule(ft.fiber);
			}else if(ft.fiber->getState()!=Fiber::TERM&&ft.fiber->getState()!=Fiber::EXCEPT){
				ft.fiber->m_state=Fiber::HOLD;
			}
			ft.reset();
		}else if(ft.cb){
			//如果执行的是函数的话
			if(cb_fiber){
				cb_fiber->reset(ft.cb);
			}else{
				cb_fiber.reset(new Fiber(ft.cb));
			}
			ft.reset();
			cb_fiber->swapIn();
			--m_activeThreadCount;
			if(cb_fiber->getState()==Fiber::READY){
				schedule(cb_fiber);
				cb_fiber.reset();
			}else if(cb_fiber->getState() == Fiber::TERM || cb_fiber->getState() == Fiber::EXCEPT){
				cb_fiber->reset(nullptr);
			}else{
				cb_fiber->m_state = Fiber::HOLD;
				cb_fiber.reset();
			}
		}else{
			if(is_active){
				--m_activeThreadCount;
				continue;
			}
			if(idle_fiber->getState() == Fiber::TERM){
				JKYI_LOG_INFO(g_logger)<<"idle fiber term";
				break;
			}
			++m_idleThreadCount;
			idle_fiber->swapIn();
			--m_idleThreadCount;
			if(idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT){
				idle_fiber->m_state = Fiber::HOLD;
			}
		}
	}
}

void Scheduler::tickle(){
	JKYI_LOG_INFO(g_logger)<<"Scheduler::tickle";
}
bool Scheduler::stopping(){
    MutexType::Lock lock(m_mutex);
	return m_autoStop&&
           m_stopping&&
           m_fibers.empty()&&
           m_activeThreadCount == 0;
}
//
void Scheduler::idle(){
	JKYI_LOG_INFO(g_logger)<<"Scheduler::idle";
    while(!stopping()){
		JKYi::Fiber::YieldToHold();
	}
}
//

}
