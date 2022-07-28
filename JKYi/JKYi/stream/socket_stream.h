#ifndef _JKYI_SOCKET_STREAM_H_
#define _JKYI_SOCKET_STREAM_H_

#include"JKYi/stream.h"
#include"JKYi/socket.h"
#include"JKYi/mutex.h"
#include"JKYi/iomanager.h"

namespace JKYi{
class SocketStream:public Stream{
public:
     typedef std::shared_ptr<SocketStream> ptr;

     SocketStream(Socket::ptr sock,bool owner = true);
     ~SocketStream();

     virtual int read(void * buffer,size_t length) override;
     virtual int read(ByteArray::ptr ba,size_t length) override;

     virtual int write(const void * buffer,size_t length) override;
     virtual int write(ByteArray::ptr ba,size_t length) override;

     virtual void close() override;

     Socket::ptr getSocket() const { return m_socket; }

     //是否处于连接状态
     bool isConnected()const;

     Address::ptr getRemoteAddress();
     Address::ptr getLocalAddress();
     std::string getRemoteAddressString();
     std::string getLocalAddressString();
private:
     //对应的socket
     Socket::ptr m_socket;
     //是否拥有该socket的控制权
     bool m_owner;
};
}

#endif
