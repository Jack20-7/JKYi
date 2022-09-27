#include"JKYi/reactor/TcpConnection.h"
#include"JKYi/log.h"
#include"JKYi/reactor/channel.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/TcpConnection.h"
#include"JKYi/macro.h"
#include"JKYi/WeakCallback.h"

#include<errno.h>

namespace JKYi{
namespace net{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

//默认的连接回调
void defaultConnectionCallback(const TcpConnectionPtr& conn){
   JKYI_LOG_INFO(g_logger) << conn->localAddress()->toString() << " -> "
                           << conn->peerAddress()->toString() << " is " 
                           << (conn->connected() ? "UP" :"DOWN");
}
void defaultMessageCallback(const TcpConnectionPtr& conn,Buffer* buf,
                                       Timestamp receiveTime){
   buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop,const std::string& nameArg,int sockfd,
                        const Address::ptr localAddr,
                        const Address::ptr peerAddr)
    :loop_(loop),
     name_(nameArg),
     state_(kConnecting),
     reading_(true),
     socket_(new Socket(sockfd)),
     channel_(new Channel(loop,sockfd)),
     localAddr_(localAddr),
     peerAddr_(peerAddr),
     highWaterMark_(64 * 1024 * 1024){
     
     //为该连接设置最内层的回调函数
     channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this,_1));
     channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite,this));
     channel_->setCloseCallback(std::bind(&TcpConnection::handleClose,this));
     channel_->setErrorCallback(std::bind(&TcpConnection::handleError,this));

     JKYI_LOG_DEBUG(g_logger) << " TcpConnection::ctor[" << name_ << "] at " 
                              << this << " fd = " << sockfd;
     socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection(){
    JKYI_LOG_DEBUG(g_logger) << " TcpConnection::dtor << [ " << name_ << " ] at "
                             << this << " fd = " << socket_->getFd() 
                             << " state = " << stateToString();
    JKYI_ASSERT(state_ == kDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info* tcpi)const{
    return socket_->getTcpInfo(tcpi);
}
std::string TcpConnection::getTcpInfoString()const{
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf,sizeof(buf));
    return buf;
}

void TcpConnection::send(const void * data,int len){
    send(StringPiece(static_cast<const char *>(data),len));
}
void TcpConnection::send(const StringPiece& message){
    if(state_ == kConnected){
        if(loop_->isInLoopThread()){
            sendInLoop(message);
        }else{
            void (TcpConnection::* fp)(const StringPiece& message) = 
                &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp,this,message.as_string()));
            //why not pass message ? 
            //                                std::forward<string>(message)
        }
    }
    return ;
}

void TcpConnection::send(Buffer * buf){
    if(state_ == kConnected){
        if(loop_->isInLoopThread()){
            sendInLoop(buf->peek(),buf->readableBytes());
            buf->retrieveAll();
        }else{
            void (TcpConnection::* fp)(const StringPiece& message) = 
                &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp,this,buf->retrieveAllAsString()));
        }
    }
}

void TcpConnection::sendInLoop(const StringPiece& message){
    sendInLoop(message.data(),message.size());
}

