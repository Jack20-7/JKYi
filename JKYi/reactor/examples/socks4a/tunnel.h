#ifndef _JKYI_NET_EXAMPLES_TUNNEL_H_
#define _JKYI_NET_EXAMPLES_TUNNEL_H_

#include"JKYi/reactor/Logging.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/address.h"
#include"JKYi/reactor/TcpClient.h"
#include"JKYi/reactor/TcpServer.h"

#include<memory>

class Tunnel : public std::enable_shared_from_this<Tunnel>,
                      JKYi::Noncopyable{
public:
    Tunnel(JKYi::net::EventLoop* loop,
            const JKYi::Address::ptr& serverAddr,
            const JKYi::net::TcpConnectionPtr& serverConn)
        :client_(loop,serverAddr,serverConn->name()),
         serverConn_(serverConn){

        LOG_INFO << "Tunnel  " << serverConn->peerAddress()->toString() 
                 << " <-> " << serverConn->localAddress()->toString();
     }

    ~Tunnel(){
        LOG_INFO << " ~Tunnel";
    }

    void connect(){
        client_.connect();
    }
    void disconnect(){
        client_.disconnect();
    }

    void setup(){
        using std::placeholders::_1;
        using std::placeholders::_2;
        using std::placeholders::_3;

        client_.setConnectionCallback(
                std::bind(&Tunnel::onClientConnection,shared_from_this(),_1));
        client_.setMessageCallback(
                std::bind(&Tunnel::onClientMessage,shared_from_this(),_1,_2,_3));
        //设置高水位回调,最高水位的限制设置为1MB
        serverConn_->setHighWaterMarkCallback(
                std::bind(&Tunnel::onHighWaterMarkWeak,std::weak_ptr<Tunnel>(shared_from_this()),kServer,_1,_2),1024 * 1024);     
    }
private:

    void teardown(){
        client_.setConnectionCallback(JKYi::net::defaultConnectionCallback);
        client_.setMessageCallback(JKYi::net::defaultMessageCallback);
        if(serverConn_){
            serverConn_->setContext(boost::any());
            serverConn_->shutdown();
        }
        clientConn_.reset();
    }
    void onClientConnection(const JKYi::net::TcpConnectionPtr& conn){
        using std::placeholders::_1;
        using std::placeholders::_2;

        LOG_DEBUG << (conn->connected() ? "UP" : "DOWN");
        if(conn->connected()){
            conn->setTcpNoDelay(true);
            conn->setHighWaterMarkCallback(
                    std::bind(&Tunnel::onHighWaterMarkWeak,std::weak_ptr<Tunnel>(shared_from_this()),kClient,_1,_2),1024 * 1024);
            serverConn_->setContext(conn);
            serverConn_->startRead();         //这个时候才能够开始读数据
            clientConn_ = conn;
            if(serverConn_->inputBuffer()->readableBytes() > 0){
                //如果客户端有发送来数据的话
                conn->send(serverConn_->inputBuffer());
            }
        }else{
            teardown();
        }
    }
    void onClientMessage(const JKYi::net::TcpConnectionPtr& conn,
                           JKYi::net::Buffer* buf,
                           JKYi::net::Timestamp receiveTime){
        LOG_DEBUG << conn->name() << " " << buf->readableBytes();
        if(serverConn_){
            serverConn_->send(buf);
        }else{
            buf->retrieveAll();
            abort();
        }
    }

    enum ServerClient{
        kServer,
        kClient,
    };
    void onHighWaterMark(ServerClient which,
                         const JKYi::net::TcpConnectionPtr& conn,
                         size_t bytesToSent){
        using std::placeholders::_1;

        LOG_INFO << (which == kServer ? "server" : "client")
                 << " onHighWaterMark " << conn->name()
                 << " bytes " << bytesToSent;

        if(which == kServer){
           if(serverConn_->outputBuffer()->readableBytes() > 0){
               clientConn_->stopRead();
               serverConn_->setWriteCompleteCallback(
                       std::bind(&Tunnel::onWriteCompleteWeak,std::weak_ptr<Tunnel>(shared_from_this()),kServer,_1));
           }
        }else{
            if(clientConn_->outputBuffer()->readableBytes() > 0){
                serverConn_->stopRead();
                clientConn_->setWriteCompleteCallback(
                        std::bind(&Tunnel::onWriteCompleteWeak,std::weak_ptr<Tunnel>(shared_from_this()),kClient,_1));
            }
        }
    }
    static void onHighWaterMarkWeak(const std::weak_ptr<Tunnel>& wkTunnel,
                                      ServerClient which,
                                      const JKYi::net::TcpConnectionPtr& conn,
                                      size_t bytesToSent){
        std::shared_ptr<Tunnel> tunnel = wkTunnel.lock();
        if(tunnel){
            tunnel->onHighWaterMark(which,conn,bytesToSent);
        }
    }
    void onWriteComplete(ServerClient which,const JKYi::net::TcpConnectionPtr& conn){
        LOG_INFO << (which == kServer ? "server" : "client") 
                 << " onWriteComplete " << conn->name();

        if(which == kServer){
            clientConn_->startRead();
            serverConn_->setWriteCompleteCallback(JKYi::net::WriteCompleteCallback());
        }else{
            serverConn_->startRead();
            clientConn_->setWriteCompleteCallback(JKYi::net::WriteCompleteCallback());
        }
    }
    static void onWriteCompleteWeak(const std::weak_ptr<Tunnel>& wkTunnel,
                                       ServerClient which,
                                       const JKYi::net::TcpConnectionPtr& conn){
        std::shared_ptr<Tunnel> tunnel = wkTunnel.lock();
        if(tunnel){
            tunnel->onWriteComplete(which,conn);
        }
    }


    //作为客户端用来与目标服务器进行连接建立
    JKYi::net::TcpClient client_;
    //与客户端之间的连接
    JKYi::net::TcpConnectionPtr serverConn_;
    //与服务器之间的连接
    JKYi::net::TcpConnectionPtr clientConn_;
};

typedef std::shared_ptr<Tunnel> TunnelPtr;

#endif
