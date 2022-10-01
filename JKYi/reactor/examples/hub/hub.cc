#include"JKYi/reactor/examples/hub/codec.h"
#include"JKYi/log.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/TcpServer.h"
#include"boost/noncopyable.hpp"
#include"JKYi/timestamp.h"

#include<map>
#include<set>
#include<stdio.h>

using namespace JKYi;
using namespace JKYi::net;

namespace pubsub{

typedef std::set<std::string> ConnectionSubscription;  //用作TcpConnection的上下文，里面存储的是该连接所关注的topic

static Logger::ptr g_logger = JKYI_LOG_NAME("system");

//topic类
class Topic{
public:
    Topic(const std::string & topic)
        :topic_(topic){}
    void add(const TcpConnection::ptr& conn){
        audiences_.insert(conn);
        if(lastPubTime_.valid()){
            conn->send(makeMessage());
        }
    }
    void remove(const TcpConnection::ptr& conn){
        audiences_.erase(conn);
    }
    void publish(const std::string& content,Timestamp time){
        content_ = content;
        lastPubTime_ = time;
        std::string message = makeMessage();
        for(auto it = audiences_.begin();it != audiences_.end();++it){
            (*it)->send(message);
        }
    }
private:
    std::string makeMessage(){
        return "pub " + topic_ + "\r\n" + content_ + "\r\n";
    }
    std::string topic_;
    std::string content_;
    Timestamp lastPubTime_;  //上一次pub的时间
    std::set<TcpConnection::ptr> audiences_;  //目前关注了该topic的sub的连接
};

//server
class PubSubServer : public boost::noncopyable{
public:
    PubSubServer(EventLoop* loop,const Address::ptr& serverAddr)
       :loop_(loop),
        server_(loop,serverAddr,"PubSubServer"){

        server_.setConnectionCallback(
                std::bind(&PubSubServer::onConnection,this,_1));
        server_.setMessageCallback(
                std::bind(&PubSubServer::onMessage,this,_1,_2,_3));
        loop_->runEvery(1.0,
                std::bind(&PubSubServer::timePublish,this));
    }
    void start(){
        server_.start();
    }
private:
    void onConnection(const TcpConnection::ptr& conn){
        if(conn->connected()){
            conn->setContext(ConnectionSubscription());  //每一天连接的context中都存储有该连接关注的topic的名称
        }else{
            const ConnectionSubscription& connSub = 
                boost::any_cast<const ConnectionSubscription&> (conn->getContext());
            for(ConnectionSubscription::const_iterator it = connSub.begin();it != connSub.end();){
                doUnsubscribe(conn,*it++);
            }
        }
    }
    void onMessage(const TcpConnection::ptr& conn,Buffer* buf,Timestamp receiveTime){
        ParseResult result = kSuccess;
        while(result == kSuccess){
            std::string cmd;
            std::string topic;
            std::string content;
            result = parseMessage(buf,&cmd,&topic,&content);
            if(result == kSuccess){
                if(cmd == "pub"){
                    doPublish(conn->name(),topic,content,receiveTime);
                }else if(cmd == "sub"){
                    JKYI_LOG_INFO(g_logger) << "subscripbes " << topic;
                    doSubscribe(conn,topic);   //conn连接订阅topic
                }else if(cmd == "unsub"){
                    doUnsubscribe(conn,topic);  //conn取消topic的订阅
                }else{
                    conn->shutdown();
                    result = kError;
                }

            }else if(result == kError){
                conn->shutdown();
            }
        }
    }
    //每一秒钟都将当前事件发送给关注了utc_time这个topic的connection
    void timePublish(){
        Timestamp now = Timestamp::now();
        doPublish("internal","utc_time",now.toFormattedString(),now);
    }
    void doSubscribe(const TcpConnection::ptr& conn,const std::string& topic){
        ConnectionSubscription* connSub = 
            boost::any_cast<ConnectionSubscription>(conn->getMutableContext());
        connSub->insert(topic);
        getTopic(topic).add(conn);
    }
    void doUnsubscribe(const TcpConnection::ptr& conn,const std::string& topic){
         JKYI_LOG_INFO(g_logger) << " ubsubscribes " << topic;
         getTopic(topic).remove(conn);
         ConnectionSubscription* connSub = 
             boost::any_cast<ConnectionSubscription>(conn->getMutableContext());
         connSub->erase(topic);
    }
    void doPublish(const std::string& source,
                   const std::string& topic,
                   const std::string& content,
                   Timestamp time){
        getTopic(topic).publish(content,time);
    }
    Topic& getTopic(const std::string& topic){
        std::map<std::string,Topic>::iterator it = topics_.find(topic);
        if(it == topics_.end()){
            it = topics_.insert(make_pair(topic,Topic(topic))).first;
        }
        return it->second;
    }

    EventLoop* loop_;
    TcpServer server_;
    std::map<std::string,Topic> topics_; 
};
}//namespace pubsub

int main(int argc,char ** argv){
    pubsub::g_logger->setLevel(LogLevel::ERROR);
   if(argc > 1){
       char buf[32];
       snprintf(buf,sizeof(buf),"127.0.0.1:%s",argv[1]);
       Address::ptr serverAddr = Address::LookupAnyIPAddress(std::string(buf));

       EventLoop loop;
       pubsub::PubSubServer server(&loop,serverAddr);
       server.start();
       loop.loop();
   }else{
       printf("too less params");
   }
   return 0;
}
