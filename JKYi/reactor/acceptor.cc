#include"JKYi/reactor/acceptor.h"
#include"JKYi/log.h"
#include"JKYi/reactor/Socket.h"
#include"JKYi/reactor/SocketsOps.h"
#include"JKYi/macro.h"

#include<errno.h>
#include<fcntl.h>
#include<unistd.h>

namespace JKYi{
namespace net{

static JKYi::Logger::ptr g_logger = JKYI_LOG_NAME("system");

Acceptor::Acceptor(EventLoop* loop,const Address::ptr& address,bool reuseport)
    :loop_(loop),
     acceptSocket_(sockets::createNonBlockingOrDie(address->getFamily())),
     acceptChannel_(loop,acceptSocket_.getFd()),
     listening_(false),
     idleFd_(::open("/dev/null",O_RDONLY | O_CLOEXEC)){

     JKYI_ASSERT(idleFd_ >= 0);
     acceptSocket_.setReuseAddr(true);
     acceptSocket_.setReusePort(reuseport);
     acceptSocket_.bindAddress(address);
     acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
     JKYI_LOG_DEBUG(g_logger) << " listening socket = " << acceptSocket_.getFd();
}

Acceptor::~Acceptor(){
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}
void Acceptor::listen(){
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead(){
   loop_->assertInLoopThread();
   Address::ptr addr;
   int connfd = acceptSocket_.accept(addr);
   if(connfd >= 0){
      if(newConnectionCallback_){
          newConnectionCallback_(connfd,addr);
       }else{
           sockets::close(connfd);
       }
   }else{
       JKYI_LOG_ERROR(g_logger) << " in Acceptor::handleRead error ";
       if(errno == EMFILE){
           //文件描述符耗尽
           ::close(idleFd_);
           idleFd_ = ::accept(acceptSocket_.getFd(),NULL,NULL);
           ::close(idleFd_);   //显式的关闭该链接
           idleFd_ = ::open("/dev/null",O_RDONLY | O_CLOEXEC);  //然后重新对文件描述符进行占用
       }
   }
}

}
}
