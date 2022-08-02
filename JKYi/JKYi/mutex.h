#ifndef _JKYI_MUTEX_H_
#define _JKYI_MUTEX_H_

#include<thread>
#include<functional>
#include<memory>
#include<pthread.h>
#include<semaphore.h>
#include<stdint.h>
#include<atomic>
#include<list>

#include"noncopyable.h"
#include"fiber.h"

namespace JKYi{
//对信号量的封装
class Semaphore:Noncopyable{
public:
   //构造函数
   Semaphore(uint32_t count=0);
   //析构函数
   ~Semaphore();
   //等待信号量
   void wait();
   //释放信号量
   void notify();
private:
   sem_t m_semaphore; 
};
//对局部锁的封装
template<class T>
class ScopedLockImpl{
public:
  //构造函数
  ScopedLockImpl(T&mutex)
  :m_mutex(mutex){
     m_mutex.lock();
	 m_locked=true;
  }
  //析构函数
  ~ScopedLockImpl(){
    m_mutex.unlock();
	m_locked=false;
  }
  //加锁的函数
  void lock(){
	  if(!m_locked){
		  m_mutex.lock();
		  m_locked=true;
	  }
  }
  //解锁的函数
  void unlock(){
	  if(m_locked){
		  m_mutex.unlock();
		  m_locked=false;
	  }
  }
  
private:
  //实际的锁
  T& m_mutex;
  //当前是否处于锁定状态
  bool m_locked;
};
///对局部读锁的封装
template<class T>
class ReadScopedLockImpl{
public:
  //构造函数
  ReadScopedLockImpl(T&mutex)
  :m_mutex(mutex){
     m_mutex.rdlock();
	 m_locked=true;
  }
  //析构函数
  ~ReadScopedLockImpl(){
	  unlock();
  }
  //加锁的函数
  void lock(){
	  if(!m_locked){
		  m_mutex.rdlock();
		  m_locked=true;
	  }
  }
  //解锁的函数
  void unlock(){
	  if(m_locked){
		  m_mutex.unlock();
		  m_locked=false;
	  }
  }
  
private:
  //实际的锁
  T& m_mutex;
  //当前是否处于锁定状态
  bool m_locked;
};
//对局部写锁的封装
template<class T>
class WriteScopedLockImpl{
public:
  //构造函数
  WriteScopedLockImpl(T&mutex)
  :m_mutex(mutex){
     m_mutex.wrlock();
	 m_locked=true;
  }
  //析构函数
  ~WriteScopedLockImpl(){
	  unlock();
  }
  //加锁的函数
  void lock(){
	  if(!m_locked){
		  m_mutex.wrlock();
		  m_locked=true;
	  }
  }
  //解锁的函数
  void unlock(){
	  if(m_locked){
		  m_mutex.unlock();
		  m_locked=false;
	  }
  }
  
private:
  //实际的锁
  T& m_mutex;
  //当前是否处于锁定状态
  bool m_locked;
};

//对互斥锁的封装
class Mutex: Noncopyable{
public:
    typedef ScopedLockImpl<Mutex>  Lock;
public:
    //构造函数
   Mutex(){
	   pthread_mutex_init(&m_mutex,nullptr);
   }
   //析构函数
   ~Mutex(){
	   pthread_mutex_destroy(&m_mutex);
   }
   //加锁
   void lock(){
	   pthread_mutex_lock(&m_mutex);
   }
   //解锁
   void unlock(){
	   pthread_mutex_unlock(&m_mutex);
   }
private:
   pthread_mutex_t m_mutex;
};
//读写锁的封装
class RWMutex:Noncopyable{
public:
    //用户平时就使用他们就行了
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
	
	typedef WriteScopedLockImpl<RWMutex> WriteLock;
    //构造函数
    RWMutex(){
		pthread_rwlock_init(&m_lock,nullptr);
	}
	//析构函数
	~RWMutex(){
       pthread_rwlock_destroy(&m_lock);
	}
	//以读锁的方式加锁
	void rdlock(){
		pthread_rwlock_rdlock(&m_lock);
	}
	//以写锁的方式加锁
	void wrlock(){
        pthread_rwlock_wrlock(&m_lock);
	}
	//解锁
	void unlock(){
		pthread_rwlock_unlock(&m_lock);
	}

private:
   pthread_rwlock_t m_lock;
};
//这里的话，封装一个自旋锁来提高效率
class SpinLock:Noncopyable{
public:
   typedef ScopedLockImpl<SpinLock>   Lock;
   //构造函数
   SpinLock(){
	   pthread_spin_init(&m_mutex,0);
   }
   //析构函数
   ~SpinLock(){
	 pthread_spin_destroy(&m_mutex); 
   }
   //加锁
   void lock(){
	   pthread_spin_lock(&m_mutex);
   }
   //解锁
   void unlock(){
	   pthread_spin_unlock(&m_mutex);
   }
private:
   pthread_spinlock_t m_mutex;
};
//封装的原子锁
class CASLock:Noncopyable{
public:
   //构造函数
   CASLock(){
	   m_mutex.clear();
   }
   //析构函数
   ~CASLock(){

   }
   //加锁
   void lock(){
	   while(std::atomic_flag_test_and_set_explicit(&m_mutex,std::memory_order_acquire));
   }
   //解锁
   void unlock(){
	   std::atomic_flag_clear_explicit(&m_mutex,std::memory_order_release);
   }
private:
   volatile std::atomic_flag m_mutex;
};

//用于协程的信号量
class Scheduler;
class FiberSemaphore{
public:
    typedef SpinLock MutexType;
    FiberSemaphore(size_t initial_concurrency = 0);
    ~FiberSemaphore();

    bool tryWait();
    void wait();
    void notify();
private:
    //自旋锁的目的就是为了保护下面的信号量的值
    MutexType m_mutex;
    std::list<std::pair<Scheduler*,Fiber::ptr> >m_waiters;
    size_t m_concurrency;

};

}
#endif
