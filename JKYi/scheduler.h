#ifndef _JKYI_SCHEDULER_H_
#define _JKYI_SCHEDULER_H_

#include<memory>
#include<vector>
#include<list>
#include<iostream>

#include"log.h"
#include"fiber.h"
#include"thread.h"

namespace JKYi{

class Scheduler{
public:
   typedef std::shared_ptr<Scheduler> ptr;
   typedef Mutex MutexType;
public:
   //这里的use_caller我的理解就是是否要使用当前创建调度器的线程
   Scheduler(size_t threads = 1,bool use_caller = true,const std::string& name = "");
   //该类会作为基类被子类所继承，所以为了避免出现内存泄漏的情况，这里的话将析构函数定义为虚函数
   virtual ~Scheduler();
   //
   const std::string& getName()const { return m_name; }
   //返回当前线程正在使用的调度器
   static Scheduler* GetThis();
   //返回当前线程的主调度协程
   static Fiber* GetMainFiber();
   //
   void start();
   //
   void stop();

   //该函数用来向调度器的工作队列中加入要执行的任务
   template<class FiberOrCb>
   void schedule(FiberOrCb fc,int thread = -1){
      bool need_tickle = false;
	  {
		  MutexType::Lock lock(m_mutex);
		  need_tickle = scheduleNoLock(fc,thread);
	  }
	  if(need_tickle){
		  tickle();
	  }
   }
   //
   //批量的往队列中插入任务
   template<class InputIterator>
   void schedule(InputIterator begin,InputIterator end){
	   bool need_tickle=false;
	   {
		   MutexType::Lock lock(m_mutex);
		   while(begin!=end){
			   need_tickle=scheduleNoLock(&*begin,-1)||need_tickle;
			   ++begin;
		   }
	   }
	   if(need_tickle){
		   tickle();
	   }
   }
   std::ostream& dump(std::ostream& os);
protected:
   //该函数是用来唤醒协程的,相当于是通知调度器有任务来了
   virtual void tickle();
   //真正执行的函数
   void run();
   //判断调度器是否满足停止的条件
   virtual bool stopping();
   //协程空闲状态执行的函数
   virtual void idle();
   //将该调度器设置为当前线程正在使用的调度器 
   void setThis();
   //判断是否拥有空闲的协程
   bool hasIdleThreads(){return m_idleThreadCount>0;}
private:
   //无所状态下向调度器的工作队列中插入任务
   template<class FiberOrCb>
   bool scheduleNoLock(FiberOrCb fc,int thread){
	   bool need_tickle = m_fibers.empty();
	   FiberAndThread ft(fc,thread);
	   if(ft.fiber||ft.cb){
		   m_fibers.push_back(ft);
	   }
	   return need_tickle;
   }
private:
  //调度器要执行的任务
  struct FiberAndThread{
     Fiber::ptr fiber; 
	 std::function<void ()> cb;
	 //用来指定该任务要到哪一个线程中去执行
	 int thread;

	 FiberAndThread(Fiber::ptr f,int thr)
	 :fiber(f),
	 thread(thr){
	 }

	 FiberAndThread(Fiber::ptr * f,int thr)
	 :thread(thr){
		 fiber.swap(*f);
	 }
	 //
	 FiberAndThread(std::function<void ()>f,int thr)
	 :cb(f),
	 thread(thr){

	 }
	 FiberAndThread(std::function<void ()>*f,int thr)
	 :thread(thr){
		 cb.swap(*f);
	 }
	 //默认构造函数
	 FiberAndThread()
	 :thread(-1){

	 }
	 //
	 void reset(){
       fiber = nullptr;
	   cb = nullptr;
	   thread = -1;
	 }
  };
private:
   MutexType m_mutex;
  //线程池
  std::vector<Thread::ptr> m_threads;
  //调度器中要处理的任务
  std::list<FiberAndThread> m_fibers;
  //调度器中的主调度协程，在use_caller=true时有效
  Fiber::ptr m_rootFiber;
  //
  std::string m_name;
protected:
   //存储创建的线程id
   std::vector<int> m_threadIds;
   //调度器中要调度的线程的数目
   size_t m_threadCount=0;
   //正在工作的线程数目
   std::atomic<size_t> m_activeThreadCount={0};
   //处于空闲状态的线程数目
   std::atomic<size_t> m_idleThreadCount={0};
   //是否正在停止
   bool m_stopping=true;
   //是否自动停止
   bool m_autoStop=false;
   //调度器主调度协程所在的线程id
   int m_rootThread=0;
};
}
#endif
