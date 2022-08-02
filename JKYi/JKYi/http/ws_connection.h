#ifndef _JKYI_WS_CONNECTION_H_
#define _JKYI_WS_CONNECTION_H_

#include"JKYi/http/http_connection.h"
#include"JKYi/http/ws_session.h"

namespace JKYi{
namespace http{

//从客户端的角度来看建立的一条websocket连接
class WSConnection:public HttpConnection{
public:
    typedef std::shared_ptr<WSConnection> ptr;

    WSConnection(Socket::ptr  sock,bool owner = true);
    static std::pair<HttpResult::ptr,WSConnection::ptr> Create(const std::string& uri,
           uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {});

    static std::pair<HttpResult::ptr,WSConnection::ptr> Create(Uri::ptr uri,
              uint64_t timeous_ms,const std::map<std::string,std::string>& header = {});

    WSFrameMessage::ptr recvMessage();

    int32_t sendMessage(WSFrameMessage::ptr msg,bool fin = true);
    int32_t sendMessage(const std::string& msg,int32_t opcode = WSFrameHead::TEXT_FRAME,
                          bool fin = true);

    int32_t ping();
    int32_t pong();

};
}
}
#endif
