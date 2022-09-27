#ifndef _JKYI_CONNECTOR_H_
#define _JKYI_CONNECTOR_H_

#include"JKYi/noncopyable.h"
#include"JKYi/address.h"

#include<functional>
#include<memory>

namespace JKYi{
namespace net{

class Channel;
class EventLoop;

//连接器
class Connector : public Noncopyable,
                  public std::enable_shared_from_this<Connector>{
public:
    typedef std::function<void (int sockfd)> NewConnectionCallback;

    Connector(EventLoop* loop,const Address::ptr& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb){
        newConnectionCallback_ = cb;
    }

    void start();       //can be called in any thread
    void restart();     //must be called in loop thread
    void stop();        //can be called in any thread

    const Address::ptr& serverAddress()const { return serverAddr_; }
private:
    enum State{
        kDisconnected,
        kConnecting,
        kConnected
    };
    static const int kMaxRetryDelayMs = 30 * 1000;     
    static const int kInitRetryDelayMs = 500;

    void setState(State s){ state_ = s; }

    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* loop_;
    Address::ptr serverAddr_;  //要连接的服务器端的地址
    bool connect_;
    State state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;
};

}
}

#endif
