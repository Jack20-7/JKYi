#ifndef _JKYI_NET_CHARGEN_H_
#define _JKYI_NET_CHARGEN_H_

#include"JKYi/reactor/TcpServer.h"

#include<string>
#include<stdint.h>

class ChargenServer{
public:
    ChargenServer(JKYi::net::EventLoop* loop,
                    const JKYi::Address::ptr& listenAddr,
                    bool print = false);
    void start();
private:

    void onConnection(const JKYi::net::TcpConnectionPtr& conn);
    void onMessage(const JKYi::net::TcpConnectionPtr& conn,
                    JKYi::net::Buffer* buf,
                    JKYi::net::Timestamp recviveTime);

    void onWriteComplete(const JKYi::net::TcpConnectionPtr& conn);
    void printThroughput();


    JKYi::net::TcpServer server_;

    std::string message_;            //发送的消息
    int64_t transferred_;            //记录发送的字节数
    JKYi::net::Timestamp startTime_; //记录开始时间
};

#endif
