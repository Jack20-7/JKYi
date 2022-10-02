#include"JKYi/reactor/http/HttpServer.h"
#include"JKYi/log.h"
#include"JKYi/reactor/http/HttpContext.h"
#include"JKYi/reactor/http/HttpRequest.h"
#include"JKYi/reactor/http/HttpResponse.h"

namespace JKYi{
namespace net{
namespace detail{

void defaultHttpCallback(const HttpRequest& req,HttpResponse* rsp){
    rsp->setStatusCode(HttpResponse::k404NotFound);
    rsp->setStatusMessage("Not Found");
    rsp->setCloseConnection(true);
}
}//namespace detail

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

HttpServer::HttpServer(EventLoop* loop,
                        const Address::ptr& serverAddr,
                        const std::string& name,
                        TcpServer::Option option)
    :server_(loop,serverAddr,name,option),
     httpCallback_(detail::defaultHttpCallback){
    
     server_.setConnectionCallback(
             std::bind(&HttpServer::onConnection,this,_1));
     server_.setMessageCallback(
             std::bind(&HttpServer::onMessage,this,_1,_2,_3));
}

void HttpServer::start(){
    JKYI_LOG_INFO(g_logger) << "HttpServer[ " << server_.name() 
                            << " ] starts,listening  on "
                            << server_.ipPort();
    server_.start();
}

void HttpServer::onConnection(const TcpConnection::ptr& conn){
    if(conn->connected()){
        conn->setContext(HttpContext());
    }
}
void HttpServer::onMessage(const TcpConnection::ptr& conn,
                           Buffer* buf,
                           Timestamp receiveTime){
    HttpContext* context = boost::any_cast<HttpContext>(conn->getMutableContext());
    if(!context->parseRequest(buf,receiveTime)){
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
    if(context->gotAll()){
        onRequest(conn,context->request());
        context->reset();
    }
}
void HttpServer::onRequest(const TcpConnection::ptr& conn,const HttpRequest& req){
    const std::string& connection = req.getHeader("Connection");
    bool close = (connection == "close") || 
                   (req.getVersion() == HttpRequest::kHttp10 && 
                     connection != "Keep-Alive");
    HttpResponse rsp(close);
    httpCallback_(req,&rsp);
    Buffer buf;
    rsp.appendToBuffer(&buf);
    conn->send(&buf);
    if(rsp.closeConnection()){
        conn->shutdown();
    }
}

}//namespace net
}//namespace JKYi
