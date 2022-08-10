#include"iomanager.h"
#include"macro.h"
#include"log.h"

#include<errno.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<string>
#include<string.h>
#include<unistd.h>

namespace JKYi{

static Logger::ptr g_logger=JKYI_LOG_NAME("system");

//
IOManager::FdContext::EventContext& IOManager::FdContext::getContext(IOManager::Event event){
    switch(event){
		case IOManager::READ:
		  return read;
		case IOManager::WRITE:
		  return write;
		default:
		  JKYI_ASSERT2(false,"getcontext");
	}
	throw std::invalid_argument("getcontext invalid event");
}
//
void IOManager::FdContext::resetContext(EventContext&ctx){
	ctx.scheduler=nullptr;
	ctx.fiber.reset();
	ctx.cb=nullptr;
}
//
void IOManager::FdContext::triggerEvent(IOManager::Event event){
	//验证该文件描述符上注册有该事件
    JKYI_ASSERT(events&event);
	//触发之后取消掉该事件
	events=(Event)(events&~event);

	EventContext&ctx=getContext(event);
	if(ctx.cb){
		ctx.scheduler->schedule(&ctx.cb);
	}else{
		ctx.scheduler->schedule(&ctx.fiber);
	}
	//
	ctx.scheduler = nullptr;
	return ;
}

//
IOManager::IOManager(size_t threads,bool use_caller,const std::string& name)
   :Scheduler(threads,use_caller,name){
	   m_epfd=epoll_create(5000);
	   JKYI_ASSERT(m_epfd>0);
	   //创建管道
	   int rt=pipe(m_tickleFds);
	   JKYI_ASSERT(!rt);
	   //对管道的读事件进行监听
	   epoll_event event;
	   memset(&event,0,sizeof(event));
	   event.events=EPOLLIN|EPOLLET;
	   event.data.fd=m_tickleFds[0];
	   //对于注册为边缘触发的socket，应该设置为非阻塞
	   rt=fcntl(m_tickleFds[0],F_SETFL,O_NONBLOCK);
	   JKYI_ASSERT(!rt);
	   //
	   rt=epoll_ctl(m_epfd,EPOLL_CTL_ADD,m_tickleFds[0],&event);
	   JKYI_ASSERT(!rt);
	   //
	   contextResize(32);
	   //开启协程调度器
	   start();
}
//
IOManager::~IOManager(){
    stop();
	
	close(m_epfd);
    close(m_tickleFds[0]);
	close(m_tickleFds[1]);
	//
	for(size_t i = 0;i < m_fdContexts.size();++i){
		if(m_fdContexts[i]){
			delete m_fdContexts[i];
		}
	}
   JKYI_LOG_DEBUG(g_logger) << "name = " << getName() << " is exit";
}
//
void IOManager::contextResize(size_t size){
	m_fdContexts.resize(size);
	//
	for(size_t i=0;i < size;++i){
		if(!m_fdContexts[i]){
		  m_fdContexts[i]=new FdContext;
		  m_fdContexts[i]->fd=i;
		}
	}
}
//
int IOManager::addEvent(int fd,Event event,std::function<void()>cb){
	//
	FdContext*fd_ctx = nullptr;
	RWMutexType::ReadLock lock(m_mutex);
	if((int)m_fdContexts.size()>fd){
		fd_ctx=m_fdContexts[fd];
		lock.unlock();
	}else{
		lock.unlock();
		//
		RWMutexType::WriteLock lock2(m_mutex);
		contextResize(fd*1.5);
		fd_ctx=m_fdContexts[fd];
	}
	//
   FdContext::MutexType::Lock lock2(fd_ctx->mutex);
   if((JKYI_UNLIKELY(fd_ctx->events & event))){
	   JKYI_LOG_ERROR(g_logger)<<"addEvent assert fd="<<fd
	                           <<" event="<<(EPOLL_EVENTS)event
							   <<" fd_ctx.events="<<(EPOLL_EVENTS)fd_ctx->events;
		JKYI_ASSERT(!(fd_ctx->events & event));
   }
   //
   int op=fd_ctx->events?EPOLL_CTL_MOD:EPOLL_CTL_ADD;
   epoll_event epevent;
   memset(&epevent,0,sizeof(epevent));
   epevent.events=EPOLLET|fd_ctx->events|event;
   epevent.data.ptr=fd_ctx;
   int rt=epoll_ctl(m_epfd,op,fd,&epevent);
   if(rt){
	 JKYI_LOG_ERROR(g_logger)<<"epoll_ctl error";
	 return -1;
   }
   //
   ++m_pendingEventCount;
   fd_ctx->events=(Event)(fd_ctx->events|event);
   FdContext::EventContext& event_ctx=fd_ctx->getContext(event);
   JKYI_ASSERT(!event_ctx.scheduler&&
               !event_ctx.fiber&&
			   !event_ctx.cb);
   event_ctx.scheduler=Scheduler::GetThis();
   if(cb){
	   event_ctx.cb.swap(cb);
   }else{
	   event_ctx.fiber=Fiber::GetThis();
	   JKYI_ASSERT2(event_ctx.fiber->getState()==Fiber::EXEC,
	                "state="<<event_ctx.fiber->getState());
   }
   return 0;
}
//
bool IOManager::delEvent(int fd,Event event){
   RWMutexType::ReadLock lock(m_mutex);
   if((int)m_fdContexts.size()<=fd){
	   return false;
   }
   //
   FdContext*fd_ctx=m_fdContexts[fd];
   lock.unlock();
   //
   FdContext::MutexType::Lock lock2(fd_ctx->mutex);
   if(!(fd_ctx->events&event)){
       return false;
   }
   Event new_events=(Event)(fd_ctx->events& ~event);
   int op=new_events?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
   epoll_event epevent;
   memset(&epevent,0,sizeof(epevent));
   epevent.events=new_events|EPOLLET;
   epevent.data.ptr=fd_ctx;
   //
   int rt=epoll_ctl(m_epfd,op,fd,&epevent);
   if(rt){
	   JKYI_LOG_ERROR(g_logger)<<"epoll_ctl error";
	   return false;
   }
   --m_pendingEventCount;
   fd_ctx->events=new_events;
   FdContext::EventContext&event_ctx=fd_ctx->getContext(event);
   fd_ctx->resetContext(event_ctx);
   return true;
}
//这个会对event事件进行触发
bool IOManager::cancelEvent(int fd,Event event){
  RWMutexType::ReadLock lock(m_mutex);
  if((int)m_fdContexts.size()<=fd){
	  return false;
  }
  FdContext*fd_ctx=m_fdContexts[fd];
  lock.unlock();
  //
  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if(!(fd_ctx->events&event)){
	  return false;
  }
  //
  Event new_events=(Event)(fd_ctx->events&~event);
  int op=new_events?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
  epoll_event epevent;
  memset(&epevent,0,sizeof(epevent));
  epevent.events=new_events|EPOLLET;
  epevent.data.ptr=fd_ctx;

  int rt=epoll_ctl(m_epfd,op,fd,&epevent);
  if(rt){
	  JKYI_LOG_ERROR(g_logger)<<"epoll_ctl error";
	  return false;
  }
  //
  --m_pendingEventCount;
  fd_ctx->triggerEvent(event);
  return true;
}
//
bool IOManager::cancelAll(int fd){
   RWMutexType::ReadLock lock(m_mutex);
   if((int)m_fdContexts.size()<=fd){
	   return false;
   }
   FdContext*fd_ctx=m_fdContexts[fd];
   lock.unlock();
   //
   FdContext::MutexType::Lock lock2(fd_ctx->mutex);
   if(!fd_ctx->events){
	   return false;
   }
   //
   int op=EPOLL_CTL_DEL;
   epoll_event epevent;
   memset(&epevent,0,sizeof(epevent));
   epevent.events=0;
   epevent.data.ptr=fd_ctx;

   int rt=epoll_ctl(m_epfd,op,fd,&epevent);
   if(rt){
	   JKYI_LOG_ERROR(g_logger)<<"epoll_ctl error";
	   return false;
   }
   //
   if(fd_ctx->events&READ){
	   fd_ctx->triggerEvent(READ);
	   --m_pendingEventCount;
   }
   if(fd_ctx->events&WRITE){
	   fd_ctx->triggerEvent(WRITE);
	   --m_pendingEventCount;
   }
   JKYI_ASSERT(fd_ctx->events==0);
   return true;
}
//
IOManager* IOManager::GetThis(){
	return dynamic_cast<IOManager*>(Scheduler::GetThis());
}
//
void IOManager::tickle(){
	if(!hasIdleThreads()){
		return ;
	}
	//先管道内写入一个字节的数据，主要是用来将epoll_wait给唤醒
	int rt=write(m_tickleFds[1],"T",1);
	//JKYI_LOG_INFO(g_logger)<<"tickle";
	JKYI_ASSERT(rt == 1);
}

//这个函数用来判断是否符合关闭的添加并且如果不符合的话，可以通过引用的方式将epoll_wait的第三个参数进行传出
bool IOManager::stopping(uint64_t &timeout){
   timeout = getNextTimer();
   return timeout == ~0ull
          && m_pendingEventCount == 0
          && Scheduler::stopping();
}
//
bool IOManager::stopping(){
	uint64_t timeout = 0;
	return stopping(timeout);
}
//关键逻辑函数
void IOManager::idle(){
   const uint64_t MAX_EVENTS = 256;   
   epoll_event* events = new epoll_event[MAX_EVENTS]();
   //使用智能指针来进行管理，但是要注意的是如果要使用shared_ptr来管理数组
   //需要自定义析构函数并且传入
   std::shared_ptr<epoll_event>shared_events(events,[](epoll_event*ptr){
      delete [] ptr;
   });

   while(true){
	   uint64_t next_timeout = 0;
	   if(stopping(next_timeout)){
		   JKYI_LOG_INFO(g_logger)<<"name = " << getName()
		                          <<" idle stopping exit";
			break;
	   }
	   int rt = 0;
	   do{
         static const int MAX_TIMEOUT = 3000;
		 if(next_timeout != ~0ull){
			 next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
		 }else{
			 next_timeout = MAX_TIMEOUT;
		 }
		 rt = epoll_wait(m_epfd,events,MAX_EVENTS,(int)next_timeout);
		 if(rt < 0 && errno == EINTR){
		 }else{
			 break;
		 }
	   }while(true);
	   //开始对发生的事件进行处理
	   std::vector<std::function<void()>>cbs;
	   listExpiredCb(cbs);
	   if(!cbs.empty()){
		   schedule(cbs.begin(),cbs.end());
		   cbs.clear();
	   }
	   //
	   for(int i = 0;i < rt;++i){
         epoll_event& event = events[i];
		 //判断是否是由于管道而被唤醒的
		 if(event.data.fd == m_tickleFds[0]){
			 uint8_t dummy[256];
			 //在while循环中一次性读完
			 while(read(m_tickleFds[0],dummy,sizeof(dummy))>0);
			 continue;
		 }
		 //如果不是管道的话
		 FdContext*fd_ctx = (FdContext*)event.data.ptr;
		 FdContext::MutexType::Lock lock(fd_ctx->mutex);
		 //
		 if(event.events & (EPOLLERR|EPOLLHUP)){
			 event.events|=(EPOLLIN|EPOLLOUT)&fd_ctx->events;
		 }
		 int real_events = NONE;
		 if(event.events & EPOLLIN){
			 real_events|=READ;
		 }
		 if(event.events & EPOLLOUT){
			 real_events|=WRITE;
		 }
		 if((fd_ctx->events&real_events) == NONE){
			 continue;
		 }
		 //
		 int left_events = (fd_ctx->events & ~real_events);
		 int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
		 event.events = EPOLLET|left_events;

		 int rt2 = epoll_ctl(m_epfd,op,fd_ctx->fd,&event);
		 if(rt2){
			 JKYI_LOG_ERROR(g_logger)<<"epoll_ctl error";
			 continue;
		 }
		 if(real_events & READ){
			 fd_ctx->triggerEvent(READ);
			 --m_pendingEventCount;
		 }
		 if(real_events & WRITE){
			 fd_ctx->triggerEvent(WRITE);
			 --m_pendingEventCount;
		 }
	   }
	   //全部事件处理完成之后，将控制权返回一下
	   Fiber::ptr cur = Fiber::GetThis();
	   auto raw_ptr = cur.get();
	   cur.reset();
	   raw_ptr->swapOut();
   }
}

//
void IOManager::onTimerInsertedAtFront(){
	tickle();
}

}
