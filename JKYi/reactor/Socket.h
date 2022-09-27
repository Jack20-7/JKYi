#ifndef _JKYI_SOCKET_H_
#define _JKYI_SOCKET_H_

#include"JKYi/noncopyable.h"
#include"JKYi/address.h"

struct tcp_info;

namespace JKYi{
namespace net{

class Socket : public Noncopyable{
public:
    explicit Socket(int sockfd)
        :sockfd_(sockfd){
    }
    ~Socket();

    int getFd()const { return sockfd_; }
    bool getTcpInfo(struct tcp_info*)const;
    bool getTcpInfoString(char * buf,int len)const;

    void bindAddress(const Address::ptr addr);
    void listen();
    int accept(Address::ptr& addr);
    
    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    const int sockfd_;
};

} 
}

#endif
