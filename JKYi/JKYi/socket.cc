#include"socket.h"
#include"log.h"
#include"iomanager.h"
#include"fdmanager.h"
#include"macro.h"
#include"hook.h"

namespace JKYi{
//这个之所以使用static修饰的原因就是为了将作用域限制在本文件
//因为每一个源文件都需要有一个日志器
static Logger::ptr g_logger=JKYI_LOG_NAME("system");

Socket::ptr Socket::CreateTCP(Address::ptr address){
    Socket::ptr sock(new Socket(address->getFamily(),TCP,0));
    return sock;
}
Socket::ptr Socket::CreateUDP(Address::ptr address){
    Socket::ptr sock(new Socket(address->getFamily(),UDP,0));
    //这里由于UDPSocket在Socket创建好了之后就直接sento、recvfrom，并没有建立连接的过程
    //所以在创建就需要我们将socket创建好
    sock->newSock();
    sock->m_isConnected=true;
    return sock;
}

Socket::ptr Socket::CreateTCPSocket(){
    Socket::ptr sock(new Socket(IPv4,TCP,0));
    return sock;
}
Socket::ptr Socket::CreateUDPSocket(){
    Socket::ptr sock(new Socket(IPv4,UDP,0));
    sock->newSock();
    sock->m_isConnected=true;
    return sock;
}

Socket::ptr Socket::CreateTCPSocket6(){
    Socket::ptr sock(new Socket(IPv6,TCP,0));
    return sock;
}
Socket::ptr Socket::CreateUDPSocket6(){
    Socket::ptr sock(new Socket(IPv6,UDP,0));
    sock->newSock();
    sock->m_isConnected=true;
    return sock;
}

Socket::ptr Socket::CreateUnixTCPSocket(){
    Socket::ptr sock(new Socket(Unix,TCP,0));
    return sock;
}
Socket::ptr Socket::CreateUnixUDPSocket(){
    Socket::ptr sock(new Socket(Unix,UDP,0));
    return sock;
}


Socket::Socket(int family,int type,int protocol)
    :m_sock(-1),
     m_family(family),
     m_type(type),
     m_protocol(protocol),
     m_isConnected(false){

}

Socket::~Socket(){
    close();
}

//下面就是收发数据包相关的函数了
int64_t Socket::getSendTimeout(){
    FdCtx::ptr ctx=FdMgr::GetInstance()->get(m_sock);
    if(ctx){
        return ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
}
void Socket::setSendTimeout(int64_t v){
   struct timeval tv{int(v/1000),int(v%1000*1000)};
   setOption(SOL_SOCKET,SO_SNDTIMEO,tv);
}

int64_t Socket::getRcvTimeout(){
    FdCtx::ptr ctx=FdMgr::GetInstance()->get(m_sock);
    if(ctx){
        return ctx->getTimeout(SO_RCVTIMEO);
    }
    return -1;
}
void Socket::setRcvTimeout(int64_t v){
    struct timeval tv{int(v/1000),int(v%1000*1000)};
    setOption(SOL_SOCKET,SO_RCVTIMEO,tv);
}
//
bool Socket::getOption(int level,int option,void *result,socklen_t *len){
    int rt=getsockopt(m_sock,level,option,result,len);
    if(rt){
        JKYI_LOG_ERROR(g_logger)<<"getOptin sock="<<m_sock
                                <<" level="<<level<<" option="
                                <<option<<" errno="<<errno
                                <<" errstr="<<strerror(errno);
        return false;
    }
    return true;
}

bool Socket::setOption(int level,int option,const void * result,socklen_t len){
    if(setsockopt(m_sock,level,option,result,len)){
        JKYI_LOG_ERROR(g_logger)<<" setoptin sock="<<m_sock
                                <<" level="<<level<<" option="
                                <<option<<" errno="<<errno
                                <<" errstr="<<strerror(errno);
        return false;
    }
    return true;
}

Socket::ptr Socket::accept(){
   Socket::ptr sock(new Socket(m_family,m_type,m_protocol));
   int newSock=::accept(m_sock,nullptr,nullptr);
   if(newSock == -1){
       JKYI_LOG_ERROR(g_logger)<<" accept("<<m_sock<<") errno ="
                               <<errno<<" errstr"<<strerror(errno);
       return nullptr;
   }
   if(sock->init(newSock)){
       return sock;
   }
   return nullptr;
}
bool Socket::init(int sock){
    FdCtx::ptr ctx=FdMgr::GetInstance()->get(sock);
    if(ctx && ctx->isSocket() && !ctx->isClose()){
        m_sock=sock;
        m_isConnected=true;
        initSock();
        getLocalAddress();
        getRemoteAddress();
        return true;
    }        
    return false;
}

bool Socket::bind(Address::ptr address){
    if(!isValid()){
       newSock();
       if(JKYI_UNLIKELY(!isValid())){
               return false;
        }
    }
    if(JKYI_UNLIKELY(address->getFamily() != m_family)){
       JKYI_LOG_ERROR(g_logger)<<" bind sock error";
       return false;
    }
    
    if(::bind(m_sock,address->getAddr(),address->getAddrLen())){
        JKYI_LOG_ERROR(g_logger)<<" bind error errno="<<errno
                                <<" errstr="<<strerror(errno); 
        return false;
    } 
    getLocalAddress();
    return true;
}

bool Socket::reconnect(int64_t timeout_ms){

    if(!m_remoteAddress){
       JKYI_LOG_ERROR(g_logger)<<"reconnect m_remoteAddress is null";
       return false;
    }
    m_localAddress.reset();
    return connect(m_remoteAddress,timeout_ms);
}

bool Socket::connect(const Address::ptr addr,int64_t timeout_ms){
    m_remoteAddress=addr;
    if(!isValid()){
      newSock();
      if(JKYI_UNLIKELY(!isValid())){
         return false; 
      }
    }

    if(JKYI_UNLIKELY(addr->getFamily() != m_family)){
          JKYI_LOG_ERROR(g_logger)<<" connect error,m_family="<<m_family<<",addr->m_family="<<addr->getFamily();
          return false;
    }
    
       //
   if(timeout_ms == (int64_t)-1){
      if(::connect(m_sock,addr->getAddr(),addr->getAddrLen())){
           JKYI_LOG_ERROR(g_logger)<<" connect error,errno="<<errno;
           close();
           return false;
      }
   }else{
      if(::connect_with_timeout(m_sock,addr->getAddr(),addr->getAddrLen(),timeout_ms)){
          JKYI_LOG_ERROR(g_logger)<<"connect_with_timeout error,errno="<<errno;
          close();
          return false;
      }
   }
   m_isConnected=true;
   getRemoteAddress();
   getLocalAddress();
   return true;
}

bool Socket::listen(int backlog){
    if(!isValid()){
      JKYI_LOG_ERROR(g_logger)<<" listen error,m_sock=-1";
      return false;
    }
    if(::listen(m_sock,backlog)){
       JKYI_LOG_ERROR(g_logger)<<" listen error,errno="<<errno
                               <<" errstr="<<strerror(errno);
        return false;
    }
    return true;
} 

bool Socket::close(){
   if(m_sock==-1 && !m_isConnected ){
      return true;
   }
   m_isConnected=false;
   if(m_sock != -1){
      ::close(m_sock);
      m_sock=-1;
   }
   return false;
}

int Socket::send(const void *buffer,size_t len,int flags){
    if(isConnected()){
        return ::send(m_sock,buffer,len,flags); 
    } 
    return -1;
}
int Socket::send(const iovec*buffers,size_t len,int flags){
    if(isConnected()){
        msghdr msg;
        memset(&msg,0,sizeof(msg));
        msg.msg_iov=(iovec*)buffers;
        msg.msg_iovlen=len;
        return ::sendmsg(m_sock,&msg,flags);
    }
    return -1;
}

int Socket::sendTo(const void *buffer,size_t len,const Address::ptr to,int flags){
      if(isConnected()){
         return ::sendto(m_sock,buffer,len,flags,to->getAddr(),to->getAddrLen());
      }
      return -1;
}

int Socket::sendTo(const struct iovec*buffers,size_t len,const Address::ptr to,int flags){
     if(isConnected()){
         struct msghdr msg;
         memset(&msg,0,sizeof(msg));
         msg.msg_iov=(iovec*)buffers;
         msg.msg_iovlen=len;
         msg.msg_name=to->getAddr();
         msg.msg_namelen=to->getAddrLen();
         return ::sendmsg(m_sock,&msg,flags);
     } 
     return -1;
}

int Socket::recv(void *buffer,size_t len,int flags){
    if(isConnected()){
        return ::recv(m_sock,buffer,len,flags);
    }
    return -1;
}

int Socket::recv(struct iovec*buffers,size_t len,int flags){
    if(isConnected()){
        msghdr msg;
        memset(&msg,0,sizeof(msg));
        msg.msg_iov=buffers;
        msg.msg_iovlen=len;
        return ::recvmsg(m_sock,&msg,flags);
    }
    return -1;
}

int Socket::recvFrom(void *buffer,size_t len,Address::ptr from,int flags){
    if(isConnected()){
        socklen_t lenFrom=from->getAddrLen();
        return ::recvfrom(m_sock,buffer,len,flags,from->getAddr(),&lenFrom);
    } 
    return -1;
}

int Socket::recvFrom(iovec*buffers,size_t len,Address::ptr from,int flags){
    if(isConnected()){
        msghdr msg;
        memset(&msg,0,sizeof(msg));
        msg.msg_iov=buffers;
        msg.msg_iovlen=len;
        msg.msg_name=from->getAddr();
        msg.msg_namelen=from->getAddrLen();
        return ::recvmsg(m_sock,&msg,flags);
    }
    return -1;
}

Address::ptr Socket::getRemoteAddress(){
    if(m_remoteAddress){
        return m_remoteAddress;
    }
    Address::ptr addr;
    switch(m_family){
      case AF_INET:
        addr.reset(new IPv4Address());
        break;
      case AF_INET6:
        addr.reset(new IPv6Address());
        break;
      case AF_UNIX:
        addr.reset(new UnixAddress());
        break;
      default:
        addr.reset(new UnknowAddress(m_family));
        break;
    }
   socklen_t addrlen=addr->getAddrLen();    
   if(getpeername(m_sock,addr->getAddr(),&addrlen)){
       return Address::ptr (new UnknowAddress(m_family));
   }
   if(m_family == AF_UNIX){
       UnixAddress::ptr address=std::dynamic_pointer_cast<UnixAddress>(addr);
       address->setAddrLen(addrlen);
   }
   m_remoteAddress=addr;
   return m_remoteAddress;
} 

Address::ptr Socket::getLocalAddress(){
    if(m_localAddress){
        return m_localAddress;
    }
    Address::ptr result;
    switch(m_family){
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknowAddress(m_family));
            break;
    }
    socklen_t addrlen=result->getAddrLen();
    if(getsockname(m_sock,result->getAddr(),&addrlen)){
        JKYI_LOG_ERROR(g_logger)<<"getsockname error sock="<<m_sock
                                <<" errno="<<errno<<" errstr"<<strerror(errno);
        return Address::ptr (new UnknowAddress(m_family));
    }
    if(m_family == AF_UNIX){
        UnixAddress::ptr addr=std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }
    m_localAddress=result;
    return m_localAddress;
}

int Socket::getError(){
    int error=0;
    socklen_t len=sizeof(error);
    if(!getOption(SOL_SOCKET,SO_ERROR,&error,&len)){
        error=errno;
    }
    return error;
}

std::ostream& Socket::dump(std::ostream&os)const{
    os<<"[Socket sock="<<m_sock
      <<" is_Connected="<<m_isConnected
      <<" family="<<m_family
      <<" type="<<m_type
      <<" protocol="<<m_protocol;
    if(m_localAddress){
        os<<" localAddress="<<m_localAddress->toString();
    }
    if(m_remoteAddress){
        os<<" remoteAddress="<<m_remoteAddress->toString();
    }
    os<<"]";
    return os;
}

std::string Socket::toString()const{
    std::stringstream os;
    dump(os);
    return os.str();
}

bool Socket::cancelRead(){
    return IOManager::GetThis()->cancelEvent(m_sock,IOManager::READ);
} 

bool Socket::cancelWrite(){
    return IOManager::GetThis()->cancelEvent(m_sock,IOManager::WRITE);
}

bool Socket::cancelAccept(){
    return IOManager::GetThis()->cancelEvent(m_sock,IOManager::READ);
}

bool Socket::cancelAll(){
    return IOManager::GetThis()->cancelAll(m_sock);
}

void Socket::initSock(){
    int val=1;
    setOption(SOL_SOCKET,SO_REUSEADDR,val);
    if(m_type==SOCK_STREAM){
        setOption(IPPROTO_TCP,TCP_NODELAY,val);
    }
}

void Socket::newSock(){
    m_sock=::socket(m_family,m_type,m_protocol);
    if(JKYI_LIKELY(m_sock!=-1)){
       initSock();         
    }else{
       JKYI_LOG_ERROR(g_logger)<<"socket error,errno="<<errno<<" errstr="<<strerror(errno);
    }
}















}
