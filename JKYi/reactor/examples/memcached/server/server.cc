#include"JKYi/reactor/examples/memcached/server/MemcacheServer.h"
#include"JKYi/reactor/EventLoop.h"

#include<stdlib.h>

using namespace JKYi;
using namespace JKYi::net;

int main(int argc,char** argv){
    EventLoop loop;
    MemcacheServer::Options options;
    options.tcpport = 8000;
    options.threads = argc > 2 ? atoi(argv[2]) : 0;
    MemcacheServer server(&loop,options);
    server.start();
    loop.loop();
}
