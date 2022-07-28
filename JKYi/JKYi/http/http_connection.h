#ifndef _JKYI_HTTP_CONNECTION_H_
#define _JKYI_HTTP_CONNECTION_H_

#include"JKYi/stream/socket_stream.h"
#include"http.h"
#include"JKYi/uri.h"
#include"JKYi/mutex.h"

#include<list>

namespace JKYi{
namespace http{

//将需要的返回信息封装成一个类
struct HttpResult{
    typedef std::shared_ptr<HttpResult> ptr;
    //错误码的枚举类
    enum class Error{
        //获取返回值成功
        OK = 0,
        //请求的是无效的URL
        INVALID_URL = 1,
        //请求的是无效的HOST
        INVALID_HOST = 2,
        //建立连接失败
        CONNECT_FAIL = 3,
        //对端以关闭连接
        SEND_CLOSE_BY_PEER = 4,
        //发送失败
        SEND_SOCKET_ERROR = 5,
        //超时
        TIMEOUT = 6,
        //创建socket失败
        CREATE_SOCKET_ERROR = 7,
        //从连接池中获取connection失败
        POOL_GET_CONNECTION = 8,
        //无效的连接
        POOL_INVALID_CONNECTION = 9,
    };

    HttpResult(int _result,HttpResponse::ptr _rsp,const std::string& _error)
        :result(_result),
         rsp(_rsp),
         error(_error){}
    std::string toString()const;

    //错误码 
    int result;
    //相应报文
    HttpResponse::ptr rsp;
    //
    std::string error;
};

class HttpConnectionPool;

//对于一条TCP连接，在服务器端被封装为HttpSession，而在客户端，则被封装为HttpConnection
class HttpConnection:public SocketStream{
friend class HttpConnectionPool;
public:
    typedef std::shared_ptr<HttpConnection> ptr;

    //下面是为了方便用户使用而封装的一些静态函数
    static HttpResult::ptr DoGet(const std::string& url,uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {},const std::string& body = "");
    static HttpResult::ptr DoGet(Uri::ptr uri,uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {},const std::string& body = "");

    static HttpResult::ptr DoPost(const std::string& url,uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {},const std::string& body =""); 
    static HttpResult::ptr DoPost(Uri::ptr uri,uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {},const std::string& body = "");

    static HttpResult::ptr DoRequest(HttpMethod method,const std::string& url,uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {},const std::string& body = "");
    static HttpResult::ptr DoRequest(HttpMethod method,Uri::ptr uri,uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {},const std::string& body = "");
    static HttpResult::ptr DoRequest(HttpRequest::ptr request,Uri::ptr uri,uint64_t timeout_ms);

    //
    HttpConnection(Socket::ptr sock,bool owner = true);

    ~HttpConnection();

    HttpResponse::ptr recvResponse();

    int sendRequest(HttpRequest::ptr req);

    uint64_t getCreateTime()const { return m_createTime; }
    uint64_t getRequests()const { return m_request; }
private:
    //该连接被创建的时间
    uint64_t m_createTime ;
    //该连接处理的请求数
    uint64_t m_request = 0;
};

//连接池,每一个连接池负责一个对端主机
class HttpConnectionPool{
public:
    typedef std::shared_ptr<HttpConnectionPool> ptr;
    typedef Mutex MutexType;

    static HttpConnectionPool::ptr Create(const std::string& uri,const std::string& vhost,uint32_t max_size,uint32_t max_alive_time,uint32_t max_request);

    HttpConnectionPool(const std::string& host,
                      const std::string& vhost,
                       uint32_t port,uint32_t max_size,
                       uint32_t max_alive_size,uint32_t max_request,bool is_https);

    HttpConnection::ptr getConnection();

    HttpResult::ptr doGet(const std::string& url,uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {},const std::string& body = "");
    HttpResult::ptr doGet(Uri::ptr uri,uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {},const std::string& body = "");

    HttpResult::ptr doPost(const std::string& url,uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {},const std::string& body = "");
    HttpResult::ptr doPost(Uri::ptr uri,uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {},const std::string& body = "");

    HttpResult::ptr doRequest(HttpMethod method,const std::string& url,uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {},const std::string& body = "");
    HttpResult::ptr doRequest(HttpMethod method,Uri::ptr uri,uint64_t timeout_ms,const std::map<std::string,std::string>& headers = {},const std::string& body = "");
    HttpResult::ptr doRequest(HttpRequest::ptr req,uint64_t timeout_ms);

    const std::atomic<int32_t>& getTotals()const { return m_total; }
private:
    //自定义析构函数
    static void ReleasePtr(HttpConnection * ptr,HttpConnectionPool * pool);
private:
    //连接池负责的对端主机
    std::string m_host;
    std::string m_vhost;
    uint32_t m_port;
    //连接池中最大连接数
    uint32_t m_maxSize;
    //连接的最大存活时间
    uint32_t m_maxAliveTime;
    //每一条连接所能够处理的最大连接数
    uint32_t m_maxRequest;

    bool m_isHttps;

    MutexType m_mutex;
    std::list<HttpConnection*> m_conns;
    std::atomic<int32_t> m_total = {0};
};

}
}
#endif
