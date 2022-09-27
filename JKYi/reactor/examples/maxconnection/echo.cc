#include"JKYi/reactor/examples/maxconnection/echo.h"
#include"JKYi/log.h"

using namespace JKYi;
using namespace JKYi::net;

Logger::ptr g_logger = JKYI_LOG_NAME("system");

EchoServer::EchoServer(EventLoop *loop,
                         const Address::ptr& serverAddr,
                         int maxConnections)
     :server_(loop,serverAddr,"EchoServer"),
      numConnected_(0), 
      kMaxConnections_(maxConnections){

    server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection,this,_1));
    server_.setMessageCallback(
            std::bind(&EchoServer::onMessage,this,_1,_2,_3));
}

void EchoServer::start(){
    server_.start();
}

void EchoServer::onConnection(const TcpConnection::ptr& conn){
    JKYI_LOG_INFO(g_logger) << "EchoServer - " << conn->peerAddress()->toString()
                            << " -> " <<conn->localAddress()->toString()
                            << " is " << (conn->connected() ? "UP" : "DOWN");
    if(conn->connected()){
        ++numConnected_;
        if(numConnected_ > kMaxConnections_){
            conn->shutdown();
            conn->forceCloseWithDelay(3.0);
        }
    }else{
        --numConnected_;
    }
    JKYI_LOG_INFO(g_logger) << " numConnected_ = " << numConnected_;
}
void EchoServer::onMessage(const TcpConnection::ptr& conn,
                            Buffer* buf,
                            Timestamp receiveTime){
    std::string str(buf->retrieveAllAsString());
    JKYI_LOG_INFO(g_logger) << conn->name() << " echo " << str.size() 
                            << " bytes at " << receiveTime.toFormattedString()
                            << "[ " << str << "]";
    conn->send(str);
}



