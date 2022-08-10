#include"http_session.h"
#include"http_parser.h"

namespace JKYi{
namespace http{

HttpSession::HttpSession(Socket::ptr sock,bool owner)
    :SocketStream(sock,owner){
} 

HttpRequest::ptr HttpSession::recvRequest(){
    HttpRequestParser::ptr parser(new HttpRequestParser());
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    //用来存放收到的http请求报文的缓存
    std::shared_ptr<char>buffer(new char[buff_size],[](char * ptr){
            delete [] ptr;
        });

    char * data = buffer.get();
    //表示的当前已读取未解析的数据的偏移量
    int offset = 0;
    do{
      int len = read(data + offset,buff_size - offset);
      if(len <= 0){
          close();
          return nullptr;
      }
      len += offset;
      size_t nparse = parser->execute(data,len);
      if(parser->hasError()){
          close();
          return nullptr;
      }
      offset = len - nparse;
      if(offset == (int)buff_size){
          close();
          return nullptr;
      }
      if(parser->isFinished()){
          break;
      }
    }while(true);
    //接下来单独对HTTP请求报文的报文主体进行处理
    int64_t length = parser->getContentLength();
    if(length > 0){
        std::string body;
        body.resize(length);

        int len = 0;
        if(length >= offset){
            memcpy(&body[0],data,offset);
            len = offset;
        }else{
            memcpy(&body[0],data,length);
            len = length;
        }
        length -= len;
        if(length > 0){
            if(readFixSize(&body[len],length) <= 0){
                close();
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }
    parser->getData()->init();
    return parser->getData();
}

int  HttpSession::sendResponse(const HttpResponse::ptr rsp){
    std::stringstream ss;
    rsp->dump(ss);
    std::string data = ss.str();
    return writeFixSize(data.c_str(),data.size());

}

}
}
