#include"JKYi/reactor/Buffer.h"
#include"JKYi/reactor/http/HttpContext.h"
#include"JKYi/log.h"

using namespace JKYi;
using namespace JKYi::net;

static JKYi::Logger::ptr g_logger = JKYI_LOG_NAME("system");

//解析请求行
bool HttpContext::processRequestLine(const char * begin,const char * end){
    bool succeed = false;
    const char * start = begin;
    const char * space = std::find(start,end,' ');
    if(space != end && request_.setMethod(start,space)){
        start = space + 1;
        space = std::find(start,end,' ');
        if(space != end){
            //GET /?refresh=1 HTTP/1.1
            const char * question = std::find(start,space,'?');
            if(question != space){
                request_.setPath(start,question);
                request_.setQuery(question ,space);
            }else{
                request_.setPath(start,space);
            }
            start = space + 1;
            succeed = (end - start == 8) && std::equal(start,end - 1,"HTTP/1.");
            if(succeed){
                if(*(end - 1) == '1'){
                    request_.setVersion(HttpRequest::kHttp11);
                }else if(*(end - 1) == '0'){
                    request_.setVersion(HttpRequest::kHttp10);
                }else{
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}

// 提供给server调用的接口,一般会在onmessage中进行调用
bool HttpContext::parseRequest(Buffer* buf,Timestamp receiveTime){
    bool ok = true;
    bool hasMore = true;
    while(hasMore){
        if(state_ == kExceptRequestLine){
            const char * crlf = buf->findCRLF();
            if(crlf){
                ok = processRequestLine(buf->peek(),crlf);
                if(ok){
                    request_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf + 2);
                    state_ = kExceptHeaders;
                }else{
                    hasMore = false;
                }
            }else{
                hasMore = false;
            }
        }else if(state_ == kExceptHeaders){
            const char * crlf = buf->findCRLF();
            if(crlf){
                const char * colon = std::find(buf->peek(),crlf,':');
                if(colon != crlf){
                    request_.addHeader(buf->peek(),colon,crlf);
                }else{
                    //空行，代表解析到了Http请求报文的请求头部和请求主体之间的间隔标志
                    //当前的http服务器是简陋的服务器，不支持http请求报文的body有数据
                    state_ = kGotAll;
                    hasMore = false;
                }
                buf->retrieveUntil(crlf + 2);
            }else{
                hasMore = false;
            }
        }else if(state_ == kExceptBody){
            //not support
        }
    }
    return ok;
}
