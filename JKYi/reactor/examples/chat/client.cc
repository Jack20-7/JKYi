#include"JKYi/reactor/examples/chat/codec.h"
#include"JKYi/reactor/Logging.h"
#include"JKYi/mutex.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/TcpClient.h"
#include"JKYi/reactor/EventLoopThread.h"

#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include<string>

using namespace JKYi;
using namespace JKYi::net;

class ChatClient : Noncopyable{
public:
    ChatClient(EventLoop* loop,const Address::ptr& serverAddr)
        :client_(loop,serverAddr,"ChatClient"),
         codec_(std::bind(&ChatClient::onStringMessage,this,_1,_2,_3)){
         
         client_.setConnectionCallback(
                 std::bind(&ChatClient::onConnection,this,_1));
         client_.setMessageCallback(
                 std::bind(&LengthHeaderCodec::onMessage,&codec_,_1,_2,_3));
         client_.enableRetry();
     }
     void connect(){
         client_.connect();
     }
     void disconnect(){
         client_.disconnect();
     }
     void write(const StringPiece& message){
         Mutex::Lock lock(mutex_);
         if(connection_){
             codec_.send(connection_.get(),message);
         }
     }
private:
    void onConnection(const TcpConnectionPtr& conn){
        LOG_INFO << conn->localAddress()->toString() << " -> "
                 << conn->peerAddress()->toString() << " is "
                 << (conn->connected() ? "UP" : "DOWN");
        Mutex::Lock lock(mutex_);
        if(conn->connected()){
            connection_ = conn;
        }else{
            connection_.reset();
        }
    }
    void onStringMessage(const TcpConnectionPtr& conn,
                          const std::string& message,
                          Timestamp receiveTime){
        printf("%s\n",message.c_str());
    }

    TcpClient client_;
    LengthHeaderCodec codec_;
    Mutex mutex_;
    TcpConnectionPtr connection_;
};


int main(int argc,char** argv){
    LOG_INFO << " pid = " << getpid();
    if(argc > 2){
        EventLoopThread loopThread;
        Address::ptr serverAddr = Address::LookupAnyIPAddress(argv[1],argv[2]);
        ChatClient client(loopThread.startLoop(),serverAddr);
        client.connect();

        std::string line;
        while(std::getline(std::cin,line)){
            client.write(line);
        }
        client.disconnect();
        CurrentThread::sleepUsec(1000 * 1000);
    }else{
        printf("Usage: %s host_ip port\n",argv[0]);
    }
    return 0;
}
