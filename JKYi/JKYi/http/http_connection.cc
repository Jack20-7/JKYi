#include"http_connection.h"
#include"http_parser.h"
#include"JKYi/log.h"

namespace JKYi{
namespace http{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

std::string HttpResult::toString()const{
    std::stringstream ss;
    ss <<"[HttpResult result = "<<result
       <<" error = "<<error
       <<" response = "<<(rsp ? rsp->toString() : "nullptr")
       <<" ]";
    return ss.str();
}

HttpConnection::HttpConnection(Socket::ptr sock,bool owner)
    :SocketStream(sock,owner),
     m_createTime(JKYi::GetCurrentMS()){
}

HttpConnection::~HttpConnection(){
    JKYI_LOG_DEBUG(g_logger)<<"HttpConnection::~HttpConnection";
}

HttpResponse::ptr HttpConnection::recvResponse(){
    HttpResponseParser::ptr parser(new HttpResponseParser);
    uint64_t buff_size = HttpResponseParser::GetHttpResponseBufferSize();
    std::shared_ptr<char>buffer(new char[buff_size + 1],[](char * ptr){
        delete [] ptr;
    });
    char * data = buffer.get();
    //记录的是data中已读但是未解析的字节数
    int offset = 0;
    do{
      int len = read(data + offset,buff_size - offset);
      if(len <= 0){
          close();
          return nullptr;
      }
      len += offset;
      data[len] = '\0';
      size_t nparse = parser->execute(data,len,false);
      if(parser->hasError()){
          JKYI_LOG_DEBUG(g_logger)<<" ----------------------error---------------";
          close();
          return nullptr;
      }
      offset = len -  nparse;
      if(offset == (int)buff_size){
          close();
          return nullptr;
      }
      if(parser->isFinished()){
          break;
      }
    }while(true);
    //通过上面的过程，将HTTP报文的报文首部解析完成了，下面就是对报文主体进行操作
    auto& client_parser = parser->getParser();
    std::string body;
    //是否分段
    if(client_parser.chunked){
        int len = offset;
        //处理每一个chunked
        do{
            //是否是第一段
            bool begin = true;
            //下面这个do while 循环用来处理每一个chunked的首部
            do{
              if(!begin || len ==0){
                  int rt = read(data + len,buff_size - len);
                  if(rt <= 0){
                      close();
                      return nullptr;
                  }
                  len += rt;
              }
              data[len] = '\0';
              size_t nparse = parser->execute(data,len,true);
              if(parser->hasError()){
                  close();
                  return nullptr;
              }
              len -= nparse;
              if(len == (int)buff_size){
                  close();
                  return nullptr;
              }
              begin = false;
            }while(!parser->isFinished());

            JKYI_LOG_DEBUG(g_logger)<<"content-len:"<<client_parser.content_len;
            if(client_parser.content_len +2 <= len){
                body.append(data,client_parser.content_len);
                memmove(data,data + client_parser.content_len + 2,len - 
                                                       client_parser.content_len-2);
                len -= client_parser.content_len;
            }else{
                body.append(data,len);
                int left = client_parser.content_len - len + 2;
                while(left > 0){
                    int rt = read(data,left > (int)buff_size ? (int)buff_size : left);
                    if(rt <= 0){
                        close();
                        return nullptr;
                    }
                    body.append(data,rt);
                    left -= rt;
                }
                body.resize(body.size() - 2);
                len = 0;
            }
        }while(!client_parser.chunks_done);
    }else{
        //如果没有分段的话
        int64_t length = parser->getContentLength();
        if(length > 0){
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
        }
    }
    ///
    if(!body.empty()){
        auto content_encoding = parser->getData()->getHeader("content-encoding");
        JKYI_LOG_DEBUG(g_logger)<<"content_encoding:"<<content_encoding
                                <<" size = "<<body.size();
        parser->getData()->setBody(body);
    }
    return parser->getData();
}

int HttpConnection::sendRequest(HttpRequest::ptr req){
    std::stringstream ss;
    ss << req->toString();
    std::string data = ss.str();
    return writeFixSize(data.c_str(),data.size());
}

HttpResult::ptr HttpConnection::DoGet(const std::string& url,uint64_t timeout_ms,
                                      const std::map<std::string,std::string>& headers,
                                      const std::string& body){
    Uri::ptr uri = Uri::Create(url);
    if(!uri){
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL,
                                            nullptr,"invalid url:" + url);
    }
    return DoGet(uri,timeout_ms,headers,body);
}

HttpResult::ptr HttpConnection::DoGet(Uri::ptr uri,uint64_t timeout_ms,
                                      const std::map<std::string,std::string>& headers,
                                      const std::string& body){
    return DoRequest(HttpMethod::GET,uri,timeout_ms,headers,body);
}

HttpResult::ptr HttpConnection::DoPost(const std::string& url,uint64_t timeout_ms,
                                       const std::map<std::string,std::string>& headers,
                                       const std::string& body){
   Uri::ptr uri = Uri::Create(url);
   if(!uri){
       return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL,nullptr,
                                          "invalid url :" + url);
   }
   return DoPost(uri,timeout_ms,headers,body);
}
HttpResult::ptr HttpConnection::DoPost(Uri::ptr uri,uint64_t timeout_ms,
                                      const std::map<std::string,std::string>& headers,
                                      const std::string& body){
    return DoRequest(HttpMethod::POST,uri,timeout_ms,headers,body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method,const std::string& url,
                                          uint64_t timeout_ms,
                                      const std::map<std::string,std::string>& headers,
                                      const std::string& body){
   Uri::ptr uri = Uri::Create(url);
   if(!uri){
       return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL,nullptr,
                                          "invalid url :" + url);
   }
   return DoRequest(method,uri,timeout_ms,headers,body);
}
HttpResult::ptr HttpConnection::DoRequest(HttpMethod method,Uri::ptr uri,
                                           uint64_t timeout_ms,
                                       const std::map<std::string,std::string>&headers,
                                       const std::string& body){
    HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setMethod(method);
    req->setPath(uri->getPath());
    req->setQuery(uri->getQuery());
    req->setFragment(uri->getFragment());

    bool has_host = false;
    for(auto& i : headers){
        if(strcasecmp(i.first.c_str(),"connection") == 0){
            if(strcasecmp(i.second.c_str(),"keep-alive") == 0){
                req->setClose(false);
            }
            continue;
        }
        if(!has_host && strcasecmp(i.first.c_str(),"host") == 0){
            has_host = !i.second.empty();
        }
        req->setHeader(i.first,i.second);
    }
    if(!has_host){
        req->setHeader("host",uri->getHost());
    }
    req->setBody(body);
    return DoRequest(req,uri,timeout_ms);
}

