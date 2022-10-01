#include"JKYi/reactor/examples/hub/codec.h"

using namespace JKYi;
using namespace JKYi::net;
using namespace pubsub;

//消息的格式
//cmd topic/r/n
//content /r/n

ParseResult pubsub::parseMessage(Buffer* buf,
                                 std::string* cmd,
                                 std::string* topic,
                                 std::string* content){
   ParseResult result = kError;
   const char * crlf = buf->findCRLF();
   if(crlf){
       const char * space = std::find(buf->peek(),crlf,' ');
       if(space != crlf){
           cmd->assign(buf->peek(),space);
           topic->assign(space + 1,crlf);
           if(*cmd == "pub"){
               const char * start = crlf + 2;
               crlf = buf->findCRLF(start);
               if(crlf){
                   content->assign(start,crlf);
                   buf->retrieveUntil(crlf + 2);
                   result = kSuccess;
               }else{
                   result = kContinue;
               }
           }else{
               //命令是sub命令，也就是sub发送来的注册topic 的信息
               buf->retrieveUntil(crlf + 2);
               result = kSuccess;
           }
       }else{
           result = kError;
       }
   }else{
       result = kContinue;
   }
   return result;
}
