#ifndef _JKYI_WS_SERVLET_H_
#define _JKYI_WS_SERVLET_H_

#include"JKYi/thread.h"
#include"JKYi/http/servlet.h"
#include"JKYi/http/ws_session.h"

namespace JKYi{
namespace http{

class WSServlet:public Servlet{
public:
    typedef std::shared_ptr<WSServlet> ptr;

    WSServlet(const std::string& name)
        :Servlet(name){
    }
    virtual ~WSServlet(){}
    virtual int32_t handle(HttpRequest::ptr req,HttpResponse::ptr rsp,
                             HttpSession::ptr session)override{
        return 0;
    }
    //三个回调函数
    virtual int32_t onConnect(JKYi::http::HttpRequest::ptr header,JKYi::http::WSSession::ptr session) = 0;
    virtual int32_t onClose(JKYi::http::HttpRequest::ptr header,JKYi::http::WSSession::ptr session) = 0;
    virtual int32_t handle(JKYi::http::HttpRequest::ptr header,JKYi::http::WSFrameMessage::ptr msg,JKYi::http::WSSession::ptr session) = 0;
    
    const std::string& getName()const { return m_name; }
protected:
    //这里之所以要自定义一个就是由于Servlet的那个name成员是priate的，自己无法访问
    std::string m_name;
};

class FunctionWSServlet:public WSServlet{
public:
    typedef std::shared_ptr<FunctionWSServlet> ptr;
    typedef std::function<int32_t (JKYi::http::HttpRequest::ptr header,JKYi::http::WSSession::ptr session) > on_connect_cb;
    typedef std::function<int32_t (JKYi::http::HttpRequest::ptr header,JKYi::http::WSSession::ptr session)> on_close_cb;
    typedef std::function<int32_t (JKYi::http::HttpRequest::ptr header,JKYi::http::WSFrameMessage::ptr msg,JKYi::http::WSSession::ptr session) > callback;

    FunctionWSServlet(callback cb,on_connect_cb connect_cb = nullptr,on_close_cb close_cb = nullptr);

    virtual int32_t onConnect(JKYi::http::HttpRequest::ptr header,JKYi::http::WSSession::ptr session)override;
    virtual int32_t onClose(JKYi::http::HttpRequest::ptr header,JKYi::http::WSSession::ptr session)override;
    virtual int32_t handle(JKYi::http::HttpRequest::ptr header,JKYi::http::WSFrameMessage::ptr msg,JKYi::http::WSSession::ptr session)override;

protected:
    callback m_callback;
    on_connect_cb m_onConnect;
    on_close_cb m_onClose;
};

class WSServletDispatch:public ServletDispatch{
public:
    typedef std::shared_ptr<WSServletDispatch> ptr;
    typedef RWMutex RWMutexType;

    WSServletDispatch();
    void addServlet(const std::string& uri,
                     FunctionWSServlet::callback cb,
                     FunctionWSServlet::on_connect_cb connect_cb = nullptr,
                     FunctionWSServlet::on_close_cb close_cb = nullptr);
    void addGlobServlet(const std::string& uri,
                     FunctionWSServlet::callback cb,
                     FunctionWSServlet::on_connect_cb connect_cb = nullptr,
                     FunctionWSServlet::on_close_cb close_cb = nullptr);
    WSServlet::ptr getWSServlet(const std::string& uri);
};
}
}
#endif
