#include"../JKYi/JKYi.h"

static JKYi::Logger::ptr g_logger=JKYI_LOG_ROOT();
void test(){
   JKYi::IPAddress::ptr addr=JKYi::Address::LookupAnyIPAddress("www.baidu.com");
   if(addr){
      JKYI_LOG_INFO(g_logger)<<"addr="<<addr->toString();
   }else{
      JKYI_LOG_ERROR(g_logger)<<" get Address fail";
      return ;
   }

   JKYi::Socket::ptr sock=JKYi::Socket::CreateTCP(addr);
   addr->setPort(80);
   if(!sock->connect(addr)){
      JKYI_LOG_ERROR(g_logger)<<" connect "<<addr->toString()<<" fail";
      return ;
   }else{
      JKYI_LOG_INFO(g_logger)<<" connect "<<addr->toString()<<" successed";
   }
   //接下来就是发送消息测试一下
   const char  msg[]="GET / HTTP/1.0\r\n\r\n";
   int rt=sock->send(msg,sizeof(msg));
   if(rt<=0){
      JKYI_LOG_ERROR(g_logger)<<" send msg error,rt="<<rt;
      return ;
   }
   //接受返回的数据
   std::string buff;
   buff.resize(4096);
   rt=sock->recv(&buff[0],buff.size());
   if(rt<=0){
       JKYI_LOG_ERROR(g_logger)<<" recv msg error,rt="<<rt;
       return ;
   }
   buff.resize(rt);
   JKYI_LOG_INFO(g_logger)<<buff;
   return ;
   
}
int main(int argc,char**argv){
    JKYi::IOManager iom;
    iom.schedule(&test);
    return 0;
}
