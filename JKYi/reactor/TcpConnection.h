#ifndef _JKYI_TCPCONNECTION_H_
#define _JKYI_TCPCONNECTION_H_

#include"JKYi/noncopyable.h"
#include"JKYi/reactor/Callbacks.h"
#include"JKYi/reactor/Buffer.h"
#include"JKYi/address.h"
#include"JKYi/reactor/SocketsOps.h"
#include"JKYi/reactor/Socket.h"
#include"JKYi/reactor/StringPiece.h"

#include<memory>
#include<boost/any.hpp>

struct tcp_info;

namespace JKYi{
namespace net{

class Channel;
class EventLoop;

class TcpConnection : public Noncopyable,
                      public std::enable_shared_from_this<TcpConnection>{
public:
    typedef std::shared_ptr<TcpConnection> ptr;

    TcpConnection(EventLoop* loop,const std::string& name,int sockfd,
                   const Address::ptr localAddr,const Address::ptr peerAddr);
    ~TcpConnection();

    EventLoop* getLoop()const { return loop_; }
    const std::string& name()const { return name_; }
    const Address::ptr& localAddress()const{ return localAddr_; }
    const Address::ptr& peerAddress() const{ return peerAddr_; }

    bool connected()const { return state_ == kConnected; }
    bool disconnected()const { return state_ == kDisconnected; }

    bool getTcpInfo(struct tcp_info* )const;
    std::string getTcpInfoString()const;

    void send(const void * message,int len);
    void send(const StringPiece& message);
    void send(Buffer* message);

    void shutdown();
    void forceClose();
    void forceCloseWithDelay(double seconds);

    void setTcpNoDelay(bool on);
    void startRead();
    void stopRead();
    bool isReading()const{
        return reading_;
    }

    void setContext(const boost::any& context){
        context_ = context;
    }
    const boost::any& getContext()const{
        return context_;
    }
    boost::any* getMutableContext(){
        return &context_;
    }

    void setConnectionCallback(const ConnectionCallback& cb){
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback& cb){
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb){
        writeCompleteCallback_ = cb;
    }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,size_t highWaterMark){
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }
    void setCloseCallback(const CloseCallback& cb){
        closeCallback_ = cb;
    }
    //
    Buffer* inputBuffer(){
        return &inputBuffer_;
    }
    Buffer* outputBuffer(){
        return &outputBuffer_;
    }

    void connectEstablished();
    void connectDestroyed();
private:
    enum StateE{
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const StringPiece& message);
    void sendInLoop(const void * message,size_t len);
    void shutdownInLoop();
    void forceCloseInLoop();

    void setState(StateE s){
        state_  = s;
    }
    const char * stateToString()const;

    void startReadInLoop();
    void stopReadInLoop();

    EventLoop* loop_;
    const std::string name_;
    StateE state_;
    bool reading_;

    std::unique_ptr<Socket>  socket_;
    std::unique_ptr<Channel> channel_;

    const Address::ptr localAddr_;
    const Address::ptr peerAddr_;
    
    //下面就是用户已经使用的时候设置的回调函数，用于实现业务逻辑
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;   //高水位回调
    CloseCallback closeCallback_;

    size_t highWaterMark_;   //水位

    //两个用户层的Buffer
    Buffer inputBuffer_;
    Buffer outputBuffer_;

    boost::any context_;    //用户自定义携带的上下文信息
};

}//namespacee net
}

#endif
