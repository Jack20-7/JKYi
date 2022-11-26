#include"JKYi/reactor/SocketsOps.h"
#include"JKYi/log.h"
#include"JKYi/endian.h"
#include"JKYi/macro.h"
#include"JKYi/Types.h"

#include<errno.h>
#include<fcntl.h>
#include<stdio.h>
#include<sys/socket.h>
#include<sys/uio.h>
#include<unistd.h>
#include<string.h>


using namespace JKYi;
using namespace JKYi::net;

static Logger::ptr g_logger = JKYI_LOG_NAME("system");
namespace {

typedef struct sockaddr SA;

#if VALGRIND || defined (NO_ACCEPT4)
void setNonBlockAndCloseOnExec(int sockfd){
    //set non_block
    int flags = ::fcntl(sockfd,F_GETFL,0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd,F_SETFL,flags);

    //set CLOEXEC
    flags = ::fcntl(sockfd,F_GETFD,0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd,F_SETFD,flags);
}
#endif

} //namespace

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in6* addr){
  return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in6* addr){
  return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr){
  return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr){
  return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* sockets::sockaddr_in6_cast(const struct sockaddr* addr){
  return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}

int sockets::createNonBlockingOrDie(sa_family_t  family){
#if VALGRIND
    int sockfd = ::socket(family,SOCK_STREAM,0);
    if(sockfd < 0){
        JKYI_LOG_ERROR(g_logger) << " sockets::createNonBlockingOrDie error";
        exit(0);
    }
    setNonBlockAndCloseOnExec(sockfd);
#else
    int sockfd = ::socket(family,SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,0);
    if(sockfd < 0){
        JKYI_LOG_ERROR(g_logger) << " sockets::createNonBlockingOrDie error";
        exit(0);
    }
#endif
    return sockfd;
}

void sockets::bindOrDie(int sockfd,const struct sockaddr* addr){
    int rt = ::bind(sockfd,addr,static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if(rt < 0){
        JKYI_LOG_ERROR(g_logger) << " sockets::bindOrie error";
        exit(0);
    }
}

void sockets::listenOrDie(int sockfd){
    int ret = ::listen(sockfd,SOMAXCONN);
    if(ret < 0){
        JKYI_LOG_ERROR(g_logger) << " sockets::listenOrDie error";
        exit(0);
    }
}

int sockets::accept(int sockfd, struct sockaddr_in6* addr){
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
#if VALGRIND || defined (NO_ACCEPT4)
  int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
  setNonBlockAndCloseOnExec(connfd);
#else
  int connfd = ::accept4(sockfd, sockaddr_cast(addr),
                         &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
  if (connfd < 0)
  {
    int savedErrno = errno;
    JKYI_LOG_ERROR(g_logger) << "Socket::accept";
    switch (savedErrno)
    {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO: // ???
      case EPERM:
      case EMFILE: // per-process lmit of open file desctiptor ???
        // expected errors
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        JKYI_LOG_FATAL(g_logger) << "unexpected error of ::accept " << savedErrno;
        break;
      default:
        JKYI_LOG_FATAL(g_logger) << "unknown error of ::accept " << savedErrno;
        break;
    }
  }
  return connfd;
}
int sockets::connect(int sockfd, const struct sockaddr* addr){
  return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

ssize_t sockets::read(int sockfd, void *buf, size_t count){
  return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt){
  return ::readv(sockfd, iov, iovcnt);
}

ssize_t sockets::write(int sockfd, const void *buf, size_t count){
  return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd){
  if (::close(sockfd) < 0){
    JKYI_LOG_ERROR(g_logger) << "sockets::close";
    exit(0);
  }
}

void sockets::shutdownWrite(int sockfd){
    if(::shutdown(sockfd,SHUT_WR) < 0){
        JKYI_LOG_ERROR(g_logger) << " sockets::shutdownWrite error";
        exit(0);
    }
}
//根据传入的addr返回对于的IP + PORT并且要是主机字节序
void sockets::toIpPort(char * buf,size_t size,const struct sockaddr* addr){
    if (addr->sa_family == AF_INET6)
  {
    buf[0] = '[';
    toIp(buf+1, size-1, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
    uint16_t port = JKYi::toLittleEndian(addr6->sin6_port);
    JKYI_ASSERT(size > end);
    snprintf(buf+end, size-end, "]:%u", port);
    return;
  }
  toIp(buf, size, addr);
  size_t end = ::strlen(buf);
  const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
  uint16_t port = JKYi::toLittleEndian(addr4->sin_port);
  JKYI_ASSERT(size > end);
  snprintf(buf+end, size-end, ":%u", port);
}

void sockets::toIp(char * buf,size_t size,const struct sockaddr* addr){
    if(addr->sa_family == AF_INET){
        JKYI_ASSERT(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
        ::inet_ntop(AF_INET,&addr4->sin_addr,buf,static_cast<socklen_t>(size));
    }else if(addr->sa_family == AF_INET6){
        JKYI_ASSERT(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
        ::inet_ntop(AF_INET6,&addr6->sin6_addr,buf,static_cast<socklen_t>(size));
    }
}

void sockets::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in* addr){
  addr->sin_family = AF_INET;
  addr->sin_port = JKYi::toNetEndian(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0){
    JKYI_LOG_ERROR(g_logger) << "sockets::fromIpPort";
  }
}

void sockets::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in6* addr){
  addr->sin6_family = AF_INET6;
  addr->sin6_port = JKYi::toNetEndian(port);
  if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0){
    JKYI_LOG_ERROR(g_logger) << "sockets::fromIpPort";
  }
}

int sockets::getSocketError(int sockfd){
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if(::getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&optval,&optlen) < 0){
        return errno;
    }else{
        return optval;
    }
}

struct sockaddr_in6 sockets::getLocalAddr(int sockfd){
  struct sockaddr_in6 localaddr;
  memset(&localaddr,0,sizeof localaddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0){
    JKYI_LOG_ERROR(g_logger) << "sockets::getLocalAddr";
  }
  return localaddr;
}

struct sockaddr_in6 sockets::getPeerAddr(int sockfd){
  struct sockaddr_in6 peeraddr;
  memset(&peeraddr,0,sizeof peeraddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0){
    JKYI_LOG_ERROR(g_logger) << "sockets::getPeerAddr";
  }
  return peeraddr;
}

//判断是否是自连接
bool sockets::isSelfConnect(int sockfd){
    struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
    if(localaddr.sin6_family == AF_INET){
      const struct sockaddr_in* laddr4 = 
                     reinterpret_cast<struct sockaddr_in*>(&localaddr); 
      const struct sockaddr_in* paddr4 =
                     reinterpret_cast<struct sockaddr_in*>(&peeraddr);
      return laddr4->sin_port == paddr4->sin_port
              && laddr4->sin_addr.s_addr == paddr4->sin_addr.s_addr;
    }else if(localaddr.sin6_family == AF_INET6){
        return localaddr.sin6_port == peeraddr.sin6_port
              && memcmp(&localaddr.sin6_addr,&peeraddr.sin6_addr,
                        sizeof(localaddr.sin6_addr)) == 0;
    }else{
        return false;
    }
}





