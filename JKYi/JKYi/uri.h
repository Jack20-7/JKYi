#ifndef _JKYI_URI_H_
#define _JKYI_URI_H_

#include<memory>
#include<string>
#include<stdint.h>
#include"address.h"

namespace JKYi{

//定义一个对uri进行封装的类

class Uri{
public:
    typedef std::shared_ptr<Uri> ptr;

    //根据传入的string进行解析，返回对应的Uri结构体对象
    static Uri::ptr Create(const std::string& uri);

    Uri();
    
    //提供给外界的get接口
    const std::string& getScheme()const { return m_scheme; }
    const std::string& getUserinfo()const { return m_userinfo; }
    const std::string& getHost()const { return m_host; }
    const std::string  getPath()const;
    const std::string& getQuery()const { return m_query; }
    const std::string& getFragment()const { return m_fragment; }
    int32_t getPort()const;

    //提供的set接口
    void setScheme(const std::string& v){ m_scheme = v; }
    void setUserinfo(const std::string& v) { m_userinfo = v; }
    void setHost(const std::string& v) { m_host = v; }
    void setPath(const std::string& v) { m_path = v; }
    void setQuery(const std::string& v) { m_query = v; }
    void setFragment(const std::string& v) { m_fragment = v; }
    void setPort(int32_t v) { m_port = v; }

    std::ostream& dump(std::ostream& os)const ;
    std::string toString()const;

    Address::ptr createAddress()const;
private:
    bool isDefaultPort()const;
private:
    std::string m_scheme;
    std::string m_userinfo;
    std::string m_host;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
    int32_t m_port;
};

}
#endif
