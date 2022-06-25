#include"../JKYi/JKYi.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<iostream>
#include<sys/epoll.h>

JKYi::Logger::ptr g_logger=JKYI_LOG_ROOT();

int sock=0;
void test_fiber(){
	JKYI_LOG_INFO(g_logger)<<"test_fiber sock="<<sock;

	sock=socket(AF_INET,SOCK_STREAM,0);
	fcntl(sock,F_SETFL,O_NONBLOCK);
	//
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(80);
	inet_pton(AF_INET, "220.181.38.148", &addr.sin_addr.s_addr);
	if(!connect(sock,(sockaddr*)&addr,sizeof(addr))){

	}else if(errno==EINPROGRESS){
	   JKYI_LOG_INFO(g_logger)<<"add event errno="<<errno<<"  "<<strerror(errno);
	   JKYi::IOManager::GetThis()->addEvent(sock,JKYi::IOManager::READ,[](){
		   JKYI_LOG_INFO(g_logger)<<"read callback";
	   });

	   JKYi::IOManager::GetThis()->addEvent(sock,JKYi::IOManager::WRITE,[](){
           JKYI_LOG_INFO(g_logger)<<"write callback";
		   //
		   JKYi::IOManager::GetThis()->cancelEvent(sock,JKYi::IOManager::READ);
		   close(sock);
	   });
	}else{
		JKYI_LOG_INFO(g_logger)<<"connectiong is establish";
	}
}


void test1(){
   JKYi::IOManager iom(2,false);
   iom.schedule(&test_fiber);
}

JKYi::Timer::ptr s_timer;
void test_timer(){
	JKYi::IOManager iom(2);
	s_timer=iom.addTimer(1000,[](){
      static int i=0;
	  JKYI_LOG_INFO(g_logger)<<"hello timer i="<<i;
	  if(++i==3){
		  s_timer->reset(2000,true);
		  //s_timer->cancel();
	  }
	},true);

}

int main(int argc,char**argv){
    //test_timer();
	test1();
	return 0;
}
