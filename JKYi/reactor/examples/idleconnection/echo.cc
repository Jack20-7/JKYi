#include"JKYi/reactor/examples/idleconnection/echo.h"
#include"JKYi/log.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/macro.h"

using namespace JKYi;
using namespace JKYi::net;

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

EchoServer::EchoServer(JKYi::net::EventLoop* loop,
                       const Address::ptr& serverAddr,
                       int idleSeconds)
    :server_(loop,serverAddr,"EchoServer"),
     connectionBuckets_(idleSeconds){

     server_.setConnectionCallback(
             std::bind(&EchoServer::onConnection,this,_1));
     server_.setMessageCallback(
             std::bind(&EchoServer::onMessage,this,_1,_2,_3));
     loop->runEvery(1.0,
             std::bind(&EchoServer::onTimer,this));
     connectionBuckets_.resize(idleSeconds);
     dumpConnectionBuckets();
}

void EchoServer::start(){
    server_.start();
}

void EchoServer::onConnection(const TcpConnection::ptr& conn){
    JKYI_LOG_INFO(g_logger) << "EchoServer - " << conn->peerAddress()->toString()
                            << " -> " << conn->localAddress()->toString()
                            << " is " << (conn->connected() ? "UP" : "DOWN");
    if(conn->connected()){
        EntryPtr entry(new Entry(conn));
        connectionBuckets_.back().insert(entry);
        dumpConnectionBuckets();
        WeakEntryPtr weakEntry(entry);
        conn->setContext(weakEntry);
    }else{
        JKYI_ASSERT(!conn->getContext().empty()); 
        WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
        JKYI_LOG_DEBUG(g_logger) << " Entry use count = " << weakEntry.use_count();
    }
}
void EchoServer::onMessage(const TcpConnection::ptr& conn,
                            Buffer* buf,Timestamp receiveTime){
    std::string str(buf->retrieveAllAsString());
    JKYI_LOG_INFO(g_logger) << conn->name() << " echo " << str.size() 
                            << " bytes at " << receiveTime.toFormattedString()
                            << "  [ " << str << " ] ";
    conn->send(str);
    JKYI_ASSERT(!conn->getContext().empty());
    WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
    EntryPtr entry(weakEntry.lock());
    if(entry){
        connectionBuckets_.back().insert(entry);// 将该Entry的shared_ptr放入到时间轮
        dumpConnectionBuckets();
    }
}

void EchoServer::onTimer(){
    connectionBuckets_.push_back(Bucket());  //会自动将第一个Buckets弹出去
    dumpConnectionBuckets();
}

void EchoServer::dumpConnectionBuckets()const{
    JKYI_LOG_INFO(g_logger) << "size = " << connectionBuckets_.size();

    int idx = 0;
    for(WeakConnectionList::const_iterator it = connectionBuckets_.begin();
                 it != connectionBuckets_.end();
                 ++it,++idx){
        const Bucket& bucket = *it;
        printf("[%d]len = %zd : ",idx,bucket.size());
        for(const auto& e : bucket){
            bool connectionDead = e->weakConn_.expired();
            printf("%p(%ld)%s",get_pointer(e),e.use_count(),connectionDead ? "DEAD" : "");
        }
        puts("");
    }
}


