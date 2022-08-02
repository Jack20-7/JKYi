#include"ws_server.h"
#include"JKYi/log.h"

namespace JKYi{
namespace http{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

WSServer::WSServer(JKYi::IOManager * worker,JKYi::IOManager * io_worker,
                    JKYi::IOManager * accept_worker)
    :TcpServer(worker,io_worker,accept_worker){
    m_dispatch.reset(new WSServletDispatch);
    m_type = "websocket";
}

void WSServer::handleClient(Socket::ptr client){
    JKYI_LOG_DEBUG(g_logger) <<"handleclient:" <<client->toString(); 
    WSSession::ptr session(new WSSession(client));
    //
    do{
        HttpRequest::ptr header = session->handleShake();
        if(!header){
            JKYI_LOG_ERROR(g_logger) << "handleShake error";
            break;
        }
        WSServlet::ptr servlet = m_dispatch->getWSServlet(header->getPath());
        if(!servlet){
            JKYI_LOG_ERROR(g_logger)<<"no match WSServlet";
            break;
        }
        int rt = servlet->onConnect(header,session);
        if(rt){
            JKYI_LOG_DEBUG(g_logger) << "onConnect return " << rt;
            break;
        }
        while(true){
            auto msg = session->recvMessage();
            if(!msg){
                break;
            }
            rt = servlet->handle(header,msg,session);
            if(rt){
                JKYI_LOG_DEBUG(g_logger) << "handle return : "<<rt;
                break;
            }
        }
        servlet->onClose(header,session);
    }while(0);
    session->close();
}

}
}
