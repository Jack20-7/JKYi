#ifndef _HTTP_PARSER_H_
#define _HTTP_PARSER_H_


#include"http.h"
#include"http11_parser.h"
#include"httpclient_parser.h"

namespace JKYi{

namespace http{

//负责对Http请求报文进行解析，将收到的HTTP请求报文解析到内部的HttpRequest里面去
class HttpRequestParser{
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;

    HttpRequestParser();

    //开始解析的接口
    //data表示要解析的文本,len表示文本的长度
    size_t execute(char * data,size_t len);

    //是否完成解析
    int isFinished();

    //是否有错误
    int hasError();

    HttpRequest::ptr getData()const { return m_data; }

    void setError(int v){ m_error=v; }

    //获取消息体的长度
    uint64_t getContentLength();

    //返回内部用来解析的结构体
    const http_parser& getParser()const { return m_parser; }
public:
    //返回HTTP请求报文解析缓存的大小
     static uint64_t GetHttpRequestBufferSize();
     //返回HTTP请求报文最大报文主体的大小
     static uint64_t GetHttpRequestMaxBodySize();
private:
    //
    http_parser m_parser;
    HttpRequest::ptr m_data;
    //1000 invalid method
    //1001 invalid version
    int m_error;
};
//用来对HTTP响应报文解析的类
class HttpResponseParser{
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;

    HttpResponseParser();

    //返回实际解析的报文长度:w
    //chunk表示正在解析chunk
    size_t execute(char * data,size_t len,bool chunk);
    int isFinished();
    int hasError();
    HttpResponse::ptr getData()const { return m_data; }
    void setError(int v) { m_error=v; }
    
    uint64_t getContentLength();

    const httpclient_parser& getParser()const { return m_parser; }
public:
    //返回解析http响应报文的缓存大小
    static uint64_t GetHttpResponseBufferSize();
    //返回http响应报文最大主体的大小
    static uint64_t GetHttpResponseMaxBodySize();
private:
    httpclient_parser m_parser;
    HttpResponse::ptr  m_data;
    int m_error;
};
}

}
#endif
