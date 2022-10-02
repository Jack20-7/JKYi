#ifndef _JKYI_HTTP_CONTEXT_H_
#define _JKYI_HTTP_CONTEXT_H_

#include"JKYi/reactor/http/HttpRequest.h"
#include"JKYi/copyable.h"

namespace JKYi{
namespace net{

class Buffer;

//负责对HttpRequest进行解析,会被放在TcpConnection的context中
class HttpContext : public JKYi::Copyable{
public:
    //解析的状态
    enum HttpRequestParseState{
        kExceptRequestLine = 0,
        kExceptHeaders = 1,
        kExceptBody = 2,
        kGotAll = 3,
    };

    HttpContext()
        :state_(kExceptRequestLine){}

    bool parseRequest(Buffer* buf,Timestamp receiveTime);
    bool gotAll()const{
        return state_ == kGotAll;
    }

    void reset(){
        state_ = kExceptRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    const HttpRequest& request()const{
        return request_;
    }
    HttpRequest& request(){
        return request_;
    }
private:
    bool processRequestLine(const char * begin,const char * end);

    HttpRequestParseState state_;  //当前解析的状态
    HttpRequest request_;  //通过用户发送来数据生成的HttpRequest
};
}
}
#endif
