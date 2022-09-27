#include"JKYi/reactor/TcpServer.h"
#include"JKYi/log.h"
#include"JKYi/address.h"
#include"JKYi/reactor/EventLoop.h"

#include<stdio.h>
#include<string>

JKYi::Logger::ptr g_logger = JKYI_LOG_NAME("system");
void OnMessage(const JKYi::net::TcpConnectionPtr& conn,JKYi::net::Buffer* buf,
                JKYi::net::Timestamp receiveTime){
    std::string str = buf->retrieveAllAsString();
    std::cout << str << std::endl;
    conn->send(str);
    return ;
}
int main(int argc,char ** argv){
    char buf[65535];
    if(argc > 1){
       snprintf(buf,sizeof(buf),"127.0.0.1:%s",argv[1]);
       std::cout << buf << std::endl;
    }
    JKYi::Address::ptr addr = JKYi::Address::LookupAnyIPAddress(std::string(buf));   
    JKYi::net::EventLoop loop;
    JKYi::net::TcpServer server(&loop,addr,"echoServer");
    server.setMessageCallback(OnMessage);
    server.start();
    loop.loop();
    return 0;
}
