#include"../JKYi/JKYi.h"

#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string>
#include<string.h>


static JKYi::Logger::ptr g_logger=JKYI_LOG_ROOT();
void test_sleep(){
	JKYi::IOManager iom(1);
	iom.schedule([](){
      sleep(2);
	  JKYI_LOG_INFO(g_logger)<<"sleep 2";
	});
	
	iom.schedule([](){
      sleep(3);
	  JKYI_LOG_INFO(g_logger)<<"sleep 3";
	});
	JKYI_LOG_INFO(g_logger)<<"test sleep";
	return ;
}

void test_sock(){
	int sock=socket(AF_INET,SOCK_STREAM,0);

	sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(80);

   inet_pton(AF_INET,"220.181.38.148",&addr.sin_addr.s_addr);

   JKYI_LOG_INFO(g_logger)<<"begin connect";
   int rt=connect(sock,(struct sockaddr*)&addr,sizeof(addr));
   JKYI_LOG_INFO(g_logger)<<"connect rt="<<rt<<" errno="<<errno;

   if(rt!=0){
	   return ;
   }
   //然后向百度服务器发送消息
   const char data[]="GET / HTTP/1.0\r\n\r\n";
   rt=send(sock,data,sizeof(data),0);
   JKYI_LOG_INFO(g_logger)<<"send rt="<<rt<<" errno="<<errno;

   if(rt<=0){
	   return ;
   }
   //
   std::string buff;
   buff.resize(4096);

   rt=recv(sock,&buff[0],buff.size(),0);
   JKYI_LOG_INFO(g_logger)<<"recv rt="<<rt<<" errno="<<errno;

   if(rt<=0){
	   return  ;
   }

   buff.resize(rt);
   JKYI_LOG_INFO(g_logger)<<buff;
}

int main(int argc,char**argv){
	test_sleep();
	//JKYi::IOManager iom;
	//iom.schedule(test_sock);
	return 0;
}
