#ifndef _JKYI_HTTP_RESPONSE_H_
#define _JKYI_HTTP_RESPONSE_H_

#include"JKYi/copyable.h"
#include"JKYi/Types.h"

#include<map>

namespace JKYi{
namespace net{

class Buffer;

class HttpResponse : public JKYi::Copyable{
public:
   enum HttpStatusCode{
       kUnknown = 0,
       k200Ok = 200,
       k301MovedPermanently = 301,
       k400BadRequest = 400,
       k404NotFound = 404,
   };
   explicit HttpResponse(bool close)
     :statusCode_(kUnknown),
      closeConnection_(close){
   }
   void setStatusCode(HttpStatusCode code){
       statusCode_ = code;
   }
   void setStatusMessage(const std::string& message){
       statusMessage_ = message;
   }
   void setCloseConnection(bool on){
       closeConnection_ = on;
   }
   bool closeConnection()const{
       return closeConnection_;
   }
   void addHeader(const std::string& key,const std::string& value){
       headers_[key] = value;
   }
   void setContentType(const std::string& type){
       addHeader("Content-Type",type);
   }

   void setBody(const std::string& body){
       body_ = body;
   }
   void appendToBuffer(Buffer* output)const;
private:
   HttpStatusCode statusCode_;  //状态码
   std::string statusMessage_;  //原因短语
   std::map<std::string,std::string> headers_;
   std::string body_;
   bool closeConnection_; //用来控制发送完response后是否要断开连接
};
}
}
#endif
