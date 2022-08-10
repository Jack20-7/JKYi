#ifndef _JKYI_HTTP_H_
#define _JKYI_HTTP_H_

#include<memory>
#include<string>
#include<map>
#include<vector>
#include<iostream>
#include<sstream>
#include<boost/lexical_cast.hpp>



namespace JKYi{

namespace http{
  /* Request Methods */
#define HTTP_METHOD_MAP(XX)             \
      XX(0,  DELETE,      DELETE)       \
      XX(1,  GET,         GET)          \
      XX(2,  HEAD,        HEAD)         \
      XX(3,  POST,        POST)         \
      XX(4,  PUT,         PUT)          \
      /* pathological */                \
      XX(5,  CONNECT,     CONNECT)      \
      XX(6,  OPTIONS,     OPTIONS)      \
      XX(7,  TRACE,       TRACE)        \
      /* WebDAV */                      \
      XX(8,  COPY,        COPY)         \
      XX(9,  LOCK,        LOCK)         \
      XX(10, MKCOL,       MKCOL)        \
      XX(11, MOVE,        MOVE)         \
      XX(12, PROPFIND,    PROPFIND)     \
      XX(13, PROPPATCH,   PROPPATCH)    \
      XX(14, SEARCH,      SEARCH)       \
      XX(15, UNLOCK,      UNLOCK)       \
      XX(16, BIND,        BIND)         \
      XX(17, REBIND,      REBIND)       \
      XX(18, UNBIND,      UNBIND)       \
      XX(19, ACL,         ACL)          \
      /* subversion */                  \
      XX(20, REPORT,      REPORT)       \
      XX(21, MKACTIVITY,  MKACTIVITY)   \
      XX(22, CHECKOUT,    CHECKOUT)     \
      XX(23, MERGE,       MERGE)        \
      /* upnp */                        \
      XX(24, MSEARCH,     M-SEARCH)     \
      XX(25, NOTIFY,      NOTIFY)       \
      XX(26, SUBSCRIBE,   SUBSCRIBE)    \
      XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
      /* RFC-5789 */                    \
      XX(28, PATCH,       PATCH)        \
      XX(29, PURGE,       PURGE)        \
      /* CalDAV */                      \
      XX(30, MKCALENDAR,  MKCALENDAR)   \
      /* RFC-2068, section 19.6.1.2 */  \
      XX(31, LINK,        LINK)         \
      XX(32, UNLINK,      UNLINK)       \
      /* icecast */                     \
      XX(33, SOURCE,      SOURCE)       \

/* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                     \
      XX(100, CONTINUE,                        Continue)                        \
      XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
      XX(102, PROCESSING,                      Processing)                      \
      XX(200, OK,                              OK)                              \
      XX(201, CREATED,                         Created)                         \
      XX(202, ACCEPTED,                        Accepted)                        \
      XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
      XX(204, NO_CONTENT,                      No Content)                      \
      XX(205, RESET_CONTENT,                   Reset Content)                   \
      XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
      XX(207, MULTI_STATUS,                    Multi-Status)                    \
      XX(208, ALREADY_REPORTED,                Already Reported)                \
      XX(226, IM_USED,                         IM Used)                         \
      XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
      XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
      XX(302, FOUND,                           Found)                           \
      XX(303, SEE_OTHER,                       See Other)                       \
      XX(304, NOT_MODIFIED,                    Not Modified)                    \
      XX(305, USE_PROXY,                       Use Proxy)                       \
      XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
      XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
      XX(400, BAD_REQUEST,                     Bad Request)                     \
      XX(401, UNAUTHORIZED,                    Unauthorized)                    \
      XX(402, PAYMENT_REQUIRED,                Payment Required)                \
      XX(403, FORBIDDEN,                       Forbidden)                       \
      XX(404, NOT_FOUND,                       Not Found)                       \
      XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
      XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
      XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
      XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
      XX(409, CONFLICT,                        Conflict)                        \
      XX(410, GONE,                            Gone)                            \
      XX(411, LENGTH_REQUIRED,                 Length Required)                 \
      XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
      XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
      XX(414, URI_TOO_LONG,                    URI Too Long)                    \
      XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
      XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
      XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
      XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
      XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
      XX(423, LOCKED,                          Locked)                          \
      XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
      XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
      XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
      XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
      XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
      XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
      XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
      XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
      XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
      XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
      XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
      XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
      XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
      XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
      XX(508, LOOP_DETECTED,                   Loop Detected)                   \
      XX(510, NOT_EXTENDED,                    Not Extended)                    \
      XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \

//HTTP method的枚举类
enum class HttpMethod{
#define XX(num,name,string) name=num,
    HTTP_METHOD_MAP(XX)
#undef XX
    INVALID_METHOD
};

//HTTP statue 的枚举类
enum class HttpStatus{
#define XX(code,name,string) name=code,
    HTTP_STATUS_MAP(XX)
#undef XX
};

//定义一些辅助的函数
//根据传入的string返回对应的枚举类对象
HttpMethod StringToHttpMethod(const std::string& str);


HttpMethod CharsToHttpMethod(const char* c);

const char * HttpMethodToString(const HttpMethod&m);

const char * HttpStatueToString(const HttpStatus&s);

//忽略大小写比较的仿函数,用于后续的map容器
struct CaseInsensitiveLess{
    bool operator() (const std::string&lhv,const std::string& rhv)const;
};

//该函数的作用就是在m中根据key找到对应的val,然后将val转化为T类型返回,如果转化是否，返回的是T类型的默认值
template<class MapType,class T>
bool checkGetAs(const MapType&m,const std::string&key,T& val,const T&def=T()){
    auto it=m.find(key);
    if(it == m.end()){
      val=def;
      return false;
    }
    
    try{
       val=boost::lexical_cast<T>(it->second);
       return true;
    }catch(...){
        val=def;
    }
    return false;
}

template<class MapType,class T>
T getAs(const MapType&m,const std::string&key,const T& def=T()){
    auto it=m.find(key);
    if(it == m.end()){
        return def;
    }
    try{
       return boost::lexical_cast<T>(it->second); 
    }catch(...){

    }
    return def;

}

class HttpResponse;

//HTTP 请求报文的类
class HttpRequest{
public:
    typedef std::shared_ptr<HttpRequest> ptr;
    typedef std::map<std::string,std::string,CaseInsensitiveLess> MapType;

   HttpRequest(uint8_t version=0x11,bool close=true);
   //根据当前请求的Http版本和是否是长连接生成一个Http响应报文
   std::shared_ptr<HttpResponse> createResponse();

   //get接口
   HttpMethod getMethod()const{ return m_method; }
   uint8_t getVersion()const { return m_version; }
   const std::string getPath()const { return m_path; }
   const std::string getQuery()const { return m_query; }
   const std::string getBody()const { return m_body; }
   const MapType& getHeaders()const { return m_headers; }
   const MapType& getParams()const{ return m_params; }
   const MapType& getCookies()const{ return m_cookies; }
   //根据key返回value,,如果不存在就返回def
   std::string getHeader(const std::string& key,const std::string& def="")const;
   std::string getParam(const std::string& key,const std::string& def="");
   std::string getCookie(const std::string& key,const  std::string & def="");

   //set接口
   void setMethod(HttpMethod m){ m_method=m; }
   void setVersion(uint8_t version){ m_version=version; }
   void setPath(const std::string& path){ m_path=path; }
   void setQuery(const std::string& v){ m_query=v; }
   void setFragment(const std::string& v){ m_fragment=v; }
   void setBody(const std::string& v){ m_body=v; }
   void setClose(bool v){ m_close=v; }
   void setHeaders(const MapType& v){ m_headers=v; }
   void setParams(const MapType& v){ m_params=v; }
   void setCookies(const MapType& v){ m_cookies=v; }
   void setHeader(const std::string& key,const std::string&value);
   void setParam(const std::string&key,const std::string&value);
   void setCookie(const std::string&key,const std::string& value);
   void setWebsocket(bool v) { m_websocket=v; }

   void delHeader(const std::string&key);
   void delParam(const std::string& key);
   void delCookie(const std::string& key);
   
   //
   bool isClose()const { return m_close; }
   bool iswebsocket()const { return m_websocket; }
   //判断是否存在key对应的value，如果存在返回true并且通过val将值进行返回，如果不存在返回false
   bool hasHeader(const std::string& key,std::string * val=nullptr );
   bool hasParam(const std::string& key,std::string * val=nullptr );
   bool hasCookie(const std::string& key,std::string * val=nullptr );

   //header
   template<class T>
   bool checkGetHeaderAs(const std::string &key,T&val,const T&def=T()){
       return checkGetAs(m_headers,key,val,def);
   }
   template<class T>
   T getHeaderAs(const std::string& key,const T& def=T()){
       return getAs(m_headers,key,def);
   }

   //param
   template<class T>
   bool checkGetParamAs(const std::string& key,T& val,const T& def=T()){
       initQueryParam();
       initBodyParam();
       return checkGetAs(m_params,key,val,def);
   }
   template<class T>
   T getParamAs(const std::string& key,const T&def=T()){
        initQueryParam();
        initBodyParam();
       return getAs(m_params,key,def);
   }

   //cookie
   template<class T>
   bool checkGetCookieAs(const std::string& key,T&val,const T& def=T()){
       initCookies();
       return checkGetAs(m_cookies,key,val,def);
   }
   template<class T>
   T getCookieAs(const std::string& key,const T& def=T()){
       initCookies();
       return getAs(m_cookies,key,def);
   }

   //
   std::ostream& dump(std::ostream& os)const;
   std::string toString()const;

   //
   void init();
   void initParam();
   void initQueryParam();
   void initBodyParam();
   void initCookies();
private:
    //请求方法
    HttpMethod m_method;
    //HTTP请求报文的版本号
    uint8_t m_version;
    //是否是短链接
    bool m_close;
    //是否是websocket
    bool m_websocket;
    //解析param时需要用到的标志位
    uint8_t m_parseParamFlag;
    //请求url
    std::string m_path;
    //请求url中携带的参数
    std::string m_query;
    //#后面的参数
    std::string m_fragment;
    //消息体
    std::string m_body;
    //首部字段
    MapType m_headers;
    //
    MapType m_params;
      
    MapType m_cookies;

};

//HTTP响应报文
class HttpResponse{
public:
    typedef std::shared_ptr<HttpResponse> ptr;
    typedef std::map<std::string,std::string,CaseInsensitiveLess> MapType;

    HttpResponse(uint8_t version=0x11,bool close=true);

    HttpStatus getStatus()const { return m_status; }
    uint8_t getVersion()const { return m_version; }
    const std::string&  getBody()const { return m_body; }
    const std::string& getReason()const { return m_reason; }
    const MapType& getHeaders()const { return m_headers; }
    bool isClose()const { return m_close; }
    bool isWebSocket()const { return m_websocket; }
    std::string getHeader(const std::string& key,const std::string& def="")const;


    //
    void setStatus(HttpStatus v) { m_status=v; }
    void setVersion(uint8_t v) { m_version=v; }
    void setBody(const std::string& v) { m_body=v; }
    void setReason(const std::string & v) { m_reason=v; }
    void setHeaders(const MapType& v) { m_headers=v; }
    void setClose(bool v) { m_close=v; } 
    void setWebsocket(bool v) { m_websocket=v; }
    void setHeader(const std::string&key,const std::string& value);

    //
    void delHeader(const std::string&key);

    template<class T>
    bool checkGetHeaderAs(const std::string&key,T& val,const T& def=T()){
        return checkGetAs(m_headers,key,val,def);
    }
    template<class T>
    T getHeaderAs(const std::string& key,const T& def=T()){
        return getAs(m_headers,key,def);
    }

    //
    std::ostream& dump(std::ostream& os)const;
    std::string toString()const;

    //重定向
    void setRedirect(const std::string& uri);
    void setCookie(const std::string & key,const std::string & value,time_t expired = 0
         ,const std::string& path = "",const std::string & domain = ""
         ,bool secure = false);
private:
    //状态码
    HttpStatus m_status;
    //
    uint8_t m_version;
    //
    bool m_close;
    //是否websocket
    bool m_websocket;
    //
    std::string m_body;
    //原因短语
    std::string m_reason;
    //
    MapType m_headers;

    //存储的是cookie相关的信息
    std::vector<std::string>m_cookies;
};

//流式输出HttpRequest
std::ostream& operator<< (std::ostream& os,const HttpRequest&req);

//流式输出HttpStatus
std::ostream& operator<< (std::ostream& os,const HttpStatus& rsp);
}

}


#endif
