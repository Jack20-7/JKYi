#include"JKYi/reactor/examples/chargen/chargen.h"
#include"JKYi/reactor/Logging.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/timestamp.h"
#include"JKYi/reactor/TcpConnection.h"

#include<stdio.h>

using namespace JKYi;
using namespace JKYi::net;

ChargenServer::ChargenServer(EventLoop* loop,
                               const Address::ptr& listenAddr,
                               bool print)
    :server_(loop,listenAddr,"ChargenServer"),
     transferred_(0),
     startTime_(Timestamp::now()){

     server_.setConnectionCallback(
             std::bind(&ChargenServer::onConnection,this,_1));
     server_.setMessageCallback(
             std::bind(&ChargenServer::onMessage,this,_1,_2,_3));
     server_.setWriteCompleteCallback(
             std::bind(&ChargenServer::onWriteComplete,this,_1));

     if(print){
         loop->runEvery(3.0,std::bind(&ChargenServer::printThroughput,this));
     }

     std::string line;
     for(int i = 33;i < 127;++i){
         line.push_back(char(i));
     }
     line += line;

     message_ += line;

     LOG_DEBUG << " line = " << line
               << " message_ = " << message_;
}

void ChargenServer::start(){
    server_.start();
}

void ChargenServer::onConnection(const TcpConnectionPtr& conn){
    LOG_INFO << "ChargenServer - " << conn->peerAddress()->toString() << "-> " 
             << conn->localAddress()->toString() << " is " 
             << (conn->connected() ? "UP" : "DOWN");

    if(conn->connected()){
        conn->setTcpNoDelay(true);
        conn->send(message_);
    }
}

void ChargenServer::onMessage(const TcpConnectionPtr& conn,
                               Buffer* buf,
                               Timestamp receiveTime){
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " disacrds " << msg.size() 
             << " bytes received at " << receiveTime.toString();
}

void ChargenServer::onWriteComplete(const TcpConnectionPtr& conn){
    transferred_ += message_.size();
    conn->send(message_);
}

void ChargenServer::printThroughput(){
    Timestamp now(Timestamp::now());

    double time = timeDifference(now,startTime_);
    printf("%4.3f MiB/s\n",static_cast<double>(transferred_) / time / 1024 / 1024);
    transferred_ = 0;
    startTime_ = now;
}


