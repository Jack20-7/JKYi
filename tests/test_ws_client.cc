#include"JKYi/http/ws_connection.h"
#include"JKYi/iomanager.h"
#include"JKYi/util.h"

JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();

void run(){
  auto rt = JKYi::http::WSConnection::Create("http://127.0.0.1:8090/JKYi",1000); 
  if(!rt.second){
      JKYI_LOG_ERROR(g_logger) << rt.first->toString();
      return ;
  }
  auto conn = rt.second;
  while(true){
      conn->sendMessage("hello,websocket",JKYi::http::WSFrameHead::TEXT_FRAME,false);

      conn->sendMessage("hello,JKYi",JKYi::http::WSFrameHead::TEXT_FRAME,true);
      auto msg = conn->recvMessage();
      if(!msg){
          break;
      }
      JKYI_LOG_INFO(g_logger) << "opcode = "<<msg->getOpcode() << "  data = " << msg->getData();

      sleep(10);
  }
}
int main(int argc,char ** argv){
    JKYi::IOManager iom;
    iom.schedule(run);
    return 0;
}
