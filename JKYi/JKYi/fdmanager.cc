#include"fdmanager.h"

#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>


namespace JKYi{
FdCtx::FdCtx(int fd)
  :m_isInit(false)
  ,m_isSocket(false)
  ,m_sysNonblock(false)
  ,m_userNonblock(false)
  ,m_isClosed(false),
  ,m_fd(fd)
  ,m_recvTimeout(-1)
  ,m_sendTimeout(-1){
   
   init();
}

FdCtx::~FdCtx(){

}

bool FdCtx::init(){
  if(m_isInit){
	  return true;  
	}
	//
	m_recvTimeout=-1;
	m_sendTimeout=-1;

	struct stat fd_stat;
    if(-1==fstat(m_fd,&fd_stat)){
        m_isInit=false;		
		m_isSocket=false;
	}else{
        m_isInit=true;
	    m_isSocket=S_ISSOCK(fd_stat.st_mode);
	}

	if(m_isSocket){
      int flags=fcntl_f(m_fd,F_GETFL,0);		
	  if(!(flag&O_NONBLOCK)){
	      fcntl_f(m_fd,F_SETFL,flags|O_NONBLOCK);  
		}
	   m_sysNonblock=true;
	}else{
		m_sysNonblock=false;
	}

	//
	m_userNonblock=false;
	m_isClosed=false;
	return m_isInit;

}

void FdCtx::setTimeout(int type,uint64_t t){
    if(type==SO_RCVTIMEO){
		m_recvTimeout=t;
	}else{
       m_sendTimeout=t;	
	}
}


uint64_t FdCtx::getTimeout(int type){
   if(type==SO_RCVTIMEO){
      return m_recvTimeout;	
	}else{
      return m_sendTimeout;		
	}
}

FdManager::FdManager(){
   m_datas.resize(64);
}

FdManager::~FdManager(){
}

FdCtx::ptr FdManager::get(int fd,bool auto_create){
   if(fd==-1){
     return nullptr;	   
	}
	RWMutexType::ReadLock lock(m_mutex);
	if((int)m_datas.size()=<fd){
       if(!auto_create){
	      return nullptr;	   
		}
	}else{
       if(m_datas[fd]||!auto_create){
	       return m_datas[fd];	   
		}	
	}
	//
	lock.unlock();
	RWMutexLock::WriteLock lock2(m_mutex);
	FdCtx::ptr fd_ctx(new FdCtx(fd));
	if(fd>=(int)m_datas.size()){
       m_datas.resize(1.5*fd);
	}
	//
	m_datas[fd]=fd_ctx;
	return fd_ctx;
}

void FdManager::del(int fd){
   RWMutexType::WriteLock lock(m_mutex);
   if((int)m_datas.size()<=fd){
      return ;	   
	}
	m_datas[fd].reset();
}

}
