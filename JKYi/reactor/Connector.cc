#include"JKYi/reactor/Connector.h"
#include"JKYi/log.h"
#include"JKYi/reactor/channel.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/SocketsOps.h"
#include"JKYi/macro.h"

#include<errno.h>

namespace JKYi{
namespace net{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop,const Address::ptr& serverAddr)
    :loop_(loop),
     serverAddr_(serverAddr),
     connect_(false),
     state_(kDisconnected),
     retryDelayMs_(kInitRetryDelayMs){

    JKYI_LOG_DEBUG(g_logger) << "Connector::Connector [ " << this << " ]";
}

Connector::~Connector(){
    JKYI_LOG_DEBUG(g_logger) << " Connector::~Connector [ " << this << "]";
    JKYI_ASSERT(!channel_);
}

void Connector::start(){
    connect_ = true;
    loop_->runInLoop(
            std::bind(&Connector::startInLoop,this));
}

void Connector::startInLoop(){
    loop_->assertInLoopThread();
    JKYI_ASSERT(state_ == kDisconnected);
    if(connect_){
        connect();
    }else{
        JKYI_LOG_DEBUG(g_logger) << " do not connect";
    }
    return ;
}

void Connector::stop(){
    connect_ = false;
    loop_->queueInLoop(
            std::bind(&Connector::stopInLoop,this));
}

void Connector::stopInLoop(){
    loop_->assertInLoopThread();
    if(state_ == kConnecting){
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect(){
   int sockfd = sockets::createNonBlockingOrDie(serverAddr_->getFamily());
   int ret = sockets::connect(sockfd,serverAddr_->getAddr());
   int savedErrno = (ret == 0) ? 0 : errno;
   switch(savedErrno){
       case 0:
       case EINPROGRESS:
       case EINTR:
       case EISCONN:
           //connec成功的分支
           connecting(sockfd);
           break;
       case EAGAIN:
       case EADDRINUSE:
       case EADDRNOTAVAIL:
       case ECONNREFUSED:
       case ENETUNREACH:
           //connect未成功，但是不算错误
           retry(sockfd);
           break;
       case EACCES:
       case EPERM:
       case EAFNOSUPPORT:
       case EALREADY:
       case EBADF:
       case EFAULT:
       case ENOTSOCK:
            JKYI_LOG_ERROR(g_logger)<< "connect error in Connector::startInLoop " 
                                    << savedErrno;
            sockets::close(sockfd);
            break;

       default:
            JKYI_LOG_ERROR(g_logger)<< "Unexpected error in Connector::startInLoop " 
                                    << savedErrno;
            sockets::close(sockfd);
            break; 
   }
}

void Connector::restart(){
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::connecting(int sockfd){
    setState(kConnecting);
    JKYI_ASSERT(!channel_);
    channel_.reset(new Channel(loop_,sockfd));
    channel_->setWriteCallback(
            std::bind(&Connector::handleWrite,this));
    channel_->setErrorCallback(
            std::bind(&Connector::handleError,this));
    channel_->enableWriting();
}

int Connector::removeAndResetChannel(){
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->getFd();
    loop_->queueInLoop(
            std::bind(&Connector::resetChannel,this));
    return sockfd;
}

void Connector::resetChannel(){
    channel_.reset();
}

void Connector::handleWrite(){
    JKYI_LOG_DEBUG(g_logger) << "Connector::handleWrite " << state_;
    if(state_ == kConnecting){
        int sockfd = removeAndResetChannel();  //注销掉写事件，避免busy-loop
        int err = sockets::getSocketError(sockfd);
        if(err){
            JKYI_LOG_WARN(g_logger) << "Connector::handleWrite - SO_ERROR = " 
                                    << err << "  " << strerror(errno);
            retry(sockfd);
        }else if(sockets::isSelfConnect(sockfd)){
            JKYI_LOG_WARN(g_logger) << " Connector::handleWrite - Self connect";
            retry(sockfd);
        }else{
            setState(kConnected);
            if(connect_){
                newConnectionCallback_(sockfd);
            }else{
                sockets::close(sockfd);
            }
        }
    }else{
        JKYI_ASSERT(state_ == kDisconnected);
    }
}

void Connector::handleError(){
    JKYI_LOG_ERROR(g_logger) << "Connector::handleError state = " << state_;
    if(state_ == kConnecting){
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        JKYI_LOG_DEBUG(g_logger) << " SO_ERROR = "  << err << "  "
                                 << strerror(errno);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd){
    sockets::close(sockfd);
    setState(kDisconnected);
    if(connect_){
        JKYI_LOG_INFO(g_logger) << "Connector::retry - Retry connecting to " 
                                << serverAddr_->toString() << " in " << retryDelayMs_
                                << "milliseconds";
        loop_->runAfter(retryDelayMs_ / 1000.0,
                std::bind(&Connector::startInLoop,shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_,2 * kMaxRetryDelayMs);
    }else{
        JKYI_LOG_DEBUG(g_logger) << " do not connect";
    }
    return ;
}




}
}

