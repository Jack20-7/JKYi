#ifndef _JKYI_TCP_SERVER_H_
#define _JKYI_TCP_SERVER_H_

#include"JKYi/atomic.h"
#include"JKYi/reactor/TcpConnection.h"

#include<map>
namespace JKYi{
namespace net{

//前向声明
class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : public Noncopyable{
public:
    typedef std::function<void (EventLoop*)> ThreadInitCallback;

    enum Option{
        kNoReusePort,
        kReusePort
    };

    TcpServer(EventLoop* loop,
              const Address::ptr& listerAddr,
              const std::string& nameArg,
              Option option = kNoReusePort);
    ~TcpServer();

    const std::string& ipPort()const { return ipPort_; }
    const std::string& name()const { return name_; }
    EventLoop* getLoop()const { return loop_; }

    // 0      单reactor单线程,也就是连接事件和IO事件都在同一个线程内处理
    // 1      连接事件在主reactor中处理，io事件在另外一个线程内处理
    // n      one loop per thread
    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb){
        threadInitCallback_ = cb;
    }
    std::shared_ptr<EventLoopThreadPool> threadPool(){
        return threadPool_;
    }

    void start();

    void setConnectionCallback(const ConnectionCallback& cb){
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback& cb){
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb){
        writeCompleteCallback_ = cb;
    }
private:
    void newConnection(int sockfd,const Address::ptr& peerAddr);
    void removeConnection(const TcpConnection::ptr& conn);
    void removeConnectionInLoop(const TcpConnection::ptr& conn);

    typedef std::map<std::string,TcpConnection::ptr> ConnectionMap;

    EventLoop* loop_;
    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_;   //监听器
    std::shared_ptr<EventLoopThreadPool> threadPool_;  //线程池

    //下面是用户手动设置的回调函数，用来完成业务逻辑
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;

    AtomicInt32 started_;

    int nextConnId_;
    ConnectionMap connections_;    
};

}
}

#endif
