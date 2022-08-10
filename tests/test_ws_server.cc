#include"JKYi/http/ws_server.h"
#include"JKYi/log.h"
#include"JKYi/iomanager.h"

static JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();

void run(){
    JKYi::http::WSServer::ptr server(new JKYi::http::WSServer());
    JKYi::Address::ptr addr = JKYi::Address::LookupAnyIPAddress("0.0.0.0:8020");
    if(!addr){
      JKYI_LOG_ERROR(g_logger) << "get addr error";
      return ;
    }
    auto fun = [](JKYi::http::HttpRequest::ptr req,JKYi::http::WSFrameMessage::ptr msg,JKYi::http::WSSession::ptr session){
                      session->sendMessage(msg);
                      return 0;
    };
    server->getWSServletDispatch()->addServlet("/JKYi",fun);
    while(!server->bind(addr)){
        JKYI_LOG_ERROR(g_logger) << "bind :" << addr->toString() << "fail";
        sleep(1);
    }
    server->start();
}
int main(int argc,char ** argv){
    JKYi::IOManager iom;
    iom.schedule(run);
    return 0;
}
