#include"hook.h"
#include"config.h"
#include"log.h"
#include"fiber.h"
#include"iomanager.h"
#include"fdmanager.h"
#include"macro.h"

#include<dlfcn.h>

JKYi::Logger::ptr g_logger=JKYI_LOG_NAME("system");

namespace JKYi{

//用来记录连接的超时时间
static ConfigVar<int>::ptr g_tcp_connect_timeout=Config::Lookup("tcp.connect.timeout",5000,"tcp connect timeout");

//记录每一个线程是否被hook住
static thread_local bool t_hook_enable=false;

#define HOOK_FUN(XX)\
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

void hook_init(){
	static bool is_inited=false;
	if(is_inited){
		return ;
	}
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT,#name);	
	HOOK_FUN(XX);
#undef XX
    is_inited=true;
}

//全局变量，记录connecti的超时时间
static uint64_t s_connect_timeout=-1;

//这里为了在mian函数执行之前对超时时间进行初始化等，利用全局变量在main函数执行之前初始化的这个特点来完成
struct _HookIniter{
	//构造函数中做文章
   _HookIniter(){

     hook_init(); 
	 
    s_connect_timeout=g_tcp_connect_timeout->getValue();

	g_tcp_connect_timeout->addListener([](const int &old_value,const int& new_value){
      JKYI_LOG_INFO(g_logger)<<"tcp connect timeout changed from "<<old_value<<"to"<<new_value;
	  s_connect_timeout=new_value;
    });
 }
};

static _HookIniter s_hook_initer;

bool is_hook_enable(){
	return t_hook_enable;
}

void set_hook_enable(bool flag){
	t_hook_enable=flag;
}

}

//该结构体用来作为作为田间定时器的条件
struct timer_info{
	int cancelled=0;
};

//下面写一个通用的函数来完成通用的逻辑
//可变参函数模板
template<typename OriginFun,typename ... Args>
static ssize_t do_io(int fd,OriginFun&fun,const char * hook_fun_name,uint32_t event,int timeout_so,Args&&...args){
    if(!JKYi::t_hook_enable){
		return fun(fd,std::forward<Args>(args)...);
	}
	//JKYI_LOG_DEBUG(g_logger)<<hook_fun_name<<"  do_io";

	JKYi::FdCtx::ptr fd_ctx=JKYi::FdMgr::GetInstance()->get(fd);
	if(!fd_ctx){
	  return fun(fd,std::forward<Args>(args)...);	
	}

    //
	if(fd_ctx->isClose()){
		errno=EBADF;
		return -1;
	}
    
	if(!fd_ctx->isSocket()||fd_ctx->getUserNonblock()){
		return fun(fd,std::forward<Args>(args)...);
	}

	uint64_t to=fd_ctx->getTimeout(timeout_so);
	std::shared_ptr<timer_info>tinfo(new timer_info);

retry:
   
	ssize_t n=fun(fd,std::forward<Args>(args)...);
	while(n==-1&&errno==EINTR){
		n=fun(fd,std::forward<Args>(args)...);
	}

	if(n==-1&&errno==EAGAIN){
	    //JKYI_LOG_DEBUG(g_logger)<<hook_fun_name<<"  again";
		JKYi::IOManager * iom=JKYi::IOManager::GetThis();
		JKYi::Timer::ptr timer;
		std::weak_ptr<timer_info>winfo(tinfo);

		if(to!=(uint64_t)-1){
		  //如果用户传入有超时时间的话，那么就可以添加一个计时器 	
		   timer=iom->addConditionTimer(to,[winfo,fd,iom,event](){
             auto t=winfo.lock();
			 if(!t||t->cancelled){
			  return ;
			 }
             //设置超时。因为如果在用户指定的时间内都还没有对应的事件发生的话
			 //就超时
			 t->cancelled=ETIMEDOUT;
			 iom->cancelEvent(fd,(JKYi::IOManager::Event)(event));
		  },winfo);
		}

		//定时器添加好了之后就对事件进行注册
		int rt=iom->addEvent(fd,(JKYi::IOManager::Event)(event));
		if(rt){
          //如果添加失败的话
		  JKYI_LOG_ERROR(g_logger)<<hook_fun_name<<"addEvent ("<<fd<<","<<event
			                     <<")";
		  if(timer){
			  timer->cancel();
		  }
		  return -1;
		}else{
          //让CPU的执行权力
          JKYi::Fiber::YieldToHold();
		  //从这里返回的话，一共有两种情况，分别是条件定时器超时/注册的事件发生
		  if(timer){
			  timer->cancel();
		  }
		  //如果是由于超时返回的话
		  if(tinfo->cancelled){
			  errno=tinfo->cancelled;
			  return -1;
		  }
		  goto retry;
	   }

	}
	return n;
}

