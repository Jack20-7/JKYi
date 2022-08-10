#ifndef _JKYI_IOMANAGER_H_
#define _JKYI_IOMANAGER_H_

#include"scheduler.h"
#include"timer.h"

namespace JKYi{
//基于epoll的io协程调度器
class IOManager:public Scheduler,public TimerManager{
public:
   typedef std::shared_ptr<IOManager> ptr;
   typedef RWMutex RWMutexType;
   //表示要注册的事件
   enum Event{
	   NONE  =  0x0,
	   READ  =  0x1,
	   WRITE =  0x4
   };
private:
   //表示要注册的文件描述符的上下文
   struct FdContext{
     typedef Mutex MutexType;
     //事件的上下文，用来对注册的每一个事件进行描述
	 struct EventContext{
	   //表示该事件该由哪一个调度器进行调度
       Scheduler* scheduler = nullptr;
	   //负责处理该事件的协程
	   Fiber::ptr fiber;
	   //负责处理该事件的回调函数
	   std::function<void ()>cb;
	 };
	 //根据传入的事件返回该事件对应的上下文
	 EventContext& getContext(Event event);
	 //重置传入的事件上下文
	 void resetContext(EventContext&ctx);
	 //触发该文件描述符上传入的事件对应的回调函数/协程
	 void triggerEvent(Event event);
	 
     //读事件对应的上下文
	 EventContext read;
     //写事件对应的上下文
	 EventContext write;
	 //对应的描述符
	 int fd=0;
	 //该文件描述符上对应的事件
	 Event events=NONE;
	 //
	 MutexType mutex;
   };
public:
   //
   IOManager(size_t threads = 1,bool use_caller = true,const std::string&name = "");

   ~IOManager();    
   //向fd上添加某一个事件
   //成功返回0，失败返回-1
   int addEvent(int fd,Event event,std::function<void ()>cb = nullptr);
   //
   bool delEvent(int fd,Event event);
   //取消fd上的event事件并且对它进行触发
   bool cancelEvent(int fd,Event event);
   //
   bool cancelAll(int fd);
   //返回当前正在使用的iomanager
   static IOManager* GetThis();

protected:
    //用来唤醒epoll_wait
   void tickle()override;
   //
   bool stopping()override;
   //
   void idle()override;
   //
   void onTimerInsertedAtFront()override;
   //根据size大小对m_fdContexts进行初始化
   void contextResize(size_t size);
   //用来判断是否可以停止
   bool stopping(uint64_t&timeout);
private:
   //记录epoll的内核注册表对应的文件描述符
   int m_epfd=0;
   //记录创建的管道的文件描述符
   //管道是用来唤醒epoll_wait的
   int m_tickleFds[2];
   //记录当前还需要处理的事件的个数
   std::atomic<size_t>m_pendingEventCount={0};
   
   RWMutexType m_mutex;
   //记录注册的socket对应的上下文
   std::vector<FdContext*>m_fdContexts;
   
};

}
#endif
