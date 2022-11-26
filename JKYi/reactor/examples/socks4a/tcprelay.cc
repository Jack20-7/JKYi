#include"JKYi/reactor/examples/socks4a/tunnel.h"
#include"JKYi/address.h"
#include"JKYi/reactor/Logging.h"
#include"JKYi/log.h"

#include<malloc.h>
#include<stdio.h>
#include<sys/resource.h>
#include<unistd.h>
#include<map>

using namespace JKYi;
using namespace JKYi::net;

static JKYi::Logger::ptr g_logger = JKYI_LOG_NAME("system");

EventLoop* g_eventLoop;
Address::ptr g_serverAddr;
std::map<std::string,TunnelPtr> g_tunnels;

void onServerConnection(const TcpConnectionPtr& conn){
    if(conn->connected()){
        //客户端连接建立成功
        conn->setTcpNoDelay(true);
        conn->stopRead();           //取消读事件的注册,也就是先停止从客户端接收数据  一直知道与服务器建立好连接之后，才能够开始接收数据
        TunnelPtr tunnel(new Tunnel(g_eventLoop,g_serverAddr,conn));
        tunnel->setup();
        tunnel->connect();
        g_tunnels[conn->name()] = tunnel;
    }else{
        assert(g_tunnels.find(conn->name()) != g_tunnels.end());
        g_tunnels[conn->name()]->disconnect();
        g_tunnels.erase(conn->name());
    }
}
void onServerMessage(const TcpConnectionPtr& conn,Buffer* buf,Timestamp receiveTime){
    if(!conn->getContext().empty()){
        const TcpConnectionPtr& clientConn = 
                           boost::any_cast<const TcpConnectionPtr&>(conn->getContext());
        clientConn->send(buf);
    }
}

//打印当前内存使用情况的函数
void memstat(){
    malloc_stats();
}

int main(int argc,char** argv){
    g_logger->setLevel(LogLevel::ERROR);
    if(argc < 4){
        fprintf(stderr,"Usage: %s <host_ip> <port> <listen_port>\n",argv[0]);
    }else{
        LOG_INFO << "pid = " << getpid() << ",tid = " << CurrentThread::tid();
        {
            size_t kOneMB = 1024 * 1024; //1MB
            rlimit rl = {256 * kOneMB,256 * kOneMB};
            setrlimit(RLIMIT_AS,&rl); //设置能够使用的内存大小限制,大小试试256MB
        }
        g_serverAddr = Address::LookupAnyIPAddress(argv[1],argv[2]);

       Address::ptr listenAddr = Address::LookupAnyIPAddress("127.0.0.1",argv[3]);

       EventLoop loop;
       g_eventLoop = &loop;
       //loop.runEvery(3.0,memstat);
       
       TcpServer server(&loop,listenAddr,"TcpRelay");
       server.setConnectionCallback(onServerConnection);
       server.setMessageCallback(onServerMessage);

       server.start();
       loop.loop();
    }
    return 0;
}
