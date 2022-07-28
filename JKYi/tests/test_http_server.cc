#include"../JKYi/JKYi.h"

static JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();

void run(){
  JKYi::http::HttpServer::ptr server(new JKYi::http::HttpServer(true));
  JKYi::Address::ptr addr = JKYi::Address::LookupAnyIPAddress("0.0.0.0:8020");
  while(!server->bind(addr)){
      sleep(2);
  }
  auto sd = server->getServletDispatch();
  sd->addServlet("/JKYi/xx",[](JKYi::http::HttpRequest::ptr request,JKYi::http::HttpResponse::ptr response,JKYi::http::HttpSession::ptr session){
          response->setBody(request->toString());
          return 0;
      });
  sd->addGlobServlet("/JKYi/*",[](JKYi::http::HttpRequest::ptr request,JKYi::http::HttpResponse::ptr response,JKYi::http::HttpSession::ptr session){
          response->setBody("Glob:\r\n" + request->toString());
          return 0;
      });
  server->start();  
}
int main(int argc,char ** argv){
    JKYi::IOManager iom;
    iom.schedule(&run);
    return 0;
}
