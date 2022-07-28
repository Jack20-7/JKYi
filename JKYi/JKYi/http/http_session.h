#ifndef _JKYI_HTTP_SESSION_H_
#define _JKYI_HTTP_SESSION_H_

#include"JKYi/stream/socket_stream.h"
#include"http.h"

namespace JKYi{
namespace http{

class HttpSession:public SocketStream{
public:
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession(Socket::ptr sock,bool owner = true);
    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr rsp);
};

}
}
#endif
