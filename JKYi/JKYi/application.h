#ifndef _JKYI_APPLICATION_H_
#define _JKYI_APPLICATION_H_

#include"JKYi/http/http_server.h"

namespace JKYi{

class Application{
public:
    Application();

    static Application* GetInstance(){ return m_instance; }
    bool init(int argc,char** argv);
    bool run();
    
    bool getServer(const std::string& type,std::vector<TcpServer::ptr>&svrs);
    void listAllServer(std::map<std::string,
                       std::vector<TcpServer::ptr>> &servers);
private:
    int main(int argc,char** argv);
    int run_fiber();
private:
    int m_argc = 0;
    char ** m_argv = nullptr;
    
    //启动的Httpserver
    //std::vector<JKYi::http::HttpServer::ptr> m_httpservers;

    // type-vector<TcpServer::ptr> 
    std::map<std::string,std::vector<TcpServer::ptr>>m_servers;
    IOManager::ptr m_mainIOManager;
    static Application* m_instance;
};
}
#endif
