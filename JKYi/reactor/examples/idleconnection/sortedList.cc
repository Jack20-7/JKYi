#include"JKYi/reactor/reactor.h"
#include"JKYi/log.h"
#include"JKYi/macro.h"

#include<stdio.h>
#include<list>
#include<unistd.h>
#include<string>

using namespace JKYi;
using namespace JKYi::net;

static Logger::ptr g_logger = JKYI_LOG_ROOT();

//改良版
class EchoServer{
public:
    EchoServer(EventLoop* loop,
               const Address::ptr& serverAddr,
               int idleSeconds);
    void start(){
        server_.start();
    }
private:
    void onConnection(const TcpConnection::ptr& conn);
    void onMessage(const TcpConnection::ptr& conn,Buffer* buf,Timestamp receiveTime);
    void onTimer();
    void dumpConnectionList()const;

    typedef std::weak_ptr<TcpConnection> WeakTcpConnectionPtr;
    typedef std::list<WeakTcpConnectionPtr> WeakConnectionList;

    struct Node : public JKYi::Copyable{
       Timestamp lastReceiveTime; 
       WeakConnectionList::iterator position;
    };

    TcpServer server_;
    int idleSeconds_;
    WeakConnectionList connectionList_;
};

EchoServer::EchoServer(EventLoop* loop,const Address::ptr& serverAddr,int idleSeconds)
    :server_(loop,serverAddr,"EchoServer"),
     idleSeconds_(idleSeconds){

    server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection,this,_1));
    server_.setMessageCallback(
            std::bind(&EchoServer::onMessage,this,_1,_2,_3));
    loop->runEvery(1.0,
            std::bind(&EchoServer::onTimer,this));
    dumpConnectionList();
}

void EchoServer::onConnection(const TcpConnection::ptr& conn){
    JKYI_LOG_INFO(g_logger) << "EchoServer - " << conn->peerAddress()->toString()
                            << " -> " << conn->localAddress()->toString()
                            << " is " << (conn->connected() ? "UP" : "DOWN");
    if(conn->connected()){
        Node node;
        node.lastReceiveTime = Timestamp::now();
        connectionList_.push_back(conn);
        node.position = --connectionList_.end();
        conn->setContext(node);
    }else{
        JKYI_ASSERT(!conn->getContext().empty());
        const Node& node = boost::any_cast<const Node&>(conn->getContext());
        connectionList_.erase(node.position);
    }
    dumpConnectionList();
}

void EchoServer::onMessage(const TcpConnection::ptr& conn,Buffer* buf,Timestamp receiveTime){
    std::string str = buf->retrieveAllAsString();
    JKYI_LOG_INFO(g_logger) << " echo  " << str.size()  << " bytes at " << receiveTime.toFormattedString() << " [ " << str << " ]";
    conn->send(str);

    JKYI_ASSERT(!conn->getContext().empty());
    Node* node = boost::any_cast<Node>(conn->getMutableContext());
    node->lastReceiveTime = receiveTime;
    connectionList_.splice(connectionList_.end(),connectionList_,node->position);   //将对于的在list容器中的元素放到末尾去
    JKYI_ASSERT(node->position == --connectionList_.end());

    dumpConnectionList();
}

void EchoServer::onTimer(){
    dumpConnectionList();
    Timestamp now = Timestamp::now();
    for(WeakConnectionList::iterator it = connectionList_.begin();
            it != connectionList_.end();){
        TcpConnection::ptr conn = it->lock();
        if(conn){
            Node* n = boost::any_cast<Node>(conn->getMutableContext());
            double age = timeDifference(now,n->lastReceiveTime);
            if(age > idleSeconds_){
                if(conn->connected()){
                    conn->shutdown();
                    JKYI_LOG_DEBUG(g_logger) << " shutting down " << conn->name();
                    conn->forceCloseWithDelay(3.5);
                }                
            }else if(age < 0){
                JKYI_LOG_WARN(g_logger) << " Time jump";
                n->lastReceiveTime = now;
            }else{
                //遍历到第一个没过期的就直接结束，类似于LRU问题
                break;
            }
            ++it;
        }else{
            JKYI_LOG_INFO(g_logger) << " Expired ";
             it = connectionList_.erase(it);
        }
    }
}

void EchoServer::dumpConnectionList()const{
    JKYI_LOG_INFO(g_logger) << " size = " << connectionList_.size();

    for(WeakConnectionList::const_iterator it = connectionList_.begin();
            it != connectionList_.end();++it){
        TcpConnection::ptr conn = it->lock();
        if(conn){
            printf("conn %p\n",get_pointer(conn));
            const Node& n = boost::any_cast<const Node&>(conn->getContext());
            printf("     time %s\n",n.lastReceiveTime.toString().c_str());
        }else{
            printf("expired\n");
        }
    }
}


int main(int argc,char ** argv){
    if(argc <= 1){
        printf("to less params");
        exit(0);
    }
    char buf[64];
    snprintf(buf,sizeof(buf),"127.0.0.1:%s",argv[1]);
    Address::ptr serverAddr = Address::LookupAnyIPAddress(std::string(buf));
    EventLoop loop;
    int idleSeconds = 10;
    EchoServer server(&loop,serverAddr,idleSeconds);
    server.start();
    loop.loop();
}
