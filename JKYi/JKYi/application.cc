#include"application.h"
#include"tcp_server.h"
#include"daemon.h"
#include"config.h"
#include"env.h"
#include"log.h"
#include"http/http_server.h"

#include<unistd.h>
#include<signal.h>

namespace JKYi{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

//
static JKYi::ConfigVar<std::string>::ptr g_server_work_path = 
               JKYi::Config::Lookup("server.work_path",std::string("/tmp/work")
                                                       ,"serve work path");
static JKYi::ConfigVar<std::string>::ptr g_server_pid_file = 
              JKYi::Config::Lookup("sverver.pid_file",std::string("JKYi.pid"),"serve pid file");



static JKYi::ConfigVar<std::vector<TcpServerConf>>::ptr g_servers_conf = 
             JKYi::Config::Lookup("servers",std::vector<TcpServerConf>(),"http server config");

//static成员变量类内声明，类外进行初始化
Application* Application::m_instance = nullptr;

Application::Application(){
    m_instance = this;
}

bool Application::init(int argc,char ** argv){
   m_argc = argc;
   m_argv = argv;

   JKYi::EnvMgr::GetInstance()->addHelp("s","start with the terminal");
   JKYi::EnvMgr::GetInstance()->addHelp("d","run as daemin");
   JKYi::EnvMgr::GetInstance()->addHelp("c","conf path default:./conf");
   JKYi::EnvMgr::GetInstance()->addHelp("p","print help");

   bool is_print_help = false;
   if(!JKYi::EnvMgr::GetInstance()->init(argc,argv)){
       is_print_help = true;
   }

   if(JKYi::EnvMgr::GetInstance()->has("p")){
       is_print_help = true;
   }

   std::string conf_path = JKYi::EnvMgr::GetInstance()->getConfigPath();
   JKYI_LOG_INFO(g_logger) << "load conf path:" << conf_path;
   JKYi::Config::LoadFromConfDir(conf_path);

   if(is_print_help){
       JKYi::EnvMgr::GetInstance()->printHelp();
       return false;
   }

   int run_type = 0;
   if(JKYi::EnvMgr::GetInstance()->has("s")){
       run_type = 1;
   }
   if(JKYi::EnvMgr::GetInstance()->has("d")){
       run_type = 2;
   }
   if(run_type == 0){
       JKYi::EnvMgr::GetInstance()->printHelp();
       return false;
   }

   std::string pidfile = g_server_work_path->getValue() + "/" + g_server_pid_file->getValue();
   //判断对于的文件是否是已存在并且该进程正在执行
   if(JKYi::FSUtil::IsRunningPidfile(pidfile)){
       JKYI_LOG_INFO(g_logger) << "server is running :"<< pidfile;
       return false;
   }

   if(!JKYi::FSUtil::Mkdir(g_server_work_path->getValue())){
       JKYI_LOG_ERROR(g_logger) << "create work path[" << g_server_work_path->getValue()
                                << " errno ="<<errno<<" errstr="<<strerror(errno);
       return false;
   }
   return true;
}
bool Application::run(){
   bool is_daemon = JKYi::EnvMgr::GetInstance()->has("d");  
   return start_daemon(m_argc,m_argv,std::bind(&Application::main,this,std::placeholders::_1,
                                         std::placeholders::_2),is_daemon);
}

int Application::main(int argc,char ** argv){
   signal(SIGPIPE,SIG_IGN);
   JKYI_LOG_INFO(g_logger) << "main";
   std::string conf_path = JKYi::EnvMgr::GetInstance()->getConfigPath();
   JKYi::Config::LoadFromConfDir(conf_path,true);
   {
       std::string pidfile = g_server_work_path->getValue() + "/" +
                               g_server_pid_file->getValue();
       std::ofstream ofs(pidfile);
       if(!ofs){
           JKYI_LOG_ERROR(g_logger) << "open pidfile "<<pidfile <<" failed";
           return false;
       }
       ofs<<getpid();
   }
   m_mainIOManager.reset(new JKYi::IOManager(1,true,"mian"));
   m_mainIOManager->schedule(std::bind(&Application::run_fiber,this));
   //m_mainIOManager->addTimer(2000,[](){
   //     JKYI_LOG_INFO(g_logger) <<" hello";
   //},true);
   m_mainIOManager->stop();
   return 0;
}

int Application::run_fiber(){
  auto http_confs = g_servers_conf->getValue(); 
  //std::vector<TcpServer::ptr> svrs;
  for(auto&i : http_confs){
      JKYI_LOG_DEBUG(g_logger) << std::endl << LexicalCast<TcpServerConf,std::string>()(i);
      std::vector<Address::ptr> address;
      for(auto& a : i.address){
          size_t pos = a.find(":");
          if(pos == std::string::npos){
              address.push_back(UnixAddress::ptr(new UnixAddress(a)));
              continue;
          }
          int32_t port = atoi(a.substr(pos + 1).c_str());
          auto addr = JKYi::IPAddress::Create(a.substr(0,pos).c_str(),port);
          if(addr){
              address.push_back(addr);
              continue;
          }
          std::vector<std::pair<Address::ptr,uint32_t> >result;
          if(JKYi::Address::GetInterfaceAddress(result,a.substr(0,pos))){
              for(auto& x:result){
                  auto ipaddr = std::dynamic_pointer_cast<IPAddress>(x.first);
                  if(ipaddr){
                      ipaddr->setPort(atoi(a.substr(pos+1).c_str()));
                  }
                  address.push_back(ipaddr);
              }
              continue;
          }

         auto aaddr = JKYi::Address::LookupAny(a);
         if(aaddr){
             address.push_back(aaddr);
             continue;
         }
         JKYI_LOG_ERROR(g_logger) << "invalid address: " << a;
         _exit(0);
      }
      //   IOManager * accept_worker = JKYi::IOManager::GetThis();
      //   IOManager * io_worker = JKYi::IOManager::GetThis();
      //   IOManager * process_worker = JKYi::IOManager::GetThis();
      //这三个调度器默认是使用当前线程的，但是如果配置文件中有显式的指定的话，就使用指定名称的
      //TcpServer::ptr server;
      JKYi::http::HttpServer::ptr server(new JKYi::http::HttpServer(i.keepalive));
      std::vector<Address::ptr> fails;
      if(!server->bind(address,fails)){
          for(auto& m : fails){
           JKYI_LOG_ERROR(g_logger) << "httpserver bind error:"
                                    << *m;
          }
         return -1;
       }
      server->start();
      m_httpservers.push_back(server);
  }
    return 0;
}

//bool Application::getServer(const std::string& type,std::vector<TcpServer::ptr>&svrs){
//    auto it = m_servers.find(type);
//    if(it == m_servers.end()){
//        return false;
//    }
//    svrs = it->second;
//    return true;
//}
//void Application::listAllServer(const std::map<std::string,std::vector<TcpServer::ptr>>&servers){
//    servers = m_servers;
//}

}

