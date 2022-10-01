#ifndef _JKYI_EXAMPLES_PUBSUB_H_
#define _JKYI_EXAMPLES_PUBSUB_H_

#include"JKYi/reactor/TcpClient.h"
#include"boost/noncopyable.hpp"

#include<functional>
#include<memory>

namespace pubsub{
//客户端,既可能是publisher,也可能是subscriber
class PubSubClient : public boost::noncopyable{
public:
    typedef std::function<void (PubSubClient*)> ConnectionCallback;
    typedef std::function<void (const std::string& topic,
                                const std::string& content,
                                JKYi::net::Timestamp)> SubscribeCallback;

    PubSubClient(JKYi::net::EventLoop* loop,
                 const JKYi::Address::ptr& serverAddr,
                 const std::string& name);
    void start();
    void stop();
    bool connected()const;

    void setConnectionCallback(const ConnectionCallback& cb){
        connectionCallback_ = cb;
    }
    bool subscribe(const std::string& topic,const SubscribeCallback& cb);
    void unsubscribe(const std::string& topic);
    bool publish(const std::string& topic,const std::string& content);
private:
    void onConnection(const JKYi::net::TcpConnection::ptr& conn);
    void onMessage(const JKYi::net::TcpConnection::ptr& conn,JKYi::net::Buffer* buf,JKYi::net::Timestamp receiveTime);
    bool send(const std::string& message);

    JKYi::net::TcpClient client_;
    JKYi::net::TcpConnection::ptr conn_;
    ConnectionCallback connectionCallback_;
    SubscribeCallback subscribeCallback_;
};

}

#endif
