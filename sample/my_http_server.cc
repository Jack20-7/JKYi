#include"JKYi/JKYi.h"

static JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();
void run(){
    g_logger->setLevel(JKYi::LogLevel::INFO);
    JKYi::Address::ptr addr = JKYi::Address::LookupAnyIPAddress("0.0.0.0:8020");
    if(!addr){
        JKYI_LOG_ERROR(g_logger)<<"get addr error";
        return ;
    }
    JKYi::http::HttpServer::ptr server(new JKYi::http::HttpServer(true));
    if(!server){
        JKYI_LOG_ERROR(g_logger)<<"get HttpServer error";
        return ;
    }
    while(!server->bind(addr)){
        sleep(2);
    }
    server->start();
}
int main(int argc,char ** argv){
    JKYi::IOManager iom(2);
    iom.schedule(run);
    return 0;
}
