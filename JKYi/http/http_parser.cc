#include"http_parser.h"
#include"JKYi/log.h"
#include"JKYi/config.h"

#include<string.h>

namespace JKYi{

namespace http{

static Logger::ptr g_logger=JKYI_LOG_NAME("system");

//用来解析HTTP请求报文的缓存的大小
static JKYi::ConfigVar<uint64_t>::ptr g_http_request_buffer_size=JKYi::Config::Lookup("http.request.buffer_size",(uint64_t)(4*1024),"http request buffer size");

//HTTP请求报文的最大报文主体的长度
static JKYi::ConfigVar<uint64_t>::ptr g_http_request_max_body_size=JKYi::Config::Lookup("http.request.max_body_size",(uint64_t)(64 * 1024 * 1024),"http request max body size");

//HHTP响应报文的解析缓存的大小
static JKYi::ConfigVar<uint64_t>::ptr g_http_response_buffer_size=JKYi::Config::Lookup("http.response.buffer_size",(uint64_t)(4*1024),"http response buffer size");

//HTTP响应报文的最大报文主体的长度
static JKYi::ConfigVar<uint64_t>::ptr g_http_response_max_body_size=JKYi::Config::Lookup("http.response.max_body_size",(uint64_t)(64*1024*1024),"http response max body size");

static uint64_t s_http_request_buffer_size=0;
static uint64_t s_http_request_max_body_size=0;
static uint64_t s_http_response_buffer_size=0;
static uint64_t s_http_response_max_body_size=0;

uint64_t HttpRequestParser::GetHttpRequestBufferSize(){
   return s_http_request_buffer_size;
}
uint64_t HttpRequestParser::GetHttpRequestMaxBodySize(){
   return s_http_request_max_body_size;
}

uint64_t HttpResponseParser::GetHttpResponseBufferSize(){
    return s_http_response_buffer_size;
}
uint64_t HttpResponseParser::GetHttpResponseMaxBodySize(){
    return s_http_response_max_body_size;
}


namespace {
    //利用全局对象在main函数之前初始化的特性，在main函数之前对一些size的值进行初始化
struct _RequestSizeIniter{
        _RequestSizeIniter(){
            s_http_request_buffer_size=g_http_request_buffer_size->getValue();
            s_http_request_max_body_size=g_http_request_max_body_size->getValue();
            s_http_response_buffer_size=g_http_response_buffer_size->getValue();
            s_http_response_max_body_size=g_http_response_max_body_size->getValue();

            //注册回调函数，该回调函数会在值修改的时候触发，用来对全局变量的值进行更新
            //
           g_http_request_buffer_size->addListener([](const uint64_t&oldValue,
                       const uint64_t& newValue){
                   s_http_request_buffer_size=newValue;
               });
           g_http_request_max_body_size->addListener([](const uint64_t&oldValue,
                       const uint64_t& newValue){
                   s_http_request_max_body_size=newValue;
               });
           g_http_response_buffer_size->addListener([](const uint64_t&oldValue,
                       const uint64_t& newValue){
                   s_http_response_buffer_size=newValue;
               });
           g_http_response_max_body_size->addListener([](const uint64_t &oldValue,
                       const uint64_t &newValue){
                   s_http_response_max_body_size=newValue;
               });
        }
};
//创建全局静态对象
static _RequestSizeIniter _init;
}

//下面时回调函数的实现，这个回调函数用来在解析报文时会被自动的调用
//request
void on_request_method(void * data,const char* at,size_t length){
   HttpRequestParser* parser=static_cast<HttpRequestParser*>(data); 
   HttpMethod m=CharsToHttpMethod(at);

   if(m == HttpMethod::INVALID_METHOD){
       JKYI_LOG_WARN(g_logger)<<"invalid httpequest method:"
                              <<std::string(at,length);
       parser->setError(1000);
       return ;
   }
   parser->getData()->setMethod(m);
}
void on_request_uri(void *data,const char * at,size_t length){

}
void on_request_fragment(void * data,const char * at,size_t length){
   HttpRequestParser* parser=static_cast<HttpRequestParser*>(data);
   parser->getData()->setFragment(std::string(at,length));
}
void on_request_path(void * data,const char * at,size_t length){
    HttpRequestParser * parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setPath(std::string(at,length));
}
void on_request_query(void * data,const char * at,size_t length){
    HttpRequestParser * parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setQuery(std::string(at,length));
}
void on_request_version(void * data,const char * at,size_t length){
    HttpRequestParser * parser = static_cast<HttpRequestParser*>(data);
    uint8_t v = 0;

    if(strncmp(at,"HTTP/1.0",length) ==0 ){
        v=0x10;
    }else if(strncmp(at,"HTTP/1.1",length) == 0){
        v=0x11;
    }else{
        JKYI_LOG_WARN(g_logger)<<"invalid httprequest version:"
                               <<std::string(at,length);
        parser->setError(1001);
        return ;
    }
    parser->getData()->setVersion(v);
}
void on_request_header_done(void * data,const char * at,size_t length){

}
void on_request_http_field(void * data,const char * field,size_t flen,
                             const char * value,size_t vlen){
    HttpRequestParser * parser = static_cast<HttpRequestParser*>(data);
    if(flen == 0){
        JKYI_LOG_WARN(g_logger)<<"invalid httprequest field length == 0";
        return ;
    }
    parser->getData()->setHeader(std::string(field,flen),std::string(value,vlen));
}
//构造函数
HttpRequestParser::HttpRequestParser()
    :m_error(0){
    m_data.reset(new HttpRequest());
    http_parser_init(&m_parser);
    m_parser.request_method=on_request_method;
    m_parser.request_uri=on_request_uri;
    m_parser.fragment=on_request_fragment;
    m_parser.request_path=on_request_path;
    m_parser.query_string=on_request_query;
    m_parser.http_version=on_request_version;
    m_parser.header_done=on_request_header_done;
    m_parser.http_field=on_request_http_field;
    m_parser.data=this;
}
uint64_t HttpRequestParser::getContentLength(){
    return m_data->getHeaderAs<uint64_t>("content-length",0);
}
//解析报文的函数
//1 解析成功
//-1 解析失败
//>0,返回的实际解析的报文字节数
size_t HttpRequestParser::execute(char * data,size_t length){
   size_t offset=http_parser_execute(&m_parser,data,length,0);
   //从data+offset开始，拷贝(len-offset)个字节到data
   memmove(data,data+offset,length-offset);
   return offset;
}
int HttpRequestParser::isFinished(){
    return http_parser_finish(&m_parser);
}
int HttpRequestParser::hasError(){
    return m_error || http_parser_has_error(&m_parser);
}

//respnonse

void on_response_reason(void * data,const char * at,size_t length){
    HttpResponseParser * parser = static_cast<HttpResponseParser*>(data);
    parser->getData()->setReason(std::string(at,length));
}
void on_response_status(void * data,const char * at,size_t length){
    HttpResponseParser * parser = static_cast<HttpResponseParser *>(data);
    HttpStatus s = (HttpStatus)(atoi(at));
    parser->getData()->setStatus(s);
}
void on_response_chunk(void * data,const char * at,size_t length){
}
void on_response_version(void * data,const char * at,size_t length){
    HttpResponseParser * parser = static_cast<HttpResponseParser*>(data);
    uint8_t v = 0;
    if(strncmp(at,"HTTP/1.0",length) == 0){
        v=0x10;
    }else if(strncmp(at,"HTTP/1.1",length) == 0){
        v=0x11;
    }else{
        JKYI_LOG_WARN(g_logger)<<"invalid httpresponse version:"
                               <<std::string(at,length);
        parser->setError(1001);
        return ;
    }
    parser->getData()->setVersion(v);
}
void on_response_header_done(void * data,const char * at,size_t length){
}
void on_response_last_chunk(void * data,const char * at,size_t length){
}
void on_response_http_field(void * data,const char * field,size_t flen,
                                      const char * value,size_t vlen){
    HttpResponseParser * parser = static_cast<HttpResponseParser*>(data);
    if(flen == 0){
        JKYI_LOG_WARN(g_logger)<<" invalid httpresponse field length ==0";
        return ;
    }
    parser->getData()->setHeader(std::string(field,flen),std::string(value,vlen));
}

HttpResponseParser::HttpResponseParser()
    :m_error(0){
    m_data.reset(new HttpResponse());
    httpclient_parser_init(&m_parser);
    m_parser.reason_phrase = on_response_reason;
    m_parser.status_code = on_response_status;
    m_parser.chunk_size = on_response_chunk;
    m_parser.http_version = on_response_version;
    m_parser.header_done = on_response_header_done;
    m_parser.last_chunk = on_response_last_chunk;
    m_parser.http_field = on_response_http_field;
    m_parser.data=this;
}
size_t HttpResponseParser::execute(char* data,size_t len,bool chunk){
   if(chunk){
       httpclient_parser_init(&m_parser);
   }
   size_t offset = httpclient_parser_execute(&m_parser,data,len,0);
   memmove(data,data+offset,(len-offset)); 
   return offset;
}
int HttpResponseParser::isFinished(){
    return httpclient_parser_finish(&m_parser);
}
int HttpResponseParser::hasError(){
    return m_error || httpclient_parser_has_error(&m_parser);
}
uint64_t HttpResponseParser::getContentLength(){
    return m_data->getHeaderAs<uint64_t>("content-length",0);
}

}
}



