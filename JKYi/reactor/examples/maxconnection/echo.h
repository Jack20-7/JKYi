#ifndef _JKYI_EXAMPLES_MAXCONNECTION_H_
#define _JKYI_EXAMPLES_MAXCONNECTION_H_

#include"JKYi/reactor/TcpServer.h"

class EchoServer{
public:
    EchoServer(JKYi::net::EventLoop *loop,
                const JKYi::Address::ptr& serverAddr,
                int maxConnections);
    void start();
private:
   void onConnection(const JKYi::net::TcpConnection::ptr& conn);
   void onMessage(const JKYi::net::TcpConnection::ptr& conn,
                     JKYi::net::Buffer* buf,
                     JKYi::net::Timestamp receiveTime);

   JKYi::net::TcpServer server_;
   int numConnected_;
   const int kMaxConnections_;   //最大连接数
};


#endif
