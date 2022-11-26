#include"JKYi/reactor/protorpc/RpcServer.h"
#include"JKYi/reactor/protorcp/RpcChannel.h"
#include"JKYi/reactor/Logging.h"

#include"/usr/local/include/google/protobuf/service.h"
#include"/usr/local/include/google/protobuf/descriptor.h"

namespace JKYi{
namespace net{

RpcServer::RpcServer(EventLoop* loop
                       const Address::ptr& listenAddr)
    :server_(loop,listenAddr,"RpcServer"){

    server_.setConnectionCallback(
            std::bind(&RpcServer::onConnection,this,_1));
}

void RpcServer::registerService(google::protobuf::Service* service){
    const google::protobuf::ServiceDescriptor* desc = service->GetDescrptor();
    services_[desc->full_name()] = service;
}

void RpcServer::start(){
    server_.start();
}

void RpcServer::onConnection(const TcpConnectionPtr& conn){
    LOG_INFO << "RcpServer-" << conn->peerAddr()->toString() << " -> "
             << conn->localAddr()->toString() << " is " 
             << (conn->connected() ? "UP" : "DOWN");
    if(conn->connected()){
        RcpChannelPtr channel(new RpcChannel(conn));
        channel->setServices(&services_);
        conn->setMessageCallback(
                std::bind(&RpcChannel::onMessage,get_pointer(channel),_1,_2,_3));
        conn->setContext(channel);
    }else{
        conn->setContext(RpcChannel());
    }
}


}//namespace net
}//namespace JKYi
