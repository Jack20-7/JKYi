#include"http_server.h"
#include"JKYi/log.h"

namespace JKYi{
namespace http{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

HttpServer::HttpServer(bool keepalive,IOManager * worker,IOManager * io_worker,
                                IOManager * accept_worker)
     :TcpServer(worker,io_worker,accept_worker),
      m_isKeepalive(keepalive){
      m_dispatch.reset(new ServletDispatch);     
      m_type = "http";
}


void HttpServer::setName(const std::string& name){
    TcpServer::setName(name);
    m_dispatch->setDefault(std::make_shared<NotFoundServlet>(name));
}

void HttpServer::handleClient(Socket::ptr client){
    //JKYI_LOG_INFO(g_logger)<<" handleClient start";
    HttpSession::ptr session(new HttpSession(client));
    do{
      auto req = session->recvRequest();
      if(!req){
          JKYI_LOG_ERROR(g_logger)<<"recvRequest faile,errno = "<<errno
                                  <<" errstr = "<<strerror(errno)
                                  <<" client = "<<client->toString()
                                  <<" keepalive = "<<m_isKeepalive;
          break;
      }
      HttpResponse::ptr rsp(new HttpResponse(req->getVersion(),req->isClose()||!m_isKeepalive));
      rsp->setHeader("Server",getName());
      m_dispatch->handle(req,rsp,session);
      session->sendResponse(rsp);
      
      if(!m_isKeepalive|| req->isClose()){
          break;
      }
    }while(true);
    session->close();
}


}
}
