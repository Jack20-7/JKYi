#ifndef _JKYI_HTTP_REQUEST_H_
#define _JKYI_HTTP_REQUEST_H_

#include"JKYi/timestamp.h"
#include"JKYi/Types.h"
#include"JKYi/macro.h"
#include"JKYi/copyable.h"

#include<map>
#include<stdio.h>


namespace JKYi{
namespace net{

class HttpRequest : public JKYi::Copyable{
public:
    enum Method{
        kInvalid = 0,
        kGet = 1,
        kPost = 2,
        kHead = 3,
        kPut = 4,
        kDelete = 5,
    };
    enum Version{
        kUnknown = 0,
        kHttp10 = 1,
        kHttp11 = 2,
    };

    HttpRequest()
        :method_(kInvalid),
         version_(kUnknown){}

    void setVersion(Version version){
        version_ = version;
    }
    Version getVersion()const{
        return version_;
    }
    bool setMethod(const char * start,const char * end){
        JKYI_ASSERT(method_ == kInvalid);
        std::string m(start,end);
        if(m == "Get"){
            method_ = kGet;
        }else if(m == "POST"){
            method_ = kPost;
        }else if(m == "HEAD"){
            method_ = kHead;
        }else if(m == "PUT"){
            method_ = kPut;
        }else if(m == "DELETE"){
            method_ = kDelete;
        }else{
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }

    Method method()const{
        return method_;
    }
    const char * methodToString()const{
        const char * result = "UNKNOWN";
        switch(method_){
#define XX(method,str)\
            case method :\
                result = str;\
                break;
            XX(kGet,"Get");
            XX(kPost,"POST");
            XX(kHead,"HEAD");
            XX(kPut,"PUT");
            XX(kDelete,"DELETE");
#undef XX
            default:
              break;
        }
        return result;
    }
    
    void setPath(const char* start,const char * end){
        path_.assign(start,end);
    }
    const std::string& path()const{
        return path_;
    }

    void setQuery(const char * start,const char * end){
        path_.assign(start,end);
    }
    const std::string& query()const{
        return query_;
    }

    void setReceiveTime(Timestamp t){
        receiveTime_ = t;
    }
    Timestamp receiveTime()const{
        return receiveTime_;
    }

    void addHeader(const char* start,const char* colon,const char * end){
        std::string field(start,colon);
        colon++;
        while(colon < end && isspace(*colon)){
            colon++;
        }
        //
        std::string value(colon,end);
        while(!value.empty() && isspace(value[value.size() - 1])){
            value.resize(value.size() - 1);
        }
        headers_[field] = value;
    }
    std::string getHeader(const std::string& field)const{
        std::string result;
        std::map<std::string,std::string>::const_iterator it = headers_.find(field);
        if(it != headers_.end()){
            return it->second;
        }
        return result;
    }
    const std::map<std::string,std::string>& headers()const{
        return headers_;
    }
    void swap(HttpRequest& rhv){
        std::swap(method_,rhv.method_);
        std::swap(version_,rhv.version_);
        path_.swap(rhv.path_);
        query_.swap(rhv.query_);
        receiveTime_.swap(rhv.receiveTime_);
        headers_.swap(rhv.headers_);
    }
private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;  //？后面的数据
    Timestamp receiveTime_;

    std::map<std::string,std::string> headers_;  //首部字段
};

}//namespace net
}//namespace JKYi

#endif
