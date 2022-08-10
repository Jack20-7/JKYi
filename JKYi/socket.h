#ifndef _JKYI_SOCKET_H_
#define _JKYI_SOCKET_H_

#include<memory>
#include<netinet/tcp.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<openssl/err.h>
#include<openssl/ssl.h>

#include"address.h"
#include"noncopyable.h"

namespace JKYi{

//对socket的封装
class Socket:public std::enable_shared_from_this<Socket>,Noncopyable{
public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weak_ptr;

    //下面自己对创建的socket的类型进行定义
    enum Type{
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };

    //然后是创建的socket的协议族
    enum Family{
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        Unix = AF_UNIX
    };
    
    //下面定义一些方便用户使用的函数
    static Socket::ptr CreateTCP(Address::ptr address);
    static Socket::ptr CreateUDP(Address::ptr address);

    //默认是IPv4
    static Socket::ptr CreateTCPSocket();
    static Socket::ptr CreateUDPSocket();

    static Socket::ptr CreateTCPSocket6();
    static Socket::ptr CreateUDPSocket6();

    //unixsocket
    static Socket::ptr CreateUnixTCPSocket();
    static Socket::ptr CreateUnixUDPSocket();

    //
    Socket(int family,int type,int protocol=0);
    //未来有需求的话可以作为基类
    virtual ~Socket();

    int64_t getSendTimeout();
    void setSendTimeout(int64_t v);
    
    int64_t getRcvTimeout();
    void setRcvTimeout(int64_t v);

    bool getOption(int level,int option,void *result,socklen_t *len);
    template<class T>
    bool getOption(int level,int option,T&result){
        int len=sizeof(result);
        return getOption(level,option,&result,&len);
    }

    bool setOption(int level,int option,const void *result,socklen_t len);
    template<class T>
    bool setOption(int level,int option,const T&result){
        socklen_t len=sizeof(result);
        return setOption(level,option,&result,len);
    }

    //网络编程相关的函数
    virtual Socket::ptr accept();
    virtual bool bind(const Address::ptr addr);
    virtual bool connect(const Address::ptr addr,int64_t timeout_ms = -1);
    virtual bool reconnect(int64_t timeout_ms=-1);
    virtual bool listen(int backlog = SOMAXCONN);
    virtual bool close();

    //下面就是收发网络包相关的函数的封装
    virtual int send(const void *buffer,size_t len,int flag=0);
    virtual int send(const iovec*buffers,size_t len,int flags=0);
    virtual int sendTo(const void * buffer,size_t len,const Address::ptr to,int flags=0);
    virtual int sendTo(const iovec*buffers,size_t len,const Address::ptr to,int flags=0);

    virtual int recv(void * buffer,size_t len,int flags=0);
    virtual int recv(iovec*buffers,size_t len,int flags=0);
    virtual int recvFrom(void *buffer,size_t len,Address::ptr from,int flags=0);
    virtual int recvFrom(iovec*buffers,size_t len,Address::ptr from,int flags=0);

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();
    int getFamily()const { return m_family ;};
    int getType()const { return m_type; }
    int getProtocol()const { return m_protocol; }
    int getSocket()const{ return m_sock; }

    bool isConnected()const { return m_isConnected; }
    bool isValid()const  { return m_sock != -1; }
    int getError();

    //该函数的左右就是将当前socket的信息按格式写入到流里面去
    virtual std::ostream& dump(std::ostream&os)const;
    virtual std::string toString()const;
    
    bool cancelRead();
    bool cancelWrite();
    bool cancelAccept();
    bool cancelAll();
protected:
    void initSock();
    void newSock();

    virtual bool init(int sock);

protected:
    //
    int m_sock;
    int m_family;
    int m_type;
    int m_protocol;
    bool m_isConnected;
    Address::ptr m_remoteAddress;
    Address::ptr m_localAddress;
};

//下面是提供对ssl的支持
class SSLSocket:public Socket{
public:
    typedef std::shared_ptr<SSLSocket> ptr;

    static SSLSocket::ptr CreateTCP(JKYi::Address::ptr addr);
    static SSLSocket::ptr CreateTCPSocket();
    static SSLSocket::ptr CreateTCPSocket6();

    SSLSocket(int family,int type,int protocol = 0);
    virtual Socket::ptr accept()override;
    virtual bool bind(const Address::ptr addr)override;
    virtual bool connect(const Address::ptr addr,int64_t timeout_ms = -1)override;
    virtual bool listen(int backlog = SOMAXCONN)override;
    virtual bool close()override;
    
    virtual int send(const void * buffer,size_t length,int flags = 0)override;
    virtual int send(const iovec* buffers,size_t length,int flags = 0)override;
    virtual int sendTo(const void * buffer,size_t length,const Address::ptr to,int flags = 0)override;
    virtual int sendTo(const iovec * buffers,size_t length,const Address::ptr to,int flags = 0)override;

    virtual int recv(void * buffer,size_t length,int flags = 0)override;
    virtual int recv(iovec * buffers,size_t length,int flags = 0)override;
    virtual int recvFrom(void * buffer,size_t length,Address::ptr from,int flags = 0)override;
    virtual int recvFrom(iovec * buffers,size_t length,Address::ptr from,int flags = 0)override;

    bool loadCertificates(const std::string& cert_file,const std::string& key_file);
    virtual std::ostream& dump(std::ostream& os)const override;
protected:
    virtual bool init(int sock)override;
private:
    std::shared_ptr<SSL_CTX> m_ctx;
    std::shared_ptr<SSL> m_ssl;
};

}



#endif
