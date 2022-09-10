#include"application.h"
#include"tcp_server.h"
#include"daemon.h"
#include"config.h"
#include"env.h"
#include"log.h"
#include"http/http_server.h"
#include"worker.h"
#include"http/ws_server.h"
#include"JKYi/module.h"
#include"JKYi/db/fox_thread.h"
#include"JKYi/db/redis.h"

#include<unistd.h>
#include<signal.h>
#include<memory.h>

namespace JKYi{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

//
static JKYi::ConfigVar<std::string>::ptr g_server_work_path = 
               JKYi::Config::Lookup("server.work_path",std::string("/tmp/work")
                                                       ,"serve work path");
static JKYi::ConfigVar<std::string>::ptr g_server_pid_file = 
              JKYi::Config::Lookup("server.pid_file",std::string("JKYi.pid"),"serve pid file");


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

   ModuleMgr::GetInstance()->init();
   std::vector<Module::ptr>modules;
   ModuleMgr::GetInstance()->listAll(modules);

   for(auto & i : modules){
       i->onBeforeArgsParse(argc,argv);
   }

   if(is_print_help){
       JKYi::EnvMgr::GetInstance()->printHelp();
       return false;
   }

   for(auto & i : modules){
       i->onAfterArgsParse(argc,argv);
   }

   modules.clear();

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
   //JKYI_LOG_INFO(g_logger) << "main";
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
   //这里的定时器是必须要加上的，这样就可以避免由于main_IOManager析构导致的异常，这个问题找了快一周了
   m_mainIOManager->addTimer(2000,[](){
        //JKYI_LOG_INFO(g_logger) <<" hello";
   },true);
   m_mainIOManager->stop();
   return 0;
}

int Application::run_fiber(){

  JKYi::WorkerMgr::GetInstance()->init();
  JKYi::FoxThreadMgr::GetInstance()->init();
  JKYi::FoxThreadMgr::GetInstance()->start();
  JKYi::RedisMgr::GetInstance();

  std::vector<Module::ptr>modules;
  ModuleMgr::GetInstance()->listAll(modules);
  bool has_error = false;
  for(auto & i : modules){
      if(!i->onLoad()){
          JKYI_LOG_ERROR(g_logger) << "module name = " << i->getName()
                                   << " version = " << i->getVersion()
                                   << " filename = " << i->getFilename();
          has_error = true;
      }
  }
  if(has_error){
      _exit(0);
  }


  auto http_confs = g_servers_conf->getValue(); 
  std::vector<TcpServer::ptr> svrs;
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
         IOManager * accept_worker = JKYi::IOManager::GetThis();
         IOManager * io_worker = JKYi::IOManager::GetThis();
         IOManager * process_worker = JKYi::IOManager::GetThis();

         if(!i.accept_worker.empty()){
             accept_worker = JKYi::WorkerMgr::GetInstance()->getAsIOManager(i.accept_worker).get();
             if(!accept_worker){
                 JKYI_LOG_ERROR(g_logger) << "accept_worker:" << i.accept_worker << "not exists";
                 _exit(0);
             }
         }
         if(!i.io_worker.empty()){
             io_worker = JKYi::WorkerMgr::GetInstance()->getAsIOManager(i.io_worker).get();
             if(!io_worker){
                 JKYI_LOG_ERROR(g_logger) << "io_worker:" << i.io_worker << " not exists";
                 _exit(0);
             }
         }
         if(!i.process_worker.empty()){
             process_worker = JKYi::WorkerMgr::GetInstance()->getAsIOManager(i.process_worker).get();
             if(!process_worker){
                 JKYI_LOG_ERROR(g_logger) << "process_worker:" << i.process_worker << "not exists";
                 _exit(0);
             }
         }
         //
         TcpServer::ptr server;
         if(i.type == "http"){
             server.reset(new JKYi::http::HttpServer(i.keepalive,process_worker,io_worker,accept_worker));
         }else if(i.type == "ws"){
             server.reset(new JKYi::http::WSServer(process_worker,io_worker,accept_worker)); 
         }else{
             JKYI_LOG_ERROR(g_logger) << "invalid server type = "<<i.type 
                                                                 << LexicalCast<TcpServerConf,std::string>()(i);
             _exit(0);
         }
         if(!i.name.empty()){
             server->setName(i.name);
         }
         std::vector<Address::ptr> fails;
         if(!server->bind(address,fails,i.ssl)){
             for(auto& x : fails){
                 JKYI_LOG_ERROR(g_logger) << "bind address fail:" << x->toString();
             }
             _exit(0);
         }
         if(i.ssl){
             if(!server->loadCertificates(i.cert_file,i.key_file)){
                 JKYI_LOG_ERROR(g_logger) << "loadCertificates fail,cert_file = " << i.cert_file
                                          << " key_file = " << i.key_file;
             }
         }

         server->setConf(i);
         m_servers[i.type].push_back(server);
         svrs.push_back(server);
    }
    
    for(auto & i : modules){
        i->onServerReady();
    }

    for(auto& i : svrs){
        i->start();
    }

    for(auto & i : modules){
        i->onServerUp();
    }
    return 0;
}

bool Application::getServer(const std::string& type,std::vector<TcpServer::ptr>&svrs){
    auto it = m_servers.find(type);
    if(it == m_servers.end()){
        return false;
    }
    svrs = it->second;
    return true;
}
void Application::listAllServer(std::map<std::string,std::vector<TcpServer::ptr>>&servers){
    servers = m_servers;
    return ;
}

}

