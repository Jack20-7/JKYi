#include"JKYi/reactor/TcpClient.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/TcpConnection.h"
#include"JKYi/log.h"
#include"JKYi/reactor/EventLoopThread.h"

#include<stdio.h>
#include<string>

JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();

JKYi::net::TcpConnection::ptr g_conn;
void onConnection(const JKYi::net::TcpConnection::ptr& conn){
    if(conn->connected()){
       JKYI_LOG_INFO(g_logger) << " connecction [ " << conn->localAddress()->toString() << " ] "
                               << " -> " << "[ " << conn->peerAddress()->toString() << " ] " 
                               << " is established";
       g_conn = conn;
    }else{
        JKYI_LOG_INFO(g_logger) << " connecction [ " << conn->localAddress()->toString() << " ] "
                               << " -> " << "[ " << conn->peerAddress()->toString() << " ] " 
                               << " is destroyed";
        g_conn = nullptr;
    }
    return ;
}

void onMessage(const JKYi::net::TcpConnection::ptr& conn,JKYi::net::Buffer* buf,JKYi::net::Timestamp receiveTime){
    std::string str(buf->retrieveAllAsString());
    JKYI_LOG_INFO(g_logger) << " receive message : " << str;
}

int main(int argc,char * *argv){
    if(argc <= 2){
        printf("to less params");
        exit(0);
    }
    char buf[32];
    snprintf(buf,sizeof(buf),"%s:%s",argv[1],argv[2]);
    JKYi::Address::ptr serverAddr = JKYi::Address::LookupAnyIPAddress(std::string(buf));

    JKYi::net::EventLoopThread thread;

    JKYi::net::TcpClient client(thread.startLoop(),serverAddr,"tcpClient");
    client.setConnectionCallback(&onConnection);
    client.setMessageCallback(onMessage);
    client.connect();

    std::string line; 
    while(std::getline(std::cin,line) && g_conn){
       g_conn->send(line); 
    }
    return 0;
}
