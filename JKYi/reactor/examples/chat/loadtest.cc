#include"JKYi/reactor/examples/chat/codec.h"
#include"JKYi/atomic.h"
#include"JKYi/reactor/Logging.h"
#include"JKYi/mutex.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/EventLoopThreadPool.h"
#include"JKYi/reactor/TcpClient.h"
#include"JKYi/log.h"

#include<stdio.h>
#include<unistd.h>
#include<vector>
#include<string>

using namespace JKYi;
using namespace JKYi::net;

static JKYi::Logger::ptr g_logger = JKYI_LOG_NAME("system");
int g_connections = 0;            //要建立的连接数
AtomicInt32 g_aliveConnections;   //当前活跃的连接数
AtomicInt32 g_messagesReceived;   //收到的消息总数
Timestamp g_startTime;            //记录最后一条信息发送的时间
std::vector<Timestamp> g_receiveTime;
EventLoop* g_loop;
std::function<void()> g_statistic;

class ChatClient : public Noncopyable{
public:
    ChatClient(EventLoop* loop,const Address::ptr& serverAddr)
        :loop_(loop),
         client_(loop,serverAddr,"loadTestClient"),
         codec_(std::bind(&ChatClient::onStringMessage,this,_1,_2,_3)){

         client_.setConnectionCallback(
                 std::bind(&ChatClient::onConnection,this,_1));
         client_.setMessageCallback(
                 std::bind(&LengthHeaderCodec::onMessage,&codec_,_1,_2,_3));
     }

     void connect(){
         client_.connect();
     }
     void disconnect(){
         client_.disconnect();
     }

     Timestamp receiveTime()const{
         return receiveTime_;
     }

private:
    void onConnection(const TcpConnectionPtr& conn){
        LOG_INFO << conn->localAddress()->toString() << " -> "
                 << conn->peerAddress()->toString() << " is " 
                 << (conn->connected() ? "UP" : "DOWN");
        if(conn->connected()){
            connection_ = conn;
            if(g_aliveConnections.incrementAndGet() == g_connections){
                LOG_INFO << "all connected ";
                loop_->runAfter(10.0,std::bind(&ChatClient::send,this));
            }
        }else{
            connection_.reset();
        }
    }
    void onStringMessage(const TcpConnectionPtr& conn,
                            const std::string& message,
                            Timestamp receiveTime){
        receiveTime_ = receiveTime;                              //记录最后一条信息接收的时间
        int received = g_messagesReceived.incrementAndGet();
        if(received == g_connections){                           //如果所有的信息都接收的话
            Timestamp endTime = Timestamp::now();
            LOG_INFO << "all received " << g_connections << " in " 
                     << timeDifference(endTime,g_startTime);
            g_loop->queueInLoop(g_statistic);
        }else if(received % 1000 == 0){
            LOG_DEBUG << received;
        }
    }

    void send(){
        g_startTime = Timestamp::now();             
        codec_.send(connection_.get(),"hello");
        LOG_DEBUG << "send";
    }

    EventLoop* loop_;
    TcpClient client_;
    LengthHeaderCodec codec_;
    TcpConnectionPtr connection_;
    Timestamp receiveTime_;
};

void statistic(const std::vector<std::unique_ptr<ChatClient>>& clients){
    LOG_INFO << "statistic " << clients.size();
    std::vector<double> seconds(clients.size());
    for(size_t i = 0;i < clients.size();++i){
        seconds[i] = timeDifference(clients[i]->receiveTime(),g_startTime);
    }
    std::sort(seconds.begin(),seconds.end());
    for(size_t i = 0;i < clients.size();i += std::max(static_cast<size_t>(1),clients.size() / 20)){
        printf("%6zd%% %.6f\n",i * 100 / clients.size(),seconds[i]);
    }
    if(clients.size() >= 100){
        printf("%6d%% %.6f\n",99,seconds[clients.size() - clients.size() / 100]);
    }
    printf("%6d%% %.6f\n",100,seconds.back());
}

int main(int argc,char** argv){
    g_logger->setLevel(LogLevel::ERROR);
    LOG_INFO << "pid = " << getpid();
    if(argc > 3){
        Address::ptr serverAddr = Address::LookupAnyIPAddress(argv[1],argv[2]);
        g_connections = atoi(argv[3]);
        int threads = 0;
        if(argc > 4){
            threads = atoi(argv[4]);
        }
        EventLoop loop;
        g_loop = &loop;
        EventLoopThreadPool pool(&loop,"chat-loadtest");
        pool.setThreadNum(threads);
        pool.start();

        g_receiveTime.reserve(g_connections);
        std::vector<std::unique_ptr<ChatClient>> clients(g_connections);
        g_statistic = std::bind(statistic,std::ref(clients));
        for(int i = 0;i < g_connections;++i){
            clients[i].reset(new ChatClient(pool.getNextLoop(),serverAddr));
            clients[i]->connect();
            usleep(200);
        }

        loop.loop();
    }else{
        printf("Usage: %s host_ip port connections [threads]\n",argv[0]);
    }
    return 0;
}
