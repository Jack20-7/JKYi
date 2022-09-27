#include"JKYi/reactor/examples/idleconnection/echo.h"
#include"JKYi/reactor/reactor.h"
#include"JKYi/log.h"

#include<stdio.h>

JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();
int main(int argc,char ** argv){
    if(argc <= 1){
       printf("ot less params");
       exit(0);
    }
    char buf[32];
    snprintf(buf,sizeof(buf),"127.0.0.1:%s",argv[1]);
    JKYi::Address::ptr serverAddr = JKYi::Address::LookupAnyIPAddress(std::string(buf));
    JKYi::net::EventLoop loop;
    EchoServer server(&loop,serverAddr,5);
    server.start();
    loop.loop();
    return 0;
}
