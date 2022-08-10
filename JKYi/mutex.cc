#include"mutex.h"
#include"macro.h"
#include"scheduler.h"

namespace JKYi{
   //构造函数
Semaphore::Semaphore(uint32_t count){
   if(sem_init(&m_semaphore,0,count)){
	   throw std::logic_error("sem_init error");
   }
}
   //析构函数
Semaphore::~Semaphore(){
    if(sem_destroy(&m_semaphore)){
		throw std::logic_error("sem_destroy error");
	}
}
   //等待信号量
void Semaphore::wait(){
   if(sem_wait(&m_semaphore)){
	   throw std::logic_error("sem_wait error");
   }
}
   //释放信号量
void Semaphore::notify(){
    if(sem_post(&m_semaphore)){
       throw std::logic_error("sem_post error");
	}
}

FiberSemaphore::FiberSemaphore(size_t inital_concurrency)
    :m_concurrency(inital_concurrency){
}

FiberSemaphore::~FiberSemaphore(){
    JKYI_ASSERT(m_waiters.empty());
}

bool FiberSemaphore::tryWait(){
    JKYI_ASSERT(Scheduler::GetThis());
    {
        MutexType::Lock lock(m_mutex);
        if(m_concurrency > 0){
            --m_concurrency;
            return true;
        }
        return false;
    }
}

void FiberSemaphore::wait(){
    JKYI_ASSERT(Scheduler::GetThis());
    {
        MutexType::Lock lock(m_mutex);
        if(m_concurrency > 0){
            --m_concurrency;
            return ;
        }
        m_waiters.push_back(std::make_pair(Scheduler::GetThis(),Fiber::GetThis()));
    }
    Fiber::YieldToHold();
}

void FiberSemaphore::notify(){
    MutexType::Lock lock(m_mutex);
    if(!m_waiters.empty()){
        auto next = m_waiters.begin();
        m_waiters.pop_front();
        next->first->schedule(next->second);
    }else{
        //如果没有等待的协程的话
        ++m_concurrency;
    }
    return ;
}


}
