#ifndef _JKYI_HTTP_SERVER_H_
#define _JKYI_HTTP_SERVER_H_

#include"JKYi/tcp_server.h"
#include"http_session.h"
#include"servlet.h"

namespace JKYi{
namespace http{

class HttpServer:public TcpServer{
public:
    typedef std::shared_ptr<HttpServer> ptr;

    HttpServer(bool keepalive = false,IOManager * worker = JKYi::IOManager::GetThis()
            ,IOManager * io_worker = JKYi::IOManager::GetThis()
               ,IOManager * accept_worker = JKYi::IOManager::GetThis());

    ServletDispatch::ptr getServletDispatch()const{ return m_dispatch; }
    void setServletDispatch(ServletDispatch::ptr v){ m_dispatch = v; }

    virtual void setName(const std::string& name)override;
private:
    virtual void handleClient(Socket::ptr client)override;
private:
    //是否支持长连接
    bool m_isKeepalive;
    //Servlet分发器
    ServletDispatch::ptr m_dispatch;
}; 
}
}
#endif