HttpResult::ptr HttpConnection::DoRequest(HttpRequest::ptr req,Uri::ptr uri,
                                          uint64_t timeout_ms){
    //bool is_ssl = (uri->getScheme() == "https");
    Address::ptr addr = uri->createAddress();
    if(!addr){
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_HOST,
                                             nullptr,"invalid host :" + uri->getHost());
    }
    Socket::ptr sock = Socket::CreateTCP(addr);
    if(!sock){
        return std::make_shared<HttpResult>((int)HttpResult::Error::CREATE_SOCKET_ERROR,
                                            nullptr,"create socket fail");
    }
    if(!sock->connect(addr)){
        return std::make_shared<HttpResult>((int)HttpResult::Error::CONNECT_FAIL,nullptr
                                              ,"connction fail:" + addr->toString());
    }
    sock->setRcvTimeout(timeout_ms);
    HttpConnection::ptr conn = std::make_shared<HttpConnection>(sock);
    int rt = conn->sendRequest(req);
    if(rt == 0){
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE_BY_PEER,
                                              nullptr,"send request closed by peer:" 
                                             + addr->toString());
    }
    if(rt < 0){
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR,
                                              nullptr,"send socket error");
    }
    auto rsp = conn->recvResponse();
    if(!rsp){
        return std::make_shared<HttpResult>((int)HttpResult::Error::TIMEOUT,nullptr,
                                              "recv response timeout:" + 
                                               std::to_string(timeout_ms));
    }
    return std::make_shared<HttpResult>((int)HttpResult::Error::OK,rsp,"ok");
}

