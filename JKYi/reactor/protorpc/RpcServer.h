#ifndef _JKYI_NET_RPC_SERVER_H_
#define _JKYI_NET_RPC_SERVER_H_

#include"JKYi/reactor/TcpServer.h"

#include<map>
#include<string>

namespace google{
namespace protobuf{

class Service;

}//namespace protobuf 
}//namespace google


namespace JKYi{
namespace net{

//rpc服务器
class RpcServer{
public:
    RpcServer(EventLoop* loop,
                const Address::ptr& listenAddr);

    void setThreadNum(int numThreads){
        servers_.setThreadNum(numThreads);
    }
    //注册服务
    void registerService(::google::protobuf::Service*);
    void start();
private:
    void onConnection(const TcpConnectionPtr& conn);

    TcpServer server_;
    std::map<std::string,::google::protobuf::Service*> services_;
};

}
}
#endif
