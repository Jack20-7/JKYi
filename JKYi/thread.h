#ifndef _JKYI_THREAD_H_
#define _JKYI_THREAD_H_


#include"mutex.h"
#include"noncopyable.h"

namespace JKYi{

class Thread:Noncopyable {
public:
   typedef std::shared_ptr<Thread> ptr;
   //构造函数 
   Thread(std::function<void()> cb,const std::string &name); 
   //析构函数
   ~Thread(); 
   //
   pid_t getId()const{return m_id;}
   
   const std::string& getName()const{return m_name;} 
   // 
   void join();
   //返回当前正在执行的线程的Thread的指针
   static Thread* GetThis();
   //返回当前的这个线程的名称
   static const std::string& GetName();
   //给当前的这个线程设置名称 
   static void setName(const std::string&name);
    
private:
   //线程的中转函数
   static void * run(void*arg);
private:
   //线程的id，也就是实际显式的id
   pid_t m_id=-1;
   //pthread_create返回的id，我们一般不使用这个id 
   pthread_t m_thread=0;
   //线程实际执行的函数   
   std::function<void()>m_cb;
   //线程的名称
   std::string m_name;
   //封装的信号量
   Semaphore m_semaphore;

};

}
#endif
