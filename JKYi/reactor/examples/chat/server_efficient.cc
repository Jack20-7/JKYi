#include"JKYi/reactor/examples/chat/codec.h"
#include"JKYi/reactor/Logging.h"
#include"JKYi/mutex.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/TcpServer.h"
#include"JKYi/address.h"

#include<set>
#include<stdio.h>
#include<unistd.h>

using namespace JKYi;
using namespace JKYi::net;

class ChatServer : public Noncopyable{
public:
    ChatServer(EventLoop* loop,
                  const Address::ptr& listenAddr)
        :server_(loop,listenAddr,"ChatServer"),
         codec_(std::bind(&ChatServer::onStringMessage,this,_1,_2,_3)),
         connections_(new ConnectionList){

         server_.setConnectionCallback(
                 std::bind(&ChatServer::onConnection,this,_1));
         server_.setMessageCallback(
                 std::bind(&LengthHeaderCodec::onMessage,&codec_,_1,_2,_3));
     }

     void setThreadNum(int numThreads){
         server_.setThreadNum(numThreads);
     }

     void start(){
         server_.start();
     }
private:
    void onConnection(const TcpConnectionPtr& conn){
        LOG_INFO << conn->peerAddress()->toString() << " -> " 
                 << conn->localAddress()->toString() << " is "
                 << (conn->connected() ? "UP" : "DOWN");
        Mutex::Lock lock(mutex_);
        //这里使用的技术是copy on write技术
        if(!connections_.unique()){
            //如果当前有其他线程在读的话
            connections_.reset(new ConnectionList(*connections_));
        }
        assert(connections_.unique());
        if(conn->connected()){
            connections_->insert(conn);
        }else{
            connections_->erase(conn);
        }
    }

    typedef std::set<TcpConnectionPtr> ConnectionList;
    typedef std::shared_ptr<ConnectionList> ConnectionListPtr;

    void onStringMessage(const TcpConnectionPtr& conn,
                           const std::string& message,
                           Timestamp receiveTime){
        //使用到了copy on write技术
        //引用计数 + 1
        ConnectionListPtr connections = getConnectionList();
        for(ConnectionList::iterator it = connections->begin();
                 it != connections->end();++it){
            codec_.send((*it).get(),message);
        }
    }

    ConnectionListPtr getConnectionList(){
        Mutex::Lock lock(mutex_);
        return connections_;
    }

    TcpServer server_;
    LengthHeaderCodec codec_;
    Mutex mutex_;
    ConnectionListPtr connections_;
};

int main(int argc,char** argv){
    LOG_INFO << "pid = " << getpid();

    if(argc > 1){
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        Address::ptr listenAddr = Address::LookupAnyIPAddress("127.0.0.1",std::to_string(port));
        ChatServer server(&loop,listenAddr);
        if(argc > 2){
            server.setThreadNum(atoi(argv[2]));
        }
        server.start();
        loop.loop();
    }else{
        printf("Usage: %s port [threadNum]\n",argv[0]);
    }
    return 0;
}
