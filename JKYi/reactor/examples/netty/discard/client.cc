#include"JKYi/reactor//reactor.h"
#include"JKYi/log.h"
#include"JKYi/macro.h"

#include<utility>
#include<stdio.h>
#include<unistd.h>
#include<string>

using namespace JKYi;
using namespace JKYi::net;

static Logger::ptr g_logger = JKYI_LOG_ROOT();
class DiscardClient : public Noncopyable{
public:
    DiscardClient(EventLoop* loop,const Address::ptr& serverAddr,int size)
        :loop_(loop),
         client_(loop,serverAddr,"DiscardServer"),
         message_(size,'H'){

         client_.setConnectionCallback(
                std::bind(&DiscardClient::onConnection,this,_1));
         client_.setMessageCallback(
                std::bind(&DiscardClient::onMessage,this,_1,_2,_3));
         client_.setWriteCompleteCallback(
                std::bind(&DiscardClient::onWriteComplete,this,_1));
    }
    void connect(){
        client_.connect();
    }
private:
    void onConnection(const TcpConnection::ptr& conn){
       JKYI_LOG_INFO(g_logger) << conn->peerAddress()->toString() << " -> "
                               << conn->localAddress()->toString() << " is " 
                               << (conn->connected() ? "UP" : "DOWN");
       if(conn->connected()){
           conn->setTcpNoDelay(true);
           conn->send(message_);
       }else{
           loop_->quit();
       }
    }
    void onMessage(const TcpConnection::ptr& conn,Buffer* buf,Timestamp receiveTime){
        buf->retrieveAll();
    }
    void onWriteComplete(const TcpConnection::ptr& conn){
        JKYI_LOG_INFO(g_logger) << " write complete " << message_.size();
        conn->send(message_);
    }

    EventLoop* loop_;
    TcpClient client_;
    std::string message_;
};


int main(int argc,char ** argv){
    if(argc <= 2){
        printf("too less params");
        exit(0);
    }
    g_logger->setLevel(LogLevel::ERROR);
    char buf[64];
    snprintf(buf,sizeof(buf),"%s:%s",argv[1],argv[2]);
    Address::ptr serverAddr = Address::LookupAnyIPAddress(std::string(buf));
    EventLoop loop;
    DiscardClient client(&loop,serverAddr,10);
    client.connect();
    loop.loop();
    return 0;
}
