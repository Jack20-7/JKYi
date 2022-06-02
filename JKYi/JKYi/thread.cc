#include"thread.h" 
#include"log.h" 
#include"util.h"

namespace JKYi{
//这里需要定义几个线程的局部存储设施
//当前正在执行的线程的Thread
static thread_local Thread* t_thread=nullptr;
//当前正在执行的线程的名称
static thread_local std::string t_thread_name="UNKNOW";


//系统配置的日志全部搭载system文件中去
static JKYi::Logger::ptr g_logger=JKYI_LOG_NAME("system");

Thread::Thread(std::function<void()> cb,const std::string &name)
:m_cb(cb),m_name(name){
  if(name.empty()){
     m_name="UNKNOW";
   }
   int rt=pthread_create(&m_thread,nullptr,&Thread::run,this);
   if(rt){
     JKYI_LOG_ERROR(g_logger)<<"pthread_create thread fail,rt="
     <<rt<<" name="<<name;
     throw std::logic_error("pthread_create eroor");
   }
   //这里需要保证构造函数返回时创建的线程已经启动
   m_semaphore.wait();
} 
	//析构函数
Thread::~Thread(){
  if(m_thread){
    pthread_detach(m_thread);    
  }
} 
	//
void Thread::join(){
   if(m_thread){
     int rt=pthread_join(m_thread,nullptr);  
     if(rt){
      JKYI_LOG_ERROR(g_logger)<<"pthread_join thread fail,rt=" 
      <<rt<<" name="<<m_name;
      throw std::logic_error("thread_join error");
    }
     m_thread=0;
   }
}

 void * Thread::run(void*arg){
   Thread* thread=(Thread*)arg;
   t_thread=thread;
   t_thread_name=thread->m_name;
   thread->m_id=JKYi::GetThreadId();
   //设置线程的名称,这样我们在top的时候，看到的线程名称就是这里设置的线程名称
   pthread_setname_np(pthread_self(),thread->m_name.substr(0,15).c_str());
   //这里之所以要swap一下是为了避免出现环形引用的问题
   std::function<void()>cb;
   cb.swap(thread->m_cb);
   //启动时唤醒
   thread->m_semaphore.notify();
   cb();

   return 0;
}
	//返回当前正在执行的线程的Thread的指针
Thread* Thread::GetThis(){
     return t_thread;
}


	//返回当前的这个线程的名称
const std::string& Thread::GetName(){
    return t_thread_name;
}

	//给当前的这个线程设置名称 
void Thread::setName(const std::string&name){
    if(name.empty()){
      return ;   
    }
    if(t_thread){
      t_thread->m_name=name; 
    }
    t_thread_name=name;
}

}
