#ifndef _JKYI_EXAMPLES_IDLECONNECTION_H_
#define _JKYI_EXAMPLES_IDLECONNECTION_H_

#include"JKYi/reactor/TcpServer.h"

#include<memory>
#include<unordered_set>
#include<boost/circular_buffer.hpp>

class EchoServer{
public:
    EchoServer(JKYi::net::EventLoop* loop,
               const JKYi::Address::ptr& serverAddr,
               int idleSeconds);
    void start();
private:
    void onConnection(const JKYi::net::TcpConnection::ptr& conn);
    void onMessage(const JKYi::net::TcpConnection::ptr& conn,JKYi::net::Buffer* buf,JKYi::net::Timestamp receiveTime);
    void onTimer();
    void dumpConnectionBuckets()const;//该函数其实就是用来对当前时间轮中的连接进行输出

    typedef std::weak_ptr<JKYi::net::TcpConnection> WeakTcpConnectionPtr;

    //时间轮中真正存储的元素
    struct Entry : public JKYi::Noncopyable{
        explicit Entry(const WeakTcpConnectionPtr& weakConn)
            :weakConn_(weakConn){
        }

        ~Entry(){
            JKYi::net::TcpConnection::ptr conn = weakConn_.lock();
            if(conn){
                conn->shutdown();
            }
        }
        WeakTcpConnectionPtr weakConn_;
    };

    typedef std::shared_ptr<Entry> EntryPtr;
    typedef std::weak_ptr<Entry> WeakEntryPtr;
    typedef std::unordered_set<EntryPtr> Bucket;
    typedef boost::circular_buffer<Bucket> WeakConnectionList;

    JKYi::net::TcpServer server_;
    WeakConnectionList connectionBuckets_;   //时间轮
};

#endif