//最终底层实际调用的发送数据的函数
void TcpConnection::sendInLoop(const void * message,size_t len){
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;          //当前已经写了的字节数
    size_t remaining = len;      //剩余要写的字节数
    bool faultError = false;
    if(state_ == kDisconnected){
        JKYI_LOG_WARN(g_logger) << " disconnected,give up writing";
        return ;
    }
    //这里会先判断能够直接写，如果能够就先直接调用write写一次
    //这样也是提高性能的一种方法，如果通过这样写完的话，也就避免的
    //再将数据拷贝到Buffer里面去.减少了一次拷贝所带来的开销
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0){
        nwrote = sockets::write(socket_->getFd(),message,len);
        if(nwrote >= 0){
            remaining = len - nwrote;
            ///如果直接写完了的话
            if(remaining == 0 && writeCompleteCallback_){
                loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
            }
        }else{ //nwrote < 0
            nwrote = 0;
            if(errno != EWOULDBLOCK){
                JKYI_LOG_ERROR(g_logger) << " TcpConnection::sendInLoop";
                if(errno == EPIPE || errno == ECONNRESET){
                    faultError = true;
                }
            }
        }
    }
    JKYI_ASSERT(remaining <= len);
    if(!faultError && remaining > 0){
        //如果上一次发送数据没有错误发送并且还有数据未发送完毕的话,就交给框架进行管理
        size_t oldLen = outputBuffer_.readableBytes();
        //水位控制
        if(oldLen + remaining  >= highWaterMark_ 
                && oldLen < highWaterMark_
                && highWaterMarkCallback_){
            loop_->queueInLoop(std::bind(highWaterMarkCallback_,shared_from_this(),
                        oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(message) + nwrote,remaining);
        if(!channel_->isWriting()){
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown(){
    if(state_ == kConnected){
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop,this));
    }
    return ;
}

void TcpConnection::shutdownInLoop(){
    loop_->assertInLoopThread();
    if(!channel_->isWriting()){
        //如果没有注册写事件
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceClose(){
    if(state_ == kConnected || state_ == kDisconnecting){
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop,
                                   shared_from_this()));
    }
}
void TcpConnection::forceCloseWithDelay(double seconds){
    if(state_ == kConnected || state_ == kDisconnecting){
        setState(kDisconnecting);
        loop_->runAfter(seconds,makeWeakCallback(shared_from_this()
                                          ,&TcpConnection::forceClose));
    }
}

void TcpConnection::forceCloseInLoop(){
    loop_->assertInLoopThread();
    if(state_ == kConnected || state_ == kDisconnecting){
        handleClose();
    }
}

const char * TcpConnection::stateToString()const{
    switch(state_){
      case kDisconnected:
        return "kDisconnected";
      case kConnecting:
        return "kConnecting";
      case kConnected:
        return "kConnected";
      case kDisconnecting:
        return "kDisconnected";
      default:
        return "unknow state";
    }
}

void TcpConnection::setTcpNoDelay(bool on){
    socket_->setTcpNoDelay(on);
}

void TcpConnection::startRead(){
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop,this));
}
void TcpConnection::startReadInLoop(){
    loop_->assertInLoopThread();
    if(!reading_ || !channel_->isReading()){
        reading_ = true;
        channel_->enableReading();
    }
}
void TcpConnection::stopRead(){
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop,this));
}
void TcpConnection::stopReadInLoop(){
    loop_->assertInLoopThread();
    if(reading_ || channel_->isReading()){
        reading_ = false;
        channel_->disableReading();
    }
}

//只会在连接建立时调用一次
void TcpConnection::connectEstablished(){
    loop_->assertInLoopThread();
    JKYI_ASSERT(state_ == kConnecting);
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed(){
    loop_->assertInLoopThread();
    if(state_ == kConnected){
        setState(kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());  //连接断开时还会调用一次连接回调函数
    }
    channel_->remove();
}
//连接socket最底层的读回调
void TcpConnection::handleRead(Timestamp receiveTime){
    loop_->assertInLoopThread();
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(socket_->getFd(),&savedErrno);
    if(n > 0){
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }else if(n == 0){
        handleClose();
    }else{
        errno = savedErrno;
        JKYI_LOG_ERROR(g_logger) << " TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite(){
    loop_->assertInLoopThread();
    if(channel_->isWriting()){
        ssize_t n = sockets::write(socket_->getFd(),inputBuffer_.peek(),
                                     inputBuffer_.readableBytes());
        if(n > 0){
          inputBuffer_.retrieve(n);
          if(inputBuffer_.readableBytes() == 0){
              channel_->disableWriting();
              if(writeCompleteCallback_){
                  loop_->queueInLoop(std::bind(writeCompleteCallback_,
                                               shared_from_this()));
              }
              if(state_ == kDisconnecting){
                  shutdownInLoop();
              }
          }
        }else{
            JKYI_LOG_ERROR(g_logger) << "TcpConnection::handleWrite";
        }
    }else{
        JKYI_LOG_ERROR(g_logger) << " connection fd = " << socket_->getFd() 
                                 << " is down ,no more writing";
    }
}

void TcpConnection::handleClose(){
    loop_->assertInLoopThread();
    JKYI_LOG_DEBUG(g_logger) << "fd = " << socket_->getFd() << "state_ = " 
                             << stateToString();
    JKYI_ASSERT(state_ == kConnected || state_ == kDisconnecting);

    setState(kDisconnected);
    channel_->disableAll();
    
    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    closeCallback_(guardThis);
}


void TcpConnection::handleError(){
    int err = sockets::getSocketError(socket_->getFd());
    JKYI_LOG_ERROR(g_logger) << " TcpConnection::handleError[ " << name_ 
                             << " ]- SO_ERROR = " << err  << " " << strerror(err);
}


}
}