HttpConnectionPool::ptr HttpConnectionPool::Create(const std::string& uri,
                                                   const std::string& vhost,
                                                   uint32_t max_size,
                                                   uint32_t max_alive_time,
                                                   uint32_t max_request){

    Uri::ptr turi = Uri::Create(uri);
    if(!turi){
      JKYI_LOG_ERROR(g_logger)<<" invalid uri :"<<turi->toString();
      return nullptr;
    }
    return std::make_shared<HttpConnectionPool>(turi->getHost(),vhost,turi->getPort(),
                                                max_size,max_alive_time,max_request,
                                                turi->getScheme() == "https");
}

HttpConnectionPool::HttpConnectionPool(const std::string& host,const std::string& vhost,
                                        uint32_t port,uint32_t max_size,
                                        uint32_t max_request,uint32_t max_alive_time,
                                         bool is_https)
    :m_host(host),
     m_vhost(vhost),
     m_port(port ? port : (is_https ? 443 : 80)),
     m_maxSize(max_size),
     m_maxAliveTime(max_alive_time),
     m_maxRequest(max_request),
     m_isHttps(is_https){
}

HttpConnection::ptr HttpConnectionPool::getConnection(){
    uint64_t now_ms = JKYi::GetCurrentMS();
    std::vector<HttpConnection*>invalid_conns;
    HttpConnection * ptr = nullptr;
    MutexType::Lock lock(m_mutex);
    while(!m_conns.empty()){
        auto conn = *m_conns.begin();
        m_conns.pop_front();
        if(!conn->isConnected()){
            invalid_conns.push_back(conn);
            continue;
        }
        if((conn->m_createTime + m_maxAliveTime) < now_ms){
            invalid_conns.push_back(conn);
            continue;
        }
        ptr = conn;
        break;
    }
    lock.unlock();
    for(auto& i: invalid_conns){
        delete i;
    }
    m_total -= invalid_conns.size();
    //如果没找到合适的就创建新的
    if(!ptr){
      IPAddress::ptr addr = Address::LookupAnyIPAddress(m_host);
      if(!addr){
          JKYI_LOG_ERROR(g_logger)<<"get addr fail:"<<m_host;
          return nullptr;
       }
      addr->setPort(m_port);
      Socket::ptr sock = Socket::CreateTCP(addr);
      if(!sock){
          JKYI_LOG_ERROR(g_logger)<<"create socket fail:"<<addr->toString();
          return nullptr;
      }
      if(!sock->connect(addr)){
          JKYI_LOG_ERROR(g_logger)<<"socket connect fail:"<<addr->toString();
          return nullptr;
      }
      ptr = new HttpConnection(sock);
      ++m_total;
    }
    return HttpConnection::ptr (ptr,std::bind(&ReleasePtr,std::placeholders::_1,this));
}

void HttpConnectionPool::ReleasePtr(HttpConnection * ptr,HttpConnectionPool * pool){
    ++ptr->m_request;
    if(!ptr->isConnected() ||
        ((ptr->m_createTime + pool->m_maxAliveTime) < JKYi::GetCurrentMS()) ||
        (ptr->m_request >= pool->m_maxRequest)){
        JKYI_LOG_INFO(g_logger)<<"createTime = "<<ptr->getCreateTime() 
                               <<" end_time = " <<(ptr->getCreateTime() + pool->m_maxAliveTime)
                               <<" now_ms = "<<JKYi::GetCurrentMS()
                               <<" requests = "<<ptr->getRequests()
                               <<" connection = "<<ptr->isConnected();
        delete ptr;
        --pool->m_total;
        return ;
    }
    MutexType::Lock lock(pool->m_mutex);
    pool->m_conns.push_back(ptr);
    return ;
}

