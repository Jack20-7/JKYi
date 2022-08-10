#ifndef _JKYI_WS_SERVER_H_
#define _JKYI_WS_SERVER_H_


#include"JKYi/tcp_server.h"
#include"ws_session.h"
#include"ws_servlet.h"

namespace JKYi{
namespace http{

class WSServer:public TcpServer{
public:
    typedef std::shared_ptr<WSServer> ptr;

    WSServer(JKYi::IOManager * worker = JKYi::IOManager::GetThis(),
              JKYi::IOManager * io_worker  = JKYi::IOManager::GetThis(),
              JKYi::IOManager * accept_worker = JKYi::IOManager::GetThis());
    WSServletDispatch::ptr getWSServletDispatch()const { return m_dispatch; }
    void setWSServletDispatch(WSServletDispatch::ptr v) { m_dispatch = v; }
protected:
    virtual void handleClient(Socket::ptr client)override;
private:
    WSServletDispatch::ptr m_dispatch;
};
}
}
#endif
