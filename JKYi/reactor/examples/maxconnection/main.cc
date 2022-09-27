#include"JKYi/reactor/examples/maxconnection/echo.h"
#include"JKYi/reactor/EventLoop.h"

#include<iostream>
#include<stdio.h>

int main(int argc,char ** argv){
   if(argc <= 1){
       printf("to less paras");
       exit(0);
   }
   char buf[32];
   snprintf(buf,sizeof(buf),"127.0.0.1:%s",argv[1]);
   JKYi::Address::ptr serverAddr = JKYi::Address::LookupAnyIPAddress(std::string(buf));
   JKYi::net::EventLoop loop;
   int maxConnections = 5;

   EchoServer server(&loop,serverAddr,maxConnections);
   server.start();
   loop.loop();
   return 0;
}
