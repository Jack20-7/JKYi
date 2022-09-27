#include"JKYi/reactor/Socket.h"
#include"JKYi/reactor/SocketsOps.h"
#include"JKYi/log.h"
#include"JKYi/address.h"

#include<netinet/in.h>
#include<netinet/tcp.h>
#include<stdio.h>

namespace JKYi{
namespace net{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

Socket::~Socket(){
    sockets::close(sockfd_);
}

bool Socket::getTcpInfo(struct tcp_info* tcpi)const{
    socklen_t len = sizeof(*tcpi);
    memset(tcpi,0,len);
    return ::getsockopt(sockfd_,SOL_TCP,TCP_INFO,tcpi,&len) == 0;
}
bool Socket::getTcpInfoString(char * buf,int len)const{
  struct tcp_info tcpi;
  bool ok = getTcpInfo(&tcpi);
  if (ok){
    snprintf(buf, len, "unrecovered=%u "
             "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
             "lost=%u retrans=%u rtt=%u rttvar=%u "
             "sshthresh=%u cwnd=%u total_retrans=%u",
             tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
             tcpi.tcpi_rto,          // Retransmit timeout in usec
             tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
             tcpi.tcpi_snd_mss,
             tcpi.tcpi_rcv_mss,
             tcpi.tcpi_lost,         // Lost packets
             tcpi.tcpi_retrans,      // Retransmitted packets out
             tcpi.tcpi_rtt,          // Smoothed round trip time in usec
             tcpi.tcpi_rttvar,       // Medium deviation
             tcpi.tcpi_snd_ssthresh,
             tcpi.tcpi_snd_cwnd,
             tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
  }
  return ok;
}

void Socket::bindAddress(Address::ptr addr){
    sockets::bindOrDie(sockfd_,addr->getAddr());
}

void Socket::listen(){
    sockets::listenOrDie(sockfd_);
}

int Socket::accept(Address::ptr& addr){
    struct sockaddr_in6 laddr ;
    memset(&addr,0,sizeof(laddr));
    int connfd = sockets::accept(sockfd_,&laddr);
    if(connfd >= 0){
        struct sockaddr* daddr = sockets::sockaddr_cast(&laddr);
        socklen_t len = static_cast<socklen_t>(sizeof(*daddr));
        addr = JKYi::Address::Create(daddr,len);
    }
    return connfd;
}

void Socket::shutdownWrite(){
    sockets::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&optval,
                static_cast<socklen_t>(sizeof(optval)));
}
void Socket::setReuseAddr(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&optval,
                static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setReusePort(bool on){
#ifndef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEPORT,&optval,
                 static_cast<socklen_t>(sizeof(optval)));
    if(ret < 0 && on){
        JKYI_LOG_ERROR(g_logger) << " SO_REUSEPORT failed";
    }
#else
    if(on){
        JKYI_LOG_ERROR(g_logger) << " SO_REUSEPORT is not supported";
    }
#endif
}

void Socket::setKeepAlive(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_KEEPALIVE,&optval,
                 static_cast<socklen_t>(sizeof(optval)));
}


}
}
