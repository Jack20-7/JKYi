#ifndef _JKYI_HTTP_SERVER_H_
#define _JKYI_HTTP_SERVER_H_

#include"JKYi/reactor/TcpServer.h"
#include"boost/noncopyable.hpp"

#include<memory>
#include<functional>

namespace JKYi{
namespace net{

class HttpRequest;
class HttpResponse;

class HttpServer : public boost::noncopyable{
public:
    typedef std::function<void (const HttpRequest& ,HttpResponse*)> HttpCallback;

    HttpServer(EventLoop* loop,
               const Address::ptr& serverAddr,
               const std::string& name,
               TcpServer::Option option  = TcpServer::kNoReusePort);
    EventLoop* getLoop()const { return server_.getLoop(); }

    void setHttpCallback(const HttpCallback& cb){
        httpCallback_ = cb;
    }
    void setThreadNum(int numThreads){
        server_.setThreadNum(numThreads);
    }
    void start();
private:
    void onConnection(const TcpConnection::ptr& conn);
    void onMessage(const TcpConnection::ptr& conn,
                    Buffer* buf,
                    Timestamp receiveTime);
    void onRequest(const TcpConnection::ptr& conn,const HttpRequest& req);

    TcpServer server_;
    HttpCallback httpCallback_;
};

}
}

#endif