extern "C"{
//初始化
#define XX(name) name ## _fun name ## _f =nullptr; 
    HOOK_FUN(XX);
#undef XX

//下面首先就是对sleep函数的hook

unsigned sleep(unsigned int second){
    if(!JKYi::t_hook_enable){
		return sleep_f(second);
	}

	JKYi::Fiber::ptr fiber=JKYi::Fiber::GetThis();
	JKYi::IOManager* iom=JKYi::IOManager::GetThis();

	iom->addTimer(second*1000,std::bind((void(JKYi::Scheduler::*)(JKYi::Fiber::ptr ,int thread))&JKYi::IOManager::schedule,iom,fiber,-1));
	JKYi::Fiber::YieldToHold();

	return 0;
}

int usleep(useconds_t usec){
	if(!JKYi::t_hook_enable){
		return usleep_f(usec);
	}
	//
	JKYi::Fiber::ptr fiber=JKYi::Fiber::GetThis();
	JKYi::IOManager* iom=JKYi::IOManager::GetThis();

	iom->addTimer(usec/1000,std::bind((void (JKYi::Scheduler::*)(JKYi::Fiber::ptr,int thread))&JKYi::IOManager::schedule,iom,fiber,-1));

	JKYi::Fiber::YieldToHold();
	
	return 0;
}

int nanosleep(const struct timespec*req,struct timespec *rem){
	if(!JKYi::t_hook_enable){
		return nanosleep_f(req,rem);
	}
	//
	int timeout_ms=req->tv_sec*1000+req->tv_nsec/1000/1000;
	JKYi::Fiber::ptr fiber=JKYi::Fiber::GetThis();
	JKYi::IOManager* iom=JKYi::IOManager::GetThis();

	iom->addTimer(timeout_ms,std::bind((void (JKYi::Scheduler::*)(JKYi::Fiber::ptr ,int thread))&JKYi::IOManager::schedule,iom,fiber,-1));
	JKYi::Fiber::YieldToHold();

	return 0;
}

int socket(int domain,int type,int protocol){
	if(!JKYi::t_hook_enable){
		return socket_f(domain,type,protocol);
	}
    //JKYI_LOG_INFO(g_logger)<<" hook socekt ---------";
	int fd=socket_f(domain,type,protocol);
	if(fd==-1){
		return fd; 
	}
	JKYi::FdMgr::GetInstance()->get(fd,true);
	return fd;
}

int connect_with_timeout(int socketfd,const struct sockaddr*addr,socklen_t addrlen,uint64_t timeout_ms){
   //
   if(!JKYi::t_hook_enable){
	   return connect_f(socketfd,addr,addrlen);
   }

   JKYi::FdCtx::ptr fd_ctx=JKYi::FdMgr::GetInstance()->get(socketfd);
   if(!fd_ctx || fd_ctx->isClose()){
	   errno=EBADF;
	   return -1;
   }
   //
   
   if(fd_ctx->getUserNonblock()){
	   return connect_f(socketfd,addr,addrlen);
   }

   int n=connect_f(socketfd,addr,addrlen);
   if(n==0){
	   return 0;
   }else if(n!=-1 || errno!= EINPROGRESS){
	   return n;
   }
   //执行到这里就代表当前连接未立刻建立

   JKYi::IOManager* iom=JKYi::IOManager::GetThis();
   JKYi::Timer::ptr  timer;

   std::shared_ptr<timer_info>tinfo(new timer_info);
   std::weak_ptr<timer_info>winfo(tinfo);
   
   if(timeout_ms!=(uint64_t)-1){
	   //用户有设有超时时间,所以这里需要加上一个定时器
	   timer=iom->addConditionTimer(timeout_ms,[winfo,socketfd,iom](){
             auto t=winfo.lock();
			 if(!t || t->cancelled){
			    return ;
			 }
			 t->cancelled=ETIMEDOUT;
			 iom->cancelEvent(socketfd,JKYi::IOManager::WRITE);
		 },winfo);
	   }
     //JKYI_LOG_DEBUG(g_logger)<<"connect_with_timeout 写事件的添加";
     int rt=iom->addEvent(socketfd,JKYi::IOManager::WRITE);
	 if(rt==0){
		 //添加成功
		 JKYi::Fiber::YieldToHold();
		 if(timer){
			 timer->cancel();
		 }
		 if(tinfo->cancelled){
			 errno=tinfo->cancelled;
			 return -1;
		 }
	 }else{
		 //如果添加失败的话
		 //
		 if(timer){
			 timer->cancel();
		 }
		 JKYI_LOG_ERROR(g_logger)<<"connection addEnent("<<socketfd<<",WRITE) error";
		 return -1;
	 }

	 //查看socket的错误码来判断连接是否建立成功
	 int error=0;
	 socklen_t len=sizeof(error);
	 if(-1==getsockopt(socketfd,SOL_SOCKET,SO_ERROR,&error,&len)){
		 return -1;
	 }
     if(error!=0){
		 errno=error;
		 return -1;
	 }
	 //连接建立成功
	 return 0;
}
 
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    //JKYI_LOG_INFO(g_logger)<<" hook.cc connect";
	return connect_with_timeout(sockfd, addr, addrlen, JKYi::s_connect_timeout);
		
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
	    int fd = do_io(s, accept_f, "accept", JKYi::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
		if(fd >= 0) {
		   JKYi::FdMgr::GetInstance()->get(fd, true);
		 }
		return fd;
}
//
ssize_t read(int fd,void *buf,size_t count){
	return do_io(fd,read_f,"read",JKYi::IOManager::READ,SO_RCVTIMEO,buf,count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
	return do_io(fd, readv_f, "readv", JKYi::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}
ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
	return do_io(sockfd, recv_f, "recv", JKYi::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
	return do_io(sockfd, recvfrom_f, "recvfrom", JKYi::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
	return do_io(sockfd, recvmsg_f, "recvmsg", JKYi::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
	return do_io(fd, write_f, "write", JKYi::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
	return do_io(fd, writev_f, "writev", JKYi::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
	return do_io(s, send_f, "send", JKYi::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", JKYi::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
   return do_io(s, sendmsg_f, "sendmsg", JKYi::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd){
	if(!JKYi::t_hook_enable){
		return close_f(fd);
	}
	JKYi::FdCtx::ptr fd_ctx=JKYi::FdMgr::GetInstance()->get(fd);

	if(fd_ctx){
		JKYi::IOManager* iom=JKYi::IOManager::GetThis();
		if(iom){
			iom->cancelAll(fd);
		}
		JKYi::FdMgr::GetInstance()->del(fd);
	}
	return close_f(fd);
}

//由于这个函数是一个可变参的函数，所以需要使用到va_list
int fcntl(int fd,int cmd,...){
   va_list va;

   //对va进行初始化，令它指向第一个可变的参数
   va_start(va,cmd);

   switch(cmd){
	   case F_SETFL:
		   {
              int arg=va_arg(va,int);
			  va_end(va);
			  //
			  JKYi::FdCtx::ptr fd_ctx=JKYi::FdMgr::GetInstance()->get(fd);
			  if(!fd_ctx || fd_ctx->isClose() ||!fd_ctx->isSocket()){
				  return fcntl_f(fd,cmd,arg);
			  }
			  //设置用户有没有主动的设置非阻塞
			  fd_ctx->setUserNonblock(arg & O_NONBLOCK);
			  //
			  if(fd_ctx->getSysNonblock()){
				  arg |= O_NONBLOCK;
			  }else{
				  arg &= ~O_NONBLOCK;
			  }
			  return fcntl_f(fd,cmd,arg);
		   }
		   break;
		case F_GETFL:
		    {
				va_end(va);
				int arg=fcntl_f(fd,cmd);

				JKYi::FdCtx::ptr fd_ctx=JKYi::FdMgr::GetInstance()->get(fd);
				if(!fd_ctx || fd_ctx->isClose() || !fd_ctx->isSocket()){
					return arg;
				}

				if(fd_ctx->getUserNonblock()){
					return arg | O_NONBLOCK;
				}else{
					return arg & ~O_NONBLOCK;
				}
			}
			break;
	    case F_DUPFD:
		case F_DUPFD_CLOEXEC:
		case F_SETFD:
		case F_SETOWN:
		case F_SETSIG:
		case F_SETLEASE:
		case F_NOTIFY:
#ifdef F_SETPIPE_SZ
		case F_SETPIPE_SZ:
#endif
            {
				int arg=va_arg(va,int);
				va_end(va);
				return fcntl_f(fd,cmd,arg);
			}
			break;
        case F_GETFD:
	    case F_GETOWN:
		case F_GETSIG:
	    case F_GETLEASE:
#ifdef F_GETPIPE_SZ
	    case F_GETPIPE_SZ:
#endif
	       {
			  va_end(va);
			  return fcntl_f(fd,cmd);
		   }
		    break;
		case F_SETLK:
		case F_SETLKW:
		case F_GETLK:
		  {
			struct flock*arg=va_arg(va,struct flock*);
			va_end(va);
			return fcntl_f(fd,cmd,arg);
		  }
		    break;
	    case F_GETOWN_EX:
		case F_SETOWN_EX:
		  {
			struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
			va_end(va);
			return fcntl_f(fd, cmd, arg);
		  }
			break;
		default:
            va_end(va);
			return fcntl_f(fd,cmd);
   }
}

int ioctl(int fd,unsigned long int request,...){
     va_list va;
	 va_start(va,request);

	 void * arg=va_arg(va,void *);
	 va_end(va);

	 if(FIONBIO==request){
		 //将int类型转化为bool类型
		 bool user_nonblock=!!*(int*)arg;
		 JKYi::FdCtx::ptr fd_ctx=JKYi::FdMgr::GetInstance()->get(fd);
		 if(!fd_ctx || fd_ctx->isClose() || !fd_ctx->isSocket()){
			 return ioctl_f(fd,request,arg);
		 }
		 fd_ctx->setUserNonblock(user_nonblock);
	 }
	 return ioctl_f(fd,request,arg);
}

int getsockopt(int fd,int level,int optname,void * optval,socklen_t* optlen){
    return getsockopt_f(fd,level,optname,optval,optlen);
}
//这个该函数就可以获得超时socket的超时时间
int setsockopt(int fd,int level,int optname,const void * optval,socklen_t optlen){
     if(!JKYi::t_hook_enable){
		 return setsockopt_f(fd,level,optname,optval,optlen);
	 }
	 if(level==SOL_SOCKET){
		 if(optname==SO_RCVTIMEO || optname==SO_SNDTIMEO){
			 JKYi::FdCtx::ptr fd_ctx=JKYi::FdMgr::GetInstance()->get(fd);
		     if(fd_ctx){
				 const timeval* v=(const timeval*)optval;
				 fd_ctx->setTimeout(optname,v->tv_sec*1000+v->tv_usec/1000);
		   }
	    }
	 }
	return setsockopt_f(fd,level,optname,optval,optlen);
}

}
