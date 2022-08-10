#ifndef _JKYI_SERVLET_H_
#define _JKYI_SERVLET_H_

#include<memory>
#include<functional>
#include<string>
#include<vector>
#include<unordered_map>

#include"http.h"
#include"http_session.h"
#include"JKYi/mutex.h"
#include"JKYi/util.h"

//模仿java的servlet，自定义c++的servlet,servlet主要就是负责对httpresponse的值进行填充
namespace JKYi{
namespace http{

//抽象类，后面就可以直接继承 它就ok了
class Servlet{
public:
    typedef std::shared_ptr<Servlet> ptr;

    Servlet(const std::string& name);
    virtual ~Servlet(){};

    virtual int32_t handle(JKYi::http::HttpRequest::ptr request,JKYi::http::HttpResponse::ptr response,JKYi::http::HttpSession::ptr session) = 0;

    const std::string getName()const { return m_name; }
protected:
    //服务器的名称
    std::string m_name;
};

class FunctionServlet:public Servlet{
public:
    typedef std::shared_ptr<FunctionServlet> ptr;
    typedef std::function<int32_t (JKYi::http::HttpRequest::ptr request,JKYi::http::HttpResponse::ptr response,JKYi::http::HttpSession::ptr session)> callback; 

    FunctionServlet(callback cb);

    virtual int32_t handle(JKYi::http::HttpRequest::ptr request,JKYi::http::HttpResponse::ptr response,JKYi::http::HttpSession::ptr session) override;
private:
    callback m_cb;
};

//下面这个是一特殊的servlet，servlet分发器，用来根据HttpResquest来对servlet 进行选择
class ServletDispatch: public Servlet{
public:
    typedef std::shared_ptr<ServletDispatch> ptr;
    typedef RWMutex RWMutexType;

    ServletDispatch();

    virtual int32_t handle(JKYi::http::HttpRequest::ptr request,JKYi::http::HttpResponse::ptr response,JKYi::http::HttpSession::ptr session) override;

    //添加就精准匹配的Servlet
    void addServlet(const std::string& uri,JKYi::http::Servlet::ptr slt);
    void addServlet(const std::string& uri,FunctionServlet::callback cb);

    //添加模糊匹配的servlet
    void addGlobServlet(const std::string& uri,JKYi::http::Servlet::ptr slt);
    void addGlobServlet(const std::string& uri,FunctionServlet::callback cb);

    void delServlet(const std::string& uri);
    void delGlobServlet(const std::string& uri);

    Servlet::ptr getDefault()const { return m_default; }
    void setDefault(Servlet::ptr v){ m_default = v; }

    Servlet::ptr getServlet(const std::string& uri);
    Servlet::ptr getGlobServlet(const std::string& uri);
    Servlet::ptr getMatchedServlet(const std::string& uri);
private:
    RWMutex m_mutex;
    //精准匹配
    std::unordered_map<std::string,Servlet::ptr> m_datas;
    //模糊匹配
    std::vector<std::pair<std::string,Servlet::ptr>> m_globs;
    //如果都没匹配上，就采用默认的Servlet
    Servlet::ptr m_default;
};

class NotFoundServlet: public Servlet{
public:
    typedef std::shared_ptr<NotFoundServlet> ptr;

    NotFoundServlet(const std::string& name);

    virtual int32_t handle(JKYi::http::HttpRequest::ptr request,JKYi::http::HttpResponse::ptr response,JKYi::http::HttpSession::ptr session)override;

private:
    std::string m_name;
    std::string m_content;
};

}
}

#endif
