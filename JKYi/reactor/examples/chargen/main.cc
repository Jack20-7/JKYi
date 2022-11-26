#include"JKYi/reactor/examples/chargen/chargen.h"
#include"JKYi/reactor/Logging.h"
#include"JKYi/reactor/EventLoop.h"
#include"JKYi/log.h"

#include<unistd.h>

static JKYi::Logger::ptr g_logger = JKYI_LOG_NAME("system");

int main(){
    g_logger->setLevel(JKYi::LogLevel::INFO);
    JKYi::net::EventLoop loop;
    JKYi::Address::ptr listenAddr = JKYi::Address::LookupAnyIPAddress("127.0.0.1","8000");

    ChargenServer server(&loop,listenAddr,true);
    server.start();
    loop.loop();

    return 0;
}
