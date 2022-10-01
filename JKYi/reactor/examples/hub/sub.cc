#include"JKYi/reactor/examples/hub/pubsub.h"
#include"JKYi/reactor/EventLoop.h"

#include<vector>
#include<stdio.h>

using namespace JKYi;
using namespace JKYi::net;
using namespace pubsub;

EventLoop* g_loop = NULL;
std::vector<std::string> g_topics;

//该函数是收到服务器推送而来的信息之后会调用的回调函数
void subscription(const std::string& topic,const std::string& content,Timestamp time){
    printf("%s: %s\n",topic.c_str(),content.c_str());
}

//该函数是连接建立成功之后会调用的回调函数，用来根据我们的输入向服务器注册topic
void connection(PubSubClient* client){
    if(client->connected()){
        for(std::vector<std::string>::iterator it = g_topics.begin();
                        it != g_topics.end();++it){
            client->subscribe(*it,subscription);
        }
    }else{
        g_loop->quit();
    }
}

int main(int argc,char ** argv){
    if(argc > 2){
        std::string hostport = argv[1];
        size_t colon = hostport.find(':');
        if(colon != std::string::npos){
            Address::ptr serverAddr = Address::LookupAnyIPAddress(hostport);
            for(int i = 2;i < argc;++i){
                g_topics.push_back(argv[i]);
            }
            EventLoop loop;
            g_loop = &loop;
            PubSubClient client(&loop,serverAddr,"PubSubClient");
            client.setConnectionCallback(connection);
            client.start();
            loop.loop();
        }else{
            printf("Usage: %s hub_ip:port topic [topic ...]\n", argv[0]);
        }
    }else{
        printf("Usage: %s hub_ip:port topic [topic ...]\n", argv[0]);
    }
}