HttpResult::ptr HttpConnectionPool::doGet(const std::string& url,uint64_t timeout_ms,
                                          const std::map<std::string,std::string>& headers,
                                           const std::string& body){
    return doRequest(HttpMethod::GET,url,timeout_ms,headers,body);
}
HttpResult::ptr HttpConnectionPool::doGet(Uri::ptr uri,uint64_t timeout_ms,
                                          const std::map<std::string,std::string>& headers,
                                          const std::string& body){
    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getQuery().empty() ? "" : "?")
       << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#")
       << uri->getFragment();
    return doGet(ss.str(),timeout_ms,headers,body);
}

HttpResult::ptr HttpConnectionPool::doPost(const std::string& url,uint64_t timeout_ms,
                                           const std::map<std::string,std::string>& headers,
                                           const std::string& body){
    return doRequest(HttpMethod::POST,url,timeout_ms,headers,body);
}
HttpResult::ptr HttpConnectionPool::doPost(Uri::ptr uri,uint64_t timeout_ms,
                                          const std::map<std::string,std::string>& headers,
                                          const std::string& body){
    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getPath().empty() ? "" : "?")
       << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#")
       << uri->getFragment();
    return doGet(ss.str(),timeout_ms,headers,body);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method,const std::string& url,uint64_t timeout_ms,
                                              const std::map<std::string,std::string>& headers,
                                              const std::string& body){
    HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setMethod(method);
    req->setPath(url);
    req->setClose(false);
    bool has_host = false;
    for(auto& i : headers){
      if(strcasecmp(i.first.c_str(),"connection") == 0){
          if(strcasecmp(i.second.c_str(),"keep-alive") == 0){
              req->setClose(false);
          }
          continue;
      }
      if(!has_host && strcasecmp(i.first.c_str(),"host") == 0){
          has_host = !i.second.empty();
      }
      req->setHeader(i.first,i.second);
    }
    if(!has_host){
        if(m_vhost.empty()){
            req->setHeader("Host",m_host);
        }else{
            req->setHeader("Host",m_vhost);
        }
    }
    req->setBody(body);
    return doRequest(req,timeout_ms);
}
HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method,Uri::ptr uri,uint64_t timeout_ms,
                                             const std::map<std::string,std::string>& headers,
                                             const std::string& body){
    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getQuery().empty() ? "" : "?")
       <<uri->getQuery()
       <<(uri->getFragment().empty() ? "": "#")
       <<uri->getFragment();
    return doRequest(method,ss.str(),timeout_ms,headers,body);
}
HttpResult::ptr HttpConnectionPool::doRequest(HttpRequest::ptr req,uint64_t timeout_ms){
    auto conn = getConnection();
    if(!conn){
        return std::make_shared<HttpResult>((int)HttpResult::Error::POOL_GET_CONNECTION,nullptr,
                                            "pool host :" + m_host + " port:" + std::to_string(m_port));
    }
    auto sock = conn->getSocket();
    if(!sock){
        return std::make_shared<HttpResult>((int)HttpResult::Error::POOL_INVALID_CONNECTION,nullptr,
                                             "pool host :" + m_host + " port:" + 
                                                  std::to_string(m_port));
    }
    sock->setRcvTimeout(timeout_ms);
    int rt = conn->sendRequest(req);
    if(rt == 0){
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE_BY_PEER,nullptr,
                                             "send request closed by peer");
    }
    if(rt < 0){
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR,nullptr,
                                             "send request socket  error");
    }
    auto rsp = conn->recvResponse();
    if(!rsp){
        return std::make_shared<HttpResult>((int)HttpResult::Error::TIMEOUT,nullptr,
                                            "recv response timeout: " + 
                                                     std::to_string(timeout_ms));
    }
    return std::make_shared<HttpResult>((int)HttpResult::Error::OK,rsp,"OK");
}


}
}

