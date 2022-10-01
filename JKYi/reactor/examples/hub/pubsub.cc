#include"JKYi/reactor/examples/hub/pubsub.h"
#include"JKYi/reactor/examples/hub/codec.h"

using namespace JKYi;
using namespace JKYi::net;
using namespace pubsub;

PubSubClient::PubSubClient(EventLoop* loop,
                           const Address::ptr& serverAddr,
                           const std::string& name)
    :client_(loop,serverAddr,name){

    client_.setConnectionCallback(
            std::bind(&PubSubClient::onConnection,this,_1));
    client_.setMessageCallback(
            std::bind(&PubSubClient::onMessage,this,_1,_2,_3));
}

void PubSubClient::start(){
    client_.connect();
}
void PubSubClient::stop(){
    client_.stop();
}

bool PubSubClient::connected()const{
    return conn_ && conn_->connected();
}

bool PubSubClient::subscribe(const std::string& topic,
                              const SubscribeCallback& cb){
    std::string message = "sub " + topic  + "\r\n";
    subscribeCallback_ = cb;
    return send(message);
}

void PubSubClient::unsubscribe(const std::string& topic){
    std::string message = "unsub " + topic + "\r\n";
    send(message);
}
//作为puhlisher，向服务器推送topic的信息
bool PubSubClient::publish(const std::string& topic,
                            const std::string& content){
    std::string message = "pub " + topic + "\r\n" + content + "\r\n";
    return send(message);
}

void PubSubClient::onConnection(const TcpConnection::ptr& conn){
    if(conn->connected()){
        conn_ = conn;
    }else{
        conn_.reset();
    }
    if(connectionCallback_){
        connectionCallback_(this);
    }
}

void PubSubClient::onMessage(const TcpConnection::ptr& conn,Buffer* buf,Timestamp time){
    ParseResult result = kSuccess;
    while(result == kSuccess){
        std::string cmd;
        std::string topic;
        std::string content;
        result = parseMessage(buf,&cmd,&topic,&content);
        if(result == kSuccess){
            //接收到了推送而来的数据
            if(cmd == "pub" && subscribeCallback_){
                subscribeCallback_(topic,content,time);
            }
        }else if(result == kError){
            conn->shutdown();
        }
    }
}

bool PubSubClient::send(const std::string& message){
    bool succeed = false;
    if(conn_ && conn_->connected()){
        conn_->send(message);
        succeed = true;
    }
    return succeed;
}



