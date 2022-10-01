#include"JKYi/reactor/reactor.h"
#include"JKYi/log.h"
#include"JKYi/atomic.h"
#include"JKYi/address.h"

#include<utility>
#include<stdio.h>
#include<unistd.h>

using namespace JKYi;
using namespace JKYi::net;

int numThreads = 0;
static Logger::ptr g_logger = JKYI_LOG_ROOT();

//计算吞吐量
class DiscardServer{
public:
    DiscardServer(EventLoop* loop,const Address::ptr& serverAddr)
        :server_(loop,serverAddr,"DisacrdServer"),
         oldCounter_(0),
         startTime_(Timestamp::now()){
         
         server_.setConnectionCallback(
                 std::bind(&DiscardServer::onConnection,this,_1));
         server_.setMessageCallback(
                 std::bind(&DiscardServer::onMessage,this,_1,_2,_3));

         server_.setThreadNum(numThreads);//one loop per thread 的开关
         loop->runEvery(3.0,
                 std::bind(&DiscardServer::printThroughput,this));
     }
    void start(){
        JKYI_LOG_INFO(g_logger) << "starting " << numThreads << " threads";
        server_.start();
    }
private:
    void onConnection(const TcpConnection::ptr& conn){
        JKYI_LOG_INFO(g_logger) << conn->peerAddress()->toString() << " -> "
                                << conn->localAddress()->toString() << " is " 
                                << (conn->connected() ? "UP" : "DOWN");
    }
    void onMessage(const TcpConnection::ptr& conn,Buffer* buf,Timestamp receiveTime){
        size_t len = buf->readableBytes();
        transferred_.add(len);
        receivedMessages_.incrementAndGet();
        buf->retrieveAll();
    }

    void printThroughput(){
        Timestamp endTime = Timestamp::now();
        int64_t newCounter = transferred_.get();
        int64_t bytes = newCounter - oldCounter_;  //这段时间交换的字节数
        int64_t msgs = receivedMessages_.getAndSet(0); //收到的消息数
        double time = timeDifference(endTime,startTime_);     //经过的时间
        printf("%4.3f MiB/s %4.3f Ki Msgs/s %6.2f bytes per msg\n",
        static_cast<double>(bytes)/time/1024/1024,
        static_cast<double>(msgs)/time/1024,
        static_cast<double>(bytes)/static_cast<double>(msgs));

        oldCounter_ = newCounter;
        startTime_ = endTime;
     }

    TcpServer server_;

    AtomicInt64 transferred_;  //记录这段时间收到的字节数
    AtomicInt64 receivedMessages_; //记录这段时间收到的消息数
    int64_t oldCounter_;  //维护上一次收到的字节数
    Timestamp startTime_;
};

int main(int argc,char ** argv){
    if(argc <= 1){
        printf("too less params");
        exit(0);
    }
    g_logger->setLevel(LogLevel::INFO);
    char buf[64];
    snprintf(buf,sizeof(buf),"127.0.0.1:%s",argv[1]);
    Address::ptr serverAddr = Address::LookupAnyIPAddress(std::string(buf));
    EventLoop loop;
    DiscardServer server(&loop,serverAddr);
    server.start();
    loop.loop();
}
