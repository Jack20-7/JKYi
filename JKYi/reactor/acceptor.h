#ifndef _JKYI_ACCEPTOR_H_
#define _JKYI_ACCEPTOR_H_

#include<functional>

#include"JKYi/reactor/channel.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/Socket.h"
#include"JKYi/address.h"

namespace JKYi{
namespace net{

class EventLoop;

//对监听器的封装
class Acceptor:public Noncopyable{
public:
    typedef std::function<void(int sockfd,const Address::ptr)> NewConnectionCallback;

    Acceptor(EventLoop* loop,const Address::ptr& address,bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb){
        newConnectionCallback_ = cb;
    }
    void listen();
    bool listening()const { return listening_; }
private:
    void handleRead();
private:
    EventLoop* loop_;
    Socket  acceptSocket_;
    Channel acceptChannel_;
    //用户注册的连接回调
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
    //用来解决文件描述符耗尽问题
    int idleFd_;
};

}
}
#endif
