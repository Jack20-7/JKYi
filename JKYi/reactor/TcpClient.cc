#include"JKYi/reactor/TcpClient.h"
#include"JKYi/log.h"
#include"JKYi/reactor/Connector.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/SocketsOps.h"
#include"JKYi/macro.h"

#include<stdio.h>

namespace JKYi{
namespace net{
namespace detail{

void removeConnection(EventLoop* loop,const TcpConnection::ptr& conn){
    loop->queueInLoop(
            std::bind(&TcpConnection::connectDestroyed,conn));
}

void removeConnector(const ConnectorPtr& connector){
}
} //namespace detail

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

TcpClient::TcpClient(EventLoop* loop,const Address::ptr& serverAddr,
                     const std::string& nameArg)
   :loop_(loop),
    connector_(new Connector(loop,serverAddr)),
    name_(nameArg),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    retry_(false),
    connect_(true),
    nextConnId_(1){

    connector_->setNewConnectionCallback(
            std::bind(&TcpClient::newConnection,this,_1));
    JKYI_LOG_INFO(g_logger) << "TcpClient::TcpClient[ " << name_ << " ]";
}

TcpClient::~TcpClient(){
    JKYI_LOG_INFO(g_logger) << " TcpClient::~TcpClient [ " << name_ << "] ";

    TcpConnection::ptr conn;
    bool unique = false;
    {
        Mutex::Lock lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if(conn){
        JKYI_ASSERT(loop_ == conn->getLoop());
        CloseCallback cb = std::bind(&detail::removeConnection,loop_,_1);
        loop_->runInLoop(std::bind(
                    &TcpConnection::setCloseCallback,conn,cb));
        if(unique){
            conn->forceClose();
        }
    }else{
        connector_->stop();
        loop_->runAfter(1,
                std::bind(&detail::removeConnector,connector_));
    }
}

void TcpClient::connect(){
    JKYI_LOG_INFO(g_logger) << " TcpClient::connect [ " << name_ << "]  connectiong to"
                            << connector_->serverAddress()->toString();
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect(){
    connect_ = false;
    {
        Mutex::Lock lcok(mutex_);
        if(connection_){
            connection_->shutdown();
        }
    }
}

void TcpClient::stop(){
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd){
    loop_->assertInLoopThread();
    struct sockaddr_in6 peerAddr_in6 = sockets::getPeerAddr(sockfd);
    struct sockaddr * peerAddr_ = sockets::sockaddr_cast(&peerAddr_in6);
    Address::ptr peerAddr = JKYi::Address::Create(peerAddr_,
                                  static_cast<socklen_t>(sizeof(*peerAddr_)));
    char buf[32];
    snprintf(buf,sizeof(buf),":%s#%d",peerAddr->toString().c_str(),nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    struct sockaddr_in6 localAddr_in6 = sockets::getLocalAddr(sockfd);
    struct sockaddr* localAddr_ = sockets::sockaddr_cast(&localAddr_in6);
    Address::ptr localAddr = JKYi::Address::Create(localAddr_,
                                   static_cast<socklen_t>(sizeof(*localAddr_)));
    TcpConnection::ptr conn(new TcpConnection(loop_,
                                              connName,
                                              sockfd,
                                              localAddr,
                                              peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
            std::bind(&TcpClient::removeConnection,this,_1));
    {
        Mutex::Lock lock(mutex_);
        connection_ = conn;
    }
    connection_->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnection::ptr& conn){
    loop_->assertInLoopThread();
    JKYI_ASSERT(loop_ == conn->getLoop()); 
    {
        Mutex::Lock lock(mutex_);
        JKYI_ASSERT(conn == connection_);
        connection_.reset();
    }
    loop_->queueInLoop(
            std::bind(&TcpConnection::connectDestroyed,conn));
    if(retry_ && connect_){
        JKYI_LOG_INFO(g_logger) << " TcpConnection::connect[ " << name_ 
                                << " ] - Reconnecting to " 
                                << connector_->serverAddress()->toString();
        connector_->restart();
    }
}


}
}
