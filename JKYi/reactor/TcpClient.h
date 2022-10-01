#ifndef _JKYI_TCPCLIENT_H_
#define _JKYI_TCPCLIENT_H_

#include"JKYi/mutex.h"
#include"JKYi/reactor/TcpConnection.h"

namespace JKYi{
namespace net{

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

class TcpClient : public Noncopyable{
public:

    TcpClient(EventLoop* loop,
               const Address::ptr& serverAddr,
               const std::string& nameArg);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnection::ptr connecetion()const{
        Mutex::Lock lock(mutex_);
        return connection_;
    }
    EventLoop* getLoop()const{
        return loop_;
    }
    bool retry()const{
        return retry_;
    }
    void enableRetry(){
        retry_ = true;
    }
    const std::string& name()const{
        return name_;
    }

    void setConnectionCallback(ConnectionCallback cb){
        connectionCallback_ = std::move(cb);
    }
    void setMessageCallback(MessageCallback cb){
        messageCallback_ = std::move(cb);
    }
    void setWriteCompleteCallback(WriteCompleteCallback cb){
        writeCompleteCallback_ = std::move(cb);
    }
private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnection::ptr& conn);

    EventLoop* loop_;
    ConnectorPtr connector_;  //连接器
    const std::string name_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    bool retry_;
    bool connect_;
    int nextConnId_;

    mutable Mutex mutex_;
    TcpConnection::ptr connection_;
};
}
}

#endif
