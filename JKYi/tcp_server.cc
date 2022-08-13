#include"tcp_server.h"
#include"log.h"


namespace JKYi{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");
//将读超时融入到配置系统中去
static ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout = 
                          Config::Lookup("tcp_server.read_timeout",(uint64_t)(60 * 1000 * 2),"tcp server read timeout");


TcpServer::TcpServer(IOManager * worker,IOManager * io_worker,IOManager * accept_worker)
    :m_worker(worker),
     m_ioWorker(io_worker),
     m_acceptWorker(accept_worker),
     m_recvTimeout(g_tcp_server_read_timeout->getValue()),
     m_name("JKYi/1.0.0"),
     m_isStop(true){
}

TcpServer::~TcpServer(){
    for(auto& i: m_socks){
        i->close();
    }
    m_socks.clear();
}

void TcpServer::setConf(const TcpServerConf& v){
    m_conf.reset(new TcpServerConf(v));
}

bool TcpServer::bind(Address::ptr addr,bool ssl){
    std::vector<Address::ptr>addrs;
    std::vector<Address::ptr>fails;
    addrs.push_back(addr);
    return bind(addrs,fails,ssl);
}

bool TcpServer::bind(const std::vector<Address::ptr>& addrs,
                         std::vector<Address::ptr>&fails,bool ssl){
    m_ssl = ssl;
    for(auto& i:addrs){
        Socket::ptr sock = ssl ? SSLSocket::CreateTCP(i) : Socket::CreateTCP(i);
        if(!sock->bind(i)){
           JKYI_LOG_ERROR(g_logger)<<"bind fail errno:"<<errno
                                   <<" errstr = "<<strerror(errno)
                                   <<" addr =["<<i->toString()<<"]";
           fails.push_back(i);
           continue;
       }       
     if(!sock->listen()){
         JKYI_LOG_ERROR(g_logger)<<"listen fail errno:"<<errno
                                 <<" errstr = "<<strerror(errno)
                                 <<" addr=["<<i->toString()<<"]";
         fails.push_back(i);
         continue;
     }
     m_socks.push_back(sock);
   }
   if(!fails.empty()){
       m_socks.clear();
       return false;
   }
   //执行到这里表示所有的监听socket都应经创建好了
   for(auto& i : m_socks){
       JKYI_LOG_INFO(g_logger)<<"type="<<m_type
                              <<" name="<<m_name
                              <<" ssl = " <<m_ssl
                              <<" server bind success:"<<i->toString();
   }
   return true;
}

void TcpServer::startAccept(Socket::ptr sock){
    while(!m_isStop){
      Socket::ptr client = sock->accept();
      if(client){
          client->setRcvTimeout(m_recvTimeout);
          m_ioWorker->schedule(std::bind(&TcpServer::handleClient,shared_from_this(),
                                                                       client));
      }else{
          JKYI_LOG_ERROR(g_logger)<<"accept error:"<<errno<<" errstr="<<strerror(errno);
      }
    }
}

bool TcpServer::start(){
   if(!m_isStop){
      return true;
   }
   m_isStop = false;
   for(auto& i : m_socks){
      m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,shared_from_this(),i));
   }
   return true;
}

void TcpServer::stop(){
   m_isStop = true;
   //这里之所以需要将智能指针传入是为了避免当该函数在执行时TcpServer时已经销毁
   auto self = shared_from_this();
   m_acceptWorker->schedule([self,this](){
         for(auto& sock : m_socks){
             sock->cancelAll();
             sock->close();
         }
         m_socks.clear();
     });
}
void TcpServer::handleClient(Socket::ptr client){
    JKYI_LOG_INFO(g_logger)<<"handleClient : "<<client->toString();
}

std::string TcpServer::toString(const std::string &prefix){
    std::stringstream ss;
    ss<<prefix<<"[type="<<m_type
      <<" name="<<m_name
      <<" worker="<<(m_worker ? m_worker->getName() : "")
      <<" accept="<<(m_acceptWorker ? m_acceptWorker->getName() : "")
      <<" recv_timeout="<<m_recvTimeout<<"]"<<std::endl;
    std::string pfx = prefix.empty() ? " " :prefix;
    for(auto& i : m_socks){
        ss << pfx << pfx << i->toString() << std::endl;
    }
    return ss.str();
}

bool TcpServer::loadCertificates(const std::string& cert_file,const std::string& key_file){
    for(auto& i : m_socks){
        auto  ssl_socket = std::dynamic_pointer_cast<SSLSocket>(i);
        if(ssl_socket){
            if(!ssl_socket->loadCertificates(cert_file,key_file)){
                return false;
            }
        }
    }
    return true;
}

}
