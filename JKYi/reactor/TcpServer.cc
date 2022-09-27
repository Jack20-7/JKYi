#include"JKYi/reactor/TcpServer.h"
#include"JKYi/log.h"
#include"JKYi/reactor/acceptor.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/EventLoopThreadPool.h"
#include"JKYi/reactor/SocketsOps.h"
#include"JKYi/macro.h"

#include<stdio.h>

namespace JKYi{
namespace net{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");
TcpServer::TcpServer(EventLoop* loop,
                     const Address::ptr& listenAddr,
                     const std::string& nameArg,
                     Option option)
    :loop_(loop),
     ipPort_(listenAddr->toString()),
     name_(nameArg),
     acceptor_(new Acceptor(loop,listenAddr,option == kReusePort)),
     threadPool_(new EventLoopThreadPool(loop,nameArg)),
     connectionCallback_(defaultConnectionCallback),
     messageCallback_(defaultMessageCallback),
     nextConnId_(1){

     acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,
                                         _1,_2));
}

TcpServer::~TcpServer(){
    loop_->assertInLoopThread();
    JKYI_LOG_DEBUG(g_logger) << "TcpServer::~TcpServer[ " << name_ << "] destructing";

    for(auto& item : connections_){
        //每一条TCP连接必须由该连接所在的线程亲自来销毁
        TcpConnection::ptr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
                std::bind(&TcpConnection::connectDestroyed,conn));
    }
}

void TcpServer::setThreadNum(int numThreads){
    JKYI_ASSERT(numThreads >= 0);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start(){
    if(started_.getAndSet(1) == 0){
        threadPool_->start(threadInitCallback_);

        JKYI_ASSERT(!acceptor_->listening());
        loop_->runInLoop(
                std::bind(&Acceptor::listen,get_pointer(acceptor_)));
    }
}

void TcpServer::newConnection(int sockfd,const Address::ptr& peerAddr){
    loop_->assertInLoopThread();
    //默认采用round-bin的方式来给创建的连接分配eventloop
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf,sizeof(buf),"-%s#%d",ipPort_.c_str(),nextConnId_);
    ++nextConnId_;

    std::string connName = name_ + buf;

    JKYI_LOG_INFO(g_logger) << " TcpServer::newConnection[ " 
                            << name_ << "] - new connection [ " 
                            << connName << " ] from " << peerAddr->toString();

    struct sockaddr_in6 addr = sockets::getLocalAddr(sockfd);
    struct sockaddr * p = sockets::sockaddr_cast(&addr);
    Address::ptr localAddr = JKYi::Address::Create(p,
              static_cast<size_t>(sizeof(*p)));

    TcpConnection::ptr conn(new TcpConnection(ioLoop,connName,sockfd,
                                              localAddr,
                                              peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
            std::bind(&TcpServer::removeConnection,this,_1));

    ioLoop->runInLoop(
            std::bind(&TcpConnection::connectEstablished,conn));
}

void TcpServer::removeConnection(const TcpConnection::ptr& conn){
    loop_->runInLoop(
            std::bind(&TcpServer::removeConnectionInLoop,this,conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnection::ptr& conn){
    loop_->assertInLoopThread();
    JKYI_LOG_INFO(g_logger) << "TcpServer::removeConnectionInLoop [ " 
                            << name_  << " ] - connection " << conn->name();

    size_t n = connections_.erase(conn->name());
    (void)n;
    JKYI_ASSERT(n == 1);

    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
            std::bind(&TcpConnection::connectDestroyed,conn));
}

}
}
