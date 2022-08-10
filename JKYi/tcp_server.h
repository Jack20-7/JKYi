#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include<memory>
#include<functional>
#include<iostream>

#include"address.h"
#include"iomanager.h"
#include"socket.h"
#include"noncopyable.h"
#include"config.h"


namespace JKYi{

//与配置模块进行整合 
struct TcpServerConf{
    typedef std::shared_ptr<TcpServerConf> ptr;

    std::vector<std::string> address;
    int keepalive = 0;
    int timeout = 1000 * 2 * 60;
    int ssl = 0;
    std::string id;
    //服务器类型
    std::string type = "http";
    std::string name ;
    std::string cert_file;
    std::string key_file;
    std::string accept_worker;
    std::string io_worker;
    std::string process_worker;
    std::map<std::string,std::string> args;

    bool isValid()const{
        return !address.empty();
    }
    bool operator == (const TcpServerConf& rhv)const {
        return address == rhv.address
               && keepalive == rhv.keepalive
               && timeout == rhv.timeout
               && ssl == rhv.ssl
               && id == rhv.id
               && type == rhv.type
               && name == rhv.name
               && cert_file == rhv.cert_file
               && key_file == rhv.key_file
               && accept_worker == rhv.accept_worker
               && io_worker == rhv.io_worker
               && process_worker == rhv.process_worker
               && args == rhv.args;
    }
};

template<>
class LexicalCast<std::string,TcpServerConf>{
public:
    TcpServerConf operator() (const std::string& v){
        YAML::Node node = YAML::Load(v);
        TcpServerConf conf;
        conf.id = node["id"].as<std::string>(conf.id);
        conf.type = node["type"].as<std::string>(conf.type);
        conf.keepalive = node["keepalive"].as<int>(conf.keepalive);
        conf.timeout = node["timeout"].as<int>(conf.timeout);
        conf.name = node["name"].as<std::string>(conf.name);
        conf.ssl = node["ssl"].as<int>(conf.ssl);
        conf.cert_file = node["cert_file"].as<std::string>(conf.cert_file);
        conf.key_file = node["key_file"].as<std::string>(conf.key_file);
        conf.accept_worker = node["accept_worker"].as<std::string>();
        conf.io_worker = node["io_worker"].as<std::string>();
        conf.process_worker = node["process_worker"].as<std::string>();
        conf.args = LexicalCast<std::string,std::map<std::string,std::string>>()(node["args"].as<std::string>(""));
        if(node["address"].IsDefined()){
            for(size_t i = 0;i<node["address"].size();++ i){
                conf.address.push_back(node["address"][i].as<std::string>());
            }
        }
        return conf;
    }
};

template<>
class LexicalCast<TcpServerConf,std::string>{
public:
    std::string operator() (TcpServerConf& conf){
        YAML::Node node;
        node["id"] = conf.id;
        node["type"] = conf.type;
        node["name"] = conf.name;
        node["keepalive"] = conf.keepalive;
        node["timeout"] = conf.timeout;
        node["ssl"] = conf.ssl;
        node["cerf_file"] = conf.cert_file;
        node["key_file"] = conf.key_file;
        node["accept_worker"] = conf.accept_worker;
        node["io_worker"] = conf.io_worker;
        node["process_worker"] = conf.process_worker;
        node["args"] = YAML::Load(LexicalCast<std::map<std::string,std::string>,std::string>()(conf.args));
        for(auto& i : conf.address){
            node["address"].push_back(i);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
//对TCP Server的封装
class TcpServer:public std::enable_shared_from_this<TcpServer>,Noncopyable{
public:
    typedef std::shared_ptr<TcpServer> ptr;
    
    TcpServer(IOManager * worker = IOManager::GetThis(),IOManager * io_worker = IOManager::GetThis(),
                                                                            IOManager * accept_worker = IOManager::GetThis());
    virtual ~TcpServer();

    virtual bool bind(Address::ptr addr,bool ssl = false);
    virtual bool bind(const std::vector<Address::ptr>& addrs,std::vector<Address::ptr>& fails,bool ssl = false);

    virtual bool start();
    virtual void stop();

    uint64_t getRecvTimeout()const { return m_recvTimeout; }
    void setRecvTimeout(uint64_t v){ m_recvTimeout = v; }

    std::string getName()const { return m_name; }
    virtual void setName(const std::string& name){ m_name = name; }

    bool isStop()const { return m_isStop; }
    std::vector<JKYi::Socket::ptr> getSocks()const { return m_socks; }

    virtual std::string toString(const std::string& prefix = "");

    void setConf(TcpServerConf::ptr v) { m_conf = v; }
    void setConf(const TcpServerConf& v);

    bool loadCertificates(const std::string& cert_file,const std::string& key_file);

    bool isHttps()const { return m_ssl == true; }
protected:
    virtual void handleClient(Socket::ptr client);
    virtual void startAccept(Socket::ptr sock);
protected:
    //存放的是服务器端的监听socekt
    std::vector<Socket::ptr> m_socks;

    IOManager * m_worker;

    //专门用来处理连接socket
    IOManager * m_ioWorker;

    //专门用来处理客户端的连接请求
    IOManager * m_acceptWorker;

    uint64_t m_recvTimeout;
    //
    std::string m_name;
    //服务器的类型
    std::string m_type = "tcp";

    //服务器是否停止运行
    bool m_isStop;

    bool m_ssl = false;

    TcpServerConf::ptr m_conf;
};
}
#endif
