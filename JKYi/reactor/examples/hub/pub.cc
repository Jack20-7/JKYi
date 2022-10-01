#include"JKYi/reactor/examples/hub/pubsub.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/reactor/EventLoopThread.h"

#include<iostream>
#include<stdio.h>
#include<string>

using namespace JKYi;
using namespace JKYi::net;
using namespace pubsub;

EventLoop* g_loop = NULL;
std::string g_topic;
std::string g_content;

void connection(PubSubClient* client){
    if(client->connected()){
        client->publish(g_topic,g_content);
        client->stop();
    }else{
        g_loop->quit();
    }
    return ;
}

int main(int argc,char ** argv){
    if(argc == 4){
        std::string hostport = argv[1];
        size_t colon = hostport.find(':');
        if(colon != std::string::npos){
            //格式正确的话
            g_topic = argv[2];
            g_content = argv[3];
            Address::ptr serverAddr = Address::LookupAnyIPAddress(hostport);
            if(g_content == "-"){
                //如果conteng需要临时输入
                EventLoopThread loopThread;
                g_loop = loopThread.startLoop();
                PubSubClient client(g_loop,serverAddr,"PubSubClient");
                client.start();

                std::string line;
                while(getline(std::cin,line)){
                    client.publish(g_topic,line);
                }
                client.stop();
            }else{
                EventLoop loop;
                g_loop = &loop;
                PubSubClient client(g_loop,serverAddr,"PubSubClient");
                client.setConnectionCallback(connection);
                client.start();
                loop.loop();
            }
        }else{
            printf("Usage:%s hub_ip:port topic content\n",argv[0]);
        }
    }else{
      printf("Usage: %s hub_ip:port topic content\n"
             "Read contents from stdin:\n"
             "  %s hub_ip:port topic -\n", argv[0], argv[0]); 
    }
}

